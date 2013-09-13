/*
 *      Copyright (C) 2013 Team XBMC
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

#include "MediaType.h"
#include "utils/StringUtils.h"

static std::map<std::string, std::string> fillDefaultMediaTypes()
{
  std::map<std::string, std::string> mediaTypes;

  mediaTypes.insert(std::make_pair(MediaTypeMusic,            MediaTypeMusic));
  mediaTypes.insert(std::make_pair(MediaTypeArtist,           MediaTypeArtist "s"));
  mediaTypes.insert(std::make_pair(MediaTypeAlbum,            MediaTypeAlbum "s"));
  mediaTypes.insert(std::make_pair(MediaTypeSong,             MediaTypeSong "s"));
  mediaTypes.insert(std::make_pair(MediaTypeVideo,            MediaTypeVideo "s"));
  mediaTypes.insert(std::make_pair(MediaTypeVideoCollection,  MediaTypeVideoCollection "s"));
  mediaTypes.insert(std::make_pair(MediaTypeMusicVideo,       MediaTypeMusicVideo "s"));
  mediaTypes.insert(std::make_pair(MediaTypeMovie,            MediaTypeMovie "s"));
  mediaTypes.insert(std::make_pair(MediaTypeTvShow,           MediaTypeTvShow "s"));
  mediaTypes.insert(std::make_pair(MediaTypeSeason,           MediaTypeSeason "s"));
  mediaTypes.insert(std::make_pair(MediaTypeEpisode,          MediaTypeEpisode "s"));

  return mediaTypes;
}

std::map<std::string, std::string> MediaTypes::m_mediaTypes = fillDefaultMediaTypes();

bool MediaTypes::IsMediaType(const std::string &strMediaType, const MediaType &mediaType)
{
  std::map<std::string, std::string>::const_iterator strMediaTypeIt = findMediaType(strMediaType);
  std::map<std::string, std::string>::const_iterator mediaTypeIt = findMediaType(mediaType);

  return strMediaTypeIt != m_mediaTypes.end() && mediaTypeIt != m_mediaTypes.end() &&
         strMediaTypeIt->first.compare(mediaTypeIt->first) == 0;
}

MediaType MediaTypes::FromString(const std::string &strMediaType)
{
  std::map<std::string, std::string>::const_iterator mediaTypeIt = findMediaType(strMediaType);
  if (mediaTypeIt == m_mediaTypes.end())
    return MediaTypeNone;

  return mediaTypeIt->first;
}

MediaType MediaTypes::ToPlural(const MediaType &mediaType)
{
  std::map<std::string, std::string>::const_iterator mediaTypeIt = findMediaType(mediaType);
  if (mediaTypeIt == m_mediaTypes.end())
    return MediaTypeNone;

  return mediaTypeIt->second;
}

std::map<std::string, std::string>::const_iterator MediaTypes::findMediaType(const std::string &mediaType)
{
  std::string strMediaType = mediaType;
  StringUtils::ToLower(strMediaType);

  std::map<std::string, std::string>::const_iterator it = m_mediaTypes.find(strMediaType);
  if (it != m_mediaTypes.end())
    return it;

  for (it = m_mediaTypes.begin(); it != m_mediaTypes.end(); ++it)
  {
    if (strMediaType.compare(it->second) == 0)
      return it;
  }

  return m_mediaTypes.end();
}
