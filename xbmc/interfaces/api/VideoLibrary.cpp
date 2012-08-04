/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "VideoLibrary.h"
#include "ApplicationMessenger.h"
#include "Util.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "video/VideoDatabase.h"

using namespace API;

APIStatus CVideoLibrary::GetMovies(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CFileItemList items;
  APIStatus ret = APIStatusOK;
  if ((ret = GetVideos(MediaTypeMovie, "videodb://1/2/", parameterObject, items, result, videodatabase)) == APIStatusOK)
    ret = GetAdditionalMovieDetails(parameterObject, items, result, videodatabase);

  return ret;
}

APIStatus CVideoLibrary::GetMovieDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  int id = (int)parameterObject["movieid"].asInteger();

  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CVideoInfoTag infos;
  if (!videodatabase.GetMovieInfo("", infos, id) || infos.m_iDbId <= 0)
    return APIStatusInvalidParameters;

  for (CVariant::const_iterator_array itr = parameterObject["properties"].begin_array(); itr != parameterObject["properties"].end_array(); itr++)
  {
    CStdString fieldValue = itr->asString();
    if (fieldValue == "streamdetails")
    {
      videodatabase.GetStreamDetails(infos);
      break;
    }
  }

  HandleFileItem("movieid", true, "moviedetails", CFileItemPtr(new CFileItem(infos)), parameterObject, parameterObject["properties"], result, false);
  return APIStatusOK;
}

APIStatus CVideoLibrary::GetMovieSets(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CFileItemList items;
  if (!videodatabase.GetSetsNav("videodb://1/7/", items, VIDEODB_CONTENT_MOVIES))
    return APIStatusInternalError;

  HandleFileItemList("setid", false, "sets", items, parameterObject, result);
  return APIStatusOK;
}

APIStatus CVideoLibrary::GetMovieSetDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  int id = (int)parameterObject["setid"].asInteger();

  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  // Get movie set details
  CVideoInfoTag infos;
  if (!videodatabase.GetSetInfo(id, infos) || infos.m_iDbId <= 0)
    return APIStatusInvalidParameters;

  HandleFileItem("setid", false, "setdetails", CFileItemPtr(new CFileItem(infos)), parameterObject, parameterObject["properties"], result, false);

  // Get movies from the set
  CFileItemList items;
  if (!videodatabase.GetMoviesNav("videodb://1/2/", items, -1, -1, -1, -1, -1, -1, id))
    return APIStatusInternalError;

  return GetAdditionalMovieDetails(parameterObject["movies"], items, result["setdetails"]["items"], videodatabase);
}

APIStatus CVideoLibrary::GetTVShows(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CFileItemList items;
  APIStatus ret = APIStatusOK;
  if ((ret = GetVideos(MediaTypeTvShow, "videodb://2/2/", parameterObject, items, result, videodatabase)) != APIStatusOK)
    return ret;

  bool additionalInfo = false;
  for (CVariant::const_iterator_array itr = parameterObject["properties"].begin_array(); itr != parameterObject["properties"].end_array(); itr++)
  {
    CStdString fieldValue = itr->asString();
    if (fieldValue == "cast")
      additionalInfo = true;
  }

  if (additionalInfo)
  {
    for (int index = 0; index < items.Size(); index++)
      videodatabase.GetTvShowInfo("", *(items[index]->GetVideoInfoTag()), items[index]->GetVideoInfoTag()->m_iDbId);
  }

  int size = items.Size();
  if (items.HasProperty("total") && items.GetProperty("total").asInteger() > size)
    size = (int)items.GetProperty("total").asInteger();
  HandleFileItemList("tvshowid", true, "tvshows", items, parameterObject, result, size, false);

  return ret;
}

APIStatus CVideoLibrary::GetTVShowDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  int id = (int)parameterObject["tvshowid"].asInteger();

  CVideoInfoTag infos;
  if (!videodatabase.GetTvShowInfo("", infos, id) || infos.m_iDbId <= 0)
    return APIStatusInvalidParameters;

  HandleFileItem("tvshowid", true, "tvshowdetails", CFileItemPtr(new CFileItem(infos)), parameterObject, parameterObject["properties"], result, false);
  return APIStatusOK;
}

