/*
 *      Copyright (C) 2016 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "ScraperUtils.h"
#include "FileItem.h"
#include "URL.h"
#include "Util.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "utils/RegExp.h"
#include "utils/URIUtils.h"
#include "video/Episode.h"

bool ScraperUtils::GetEpisodeDetailsFromName(const std::string& episodeName, const std::string& path,
  const TvShowRegexpList& regExps, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes)
{
  return GetEpisodeDetailsFromName(episodeName, path, "", regExps, multipartRegExp, episodes);
}

bool ScraperUtils::GetEpisodeDetailsFromName(const CFileItem& item,
  const TvShowRegexpList& regExps, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes)
{
  std::string episodeName = item.GetPath();

  // remove path to main file if it's a BD or DVD folder to regex the right (folder) name
  if (item.IsOpticalMediaFile())
  {
    episodeName = item.GetLocalMetadataPath();
    URIUtils::RemoveSlashAtEnd(episodeName);
  }

  // URL decode in case an episode is on a http|https|dav|davs:// source and URL-encoded like foo%201x01%20bar.avi
  episodeName = CURL::Decode(episodeName);

  return GetEpisodeDetailsFromName(episodeName, item.GetPath(), item.GetBaseMoviePath(true), regExps, multipartRegExp, episodes);
}

bool ScraperUtils::GetEpisodeDetailsFromName(const std::string& episodeName, const std::string& path, const std::string& basePath,
  const TvShowRegexpList& regExps, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes)
{
  if (episodeName.empty())
    return false;

  for (const auto& regExp : regExps)
  {
    if (GetEpisodeDetailsFromName(episodeName, path, basePath, regExp, multipartRegExp, episodes))
      return true;
  }

  return false;
}

bool ScraperUtils::GetEpisodeDetailsFromName(const std::string& episodeName, const std::string& path, const std::string& basePath,
  const TVShowRegexp& regExp, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes)
{
  if (episodeName.empty())
    return false;

  CRegExp reg(true, CRegExp::autoUtf8);
  if (!reg.RegComp(regExp.regexp))
    return false;

  int regexppos = reg.RegFind(episodeName.c_str());
  if (regexppos < 0)
    return false;

  VIDEO::EPISODE episode;
  episode.strPath = path;
  episode.iSeason = -1;
  episode.iEpisode = -1;
  episode.cDate.SetValid(false);
  episode.isFolder = false;

  bool byDate = regExp.byDate ? true : false;
  int defaultSeason = regExp.defaultSeason;

  if (byDate)
  {
    if (!GetAirDateFromRegExp(reg, episode))
      return false;

    CLog::Log(LOGDEBUG, "ScraperUtils: found date based match %s (%s) [%s]", CURL::GetRedacted(episode.strPath).c_str(),
      episode.cDate.GetAsLocalizedDate().c_str(), regExp.regexp.c_str());
  }
  else
  {
    if (!GetEpisodeAndSeasonFromRegExp(reg, episode, defaultSeason))
      return false;

    CLog::Log(LOGDEBUG, "ScraperUtils: found episode match %s (s%ie%i) [%s]", CURL::GetRedacted(episode.strPath).c_str(),
      episode.iSeason, episode.iEpisode, regExp.regexp.c_str());
  }

  // Grab the remainder from first regexp run
  // as second run might modify or empty it.
  std::string remainder(reg.GetMatch(3));

  /*
  * Check if the files base path is a dedicated folder that contains
  * only this single episode. If season and episode match with the
  * actual media file, we set episode.isFolder to true.
  */
  std::string strBasePath = basePath;
  URIUtils::RemoveSlashAtEnd(strBasePath);
  strBasePath = URIUtils::GetFileName(strBasePath);

  if (reg.RegFind(strBasePath.c_str()) > -1)
  {
    VIDEO::EPISODE parent;
    if (byDate)
    {
      GetAirDateFromRegExp(reg, parent);
      if (episode.cDate == parent.cDate)
        episode.isFolder = true;
    }
    else
    {
      GetEpisodeAndSeasonFromRegExp(reg, parent, defaultSeason);
      if (episode.iSeason == parent.iSeason && episode.iEpisode == parent.iEpisode)
        episode.isFolder = true;
    }
  }

  // add what we found by now
  episodes.push_back(episode);

  // check the remainder of the string for any further episodes.
  CRegExp reg2(true, CRegExp::autoUtf8);
  if (!byDate && reg2.RegComp(multipartRegExp))
  {
    int offset = 0;
    int regexp2pos;

    // we want "long circuit" OR below so that both offsets are evaluated
    while (((regexp2pos = reg2.RegFind(remainder.c_str() + offset)) > -1) | ((regexppos = reg.RegFind(remainder.c_str() + offset)) > -1))
    {
      if (((regexppos <= regexp2pos) && regexppos != -1) ||
        (regexppos >= 0 && regexp2pos == -1))
      {
        GetEpisodeAndSeasonFromRegExp(reg, episode, defaultSeason);

        CLog::Log(LOGDEBUG, "ScraperUtils: found new season %u, multipart episode %u [%s]",
          episode.iSeason, episode.iEpisode, multipartRegExp.c_str());

        episodes.push_back(episode);
        remainder = reg.GetMatch(3);
        offset = 0;
      }
      else if (((regexp2pos < regexppos) && regexp2pos != -1) ||
        (regexp2pos >= 0 && regexppos == -1))
      {
        episode.iEpisode = static_cast<int>(strtol(reg2.GetMatch(1).c_str(), nullptr, 10));
        CLog::Log(LOGDEBUG, "ScraperUtils: found multipart episode %u [%s]",
          episode.iEpisode, multipartRegExp.c_str());

        episodes.push_back(episode);
        offset += regexp2pos + reg2.GetFindLen();
      }
    }
  }

  return true;
}

