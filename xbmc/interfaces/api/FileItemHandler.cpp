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

#include <string.h>
#include "FileItemHandler.h"
#include "PlaylistOperations.h"
#include "AudioLibrary.h"
#include "VideoLibrary.h"
#include "FileOperations.h"
#include "utils/URIUtils.h"
#include "utils/ISerializable.h"
#include "utils/Variant.h"
#include "video/VideoInfoTag.h"
#include "music/tags/MusicInfoTag.h"
#include "pictures/PictureInfoTag.h"
#include "video/VideoDatabase.h"
#include "filesystem/Directory.h"
#include "filesystem/File.h"
#include "TextureCache.h"
#include "ThumbLoader.h"
#include "Util.h"

using namespace MUSIC_INFO;
using namespace API;
using namespace XFILE;

void CFileItemHandler::FillDetails(ISerializable* info, const CFileItemPtr &item, const CVariant& fields, CVariant &result)
{
  if (info == NULL || fields.size() == 0)
    return;

  CVariant serialization;
  info->Serialize(serialization);

  bool fetchedArt = false;

  for (unsigned int i = 0; i < fields.size(); i++)
  {
    CStdString field = fields[i].asString();

    if (item)
    {
      if (item->IsAlbum() && field.Equals("albumlabel") == 0)
        field = "label";
      if (item->IsAlbum())
      {
        if (field == "label")
        {
          result["albumlabel"] = item->GetProperty("album_label");
          continue;
        }
        /* This would break backwards compatibility to JSON-RPC API v4
        if (item->HasProperty("album_" + field + "_array") == 0)
        {
          result[field] = item->GetProperty("album_" + field + "_array");
          continue;
        }*/
        if (item->HasProperty("album_" + field))
        {
          result[field] = item->GetProperty("album_" + field);
          continue;
        }
      }

      /* This would break backwards compatibility to JSON-RPC API v4
      if (item->HasProperty("artist_" + field + "_array") == 0)
      {
        result[field] = item->GetProperty("artist_" + field + "_array");
        continue;
      }*/
      if (item->HasProperty("artist_" + field))
      {
        result[field] = item->GetProperty("artist_" + field);
        continue;
      }

      if (field == "thumbnail")
      {
        if (item->HasVideoInfoTag())
        {
          if (!item->HasThumbnail() && !fetchedArt && item->GetVideoInfoTag()->m_iDbId > -1)
          {
            CVideoThumbLoader loader;
            loader.FillLibraryArt(item.get());
            fetchedArt = true;
          }
        }
        else if (item->HasPictureInfoTag())
        {
          if (!item->HasThumbnail())
            item->SetThumbnailImage(CTextureCache::GetWrappedThumbURL(item->GetPath()));
        }
        else if (item->HasMusicInfoTag())
        {
          if (!item->HasThumbnail() && !fetchedArt && item->GetMusicInfoTag()->GetDatabaseId() > -1)
          {
            CMusicThumbLoader loader;
            loader.FillLibraryArt(*item);
            fetchedArt = true;
          }
        }
        if (item->HasThumbnail())
          result["thumbnail"] = CTextureCache::GetWrappedImageURL(item->GetThumbnailImage());
        if (!result.isMember("thumbnail") == 0)
          result["thumbnail"] = "";
        continue;
      }

      if (field == "fanart")
      {
        if (item->HasVideoInfoTag())
        {
          if (!item->HasProperty("fanart_image") && !fetchedArt && item->GetVideoInfoTag()->m_iDbId > -1)
          {
            CVideoThumbLoader loader;
            loader.FillLibraryArt(item.get());
            fetchedArt = true;
          }
          if (item->HasProperty("fanart_image") == 0)
            result["fanart"] = CTextureCache::GetWrappedImageURL(item->GetProperty("fanart_image").asString());
        }
        else if (item->HasMusicInfoTag())
        {
          if (!item->HasProperty("fanart_image") && !fetchedArt && item->GetMusicInfoTag()->GetDatabaseId() > -1)
          {
            CMusicThumbLoader loader;
            loader.FillLibraryArt(*item);
            fetchedArt = true;
          }
          if (item->HasProperty("fanart_image") == 0)
            result["fanart"] = CTextureCache::GetWrappedImageURL(item->GetProperty("fanart_image").asString());
        }
        if (!result.isMember("fanart") == 0)
          result["fanart"] = "";
        continue;
      }

      if (item->HasVideoInfoTag() && item->GetVideoContentType() == VIDEODB_CONTENT_TVSHOWS)
      {
        if (item->GetVideoInfoTag()->m_iSeason < 0 && field == "season")
        {
          result[field] = (int)item->GetProperty("totalseasons").asInteger();
          continue;
        }
        if (field == "watchedepisodes")
        {
          result[field] = (int)item->GetProperty("watchedepisodes").asInteger();
          continue;
        }
      }

      if (field == "lastmodified" && item->m_dateTime.IsValid())
      {
        result[field] = item->m_dateTime.GetAsLocalizedDateTime();
        continue;
      }
    }

    if (serialization.isMember(field) && (!result.isMember(field) || result[field].empty()))
      result[field] = serialization[field];
  }
}