APIStatus CVideoLibrary::GetSeasons(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  int tvshowID = (int)parameterObject["tvshowid"].asInteger();

  CStdString strPath;
  strPath.Format("videodb://2/2/%i/", tvshowID);
  CFileItemList items;
  if (!videodatabase.GetSeasonsNav(strPath, items, -1, -1, -1, -1, tvshowID))
    return APIStatusInternalError;

  HandleFileItemList(NULL, false, "seasons", items, parameterObject, result);
  return APIStatusOK;
}

APIStatus CVideoLibrary::GetEpisodes(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  SortDescription sorting;
  ParseLimits(parameterObject, sorting.limitStart, sorting.limitEnd);
  if (!ParseSorting(parameterObject, sorting.sortBy, sorting.sortOrder, sorting.sortAttributes))
    return APIStatusInvalidParameters;

  int tvshowID = (int)parameterObject["tvshowid"].asInteger();
  int season   = (int)parameterObject["season"].asInteger();

  CStdString strPath;
  strPath.Format("videodb://2/2/%i/%i/", tvshowID, season);
  CFileItemList items;
  if (!videodatabase.GetEpisodesNav(strPath, items, -1, -1, -1, -1, tvshowID, season, sorting))
    return APIStatusInternalError;

  return GetAdditionalEpisodeDetails(parameterObject, items, result, videodatabase);
}

APIStatus CVideoLibrary::GetEpisodeDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  int id = (int)parameterObject["episodeid"].asInteger();

  CVideoInfoTag infos;
  if (!videodatabase.GetEpisodeInfo("", infos, id) || infos.m_iDbId <= 0)
    return APIStatusInvalidParameters;

  for (CVariant::const_iterator_array itr = parameterObject["properties"].begin_array(); itr != parameterObject["properties"].end_array(); itr++)
  {
    CStdString fieldValue = itr->asString();
    if (fieldValue == "streamdetails")
    {
      videodatabase.GetStreamDetails(infos);
      break;
    }
  }

  CFileItemPtr pItem = CFileItemPtr(new CFileItem(infos));
  // We need to set the correct base path to get the valid fanart
  int tvshowid = infos.m_iIdShow;
  if (tvshowid <= 0)
    tvshowid = videodatabase.GetTvShowForEpisode(id);

  CStdString basePath; basePath.Format("videodb://2/2/%ld/%ld/%ld", tvshowid, infos.m_iSeason, id);
  pItem->SetPath(basePath);

  HandleFileItem("episodeid", true, "episodedetails", pItem, parameterObject, parameterObject["properties"], result, false);
  return APIStatusOK;
}

APIStatus CVideoLibrary::GetMusicVideos(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  SortDescription sorting;
  ParseLimits(parameterObject, sorting.limitStart, sorting.limitEnd);
  if (!ParseSorting(parameterObject, sorting.sortBy, sorting.sortOrder, sorting.sortAttributes))
    return APIStatusInvalidParameters;

  int artistID = (int)parameterObject["artistid"].asInteger();
  int albumID  = (int)parameterObject["albumid"].asInteger();

  CFileItemList items;
  if (!videodatabase.GetMusicVideosNav("videodb://3/2/", items, -1, -1, artistID, -1, -1, albumID, sorting))
    return APIStatusInternalError;

  return GetAdditionalMusicVideoDetails(parameterObject, items, result, videodatabase);
}

APIStatus CVideoLibrary::GetMusicVideoDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  int id = (int)parameterObject["musicvideoid"].asInteger();

  CVideoInfoTag infos;
  if (!videodatabase.GetMusicVideoInfo("", infos, id) || infos.m_iDbId <= 0)
    return APIStatusInvalidParameters;

  for (CVariant::const_iterator_array itr = parameterObject["properties"].begin_array(); itr != parameterObject["properties"].end_array(); itr++)
  {
    CStdString fieldValue = itr->asString();
    if (fieldValue == "streamdetails")
    {
      videodatabase.GetStreamDetails(infos);
      break;
    }
  }

  HandleFileItem("musicvideoid", true, "musicvideodetails", CFileItemPtr(new CFileItem(infos)), parameterObject, parameterObject["properties"], result, false);
  return APIStatusOK;
}