bool ScraperUtils::GetAirDateFromRegExp(const CRegExp &reg, VIDEO::EPISODE &episodeInfo)
{
  std::string param1(reg.GetMatch(1));
  std::string param2(reg.GetMatch(2));
  std::string param3(reg.GetMatch(3));

  if (param1.empty() || param2.empty() || param3.empty())
    return false;

  // regular expression by date
  int len1 = param1.size();
  int len2 = param2.size();
  int len3 = param3.size();

  if (len1 == 4 && len2 == 2 && len3 == 2)
  {
    // yyyy mm dd format
    episodeInfo.cDate.SetDate(
      static_cast<int>(strtol(param1.c_str(), nullptr, 10)),
      static_cast<int>(strtol(param2.c_str(), nullptr, 10)),
      static_cast<int>(strtol(param3.c_str(), nullptr, 10)));
  }
  else if (len1 == 2 && len2 == 2 && len3 == 4)
  {
    // mm dd yyyy format
    episodeInfo.cDate.SetDate(
      static_cast<int>(strtol(param3.c_str(), nullptr, 10)),
      static_cast<int>(strtol(param1.c_str(), nullptr, 10)),
      static_cast<int>(strtol(param2.c_str(), nullptr, 10)));
  }

  return episodeInfo.cDate.IsValid();
}

bool ScraperUtils::GetEpisodeAndSeasonFromRegExp(const CRegExp &reg, VIDEO::EPISODE &episodeInfo, int defaultSeason)
{
  std::string season(reg.GetMatch(1));
  std::string episode(reg.GetMatch(2));

  if (season.empty() && episode.empty())
    return false;

  // if episode is empty and season not, the season is actually an episode
  if (episode.empty())
  {
    episode = season;
    season.clear();
  }

  // no season specified -> assume defaultSeason
  if (season.empty())
    episodeInfo.iSeason = defaultSeason;
  else
    episodeInfo.iSeason = static_cast<int>(strtol(season.c_str(), nullptr, 10));

  char* endptr = nullptr;

  // try to parse the episode
  episodeInfo.iEpisode = CUtil::TranslateRomanNumeral(episode.c_str());
  if (episodeInfo.iEpisode == -1)
    episodeInfo.iEpisode = static_cast<int>(strtol(episode.c_str(), &endptr, 10));

  // check if there's a subepisode
  if (endptr != nullptr)
  {
    if (isalpha(*endptr))
      episodeInfo.iSubepisode = *endptr - (islower(*endptr) ? 'a' : 'A') + 1;
    else if (*endptr == '.')
      episodeInfo.iSubepisode = strtol(endptr + 1, nullptr, 10);
  }

  return true;
}
