/*
 *      Copyright (C) 2015 Team XBMC
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

#include "UPnPMusicVideoClipItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"
#include "video/VideoDatabase.h"
#include "video/VideoInfoTag.h"
#include "video/VideoThumbLoader.h"

CUPnPMusicVideoClipItem::CUPnPMusicVideoClipItem()
  : CUPnPMusicVideoClipItem("object.item.videoItem.musicVideoClip")
{ }

CUPnPMusicVideoClipItem::CUPnPMusicVideoClipItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPVideoItem(classType, className)
{ }

CUPnPMusicVideoClipItem::CUPnPMusicVideoClipItem(const CUPnPMusicVideoClipItem& musicVideoClipItem)
  : CUPnPVideoItem(musicVideoClipItem)
{
  copyPropertyValidity(&musicVideoClipItem);
}

CUPnPMusicVideoClipItem::~CUPnPMusicVideoClipItem()
{ }

bool CUPnPMusicVideoClipItem::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPVideoItem::CanHandleFileItem(item) && item.GetVideoInfoTag()->m_type == MediaTypeMusicVideo;
}

bool CUPnPMusicVideoClipItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPVideoItem::ToFileItem(item, context))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  videoInfo.m_type = MediaTypeMusicVideo;

  // artists
  auto artists = GetArtists();
  for (const auto& artist : artists)
    videoInfo.m_artist.push_back(artist->GetPerson());

  // album
  auto albums = GetAlbums();
  if (!albums.empty())
    videoInfo.m_strAlbum = albums.front();

  return true;
}

bool CUPnPMusicVideoClipItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  // create a copy of the item, retrieve additional details and serialize it
  CFileItem detailedItem = item;

  CVideoInfoTag& videoInfo = *detailedItem.GetVideoInfoTag();

  // TODO: very expensive
  {
    CVideoDatabase db;
    if (db.Open())
      db.GetMusicVideoInfo(videoInfo.GetPath(), videoInfo, videoInfo.m_iDbId);
  }

  if (!CUPnPVideoItem::FromFileItem(detailedItem, context))
    return false;

  SetArtists(videoInfo.m_artist);

  if (!videoInfo.m_strAlbum.empty())
    SetAlbums({ videoInfo.m_strAlbum });

  return true;
}