APIStatus CVideoLibrary::GetRecentlyAddedMovies(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CFileItemList items;
  if (!videodatabase.GetRecentlyAddedMoviesNav("videodb://4/", items))
    return APIStatusInternalError;

  return GetAdditionalMovieDetails(parameterObject, items, result, videodatabase);
}

APIStatus CVideoLibrary::GetRecentlyAddedEpisodes(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CFileItemList items;
  if (!videodatabase.GetRecentlyAddedEpisodesNav("videodb://5/", items))
    return APIStatusInternalError;

  return GetAdditionalEpisodeDetails(parameterObject, items, result, videodatabase);
}

APIStatus CVideoLibrary::GetRecentlyAddedMusicVideos(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CFileItemList items;
  if (!videodatabase.GetRecentlyAddedMusicVideosNav("videodb://6/", items))
    return APIStatusInternalError;

  return GetAdditionalMusicVideoDetails(parameterObject, items, result, videodatabase);
}

APIStatus CVideoLibrary::GetGenres(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CStdString media = parameterObject["type"].asString();
  media = media.ToLower();
  int idContent = -1;

  CStdString strPath = "videodb://";
  /* select which video content to get genres from*/
  if (media.Equals("movie"))
  {
    idContent = VIDEODB_CONTENT_MOVIES;
    strPath += "1";
  }
  else if (media.Equals("tvshow"))
  {
    idContent = VIDEODB_CONTENT_TVSHOWS;
    strPath += "2";
  }
  else if (media.Equals("musicvideo"))
  {
    idContent = VIDEODB_CONTENT_MUSICVIDEOS;
    strPath += "3";
  }
  strPath += "/1/";
 
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CFileItemList items;
  if (!videodatabase.GetGenresNav(strPath, items, idContent))
    return APIStatusInternalError;

  /* need to set strTitle in each item*/
  for (unsigned int i = 0; i < (unsigned int)items.Size(); i++)
    items[i]->GetVideoInfoTag()->m_strTitle = items[i]->GetLabel();

  HandleFileItemList("genreid", false, "genres", items, parameterObject, result);
  return APIStatusOK;
}

APIStatus CVideoLibrary::SetMovieDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  int id = (int)parameterObject["movieid"].asInteger();

  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CVideoInfoTag infos;
  if (!videodatabase.GetMovieInfo("", infos, id) || infos.m_iDbId <= 0)
    return APIStatusInvalidParameters;

  std::map<std::string, std::string> artwork;
  videodatabase.GetArtForItem(infos.m_iDbId, infos.m_type, artwork);

  int playcount = infos.m_playCount;
  CDateTime lastPlayed = infos.m_lastPlayed;

  UpdateVideoTag(parameterObject, infos, artwork);

  if (videodatabase.SetDetailsForMovie(infos.m_strFileNameAndPath, infos, artwork, id) <= 0)
    return APIStatusInternalError;

  if (playcount != infos.m_playCount || lastPlayed != infos.m_lastPlayed)
  {
    // restore original playcount or the new one won't be announced
    int newPlaycount = infos.m_playCount;
    infos.m_playCount = playcount;
    videodatabase.SetPlayCount(CFileItem(infos), newPlaycount, infos.m_lastPlayed.IsValid() ? infos.m_lastPlayed : CDateTime::GetCurrentDateTime());
  }

  return APIStatusOK;
}

APIStatus CVideoLibrary::SetTVShowDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  int id = (int)parameterObject["tvshowid"].asInteger();

  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CVideoInfoTag infos;
  if (!videodatabase.GetTvShowInfo("", infos, id) || infos.m_iDbId <= 0)
    return APIStatusInvalidParameters;

  std::map<std::string, std::string> artwork;
  videodatabase.GetArtForItem(infos.m_iDbId, infos.m_type, artwork);

  std::map<int, std::string> seasonArt;
  videodatabase.GetTvShowSeasonArt(infos.m_iDbId, seasonArt);

  int playcount = infos.m_playCount;
  CDateTime lastPlayed = infos.m_lastPlayed;

  UpdateVideoTag(parameterObject, infos, artwork);

  if (videodatabase.SetDetailsForTvShow(infos.m_strFileNameAndPath, infos, artwork, seasonArt, id) <= 0)
    return APIStatusInternalError;

  if (playcount != infos.m_playCount || lastPlayed != infos.m_lastPlayed)
  {
    // restore original playcount or the new one won't be announced
    int newPlaycount = infos.m_playCount;
    infos.m_playCount = playcount;
    videodatabase.SetPlayCount(CFileItem(infos), newPlaycount, infos.m_lastPlayed.IsValid() ? infos.m_lastPlayed : CDateTime::GetCurrentDateTime());
  }

  return APIStatusOK;
}

