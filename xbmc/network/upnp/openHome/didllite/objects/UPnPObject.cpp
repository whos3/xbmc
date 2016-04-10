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

#include "UPnPObject.h"
#include "FileItem.h"
#include "TextureDatabase.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPAlbumArt.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPRating.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPResource.h"
#include "network/upnp/openHome/utils/DlnaUtils.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

CUPnPObject::CUPnPObject(const std::string& elementName, const std::string& classType, const std::string& className /* = "" */)
  : IFileItemElement(elementName),
    m_parentID("0"),
    m_class(classType, className),
    m_restricted(true),
    m_writeStatus("UNKNOWN"),
    m_playbackCount(0),
    m_lastPlaybackTime("lastPlaybackTime"),
    m_recordedStartDateTime("recordedStartDateTime"),
    m_recordedEndDateTime("recordedEndDateTime"),
    m_recordable(false),
    m_episodeCount(0),
    m_episodeNumber(-1),
    m_episodeSeason(-1),
    m_payPerView(false),
    m_channelNr(0),
    m_scheduledEndTime("scheduledEndTime"),
    m_signalStrength(-1),
    m_signalLocked(false),
    m_tuned(false),
    m_neverPlayable(true),
    m_DVDRegionCode(0),
    m_originalTrackNumber(0),
    m_objectUpdateID(0)
{
  initializeProperties();
}

