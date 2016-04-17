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

#include <algorithm>

#include "UPnPVideoItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPRating.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPResource.h"
#include "settings/AdvancedSettings.h"
#include "utils/Mime.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoInfoTag.h"
#include "video/VideoThumbLoader.h"

static void FillResource(const CFileItem& item, const OhUPnPRootDeviceContext& context, CUPnPResource* resource, const std::string& protocol = "")
{
  if (resource == nullptr)
    return;

  // set the size
  if (item.m_dwSize > 0)
    resource->SetSize(item.m_dwSize);

  const auto& videoDetails = *item.GetVideoInfoTag();

  // set the duration (might be overwritten by stream details)
  resource->SetDuration(videoDetails.m_duration);

  // set the protocol
  auto protocolInfo = resource->GetProtocolInfo();
  if (!protocol.empty())
    protocolInfo.SetProtocol(protocol);
  else
    protocolInfo.SetProtocol("http-get");

  // set the MIME type
  protocolInfo.SetContentType(context.profile.GetMimeType(URIUtils::GetExtension(videoDetails.GetPath())));

  // set extra information
  protocolInfo.GetExtras().SetOperation(static_cast<CUPnPResource::CProtocolInfo::DLNA::Operation>(
    static_cast<int>(CUPnPResource::CProtocolInfo::DLNA::Operation::Range) |
    static_cast<int>(CUPnPResource::CProtocolInfo::DLNA::Operation::TimeSeek)));
  protocolInfo.GetExtras().SetConversionIndicator(CUPnPResource::CProtocolInfo::DLNA::ConversionIndicator::None);
  protocolInfo.GetExtras().SetPlaySpeed(CUPnPResource::CProtocolInfo::DLNA::PlaySpeed::Normal);
  protocolInfo.GetExtras().SetFlags(CUPnPResource::CProtocolInfo::DLNA::Flags::ByteBasedSeek);

  // set details based on available streams
  if (videoDetails.HasStreamDetails())
  {
    const auto& streamDetails = videoDetails.m_streamDetails;

    if (streamDetails.GetVideoDuration() > 0)
      resource->SetDuration(streamDetails.GetVideoDuration());

    resource->SetNumberOfAudioChannels(streamDetails.GetAudioChannels());

    auto resolution = resource->GetResolution();
    resolution.SetWidth(streamDetails.GetVideoWidth());
    resolution.SetHeight(streamDetails.GetVideoHeight());
    resource->SetResolution(resolution);
  }

  resource->SetProtocolInfo(protocolInfo);
}

CUPnPVideoItem::CUPnPVideoItem()
  : CUPnPVideoItem("object.item.videoItem")
{ }

CUPnPVideoItem::CUPnPVideoItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPItem(classType, className)
{ }

CUPnPVideoItem::CUPnPVideoItem(const CUPnPVideoItem& videoItem)
  : CUPnPItem(videoItem)
{
  copyPropertyValidity(&videoItem);
}

CUPnPVideoItem::~CUPnPVideoItem()
{ }

bool CUPnPVideoItem::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPItem::CanHandleFileItem(item) && item.HasVideoInfoTag();
}