APIStatus CVideoLibrary::SetEpisodeDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  int id = (int)parameterObject["episodeid"].asInteger();

  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CVideoInfoTag infos;
  videodatabase.GetEpisodeInfo("", infos, id);
  if (infos.m_iDbId <= 0)
  {
    videodatabase.Close();
    return APIStatusInvalidParameters;
  }

  int tvshowid = videodatabase.GetTvShowForEpisode(id);
  if (tvshowid <= 0)
  {
    videodatabase.Close();
    return APIStatusInvalidParameters;
  }

  std::map<std::string, std::string> artwork;
  videodatabase.GetArtForItem(infos.m_iDbId, infos.m_type, artwork);

  int playcount = infos.m_playCount;
  CDateTime lastPlayed = infos.m_lastPlayed;

  UpdateVideoTag(parameterObject, infos, artwork);

  if (videodatabase.SetDetailsForEpisode(infos.m_strFileNameAndPath, infos, artwork, tvshowid, id) <= 0)
    return APIStatusInternalError;

  if (playcount != infos.m_playCount || lastPlayed != infos.m_lastPlayed)
  {
    // restore original playcount or the new one won't be announced
    int newPlaycount = infos.m_playCount;
    infos.m_playCount = playcount;
    videodatabase.SetPlayCount(CFileItem(infos), newPlaycount, infos.m_lastPlayed.IsValid() ? infos.m_lastPlayed : CDateTime::GetCurrentDateTime());
  }

  return APIStatusOK;
}

APIStatus CVideoLibrary::SetMusicVideoDetails(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  int id = (int)parameterObject["musicvideoid"].asInteger();

  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  CVideoInfoTag infos;
  videodatabase.GetMusicVideoInfo("", infos, id);
  if (infos.m_iDbId <= 0)
  {
    videodatabase.Close();
    return APIStatusInvalidParameters;
  }

  std::map<std::string, std::string> artwork;
  videodatabase.GetArtForItem(infos.m_iDbId, infos.m_type, artwork);

  int playcount = infos.m_playCount;
  CDateTime lastPlayed = infos.m_lastPlayed;

  UpdateVideoTag(parameterObject, infos, artwork);

  if (videodatabase.SetDetailsForMusicVideo(infos.m_strFileNameAndPath, infos, artwork, id) <= 0)
    return APIStatusInternalError;

  if (playcount != infos.m_playCount || lastPlayed != infos.m_lastPlayed)
  {
    // restore original playcount or the new one won't be announced
    int newPlaycount = infos.m_playCount;
    infos.m_playCount = playcount;
    videodatabase.SetPlayCount(CFileItem(infos), newPlaycount, infos.m_lastPlayed.IsValid() ? infos.m_lastPlayed : CDateTime::GetCurrentDateTime());
  }

  return APIStatusOK;
}

APIStatus CVideoLibrary::RemoveMovie(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return RemoveVideo(parameterObject);
}

APIStatus CVideoLibrary::RemoveTVShow(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return RemoveVideo(parameterObject);
}

APIStatus CVideoLibrary::RemoveEpisode(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return RemoveVideo(parameterObject);
}

APIStatus CVideoLibrary::RemoveMusicVideo(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return RemoveVideo(parameterObject);
}

APIStatus CVideoLibrary::Scan(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::string directory = parameterObject["directory"].asString();
  CStdString cmd;
  if (directory.empty())
    cmd = "updatelibrary(video)";
  else
    cmd.Format("updatelibrary(video, %s)", directory.c_str());

  CApplicationMessenger::Get().ExecBuiltIn(cmd);
  return APIStatusOK;
}

