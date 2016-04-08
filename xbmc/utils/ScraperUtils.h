#pragma once
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

#include <string>
#include <vector>

class CFileItem;
class CRegExp;

namespace VIDEO
{
  struct EPISODE;
}

struct TVShowRegexp
{
  bool byDate;
  std::string regexp;
  int defaultSeason;

  TVShowRegexp(bool byDate, const std::string& regexp, int defaultSeason = 1)
    : byDate(byDate)
    , regexp(regexp)
    , defaultSeason(defaultSeason)
  { }
};
using TvShowRegexpList = std::vector<TVShowRegexp>;

class ScraperUtils
{
public:
  static bool GetEpisodeDetailsFromName(const std::string& episodeName, const std::string& path,
    const TvShowRegexpList& regExps, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes);
  static bool GetEpisodeDetailsFromName(const CFileItem& item,
    const TvShowRegexpList& regExps, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes);
  // TODO

private:
  ScraperUtils() = delete;
  ~ScraperUtils() = delete;

  static bool GetEpisodeDetailsFromName(const std::string& episodeName, const std::string& path, const std::string& basePath,
    const TvShowRegexpList& regExps, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes);

  static bool GetEpisodeDetailsFromName(const std::string& episodeName, const std::string& path, const std::string& basePath,
    const TVShowRegexp& regExp, const std::string& multipartRegExp, std::vector<VIDEO::EPISODE>& episodes);

  /*! \brief Extract episode air-date from a processed regexp
  \param reg Regular expression object with at least 3 matches
  \param episodeInfo Episode information to fill in.
  \return true on success (3 matches), false on failure (fewer than 3 matches)
  */
  static bool GetAirDateFromRegExp(const CRegExp &reg, VIDEO::EPISODE &episodeInfo);

  /*! \brief Extract episode and season numbers from a processed regexp
  \param reg Regular expression object with at least 2 matches
  \param episodeInfo Episode information to fill in.
  \param defaultSeason Season to use if not found in reg.
  \return true on success (2 matches), false on failure (fewer than 2 matches)
  */
  static bool GetEpisodeAndSeasonFromRegExp(const CRegExp &reg, VIDEO::EPISODE &episodeInfo, int defaultSeason);
};