CUPnPObject::CUPnPObject(const CUPnPObject& obj)
  : IFileItemElement(obj),
    m_id(obj.m_id),
    m_parentID(obj.m_parentID),
    m_restricted(obj.m_restricted),
    m_class(obj.m_class),
    m_title(obj.m_title),
    m_creator(obj.m_creator),
    m_writeStatus(obj.m_writeStatus),
    m_producer(obj.m_producer),
    m_director(obj.m_director),
    m_publisher(obj.m_publisher),
    m_contributor(obj.m_contributor),
    m_album(obj.m_album),
    m_playlist(obj.m_playlist),
    m_albumArtURI(obj.m_albumArtURI),
    m_artistDiscographyURI(obj.m_artistDiscographyURI),
    m_lyricsURI(obj.m_lyricsURI),
    m_relation(obj.m_relation),
    m_storageMedium(obj.m_storageMedium),
    m_description(obj.m_description),
    m_longDescription(obj.m_longDescription),
    m_icon(obj.m_icon),
    m_region(obj.m_region),
    m_rights(obj.m_rights),
    m_date(obj.m_date),
    m_language(obj.m_language),
    m_playbackCount(obj.m_playbackCount),
    m_lastPlaybackTime(obj.m_lastPlaybackTime),
    m_lastPlaybackPosition(obj.m_lastPlaybackPosition),
    m_recordedStartDateTime(obj.m_recordedStartDateTime),
    m_recordedEndDateTime(obj.m_recordedEndDateTime),
    m_recordedDuration(obj.m_recordedDuration),
    m_recordedDayOfWeek(obj.m_recordedDayOfWeek),
    m_srsRecordScheduleID(obj.m_srsRecordScheduleID),
    m_srsRecordTaskID(obj.m_srsRecordTaskID),
    m_recordable(obj.m_recordable),
    m_programTitle(obj.m_programTitle),
    m_seriesTitle(obj.m_seriesTitle),
    // TODO: upnp:programID, @type
    // TODO: upnp:seriesID, @type
    // TODO: upnp:channelID, @type, @distriNetworkName, @distriNetworkID
    m_episodeType(obj.m_episodeType),
    m_episodeCount(obj.m_episodeCount),
    m_episodeNumber(obj.m_episodeNumber),
    m_episodeSeason(obj.m_episodeSeason),
    // TODO: upnp:programCode, @type
    // TODO: upnp:channelGroupName, @id
    m_callSign(obj.m_callSign),
    m_networkAffiliation(obj.m_networkAffiliation),
    m_serviceProvider(obj.m_serviceProvider),
    m_price(obj.m_price),
    m_payPerView(obj.m_payPerView),
    m_epgProviderName(obj.m_epgProviderName),
    // TODO: upnp:dateTimeRange, @daylightSaving
    // TODO: upnp:programPreserved, @startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
    // TODO: upnp:preservedTimeRange, e@startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
    // TODO: upnp:programList, ::program, ::program@preserved
    m_radioCallSign(obj.m_radioCallSign),
    m_radioStationID(obj.m_radioStationID),
    m_radioBand(obj.m_radioBand),
    m_channelNr(obj.m_channelNr),
    m_channelName(obj.m_channelName),
    // TODO: upnp:scheduledStartTime, @usage, @daylightSaving
    m_scheduledEndTime(obj.m_scheduledEndTime),
    m_scheduledDuration(obj.m_scheduledDuration),
    m_signalStrength(obj.m_signalStrength),
    m_signalLocked(obj.m_signalLocked),
    m_tuned(obj.m_tuned),
    m_neverPlayable(obj.m_neverPlayable),
    m_segmentIDs(obj.m_segmentIDs),
    m_bookmarkIDs(obj.m_bookmarkIDs),
    m_bookmarkedObjectID(obj.m_bookmarkedObjectID),
    // TODO: upnp:deviceUDN, @serviceType, @serviceID
    // TODO: upnp:stateVariableCollection, @serviceName, @rcsInstanceType, ::stateVariable, ::stateVariable@variableName
    m_DVDRegionCode(obj.m_DVDRegionCode),
    m_originalTrackNumber(obj.m_originalTrackNumber),
    m_toc(obj.m_toc),
    m_userAnnotation(obj.m_userAnnotation),
    m_objectUpdateID(obj.m_objectUpdateID)
    // TODO: upnp:inclusionControl, ::role
    // TODO: upnp:objectOwner, @lock, ::role
    // TODO: upnp:objectLink, @groupID, @headObjID, @nextObjID, @prevObjID, ::title, ::startObject, ::mode, ::relatedInfo, ..., ::startInfo, ..., @endAction, @endAction@action, ::endAction, ...
    // TODO: upnp:objectLinkRef, ....
    // TODO: upnp:foreignMetadata, ...
{
  copyElementProperty(m_res, obj.m_res);
  // TODO: resExt
  copyElementProperty(m_artist, obj.m_artist);
  copyElementProperty(m_actor, obj.m_actor);
  copyElementProperty(m_author, obj.m_author);
  copyElementProperty(m_genre, obj.m_genre);
  copyElementProperty(m_rating, obj.m_rating);

  initializeProperties();
  copyPropertyValidity(&obj);
}

CUPnPObject::~CUPnPObject()
{
  clearElementProperty(m_res);
  clearElementProperty(m_artist);
  clearElementProperty(m_actor);
  clearElementProperty(m_author);
  clearElementProperty(m_genre);
  clearElementProperty(m_rating);
}

bool CUPnPObject::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (m_id.empty())
    return false;

  item.Reset();

  item.SetPath(m_id);
  // TODO: m_parentID
  item.SetLabel(m_title);
  item.SetLabelPreformated(true);
  item.m_strTitle = m_title;
  item.m_dateTime = GetDate();
  // TODO: m_res

  item.m_bIsFolder = StringUtils::StartsWith(m_class.GetType(), "object.container");

  if (!m_albumArtURI.empty())
    item.SetArt("thumb", m_albumArtURI.at(0)->GetUrl());
  else if (!m_icon.empty())
    item.SetArt("thumb", m_icon);

  return true;
}

