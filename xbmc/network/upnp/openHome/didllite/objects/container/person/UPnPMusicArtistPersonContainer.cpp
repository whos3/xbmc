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

#include "UPnPMusicArtistPersonContainer.h"
#include "FileItem.h"
#include "music/tags/MusicInfoTag.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"

CUPnPMusicArtistPersonContainer::CUPnPMusicArtistPersonContainer()
  : CUPnPMusicArtistPersonContainer("object.container.person.musicArtist")
{ }

CUPnPMusicArtistPersonContainer::CUPnPMusicArtistPersonContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPPersonContainer(classType, className)
{ }

CUPnPMusicArtistPersonContainer::CUPnPMusicArtistPersonContainer(const CUPnPMusicArtistPersonContainer& musicArtistContainer)
  : CUPnPPersonContainer(musicArtistContainer)
{
  copyPropertyValidity(&musicArtistContainer);
}

CUPnPMusicArtistPersonContainer::~CUPnPMusicArtistPersonContainer()
{
  auto genres = GetGenres();
  for (const auto& genre : genres)
    delete genre;
}

bool CUPnPMusicArtistPersonContainer::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPContainer::CanHandleFileItem(item) && item.HasMusicInfoTag() && item.GetMusicInfoTag()->GetType() == MediaTypeArtist;
}