APIStatus CVideoLibrary::Export(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CStdString cmd;
  if (parameterObject["options"].isMember("path"))
    cmd.Format("exportlibrary(video, false, %s)", parameterObject["options"]["path"].asString());
  else
    cmd.Format("exportlibrary(video, true, %s, %s, %s)",
      parameterObject["options"]["images"].asBoolean() ? "true" : "false",
      parameterObject["options"]["overwrite"].asBoolean() ? "true" : "false",
      parameterObject["options"]["actorthumbs"].asBoolean() ? "true" : "false");

  CApplicationMessenger::Get().ExecBuiltIn(cmd);
  return APIStatusOK;
}

APIStatus CVideoLibrary::Clean(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CApplicationMessenger::Get().ExecBuiltIn("cleanlibrary(video)");
  return APIStatusOK;
}

bool CVideoLibrary::FillFileItem(const std::string &strFilename, CFileItem &item)
{
  CVideoDatabase videodatabase;
  if (strFilename.empty() || !videodatabase.Open())
    return false;

  CVideoInfoTag details;
  if (!videodatabase.LoadVideoInfo(strFilename, details))
    return false;

  item = CFileItem(details);
  return true;
}

bool CVideoLibrary::FillFileItemList(const CVariant &parameterObject, CFileItemList &list)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return false;

  CStdString file = parameterObject["file"].asString();
  int movieID = (int)parameterObject["movieid"].asInteger(-1);
  int episodeID = (int)parameterObject["episodeid"].asInteger(-1);
  int musicVideoID = (int)parameterObject["musicvideoid"].asInteger(-1);

  bool success = false;
  CFileItem fileItem;
  if (FillFileItem(file, fileItem))
  {
    success = true;
    list.Add(CFileItemPtr(new CFileItem(fileItem)));
  }

  if (movieID > 0)
  {
    CVideoInfoTag details;
    videodatabase.GetMovieInfo("", details, movieID);
    if (!details.IsEmpty())
    {
      list.Add(CFileItemPtr(new CFileItem(details)));
      success = true;
    }
  }
  if (episodeID > 0)
  {
    CVideoInfoTag details;
    if (videodatabase.GetEpisodeInfo("", details, episodeID) && !details.IsEmpty())
    {
      list.Add(CFileItemPtr(new CFileItem(details)));
      success = true;
    }
  }
  if (musicVideoID > 0)
  {
    CVideoInfoTag details;
    videodatabase.GetMusicVideoInfo("", details, musicVideoID);
    if (!details.IsEmpty())
    {
      list.Add(CFileItemPtr(new CFileItem(details)));
      success = true;
    }
  }

  return success;
}

APIStatus CVideoLibrary::GetVideos(MediaType mediaType, const std::string &strBaseDir, const CVariant &parameterObject, CFileItemList &items, CVariant &result, CVideoDatabase &videodatabase)
{
  SortDescription sorting;
  ParseLimits(parameterObject, sorting.limitStart, sorting.limitEnd);
  if (!ParseSorting(parameterObject, sorting.sortBy, sorting.sortOrder, sorting.sortAttributes))
    return APIStatusInvalidParameters;

  return videodatabase.GetSortedVideos(mediaType, strBaseDir, sorting, items) ? APIStatusOK : APIStatusInternalError;
}

APIStatus CVideoLibrary::GetAdditionalMovieDetails(const CVariant &parameterObject, CFileItemList &items, CVariant &result, CVideoDatabase &videodatabase)
{
  if (!videodatabase.Open())
    return APIStatusInternalError;

  bool additionalInfo = false;
  bool streamdetails = false;
  for (CVariant::const_iterator_array itr = parameterObject["properties"].begin_array(); itr != parameterObject["properties"].end_array(); itr++)
  {
    CStdString fieldValue = itr->asString();
    if (fieldValue == "cast" || fieldValue == "set" || fieldValue == "setid" || fieldValue == "showlink" || fieldValue == "resume")
      additionalInfo = true;
    else if (fieldValue == "streamdetails")
      streamdetails = true;
  }

  if (additionalInfo || streamdetails)
  {
    for (int index = 0; index < items.Size(); index++)
    {
      if (additionalInfo)
        videodatabase.GetMovieInfo("", *(items[index]->GetVideoInfoTag()), items[index]->GetVideoInfoTag()->m_iDbId);
      if (streamdetails)
        videodatabase.GetStreamDetails(*(items[index]->GetVideoInfoTag()));
    }
  }

  int size = items.Size();
  if (items.HasProperty("total") && items.GetProperty("total").asInteger() > size)
    size = (int)items.GetProperty("total").asInteger();
  HandleFileItemList("movieid", true, "movies", items, parameterObject, result, size, false);

  return APIStatusOK;
}