bool CUPnPObject::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (item.GetPath().empty())
    return false;

  SetId(item.GetPath());
  SetParentId(URIUtils::GetParentPath(item.GetPath())); // TODO: properly handle root items
  SetTitle(item.GetLabel());

  // icon
  const std::string& thumbArt = item.GetArt("thumb");
  if (!thumbArt.empty())
  {
    const std::string thumbArtUrl = COhUPnPResourceManager::GetFullResourceUri(context.resourceUriPrefix, CTextureUtils::GetWrappedImageURL(thumbArt));

    SetIcon(thumbArtUrl);
    SetAlbumArt({ new CUPnPAlbumArt(thumbArtUrl, CDlnaUtils::GetProfileID(thumbArt)) });
  }

  return true;
}

void CUPnPObject::SetId(const std::string& id)
{
  m_id = id;
  setPropertyValid("@id");
}

void CUPnPObject::SetParentId(const std::string& parentId)
{
  if (!parentId.empty())
    m_parentID = parentId;
  else
    m_parentID = "0";
}

void CUPnPObject::SetRestricted(bool restricted)
{
  m_restricted = restricted;
}

void CUPnPObject::SetTitle(const std::string& title)
{
  m_title = title;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "title", !m_title.empty());
}

void CUPnPObject::SetCreator(const std::string& creator)
{
  m_creator = creator;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "creator", !m_creator.empty());
}

void CUPnPObject::SetWriteStatus(const std::string& writeStatus)
{
  m_writeStatus = writeStatus;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "writeStatus", !m_writeStatus.empty() && m_writeStatus != "UNKNOWN");
}

void CUPnPObject::SetResources(const std::vector<CUPnPResource*>& resources)
{
  setAndValidateMultiValueProperty(m_res, resources, "res");
}

void CUPnPObject::AddResource(CUPnPResource* resource)
{
  if (resource == nullptr)
    return;

  m_res.push_back(resource);

  setPropertyValid("res");
}

void CUPnPObject::SetArtists(const std::vector<CUPnPArtist*>& artists)
{
  setAndValidateMultiValueProperty(m_artist, artists, UPNP_DIDL_UPNP_NAMESPACE, "artist");
}

void CUPnPObject::SetArtists(const std::vector<std::string>& artists)
{
  setAndValidateMultiValueProperty(m_artist, artists, UPNP_DIDL_UPNP_NAMESPACE, "artist");
}

void CUPnPObject::SetActors(const std::vector<CUPnPActor*>& actors)
{
  setAndValidateMultiValueProperty(m_actor, actors, UPNP_DIDL_UPNP_NAMESPACE, "actor");
}

void CUPnPObject::SetAuthors(const std::vector<CUPnPAuthor*>& authors)
{
  setAndValidateMultiValueProperty(m_author, authors, UPNP_DIDL_UPNP_NAMESPACE, "author");
}

void CUPnPObject::SetAuthors(const std::vector<std::string>& authors)
{
  setAndValidateMultiValueProperty(m_author, authors, UPNP_DIDL_UPNP_NAMESPACE, "author");
}

void CUPnPObject::SetProducers(const std::vector<std::string>& producers)
{
  m_producer = producers;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "producer", !m_producer.empty());
}

void CUPnPObject::SetDirectors(const std::vector<std::string>& directors)
{
  m_director = directors;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "director", !m_director.empty());
}

void CUPnPObject::SetPublishers(const std::vector<std::string>& publishers)
{
  m_publisher = publishers;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "publisher", !m_publisher.empty());
}

void CUPnPObject::SetContributors(const std::vector<std::string>& contributors)
{
  m_contributor = contributors;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "contributor", !m_contributor.empty());
}

void CUPnPObject::SetGenres(const std::vector<CUPnPGenre*>& genres)
{
  setAndValidateMultiValueProperty(m_genre, genres, UPNP_DIDL_UPNP_NAMESPACE, "genre");
}

void CUPnPObject::SetGenres(const std::vector<std::string>& genres)
{
  setAndValidateMultiValueProperty(m_genre, genres, UPNP_DIDL_UPNP_NAMESPACE, "genre");
}

