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
#include "music/MusicThumbLoader.h"
#include "music/tags/MusicInfoTag.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "utils/Variant.h"
#include "video/VideoInfoTag.h"
#include "video/VideoThumbLoader.h"

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
{ }

bool CUPnPMusicArtistPersonContainer::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPPersonContainer::CanHandleFileItem(item) &&
    ((item.HasMusicInfoTag() && item.GetMusicInfoTag()->GetType() == MediaTypeArtist) ||
     (item.HasVideoInfoTag() && (item.GetVideoInfoTag()->m_type == MediaTypeArtist ||
      item.GetVideoInfoTag()->m_type == "actor"))); // TODO: this is a necessary hack for now
}

bool CUPnPMusicArtistPersonContainer::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPPersonContainer::ToFileItem(item, context))
    return false;

  if (item.IsVideoDb())
  {
    CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();

    const auto& artists = GetArtists();
    for (const auto& artist : artists)
      videoInfo.m_artist.push_back(artist->GetPerson());

    const auto& genres = GetGenres();
    for (const auto& genre : genres)
      videoInfo.m_genre.push_back(genre->GetGenre());

    const auto& longDescription = GetLongDescription();
    const auto& description = GetDescription();
    videoInfo.m_strPlot = !longDescription.empty() ? longDescription : description;
    videoInfo.m_strTagLine = description;
  }
  else
  {
    MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

    musicInfo.SetDatabaseId(0, MediaTypeArtist);

    const auto& artists = GetArtists();
    for (const auto& artist : artists)
    {
      musicInfo.AppendArtist(artist->GetPerson());

      if (!artist->GetPersonRole().empty())
        musicInfo.AddArtistRole(artist->GetPersonRole(), artist->GetPerson());
    }

    const auto& genres = GetGenres();
    for (const auto& genre : genres)
      musicInfo.AppendGenre(genre->GetGenre());

    item.SetProperty("artist_description", GetLongDescription());
    musicInfo.SetComment(GetDescription());

    musicInfo.SetLoaded();
  }

  return true;
}

bool CUPnPMusicArtistPersonContainer::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  // create a copy of the item, retrieve additional details and serialize it
  CFileItem detailedItem = item;

  if (item.HasVideoInfoTag())
  {
    CVideoInfoTag& videoInfo = *detailedItem.GetVideoInfoTag();

    // TODO: very expensive
    {
      auto& it = context.thumbLoaders.find("video");
      if (it == context.thumbLoaders.end())
      {
        it = context.thumbLoaders.insert({ "video", std::make_shared<CVideoThumbLoader>() }).first;
        it->second->OnLoaderStart();
      }

      it->second->LoadItem(&detailedItem);
    }

    if (!CUPnPPersonContainer::FromFileItem(detailedItem, context))
      return false;

    SetArtists(videoInfo.m_artist);
    SetGenres(videoInfo.m_genre);
    SetLongDescription(videoInfo.m_strPlot);
  }
  else if (item.HasMusicInfoTag())
  {
    MUSIC_INFO::CMusicInfoTag& musicInfo = *detailedItem.GetMusicInfoTag();

    // TODO: very expensive
    {
      auto& it = context.thumbLoaders.find("music");
      if (it == context.thumbLoaders.end())
      {
        it = context.thumbLoaders.insert({ "music", std::make_shared<CMusicThumbLoader>() }).first;
        it->second->OnLoaderStart();
      }

      it->second->LoadItem(&detailedItem);
    }

    if (!CUPnPPersonContainer::FromFileItem(detailedItem, context))
      return false;

    SetArtists(musicInfo.GetArtist());
    SetGenres(musicInfo.GetGenre());

    SetLongDescription(detailedItem.GetProperty("artist_description").asString());
    SetDescription(musicInfo.GetComment());
  }

  return true;
}
