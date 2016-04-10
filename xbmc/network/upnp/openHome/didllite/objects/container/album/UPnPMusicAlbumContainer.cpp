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
#include "music/MusicThumbLoader.h"
#include "music/tags/MusicInfoTag.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"
#include "utils/Variant.h"
#include "video/VideoInfoTag.h"
#include "video/VideoThumbLoader.h"

static const std::string ArtistRolePerformer = "Performer";
static const std::string ArtistRoleAlbumArtist = "AlbumArtist";
static const std::string ArtistRoleProducer = "Producer";

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
  return CUPnPContainer::CanHandleFileItem(item) &&
    ((item.HasMusicInfoTag() && item.GetMusicInfoTag()->GetType() == MediaTypeAlbum) ||
     (item.HasVideoInfoTag() && item.GetVideoInfoTag()->m_type == MediaTypeAlbum));
}

bool CUPnPMusicAlbumContainer::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPAlbumContainer::ToFileItem(item, context))
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

    videoInfo.m_studio = GetProducers();
  }
  else
  {
    MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

    musicInfo.SetDatabaseId(0, MediaTypeAlbum);

    const auto& albums = GetAlbums();
    if (!albums.empty())
      musicInfo.SetAlbum(albums.front());
    else
    {
      const auto& title = GetTitle();
      if (!title.empty())
        musicInfo.SetAlbum(title);
      else
        musicInfo.SetAlbum(item.GetLabel());
    }

    auto artists = GetArtists();
    for (const auto& artist : artists)
    {
      const auto& artistName = artist->GetPerson();
      const auto& role = artist->GetPersonRole();
      if (role.empty() || role == ArtistRolePerformer)
        musicInfo.AppendArtist(artistName);
      else if (role == ArtistRoleAlbumArtist)
        musicInfo.AppendAlbumArtist(artistName);
      else
        musicInfo.AppendArtistRole({ role, artistName });
    }

    if (musicInfo.GetArtistString().empty())
      musicInfo.SetArtist(GetCreator());

    if (musicInfo.GetAlbumArtist().empty())
      musicInfo.SetAlbumArtist(musicInfo.GetArtist());
    else if (musicInfo.GetArtist().empty())
      musicInfo.SetArtist(musicInfo.GetAlbumArtist());

    musicInfo.SetArtistDesc(musicInfo.GetArtistString());
    musicInfo.SetAlbumArtistDesc(musicInfo.GetAlbumArtistString());

    auto genres = GetGenres();
    for (const auto& genre : genres)
      musicInfo.AppendGenre(genre->GetGenre());

    // TODO: producer

    musicInfo.SetLoaded();

    item.SetProperty("album_description", GetLongDescription());
    item.SetLabelPreformated(false);
  }

  return true;
}

bool CUPnPMusicAlbumContainer::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
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

    if (!CUPnPAlbumContainer::FromFileItem(detailedItem, context))
      return false;

    const auto& artists = videoInfo.m_artist;
    std::vector<CUPnPArtist*> artistVec;
    for (const auto& artist : artists)
    {
      artistVec.emplace_back(new CUPnPArtist(artist));
      artistVec.emplace_back(new CUPnPArtist(artist, ArtistRolePerformer));
      artistVec.emplace_back(new CUPnPArtist(artist, ArtistRoleAlbumArtist));
    }
    SetArtists(videoInfo.m_artist);

    SetGenres(videoInfo.m_genre);
    SetProducers(videoInfo.m_studio);
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

    if (!CUPnPAlbumContainer::FromFileItem(detailedItem, context))
      return false;

    const auto& albumArtists = musicInfo.GetAlbumArtist();
    if (!albumArtists.empty())
      SetCreator(musicInfo.GetAlbumArtistString());
    else
      SetCreator(musicInfo.GetArtistString());

    auto artists = musicInfo.GetArtist();
    if (artists.empty() && !musicInfo.GetArtistString().empty())
      artists.push_back(musicInfo.GetArtistString());

    std::vector<CUPnPArtist*> artistVec;
    for (const auto& artist : artists)
    {
      artistVec.emplace_back(new CUPnPArtist(artist));
      artistVec.emplace_back(new CUPnPArtist(artist, ArtistRolePerformer));
      if (albumArtists.empty())
        artistVec.emplace_back(new CUPnPArtist(artist, ArtistRoleAlbumArtist));
    }

    for (const auto& albumArtist : albumArtists)
      artistVec.emplace_back(new CUPnPArtist(albumArtist, ArtistRoleAlbumArtist));

    SetArtists(artistVec);

    SetGenres(musicInfo.GetGenre());
    SetProducers({ musicInfo.GetArtistStringForRole(ArtistRoleProducer) }); // TODO: get a vector

    SetLongDescription(detailedItem.GetProperty("album_description").asString());
  }

  return true;
}