void CUPnPObject::SetAlbums(const std::vector<std::string>& albums)
{
  m_album = albums;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "album", !m_album.empty());
}

void CUPnPObject::SetPlaylists(const std::vector<std::string>& playlists)
{
  m_playlist = playlists;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "playlist", !m_playlist.empty());
}

void CUPnPObject::SetAlbumArt(const std::vector<CUPnPAlbumArt*>& albumArt)
{
  setAndValidateMultiValueProperty(m_albumArtURI, albumArt, UPNP_DIDL_UPNP_NAMESPACE, "albumArtURI");
}

void CUPnPObject::SetAlbumArt(const std::vector<std::string>& albumArt)
{
  setAndValidateMultiValueProperty(m_albumArtURI, albumArt, UPNP_DIDL_UPNP_NAMESPACE, "albumArtURI");
}

void CUPnPObject::SetArtistDiscographyURI(const std::string& artistDiscographyURI)
{
  m_artistDiscographyURI = artistDiscographyURI;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "artistDiscographyURI", !m_artistDiscographyURI.empty());
}

void CUPnPObject::SetLyricsURI(const std::string& lyricsURI)
{
  m_lyricsURI = lyricsURI;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "lyricsURI", !m_lyricsURI.empty());
}

void CUPnPObject::SetRelations(const std::vector<std::string>& relations)
{
  m_relation = relations;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "relation", !m_relation.empty());
}

void CUPnPObject::SetStorageMedium(const std::string& storageMedium)
{
  m_storageMedium = storageMedium;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "storageMedium", !m_storageMedium.empty());
}

void CUPnPObject::SetDescription(const std::string& description)
{
  m_description = description;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "description", !m_description.empty());
}

void CUPnPObject::SetLongDescription(const std::string& longDescription)
{
  m_longDescription = longDescription;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "longDescription", !m_longDescription.empty());
}

void CUPnPObject::SetIcon(const std::string& icon)
{
  m_icon = icon;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "icon", !m_icon.empty());
}

void CUPnPObject::SetRegions(const std::vector<std::string>& regions)
{
  m_region = regions;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "region", !m_region.empty());
}

void CUPnPObject::SetRights(const std::vector<std::string>& rights)
{
  m_rights = rights;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "rights", !m_rights.empty());
}

void CUPnPObject::SetDate(const CDateTime& date)
{
  m_date.SetDate(date);
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "date", m_date.GetDate().IsValid());
}

void CUPnPObject::SetLanguages(const std::vector<std::string>& languages)
{
  m_language = languages;
  setPropertyValidity(UPNP_DIDL_DC_NAMESPACE, "language", !m_language.empty());
}

void CUPnPObject::SetPlaybackCount(int32_t playbackCount)
{
  m_playbackCount = playbackCount;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "playbackCount", m_playbackCount >= 0);
}

void CUPnPObject::SetLastPlaybackTime(const CDateTime& lastPlaybackTime)
{
  m_lastPlaybackTime.SetDateTime(lastPlaybackTime);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackTime", m_lastPlaybackTime.GetDateTime().IsValid());
}

void CUPnPObject::SetRecordedStartDateTime(const CDateTime& recordedStartDateTime)
{
  m_recordedStartDateTime.SetDateTime(recordedStartDateTime);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "recordedStartDateTime", m_recordedStartDateTime.GetDateTime().IsValid());
}

void CUPnPObject::SetRecordedEndDateTime(const CDateTime& recordedEndDateTime)
{
  m_recordedEndDateTime.SetDateTime(recordedEndDateTime);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "recordedEndDateTime", m_recordedEndDateTime.GetDateTime().IsValid());
}

void CUPnPObject::SetRecordedDuration(int64_t recordedDuration)
{
  m_recordedDuration = DidlLiteUtils::GetDurationFromSeconds(recordedDuration);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "recordedDuration", recordedDuration > 0);
}