bool CUPnPVideoItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPItem::ToFileItem(item, context))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();

  // year
  const CDateTime& date = GetDate();
  if (date.IsValid())
    videoInfo.m_iYear = date.GetYear();

  // genre
  auto genres = GetGenres();
  for (const auto& genre : genres)
    videoInfo.m_genre.push_back(genre->GetGenre());

  // plot and tagline
  videoInfo.m_strPlot = GetLongDescription();
  if (videoInfo.m_strPlot.empty())
    videoInfo.m_strPlot = GetDescription();
  videoInfo.m_strTagLine = GetDescription();

  // MPAA rating
  auto ratings = GetRatings();
  if (!ratings.empty())
  {
    std::vector<std::string> strRatings;
    for (const auto& rating : ratings)
      strRatings.push_back(rating->GetRating());

    videoInfo.m_strMPAARating = StringUtils::Join(strRatings, g_advancedSettings.m_videoItemSeparator);
  }

  // actors
  auto actors = GetActors();
  for (const auto& actor : actors)
  {
    SActorInfo actorInfo;
    actorInfo.strName = actor->GetPerson();
    actorInfo.strRole = actor->GetPersonRole();
    videoInfo.m_cast.push_back(actorInfo);
  }

  // authors
  auto authors = GetAuthors();
  for (const auto& author : authors)
    videoInfo.m_writingCredits.push_back(author->GetPerson());

  videoInfo.m_director = GetDirectors();
  videoInfo.m_studio = GetPublishers();

  if (GetPlaybackCount() >= 0)
    videoInfo.m_playCount = GetPlaybackCount();
  item.SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, videoInfo.m_playCount > 0);

  videoInfo.m_lastPlayed = GetLastPlaybackTime();
  videoInfo.m_resumePoint.timeInSeconds = static_cast<double>(GetLastPlaybackPosition());

  // copy the title from CFileItem
  if (!item.m_strTitle.empty())
    videoInfo.m_strTitle = item.m_strTitle;
  else if (!item.GetLabel().empty())
    videoInfo.m_strTitle = item.GetLabel();

  auto resources = GetResources();
  const auto& resourceIt = std::find_if(resources.cbegin(), resources.cend(), CUPnPResourceFinder::ByProtocolAndContentType("http-get", "video"));
  if (resourceIt != resources.cend())
  {
    const auto& resource = *resourceIt;
    item.SetMimeType(resource->GetProtocolInfo().GetContentType());
    item.m_dwSize = static_cast<int64_t>(resource->GetSize());

    CStreamDetailVideo* videoStream = new CStreamDetailVideo();
    // TODO: videoStream->m_strCodec = resource->GetProtocolInfo().GetContentType();

    // handle the duration
    double duration = resource->GetDuration();
    if (duration > 0.0)
    {
      videoInfo.m_duration = static_cast<int>(std::ceil(duration));
      videoStream->m_iDuration = videoInfo.m_duration;

      // complete the resume point state
      if (videoInfo.m_resumePoint.timeInSeconds > 0.0)
        videoInfo.m_resumePoint.totalTimeInSeconds = duration;
    }

    // handle video stream details
    const auto& resolution = resource->GetResolution();
    if (resolution.IsValid())
    {
      videoStream->m_iWidth = static_cast<int>(resolution.GetWidth());
      videoStream->m_iHeight = static_cast<int>(resolution.GetHeight());
    }
    videoInfo.m_streamDetails.AddStream(videoStream);

    // handle audio stream details
    if (resource->GetNumberOfAudioChannels() > 0)
    {
      CStreamDetailAudio* detail = new CStreamDetailAudio();
      detail->m_iChannels = resource->GetNumberOfAudioChannels();
      videoInfo.m_streamDetails.AddStream(detail);
    }
  }

  item.SetLabelPreformated(false);

  return true;
}

bool CUPnPVideoItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  // create a copy of the item, retrieve additional details and serialize it
  CFileItem detailedItem = item;

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

  if (!CUPnPItem::FromFileItem(detailedItem, context))
    return false;

  const CVideoInfoTag& videoInfo = *detailedItem.GetVideoInfoTag();

  if (!videoInfo.m_strTitle.empty())
    SetTitle(videoInfo.m_strTitle);

  // year
  if (videoInfo.m_iYear > 0)
    SetDate(CDateTime(videoInfo.m_iYear, 1, 1, 0, 0, 0));

  SetLongDescription(videoInfo.m_strPlot);
  SetDescription(videoInfo.m_strTagLine);
  SetPlaybackCount(videoInfo.m_playCount);
  SetLastPlaybackTime(videoInfo.m_lastPlayed);
  SetLastPlaybackPosition(static_cast<int64_t>(videoInfo.m_resumePoint.timeInSeconds));
  SetGenres(videoInfo.m_genre);
  SetDirectors(videoInfo.m_director);
  SetPublishers(videoInfo.m_studio);
  SetAuthors(videoInfo.m_writingCredits);

  // actors
  std::vector<CUPnPActor*> actors;
  for (const auto& actor : videoInfo.m_cast)
    actors.push_back(new CUPnPActor(actor.strName, actor.strRole));
  SetActors(actors);

  // MPAA rating
  std::vector<std::string> mpaaRatings = StringUtils::Split(videoInfo.m_strMPAARating, g_advancedSettings.m_videoItemSeparator);
  std::vector<CUPnPRating*> ratings;
  for (const auto& mpaaRating : mpaaRatings)
    ratings.push_back(new CUPnPRating(mpaaRating));
  SetRatings(ratings);

  // add the item as a resource through the resource manager
  CUPnPResource* resource = new CUPnPResource(COhUPnPResourceManager::GetFullResourceUri(context.resourceUriPrefix, videoInfo.GetPath()));
  FillResource(detailedItem, context, resource);
  AddResource(resource);

  // if the item is on a remote server also add the direct URL
  if (URIUtils::IsRemote(videoInfo.GetPath()))
  {
    CUPnPResource* directResource = new CUPnPResource(videoInfo.GetPath());
    FillResource(detailedItem, context, directResource, "xbmc-get");
    AddResource(directResource);
  }

  return true;
}