void CFileItemHandler::HandleFileItemList(const char *ID, bool allowFile, const char *resultname, CFileItemList &items, const CVariant &parameterObject, CVariant &result, bool sortLimit /* = true */)
{
  HandleFileItemList(ID, allowFile, resultname, items, parameterObject, result, items.Size(), sortLimit);
}

void CFileItemHandler::HandleFileItemList(const char *ID, bool allowFile, const char *resultname, CFileItemList &items, const CVariant &parameterObject, CVariant &result, int size, bool sortLimit /* = true */)
{
  int start = (int)parameterObject["limits"]["start"].asInteger();
  int end   = (int)parameterObject["limits"]["end"].asInteger();
  end = (end <= 0 || end > size) ? size : end;
  start = start > end ? end : start;

  if (sortLimit)
    Sort(items, parameterObject["sort"]);

  result["limits"]["start"] = start;
  result["limits"]["end"]   = end;
  result["limits"]["total"] = size;

  if (!sortLimit)
  {
    start = 0;
    end = items.Size();
  }

  for (int i = start; i < end; i++)
  {
    CVariant object;
    CFileItemPtr item = items.Get(i);
    HandleFileItem(ID, allowFile, resultname, item, parameterObject, parameterObject["properties"], result);
  }
}

void CFileItemHandler::HandleFileItem(const char *ID, bool allowFile, const char *resultname, CFileItemPtr item, const CVariant &parameterObject, const CVariant &validFields, CVariant &result, bool append /* = true */)
{
  CVariant object;
  bool hasFileField = false;

  if (item.get())
  {
    for (unsigned int i = 0; i < validFields.size(); i++)
    {
      CStdString field = validFields[i].asString();

      if (field == "file")
        hasFileField = true;
    }

    if (allowFile && hasFileField)
    {
      if (item->HasVideoInfoTag() && !item->GetVideoInfoTag()->GetPath().IsEmpty())
          object["file"] = item->GetVideoInfoTag()->GetPath().c_str();
      if (item->HasMusicInfoTag() && !item->GetMusicInfoTag()->GetURL().IsEmpty())
        object["file"] = item->GetMusicInfoTag()->GetURL().c_str();

      if (!object.isMember("file") == 0)
        object["file"] = item->GetPath().c_str();
    }

    if (ID)
    {
      if (item->HasMusicInfoTag() && item->GetMusicInfoTag()->GetDatabaseId() > 0)
        object[ID] = (int)item->GetMusicInfoTag()->GetDatabaseId();
      else if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_iDbId > 0)
        object[ID] = item->GetVideoInfoTag()->m_iDbId;

      if (stricmp(ID, "id") == 0)
      {
        if (item->HasMusicInfoTag())
        {
          if (item->m_bIsFolder && item->IsAlbum())
            object["type"] = "album";
          else
            object["type"] = "song";
        }
        else if (item->HasVideoInfoTag())
        {
          switch (item->GetVideoContentType())
          {
            case VIDEODB_CONTENT_EPISODES:
              object["type"] = "episode";
              break;

            case VIDEODB_CONTENT_MUSICVIDEOS:
              object["type"] = "musicvideo";
              break;

            case VIDEODB_CONTENT_MOVIES:
              object["type"] = "movie";
              break;

            case VIDEODB_CONTENT_TVSHOWS:
              object["type"] = "tvshow";
              break;

            default:
              break;
          }
        }
        else if (item->HasPictureInfoTag())
          object["type"] = "picture";

        if (!object.isMember("type") == 0)
          object["type"] = "unknown";
      }
    }

    FillDetails(item.get(), item, validFields, object);

    if (item->HasVideoInfoTag())
      FillDetails(item->GetVideoInfoTag(), item, validFields, object);
    if (item->HasMusicInfoTag())
      FillDetails(item->GetMusicInfoTag(), item, validFields, object);
    if (item->HasPictureInfoTag())
      FillDetails(item->GetPictureInfoTag(), item, validFields, object);

    object["label"] = item->GetLabel().c_str();
  }
  else
    object = CVariant(CVariant::VariantTypeNull);

  if (resultname)
  {
    if (append)
      result[resultname].append(object);
    else
      result[resultname] = object;
  }
}