void CUPnPObject::SetRecordedDayOfWeek(const std::string& recordedDayOfWeek)
{
  m_recordedDayOfWeek = recordedDayOfWeek;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "recordedDayOfWeek", !m_recordedDayOfWeek.empty());
}

void CUPnPObject::SetRecordedDayOfWeek(UPnPDayOfWeek recordedDayOfWeek)
{
  std::string recordedDayOfWeekStr;
  if (!UPnPEnums::DayOfWeekToString(recordedDayOfWeek, recordedDayOfWeekStr))
    recordedDayOfWeekStr.clear();

  SetRecordedDayOfWeek(recordedDayOfWeekStr);
}

void CUPnPObject::SetSrsRecordScheduleID(const std::string& srsRecordScheduleID)
{
  m_srsRecordScheduleID = srsRecordScheduleID;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "srsRecordScheduleID", !m_srsRecordScheduleID.empty());
}

void CUPnPObject::SetLastPlaybackPosition(int64_t lastPlaybackPosition)
{
  m_lastPlaybackPosition = DidlLiteUtils::GetDurationFromSeconds(lastPlaybackPosition);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackPosition", lastPlaybackPosition > 0);
}

void CUPnPObject::SetRecordable(bool recordable)
{
  m_recordable = recordable;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "recordable");
}

void CUPnPObject::SetProgramTitle(const std::string& programTitle)
{
  m_programTitle = programTitle;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "programTitle", !m_programTitle.empty());
}

void CUPnPObject::SetSeriesTitle(const std::string& seriesTitle)
{
  m_seriesTitle = seriesTitle;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "seriesTitle", !m_seriesTitle.empty());
}

void CUPnPObject::SetEpisodeType(const std::string& episodeType)
{
  m_episodeType = episodeType;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "episodeType", !m_episodeType.empty());
}

void CUPnPObject::SetEpisodeType(UPnPRecordedEpisodeType episodeType)
{
  std::string episodeTypeStr;
  if (!UPnPEnums::RecordedEpisodeTypeToString(episodeType, episodeTypeStr))
    episodeTypeStr.clear();

  SetEpisodeType(episodeTypeStr);
}

void CUPnPObject::SetEpisodeCount(uint32_t episodeCount)
{
  m_episodeCount = episodeCount;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "episodeCount");
}

void CUPnPObject::SetEpisodeNumber(uint32_t episodeNumber)
{
  m_episodeNumber = episodeNumber;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "episodeNumber");
}

void CUPnPObject::SetEpisodeSeason(uint32_t episodeSeason)
{
  m_episodeSeason = episodeSeason;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "episodeSeason");
}

void CUPnPObject::SetRatings(const std::vector<CUPnPRating*>& ratings)
{
  setAndValidateMultiValueProperty(m_rating, ratings, UPNP_DIDL_UPNP_NAMESPACE, "rating");
}

void CUPnPObject::SetRatings(const std::vector<std::string>& ratings)
{
  setAndValidateMultiValueProperty(m_rating, ratings, UPNP_DIDL_UPNP_NAMESPACE, "rating");
}

void CUPnPObject::SetCallSign(const std::string& callSign)
{
  m_callSign = callSign;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "callSign", !m_callSign.empty());
}

void CUPnPObject::SetNetworkAffiliation(const std::string& networkAffiliation)
{
  m_networkAffiliation = networkAffiliation;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "networkAffiliation", !m_networkAffiliation.empty());
}

void CUPnPObject::SetServiceProvider(const std::string& serviceProvider)
{
  m_serviceProvider = serviceProvider;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "serviceProvider", !m_serviceProvider.empty());
}

void CUPnPObject::SetPrice(float price)
{
  m_price.SetPrice(price);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "price", m_price.GetPrice() > 0.0f);
}

void CUPnPObject::SetPrice(float price, const std::string& currency)
{
  m_price.SetPrice(price);
  m_price.SetCurrency(currency);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "price", m_price.GetPrice() > 0.0f);
}

