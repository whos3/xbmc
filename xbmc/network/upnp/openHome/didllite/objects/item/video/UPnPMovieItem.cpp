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

#include "UPnPMovieItem.h"
#include "FileItem.h"
#include "video/VideoDatabase.h"
#include "video/VideoInfoTag.h"

CUPnPMovieItem::CUPnPMovieItem()
  : CUPnPMovieItem("object.item.videoItem.movie")
{ }

CUPnPMovieItem::CUPnPMovieItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPVideoItem(classType, className)
{ }

CUPnPMovieItem::CUPnPMovieItem(const CUPnPMovieItem& movieItem)
  : CUPnPVideoItem(movieItem)
{
  copyPropertyValidity(&movieItem);
}

CUPnPMovieItem::~CUPnPMovieItem()
{ }

bool CUPnPMovieItem::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPVideoItem::CanHandleFileItem(item) && item.GetVideoInfoTag()->m_type == MediaTypeMovie;
}

bool CUPnPMovieItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPVideoItem::ToFileItem(item, context))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  videoInfo.m_type = MediaTypeMovie;
  videoInfo.m_premiered = GetDate();

  return true;
}

bool CUPnPMovieItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  // create a copy of the item, retrieve additional details and serialize it
  CFileItem detailedItem = item;

  CVideoInfoTag& videoInfo = *detailedItem.GetVideoInfoTag();

  // TODO: very expensive
  {
    CVideoDatabase db;
    if (db.Open())
      db.GetMovieInfo(videoInfo.GetPath(), videoInfo, videoInfo.m_iDbId);
  }

  if (!CUPnPVideoItem::FromFileItem(detailedItem, context))
    return false;

  if (videoInfo.m_premiered.IsValid())
    SetDate(videoInfo.m_premiered);

  return true;
}