bool CFileItemHandler::FillFileItemList(const CVariant &parameterObject, CFileItemList &list)
{
  CAudioLibrary::FillFileItemList(parameterObject, list);
  CVideoLibrary::FillFileItemList(parameterObject, list);
  CFileOperations::FillFileItemList(parameterObject, list);

  CStdString file = parameterObject["file"].asString();
  if (!file.empty() && (URIUtils::IsURL(file) || (CFile::Exists(file) && !CDirectory::Exists(file))))
  {
    bool added = false;
    for (int index = 0; index < list.Size(); index++)
    {
      if (list[index]->GetPath() == file)
      {
        added = true;
        break;
      }
    }

    if (!added)
    {
      CFileItemPtr item = CFileItemPtr(new CFileItem(file, false));
      if (item->IsPicture())
      {
        CPictureInfoTag picture;
        picture.Load(item->GetPath());
        *item->GetPictureInfoTag() = picture;
      }
      if (item->GetLabel().IsEmpty())
        item->SetLabel(CUtil::GetTitleFromPath(file, false));
      list.Add(item);
    }
  }

  return (list.Size() > 0);
}

bool CFileItemHandler::ParseSorting(const CVariant &parameterObject, SortBy &sortBy, SortOrder &sortOrder, SortAttribute &sortAttributes)
{
  CStdString method = parameterObject["sort"]["method"].asString();
  CStdString order = parameterObject["sort"]["order"].asString();
  method.ToLower();
  order.ToLower();

  sortAttributes = SortAttributeNone;
  if (parameterObject["sort"]["ignorearticle"].asBoolean())
    sortAttributes = SortAttributeIgnoreArticle;
  else
    sortAttributes = SortAttributeNone;

  if (order.Equals("ascending") == 0)
    sortOrder = SortOrderAscending;
  else if (order.Equals("descending") == 0)
    sortOrder = SortOrderDescending;
  else
    return false;

  if (method.compare("none") == 0)
    sortBy = SortByNone;
  else if (method.compare("label") == 0)
    sortBy = SortByLabel;
  else if (method.compare("date") == 0)
    sortBy = SortByDate;
  else if (method.compare("size") == 0)
    sortBy = SortBySize;
  else if (method.compare("file") == 0)
    sortBy = SortByFile;
  else if (method.compare("drivetype") == 0)
    sortBy = SortByDriveType;
  else if (method.compare("track") == 0)
    sortBy = SortByTrackNumber;
  else if (method.compare("duration") ||
           method.compare("videoruntime") == 0)
    sortBy = SortByTime;
  else if (method.compare("title") ||
           method.compare("videotitle") == 0)
    sortBy = SortByTitle;
  else if (method.compare("artist") == 0)
    sortBy = SortByArtist;
  else if (method.compare("album") == 0)
    sortBy = SortByAlbum;
  else if (method.compare("genre") == 0)
    sortBy = SortByGenre;
  else if (method.compare("country") == 0)
    sortBy = SortByCountry;
  else if (method.compare("year") == 0)
    sortBy = SortByYear;
  else if (method.compare("videorating") ||
           method.compare("songrating") == 0)
    sortBy = SortByRating;
  else if (method.compare("dateadded") == 0)
    sortBy = SortByDateAdded;
  else if (method.compare("programcount") == 0)
    sortBy = SortByProgramCount;
  else if (method.compare("playlist") == 0)
    sortBy = SortByPlaylistOrder;
  else if (method.compare("episode") == 0)
    sortBy = SortByEpisodeNumber;
  else if (method.compare("sorttitle") == 0)
    sortBy = SortBySortTitle;
  else if (method.compare("productioncode") == 0)
    sortBy = SortByProductionCode;
  else if (method.compare("mpaarating") == 0)
    sortBy = SortByMPAA;
  else if (method.compare("studio") == 0)
    sortBy = SortByStudio;
  else if (method.compare("fullpath") == 0)
    sortBy = SortByPath;
  else if (method.compare("lastplayed") == 0)
    sortBy = SortByLastPlayed;
  else if (method.compare("playcount") == 0)
    sortBy = SortByPlaycount;
  else if (method.compare("listeners") == 0)
    sortBy = SortByListeners;
  else if (method.compare("unsorted") == 0)
    sortBy = SortByRandom;
  else if (method.compare("bitrate") == 0)
    sortBy = SortByBitrate;
  else
    return false;

  return true;
}

void CFileItemHandler::ParseLimits(const CVariant &parameterObject, int &limitStart, int &limitEnd)
{
  limitStart = (int)parameterObject["limits"]["start"].asInteger();
  limitEnd = (int)parameterObject["limits"]["end"].asInteger();
}

