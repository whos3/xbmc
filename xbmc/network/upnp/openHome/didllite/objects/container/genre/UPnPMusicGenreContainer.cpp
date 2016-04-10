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

#include "UPnPMusicGenreContainer.h"
#include "FileItem.h"
#include "filesystem/MusicDatabaseDirectory.h"
#include "music/tags/MusicInfoTag.h"

CUPnPMusicGenreContainer::CUPnPMusicGenreContainer()
  : CUPnPMusicGenreContainer("object.container.genre.musicGenre")
{ }

CUPnPMusicGenreContainer::CUPnPMusicGenreContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPGenreContainer(classType, className)
{ }

CUPnPMusicGenreContainer::CUPnPMusicGenreContainer(const CUPnPMusicGenreContainer& musicGenreContainer)
  : CUPnPGenreContainer(musicGenreContainer)
{ }

CUPnPMusicGenreContainer::~CUPnPMusicGenreContainer()
{ }

bool CUPnPMusicGenreContainer::CanHandleFileItem(const CFileItem& item) const
{
  if (!CUPnPGenreContainer::CanHandleFileItem(item))
    return false;

  if (item.HasMusicInfoTag())
    return item.GetMusicInfoTag()->GetType() == "genre";

  if (item.IsMusicDb())
  {
    XFILE::MUSICDATABASEDIRECTORY::NODE_TYPE node = XFILE::CMusicDatabaseDirectory::GetDirectoryType(item.GetPath());
    return node == XFILE::MUSICDATABASEDIRECTORY::NODE_TYPE_GENRE;
  }

  return false;
}

bool CUPnPMusicGenreContainer::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPGenreContainer::ToFileItem(item, context))
    return false;

  MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

  auto description = GetLongDescription();
  if (description.empty())
    description = GetDescription();
  musicInfo.SetComment(description);

  return true;
}

bool CUPnPMusicGenreContainer::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (!CUPnPGenreContainer::FromFileItem(item, context))
    return false;

  if (!item.HasMusicInfoTag())
    return true;

  const MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

  if (musicInfo.GetComment().empty())
    SetLongDescription(musicInfo.GetComment());

  return true;
}