APIStatus CVideoLibrary::GetAdditionalEpisodeDetails(const CVariant &parameterObject, CFileItemList &items, CVariant &result, CVideoDatabase &videodatabase)
{
  if (!videodatabase.Open())
    return APIStatusInternalError;

  bool additionalInfo = false;
  bool streamdetails = false;
  for (CVariant::const_iterator_array itr = parameterObject["properties"].begin_array(); itr != parameterObject["properties"].end_array(); itr++)
  {
    CStdString fieldValue = itr->asString();
    if (fieldValue == "cast" || fieldValue == "resume")
      additionalInfo = true;
    else if (fieldValue == "streamdetails")
      streamdetails = true;
  }

  if (additionalInfo || streamdetails)
  {
    for (int index = 0; index < items.Size(); index++)
    {
      if (additionalInfo)
        videodatabase.GetEpisodeInfo("", *(items[index]->GetVideoInfoTag()), items[index]->GetVideoInfoTag()->m_iDbId);
      if (streamdetails)
        videodatabase.GetStreamDetails(*(items[index]->GetVideoInfoTag()));
    }
  }
  
  int size = items.Size();
  if (items.HasProperty("total") && items.GetProperty("total").asInteger() > size)
    size = (int)items.GetProperty("total").asInteger();
  HandleFileItemList("episodeid", true, "episodes", items, parameterObject, result, size, false);

  return APIStatusOK;
}

APIStatus CVideoLibrary::GetAdditionalMusicVideoDetails(const CVariant &parameterObject, CFileItemList &items, CVariant &result, CVideoDatabase &videodatabase)
{
  if (!videodatabase.Open())
    return APIStatusInternalError;

  bool additionalInfo = false;
  bool streamdetails = false;
  for (CVariant::const_iterator_array itr = parameterObject["properties"].begin_array(); itr != parameterObject["properties"].end_array(); itr++)
  {
    CStdString fieldValue = itr->asString();
    if (fieldValue == "resume")
      additionalInfo = true;
    else if (fieldValue == "streamdetails")
      streamdetails = true;
  }

  if (additionalInfo || streamdetails)
  {
    for (int index = 0; index < items.Size(); index++)
    {
      if (additionalInfo)
        videodatabase.GetMusicVideoInfo("", *(items[index]->GetVideoInfoTag()), items[index]->GetVideoInfoTag()->m_iDbId);
      if (streamdetails)
        videodatabase.GetStreamDetails(*(items[index]->GetVideoInfoTag()));
    }
  }

  int size = items.Size();
  if (items.HasProperty("total") && items.GetProperty("total").asInteger() > size)
    size = (int)items.GetProperty("total").asInteger();
  HandleFileItemList("musicvideoid", true, "musicvideos", items, parameterObject, result, size, false);

  return APIStatusOK;
}

APIStatus CVideoLibrary::RemoveVideo(const CVariant &parameterObject)
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return APIStatusInternalError;

  if (parameterObject.isMember("movieid"))
    videodatabase.DeleteMovie((int)parameterObject["movieid"].asInteger());
  else if (parameterObject.isMember("tvshowid"))
    videodatabase.DeleteTvShow((int)parameterObject["tvshowid"].asInteger());
  else if (parameterObject.isMember("episodeid"))
    videodatabase.DeleteEpisode((int)parameterObject["episodeid"].asInteger());
  else if (parameterObject.isMember("musicvideoid"))
    videodatabase.DeleteMusicVideo((int)parameterObject["musicvideoid"].asInteger());
  return APIStatusOK;
}

