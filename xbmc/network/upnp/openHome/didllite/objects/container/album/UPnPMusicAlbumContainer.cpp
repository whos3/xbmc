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

#include "UPnPMusicAlbumContainer.h"
#include "FileItem.h"
#include "music/tags/MusicInfoTag.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"

CUPnPMusicAlbumContainer::CUPnPMusicAlbumContainer()
  : CUPnPMusicAlbumContainer("object.container.album.musicAlbum")
{ }

CUPnPMusicAlbumContainer::CUPnPMusicAlbumContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPAlbumContainer(classType, className)
{ }

CUPnPMusicAlbumContainer::CUPnPMusicAlbumContainer(const CUPnPMusicAlbumContainer& musicAlbumContainer)
  : CUPnPAlbumContainer(musicAlbumContainer)
{
  copyPropertyValidity(&musicAlbumContainer);
}

CUPnPMusicAlbumContainer::~CUPnPMusicAlbumContainer()
{ }

bool CUPnPMusicAlbumContainer::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPAlbumContainer::CanHandleFileItem(item) && item.HasMusicInfoTag() && item.GetMusicInfoTag()->GetType() == MediaTypeAlbum;
}

bool CUPnPMusicAlbumContainer::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPAlbumContainer::ToFileItem(item, context))
    return false;

  MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();
  // TODO: differentiate between album artist and normal artist
  auto artists = GetArtists();
  for (const auto& artist : artists)
    musicInfo.AppendArtist(artist->GetPerson());

  auto genres = GetGenres();
  for (const auto& genre : genres)
    musicInfo.AppendGenre(genre->GetGenre());

  return true;
}

bool CUPnPMusicAlbumContainer::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (!CUPnPAlbumContainer::FromFileItem(item, context))
    return false;

  const MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();
  // TODO

  return true;
}