void CUPnPObject::SetPayPerView(bool payPerView)
{
  m_payPerView = payPerView;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "payPerView");
}

void CUPnPObject::SetEpgProviderName(const std::string& epgProviderName)
{
  m_epgProviderName = epgProviderName;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "epgProviderName", !m_epgProviderName.empty());
}

void CUPnPObject::SetRadioCallSign(const std::string& radioCallSign)
{
  m_radioCallSign = radioCallSign;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "radioCallSign", !m_radioCallSign.empty());
}

void CUPnPObject::SetRadioStationID(const std::string& radioStationID)
{
  m_radioStationID = radioStationID;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "radioStationID", !m_radioStationID.empty());
}

void CUPnPObject::SetRadioBand(const std::string& radioBand)
{
  m_radioBand = radioBand;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "radioBand", !m_radioBand.empty());
}

void CUPnPObject::SetRadioBand(UPnPRadioBand radioBand)
{
  std::string radioBandStr;
  if (!UPnPEnums::RadioBandToString(radioBand, radioBandStr))
    radioBandStr.clear();

  SetRadioBand(radioBandStr);
}

void CUPnPObject::SetChannelNr(int32_t channelNr)
{
  m_channelNr = channelNr;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "channelNr", channelNr > 0);
}

void CUPnPObject::SetChannelName(const std::string& channelName)
{
  m_channelName = channelName;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "channelName", !m_channelName.empty());
}

void CUPnPObject::SetScheduledEndTime(const CDateTime& scheduledEndTime)
{
  m_scheduledEndTime.SetDateTime(scheduledEndTime);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "scheduledEndTime", m_scheduledEndTime.GetDateTime().IsValid());
}

void CUPnPObject::SetScheduledDuration(int64_t scheduledDuration)
{
  m_scheduledDuration = DidlLiteUtils::GetDurationFromSeconds(scheduledDuration);
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "scheduledDuration", scheduledDuration > 0);
}

void CUPnPObject::SetSignalStrength(int32_t signalStrength)
{
  if (signalStrength < -1)
    signalStrength = -1;
  else if (signalStrength > 100)
    signalStrength = 100;

  m_signalStrength = signalStrength;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "signalStrength", m_signalStrength > -1);
}

void CUPnPObject::SetSignalLocked(bool signalLocked)
{
  m_signalLocked = signalLocked;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "signalLocked");
}

void CUPnPObject::SetTuned(bool tuned)
{
  m_tuned = tuned;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "tuned");
}

void CUPnPObject::SetNeverPlayable(bool neverPlayable)
{
  m_neverPlayable = neverPlayable;
  setPropertyValid("@neverPlayable");
}

void CUPnPObject::SetSegmentIDs(const std::vector<std::string>& segmentIDs)
{
  m_segmentIDs = segmentIDs;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "segmentID", !m_segmentIDs.empty());
}

void CUPnPObject::SetBookmarkIDs(const std::vector<std::string>& bookmarkIDs)
{
  m_bookmarkIDs = bookmarkIDs;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "bookmarkID", !m_bookmarkIDs.empty());
}

void CUPnPObject::SetBookmarkedObjectID(const std::string& bookmarkedObjectID)
{
  m_bookmarkedObjectID = bookmarkedObjectID;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "bookmarkedObjectID", !m_bookmarkedObjectID.empty());
}

void CUPnPObject::SetDVDRegionCode(int32_t DVDRegionCode)
{
  m_DVDRegionCode = DVDRegionCode;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "DVDRegionCode");
}

void CUPnPObject::SetOriginalTrackNumber(int32_t originalTrackNumber)
{
  m_originalTrackNumber = originalTrackNumber;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "originalTrackNumber", m_originalTrackNumber > 0);
}

void CUPnPObject::SetTableOfContents(const std::string& toc)
{
  m_toc = toc;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "toc", !m_toc.empty());
}