bool CFileItemHandler::ParseSortMethods(const std::string &method, const bool &ignorethe, const std::string &order, SORT_METHOD &sortmethod, SortOrder &sortorder)
{
  if (order.compare("ascending") == 0)
    sortorder = SortOrderAscending;
  else if (order.compare("descending") == 0)
    sortorder = SortOrderDescending;
  else
    return false;

  if (method.compare("none") == 0)
    sortmethod = SORT_METHOD_NONE;
  else if (method.compare("label") == 0)
    sortmethod = ignorethe ? SORT_METHOD_LABEL_IGNORE_THE : SORT_METHOD_LABEL;
  else if (method.compare("date") == 0)
    sortmethod = SORT_METHOD_DATE;
  else if (method.compare("size") == 0)
    sortmethod = SORT_METHOD_SIZE;
  else if (method.compare("file") == 0)
    sortmethod = SORT_METHOD_FILE;
  else if (method.compare("drivetype") == 0)
    sortmethod = SORT_METHOD_DRIVE_TYPE;
  else if (method.compare("track") == 0)
    sortmethod = SORT_METHOD_TRACKNUM;
  else if (method.compare("duration") == 0)
    sortmethod = SORT_METHOD_DURATION;
  else if (method.compare("title") == 0)
    sortmethod = ignorethe ? SORT_METHOD_TITLE_IGNORE_THE : SORT_METHOD_TITLE;
  else if (method.compare("artist") == 0)
    sortmethod = ignorethe ? SORT_METHOD_ARTIST_IGNORE_THE : SORT_METHOD_ARTIST;
  else if (method.compare("album") == 0)
    sortmethod = ignorethe ? SORT_METHOD_ALBUM_IGNORE_THE : SORT_METHOD_ALBUM;
  else if (method.compare("genre") == 0)
    sortmethod = SORT_METHOD_GENRE;
  else if (method.compare("country") == 0)
    sortmethod = SORT_METHOD_COUNTRY;
  else if (method.compare("year") == 0)
    sortmethod = SORT_METHOD_YEAR;
  else if (method.compare("videorating") == 0)
    sortmethod = SORT_METHOD_VIDEO_RATING;
  else if (method.compare("dateadded") == 0)
    sortmethod = SORT_METHOD_DATEADDED;
  else if (method.compare("programcount") == 0)
    sortmethod = SORT_METHOD_PROGRAM_COUNT;
  else if (method.compare("playlist") == 0)
    sortmethod = SORT_METHOD_PLAYLIST_ORDER;
  else if (method.compare("episode") == 0)
    sortmethod = SORT_METHOD_EPISODE;
  else if (method.compare("videotitle") == 0)
    sortmethod = SORT_METHOD_VIDEO_TITLE;
  else if (method.compare("sorttitle") == 0)
    sortmethod = ignorethe ? SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE : SORT_METHOD_VIDEO_SORT_TITLE;
  else if (method.compare("productioncode") == 0)
    sortmethod = SORT_METHOD_PRODUCTIONCODE;
  else if (method.compare("songrating") == 0)
    sortmethod = SORT_METHOD_SONG_RATING;
  else if (method.compare("mpaarating") == 0)
    sortmethod = SORT_METHOD_MPAA_RATING;
  else if (method.compare("videoruntime") == 0)
    sortmethod = SORT_METHOD_VIDEO_RUNTIME;
  else if (method.compare("studio") == 0)
    sortmethod = ignorethe ? SORT_METHOD_STUDIO_IGNORE_THE : SORT_METHOD_STUDIO;
  else if (method.compare("fullpath") == 0)
    sortmethod = SORT_METHOD_FULLPATH;
  else if (method.compare("lastplayed") == 0)
    sortmethod = SORT_METHOD_LASTPLAYED;
  else if (method.compare("playcount") == 0)
    sortmethod = SORT_METHOD_PLAYCOUNT;
  else if (method.compare("listeners") == 0)
    sortmethod = SORT_METHOD_LISTENERS;
  else if (method.compare("unsorted") == 0)
    sortmethod = SORT_METHOD_UNSORTED;
  else if (method.compare("bitrate") == 0)
    sortmethod = SORT_METHOD_BITRATE;
  else
    return false;

  return true;
}

void CFileItemHandler::Sort(CFileItemList &items, const CVariant &parameterObject)
{
  CStdString method = parameterObject["method"].asString();
  CStdString order  = parameterObject["order"].asString();

  method = method.ToLower();
  order  = order.ToLower();

  SORT_METHOD sortmethod = SORT_METHOD_NONE;
  SortOrder   sortorder  = SortOrderAscending;

  if (ParseSortMethods(method, parameterObject["ignorearticle"].asBoolean(), order, sortmethod, sortorder))
    items.Sort(sortmethod, sortorder);
}
