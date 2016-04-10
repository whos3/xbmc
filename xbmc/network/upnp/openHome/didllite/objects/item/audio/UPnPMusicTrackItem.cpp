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

#include "UPnPMusicTrackItem.h"
#include "FileItem.h"
#include "music/tags/MusicInfoTag.h"

CUPnPMusicTrackItem::CUPnPMusicTrackItem()
  : CUPnPMusicTrackItem("object.item.audioItem.musicTrack")
{ }

CUPnPMusicTrackItem::CUPnPMusicTrackItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPAudioItem(classType, className)
{ }

CUPnPMusicTrackItem::CUPnPMusicTrackItem(const CUPnPMusicTrackItem& musicTrackItem)
  : CUPnPAudioItem(musicTrackItem)
{
  copyPropertyValidity(&musicTrackItem);
}

CUPnPMusicTrackItem::~CUPnPMusicTrackItem()
{ }

bool CUPnPMusicTrackItem::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPAudioItem::CanHandleFileItem(item) && item.GetMusicInfoTag()->GetType() == MediaTypeSong;
}

bool CUPnPMusicTrackItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPAudioItem::ToFileItem(item, context))
    return false;

  MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

  musicInfo.SetDatabaseId(0, MediaTypeSong);
  musicInfo.SetTrackNumber(GetOriginalTrackNumber());

  return true;
}

bool CUPnPMusicTrackItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (!CUPnPAudioItem::FromFileItem(item, context))
    return false;

  const MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

  SetOriginalTrackNumber(musicInfo.GetTrackNumber());

  return true;
}
