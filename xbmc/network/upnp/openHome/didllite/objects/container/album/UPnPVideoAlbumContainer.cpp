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

#include "UPnPVideoAlbumContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPRating.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"
#include "video/VideoDatabase.h"
#include "video/VideoInfoTag.h"
#include "video/VideoThumbLoader.h"

CUPnPVideoAlbumContainer::CUPnPVideoAlbumContainer()
  : CUPnPVideoAlbumContainer("object.container.album.videoAlbum")
{ }

CUPnPVideoAlbumContainer::CUPnPVideoAlbumContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPAlbumContainer(classType, className)
{ }

CUPnPVideoAlbumContainer::CUPnPVideoAlbumContainer(const CUPnPVideoAlbumContainer& videoAlbumContainer)
  : CUPnPAlbumContainer(videoAlbumContainer)
{
  copyPropertyValidity(&videoAlbumContainer);
}

CUPnPVideoAlbumContainer::~CUPnPVideoAlbumContainer()
{ }

bool CUPnPVideoAlbumContainer::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPContainer::CanHandleFileItem(item) && item.HasVideoInfoTag() &&
    (item.GetVideoInfoTag()->m_type == MediaTypeTvShow || item.GetVideoInfoTag()->m_type == MediaTypeSeason);
}

bool CUPnPVideoAlbumContainer::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPAlbumContainer::ToFileItem(item, context))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  // if a season number is defined it's a season otherwise it's a tvshow
  if (isPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "episodeSeason"))
    videoInfo.m_type = MediaTypeSeason;
  else
    videoInfo.m_type = MediaTypeTvShow;

  // copy the title from CFileItem
  if (!item.m_strTitle.empty())
    videoInfo.m_strTitle = item.m_strTitle;
  else if (!item.GetLabel().empty())
    videoInfo.m_strTitle = item.GetLabel();

  videoInfo.m_strShowTitle = GetSeriesTitle();
  if (videoInfo.m_strShowTitle.empty() && videoInfo.m_type == MediaTypeTvShow)
    videoInfo.m_strShowTitle = videoInfo.m_strTitle;

  if (videoInfo.m_type == MediaTypeSeason)
    videoInfo.m_iSeason = GetEpisodeSeason();

  // premiered and year
  const CDateTime& date = GetDate();
  if (date.IsValid())
    videoInfo.m_premiered = date;
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

  videoInfo.m_iEpisode = GetEpisodeCount();

  if (GetPlaybackCount() >= 0)
    videoInfo.m_playCount = GetPlaybackCount();
  item.SetProperty("totalepisodes", videoInfo.m_iEpisode);
  item.SetProperty("watchedepisodes", videoInfo.m_playCount > 0 ? videoInfo.m_iEpisode : 0);
  item.SetProperty("unwatchedepisodes", videoInfo.m_playCount > 0 ? 0 : videoInfo.m_iEpisode);
  item.SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, videoInfo.m_playCount > 0);

  item.SetLabelPreformated(false);

  return true;
}

bool CUPnPVideoAlbumContainer::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  // create a copy of the item, retrieve additional details and serialize it
  CFileItem detailedItem = item;

  CVideoInfoTag& videoInfo = *detailedItem.GetVideoInfoTag();

  // TODO: very expensive
  {
    CVideoDatabase db;
    if (db.Open())
    {
      if (videoInfo.m_type == MediaTypeSeason)
        db.GetSeasonInfo(videoInfo.m_iDbId, videoInfo);
      else
        db.GetTvShowInfo(videoInfo.GetPath(), videoInfo, videoInfo.m_iDbId, &detailedItem);
    }
  }

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

  if (!videoInfo.m_strTitle.empty())
    SetTitle(videoInfo.m_strTitle);

  SetSeriesTitle(videoInfo.m_strShowTitle);

  if (videoInfo.m_type == MediaTypeSeason)
    SetEpisodeSeason(videoInfo.m_iSeason);

  SetEpisodeCount(videoInfo.m_iEpisode);

  if (videoInfo.m_premiered.IsValid())
    SetDate(videoInfo.m_premiered);
  else if (videoInfo.m_iYear > 0)
    SetDate(CDateTime(videoInfo.m_iYear, 1, 1, 0, 0, 0));

  SetLongDescription(videoInfo.m_strPlot);
  SetDescription(videoInfo.m_strTagLine);
  SetPlaybackCount(videoInfo.m_playCount);
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

  return true;
}
