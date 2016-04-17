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

#include <algorithm>

#include "UPnPAudioItem.h"
#include "FileItem.h"
#include "music/MusicThumbLoader.h"
#include "music/tags/MusicInfoTag.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPResource.h"
#include "utils/Mime.h"
#include "utils/URIUtils.h"
/* TODO
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"
*/

static const std::string ArtistRolePerformer = "Performer";
static const std::string ArtistRoleAlbumArtist = "AlbumArtist";
static const std::string ArtistRoleProducer = "Producer";

static void FillResource(const CFileItem& item, const OhUPnPRootDeviceContext& context, CUPnPResource* resource, const std::string& protocol = "")
{
  if (resource == nullptr)
    return;

  // set the size
  if (item.m_dwSize > 0)
    resource->SetSize(item.m_dwSize);

  const auto& musicDetails = *item.GetMusicInfoTag();

  // set the duration
  resource->SetDuration(musicDetails.GetDuration());

  // set the protocol
  auto protocolInfo = resource->GetProtocolInfo();
  if (!protocol.empty())
    protocolInfo.SetProtocol(protocol);
  else
    protocolInfo.SetProtocol("http-get");

  // set the MIME type
  protocolInfo.SetContentType(context.profile.GetMimeType(URIUtils::GetExtension(musicDetails.GetURL())));

  // set extra information
  protocolInfo.GetExtras().SetOperation(static_cast<CUPnPResource::CProtocolInfo::DLNA::Operation>(
    static_cast<int>(CUPnPResource::CProtocolInfo::DLNA::Operation::Range) |
    static_cast<int>(CUPnPResource::CProtocolInfo::DLNA::Operation::TimeSeek)));
  protocolInfo.GetExtras().SetConversionIndicator(CUPnPResource::CProtocolInfo::DLNA::ConversionIndicator::None);
  protocolInfo.GetExtras().SetPlaySpeed(CUPnPResource::CProtocolInfo::DLNA::PlaySpeed::Normal);
  protocolInfo.GetExtras().SetFlags(CUPnPResource::CProtocolInfo::DLNA::Flags::ByteBasedSeek);

  resource->SetProtocolInfo(protocolInfo);
}

CUPnPAudioItem::CUPnPAudioItem()
  : CUPnPAudioItem("object.item.audioItem")
{ }

CUPnPAudioItem::CUPnPAudioItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPItem(classType, className)
{ }

CUPnPAudioItem::CUPnPAudioItem(const CUPnPAudioItem& audioItem)
  : CUPnPItem(audioItem)
{
  copyPropertyValidity(&audioItem);
}

CUPnPAudioItem::~CUPnPAudioItem()
{ }

bool CUPnPAudioItem::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPItem::CanHandleFileItem(item) && item.HasMusicInfoTag();
}

bool CUPnPAudioItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPItem::ToFileItem(item, context))
    return false;

  MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

  // year
  const CDateTime& date = GetDate();
  if (date.IsValid())
    musicInfo.SetYear(date.GetYear());

  auto genres = GetGenres();
  for (const auto& genre : genres)
    musicInfo.AppendGenre(genre->GetGenre());

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

  const auto& albums = GetAlbums();
  if (!albums.empty())
    musicInfo.SetAlbum(albums.front());

  musicInfo.SetLastPlayed(GetLastPlaybackTime());
  musicInfo.SetPlayCount(GetPlaybackCount());

  // copy the title from CFileItem
  if (!item.m_strTitle.empty())
    musicInfo.SetTitle(item.m_strTitle);
  else if (!item.GetLabel().empty())
    musicInfo.SetTitle(item.GetLabel());

  auto resources = GetResources();
  const auto& resourceIt = std::find_if(resources.cbegin(), resources.cend(), CUPnPResourceFinder::ByProtocolAndContentType("http-get", "audio"));
  if (resourceIt != resources.cend())
  {
    const auto& resource = *resourceIt;
    item.SetMimeType(resource->GetProtocolInfo().GetContentType());
    item.m_dwSize = static_cast<int64_t>(resource->GetSize());

    // handle the duration
    double duration = resource->GetDuration();
    if (duration > 0.0)
      musicInfo.SetDuration(static_cast<int>(std::ceil(duration)));
  }

  musicInfo.SetLoaded();

  item.SetLabelPreformated(false);

  return true;
}

bool CUPnPAudioItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  // create a copy of the item, retrieve additional details and serialize it
  CFileItem detailedItem = item;

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

  if (!CUPnPItem::FromFileItem(detailedItem, context))
    return false;

  const MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();

  if (!musicInfo.GetTitle().empty())
    SetTitle(musicInfo.GetTitle());

  // year
  if (musicInfo.GetYear() > 0)
    SetDate(CDateTime(musicInfo.GetYear(), 1, 1, 0, 0, 0));

  SetGenres(musicInfo.GetGenre());
  SetAlbums({ musicInfo.GetAlbum() });

  const auto& albumArtists = musicInfo.GetAlbumArtist();
  if (!albumArtists.empty())
    SetCreator(musicInfo.GetAlbumArtistString());
  else
    SetCreator(musicInfo.GetArtistString());

  const auto& artists = musicInfo.GetArtist();
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

  SetLastPlaybackTime(musicInfo.GetLastPlayed());
  SetPlaybackCount(musicInfo.GetPlayCount());

  // add the item as a resource through the resource manager
  CUPnPResource* resource = new CUPnPResource(COhUPnPResourceManager::GetFullResourceUri(context.resourceUriPrefix, musicInfo.GetURL()));
  FillResource(detailedItem, context, resource);
  AddResource(resource);

  // if the item is on a remote server also add the direct URL
  if (URIUtils::IsRemote(musicInfo.GetURL()))
  {
    CUPnPResource* directResource = new CUPnPResource(musicInfo.GetURL());
    FillResource(detailedItem, context, directResource, "xbmc-get");
    AddResource(directResource);
  }

  return true;
}