void CVideoLibrary::UpdateVideoTag(const CVariant &parameterObject, CVideoInfoTag& details, std::map<std::string, std::string> &artwork)
{
  if (ParameterNotNull(parameterObject, "title"))
    details.m_strTitle = parameterObject["title"].asString();
  if (ParameterNotNull(parameterObject, "playcount"))
    details.m_playCount = (int)parameterObject["playcount"].asInteger();
  if (ParameterNotNull(parameterObject, "runtime"))
    details.m_strRuntime = parameterObject["runtime"].asString();
  if (ParameterNotNull(parameterObject, "director"))
    CopyStringArray(parameterObject["director"], details.m_director);
  if (ParameterNotNull(parameterObject, "studio"))
    CopyStringArray(parameterObject["studio"], details.m_studio);
  if (ParameterNotNull(parameterObject, "year"))
    details.m_iYear = (int)parameterObject["year"].asInteger();
  if (ParameterNotNull(parameterObject, "plot"))
    details.m_strPlot = parameterObject["plot"].asString();
  if (ParameterNotNull(parameterObject, "album"))
    details.m_strAlbum = parameterObject["album"].asString();
  if (ParameterNotNull(parameterObject, "artist"))
    CopyStringArray(parameterObject["artist"], details.m_artist);
  if (ParameterNotNull(parameterObject, "genre"))
    CopyStringArray(parameterObject["genre"], details.m_genre);
  if (ParameterNotNull(parameterObject, "track"))
    details.m_iTrack = (int)parameterObject["track"].asInteger();
  if (ParameterNotNull(parameterObject, "rating"))
    details.m_fRating = parameterObject["rating"].asFloat();
  if (ParameterNotNull(parameterObject, "mpaa"))
    details.m_strMPAARating = parameterObject["mpaa"].asString();
  if (ParameterNotNull(parameterObject, "imdbnumber"))
    details.m_strIMDBNumber = parameterObject["imdbnumber"].asString();
  if (ParameterNotNull(parameterObject, "premiered"))
    details.m_premiered.SetFromDBDate(parameterObject["premiered"].asString());
  if (ParameterNotNull(parameterObject, "votes"))
    details.m_strVotes = parameterObject["votes"].asString();
  if (ParameterNotNull(parameterObject, "lastplayed"))
    details.m_lastPlayed.SetFromDBDateTime(parameterObject["lastplayed"].asString());
  if (ParameterNotNull(parameterObject, "firstaired"))
    details.m_firstAired.SetFromDBDateTime(parameterObject["firstaired"].asString());
  if (ParameterNotNull(parameterObject, "productioncode"))
    details.m_strProductionCode = parameterObject["productioncode"].asString();
  if (ParameterNotNull(parameterObject, "season"))
    details.m_iSeason = (int)parameterObject["season"].asInteger();
  if (ParameterNotNull(parameterObject, "episode"))
    details.m_iEpisode = (int)parameterObject["episode"].asInteger();
  if (ParameterNotNull(parameterObject, "originaltitle"))
    details.m_strOriginalTitle = parameterObject["originaltitle"].asString();
  if (ParameterNotNull(parameterObject, "trailer"))
    details.m_strTrailer = parameterObject["trailer"].asString();
  if (ParameterNotNull(parameterObject, "tagline"))
    details.m_strTagLine = parameterObject["tagline"].asString();
  if (ParameterNotNull(parameterObject, "plotoutline"))
    details.m_strPlotOutline = parameterObject["plotoutline"].asString();
  if (ParameterNotNull(parameterObject, "writer"))
    CopyStringArray(parameterObject["writer"], details.m_writingCredits);
  if (ParameterNotNull(parameterObject, "country"))
    CopyStringArray(parameterObject["country"], details.m_country);
  if (ParameterNotNull(parameterObject, "top250"))
    details.m_iTop250 = (int)parameterObject["top250"].asInteger();
  if (ParameterNotNull(parameterObject, "sorttitle"))
    details.m_strSortTitle = parameterObject["sorttitle"].asString();
  if (ParameterNotNull(parameterObject, "episodeguide"))
    details.m_strEpisodeGuide = parameterObject["episodeguide"].asString();
  if (ParameterNotNull(parameterObject, "set"))
    CopyStringArray(parameterObject["set"], details.m_set);
  if (ParameterNotNull(parameterObject, "showlink"))
    CopyStringArray(parameterObject["showlink"], details.m_showLink);
  if (ParameterNotNull(parameterObject, "thumbnail"))
    artwork["thumb"] = parameterObject["thumbnail"].asString();
  if (ParameterNotNull(parameterObject, "fanart"))
    artwork["fanart"] = parameterObject["fanart"].asString();
}
