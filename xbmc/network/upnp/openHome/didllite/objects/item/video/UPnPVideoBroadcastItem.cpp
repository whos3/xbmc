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

#include "UPnPVideoBroadcastItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "video/VideoDatabase.h"
#include "video/VideoInfoTag.h"
#include "video/VideoThumbLoader.h"

CUPnPVideoBroadcastItem::CUPnPVideoBroadcastItem()
  : CUPnPVideoBroadcastItem("object.item.videoItem.videoBroadcast")
{ }

CUPnPVideoBroadcastItem::CUPnPVideoBroadcastItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPVideoItem(classType, className)
{ }

CUPnPVideoBroadcastItem::CUPnPVideoBroadcastItem(const CUPnPVideoBroadcastItem& videoBroadcastItem)
  : CUPnPVideoItem(videoBroadcastItem)
{
  copyPropertyValidity(&videoBroadcastItem);
}

CUPnPVideoBroadcastItem::~CUPnPVideoBroadcastItem()
{ }

bool CUPnPVideoBroadcastItem::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPVideoItem::CanHandleFileItem(item) && item.GetVideoInfoTag()->m_type == MediaTypeEpisode;
}

bool CUPnPVideoBroadcastItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPVideoItem::ToFileItem(item, context))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  videoInfo.m_type = MediaTypeEpisode;
  videoInfo.m_strShowTitle = GetSeriesTitle();
  videoInfo.m_iSeason = GetEpisodeSeason();
  videoInfo.m_iEpisode = GetEpisodeNumber();
  videoInfo.m_firstAired = GetDate();

  // parse the title? SXXEYY: <episode title>

  return true;
}

bool CUPnPVideoBroadcastItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  // create a copy of the item, retrieve additional details and serialize it
  CFileItem detailedItem = item;

  CVideoInfoTag& videoInfo = *detailedItem.GetVideoInfoTag();

  // TODO: very expensive
  {
    CVideoDatabase db;
    if (db.Open())
      db.GetEpisodeInfo(videoInfo.GetPath(), videoInfo, videoInfo.m_iDbId);
  }

  if (!CUPnPVideoItem::FromFileItem(detailedItem, context))
    return false;

  SetSeriesTitle(videoInfo.m_strShowTitle);
  SetEpisodeSeason(videoInfo.m_iSeason);
  SetEpisodeNumber(videoInfo.m_iEpisode);
  if (videoInfo.m_firstAired.IsValid())
    SetDate(videoInfo.m_firstAired);

  return true;
}