void CUPnPObject::SetUserAnnotation(const std::string& userAnnotation)
{
  m_userAnnotation = userAnnotation;
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "userAnnotation", !m_userAnnotation.empty());
}

void CUPnPObject::SetObjectUpdateID(uint32_t objectUpdateID)
{
  m_objectUpdateID = objectUpdateID;
  setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "objectUpdateID");
}

void CUPnPObject::initializeProperties()
{
  // define all attributes
  addStringProperty("@id", &m_id).AsAttribute().SetRequired();
  addStringProperty("@parentID", &m_parentID).AsAttribute().SetRequired();
  addBooleanProperty("@restricted", &m_restricted).AsAttribute().SetRequired();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "class", &m_class).SetRequired();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "title", &m_title).SetRequired();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "creator", &m_creator).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "writeStatus", &m_writeStatus).SetOptional();
  addElementProperty("res", &m_res).SetOptional().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPResource>());
  // TODO: resExt
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "artist", &m_artist).SetOptional().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPArtist>());
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "actor", &m_actor).SetOptional().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPActor>());
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "author", &m_actor).SetOptional().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPAuthor>());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "producer", &m_producer).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "director", &m_director).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "publisher", &m_publisher).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "contributor", &m_contributor).SetOptional().SupportMultipleValues();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "genre", &m_genre).SetOptional().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPGenre>());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "album", &m_album).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "playlist", &m_playlist).SetOptional().SupportMultipleValues();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "albumArtURI", &m_albumArtURI).SetOptional().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPAlbumArt>());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "artistDiscographyURI", &m_artistDiscographyURI).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "lyricsURI", &m_lyricsURI).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "relation", &m_relation).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageMedium", &m_storageMedium).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "description", &m_description).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "longDescription", &m_longDescription).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "icon", &m_icon).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "region", &m_region).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "rights", &m_rights).SetOptional().SupportMultipleValues();
  addElementProperty(UPNP_DIDL_DC_NAMESPACE, "date", &m_date).SetOptional().SetGenerator(std::make_shared<CUPnPDate>());
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "language", &m_language).SetOptional().SupportMultipleValues();
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "playbackCount", &m_playbackCount).SetOptional().SetMinimumVersion(2);
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackTime", &m_lastPlaybackTime).SetOptional().SetGenerator(std::make_shared<CUPnPDateTime>("lastPlaybackTime")).SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackPosition", &m_lastPlaybackPosition).SetOptional().SetMinimumVersion(2);
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "recordedStartDateTime", &m_recordedStartDateTime).SetOptional().SetGenerator(std::make_shared<CUPnPDateTime>("recordedStartDateTime")).SetMinimumVersion(2);
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "recordedEndDateTime", &m_recordedEndDateTime).SetOptional().SetGenerator(std::make_shared<CUPnPDateTime>("recordedEndDateTime")).SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "recordedDuration", &m_recordedDuration).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "recordedDayOfWeek", &m_recordedDayOfWeek).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "srsRecordScheduleID", &m_srsRecordScheduleID).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "srsRecordTaskID", &m_srsRecordTaskID).SetOptional().SetMinimumVersion(2);
  addBooleanProperty(UPNP_DIDL_UPNP_NAMESPACE, "recordable", &m_programTitle).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "programTitle", &m_programTitle).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "seriesTitle", &m_seriesTitle).SetOptional().SetMinimumVersion(2);
  // TODO: upnp:programID, @type
  // TODO: upnp:seriesID, @type
  // TODO: upnp:channelID, @type, @distriNetworkName, @distriNetworkID
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "episodeType", &m_episodeType).SetOptional().SetMinimumVersion(2);
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "episodeCount", &m_episodeCount).SetOptional().SetMinimumVersion(2);
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "episodeNumber", &m_episodeNumber).SetOptional().SetMinimumVersion(2);
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "episodeSeason", &m_episodeSeason).SetOptional().SetMinimumVersion(4);
  // TODO: upnp:programCode, @type
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "rating", &m_rating).SetOptional().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPRating>()).SetMinimumVersion(2);
  // TODO: upnp:channelGroupName, @id
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "callSign", &m_callSign).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "networkAffiliation", &m_networkAffiliation).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "serviceProvider", &m_serviceProvider).SetOptional().SetMinimumVersion(2);
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "price", &m_price).SetOptional().SetGenerator(std::make_shared<CUPnPPrice>()).SetMinimumVersion(2);
  // TODO: upnp:price, upnp:price@currency
  addBooleanProperty(UPNP_DIDL_UPNP_NAMESPACE, "payPerView", &m_tuned).SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "epgProviderName", &m_epgProviderName).SetOptional().SetMinimumVersion(2);
  // TODO: upnp:dateTimeRange, @daylightSaving
  // TODO: upnp:programPreserved, @startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
  // TODO: upnp:preservedTimeRange, e@startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
  // TODO: upnp:programList, ::program, ::program@preserved
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "radioCallSign", &m_radioCallSign).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "radioStationID", &m_radioStationID).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "radioBand", &m_radioBand).SetOptional();
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "channelNr", &m_channelNr).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "channelName", &m_channelName).SetOptional();
  // TODO: upnp:scheduledStartTime, @usage, @daylightSaving
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "scheduledEndTime", &m_scheduledEndTime).SetOptional().SetGenerator(std::make_shared<CUPnPDateTime>("scheduledEndTime")).SetMinimumVersion(3);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "scheduledDuration", &m_scheduledDuration).SetOptional().SetMinimumVersion(3);
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "signalStrength", &m_signalStrength).SetOptional().SetMinimumVersion(2);
  addBooleanProperty(UPNP_DIDL_UPNP_NAMESPACE, "signalLocked", &m_signalLocked).SetOptional().SetMinimumVersion(2);
  addBooleanProperty(UPNP_DIDL_UPNP_NAMESPACE, "tuned", &m_tuned).SetOptional().SetMinimumVersion(2);
  addBooleanProperty("@neverPlayable", &m_neverPlayable).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "segmentID", &m_bookmarkIDs).SetOptional().SupportMultipleValues().SetMinimumVersion(4);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "bookmarkID", &m_bookmarkIDs).SetOptional().SupportMultipleValues().SetMinimumVersion(2);
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "bookmarkedObjectID", &m_bookmarkedObjectID).SetOptional().SetMinimumVersion(2);
  // TODO: upnp:deviceUDN, @serviceType, @serviceID
  // TODO: upnp:stateVariableCollection, @serviceName, @rcsInstanceType, ::stateVariable, ::stateVariable@variableName
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "DVDRegionCode", &m_DVDRegionCode).SetOptional();
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "originalTrackNumber", &m_originalTrackNumber).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "toc", &m_toc).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "userAnnotation", &m_userAnnotation).SetOptional();
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "objectUpdateID", &m_objectUpdateID).SetOptional().SetMinimumVersion(3);
  // TODO: upnp:inclusionControl, ::role
  // TODO: upnp:objectOwner, @lock, ::role
  // TODO: upnp:objectLink, @groupID, @headObjID, @nextObjID, @prevObjID, ::title, ::startObject, ::mode, ::relatedInfo, ..., ::startInfo, ..., @endAction, @endAction@action, ::endAction, ...
  // TODO: upnp:objectLinkRef, ....
  // TODO: upnp:foreignMetadata, ...

  // set the validity of certain attributes
  setPropertyValid("@parentID");
  setPropertyValid("@restricted");
  setPropertyValidity(UPNP_DIDL_UPNP_NAMESPACE, "class", !m_class.GetType().empty());
  setPropertyValid(UPNP_DIDL_DC_NAMESPACE, "creator");
}