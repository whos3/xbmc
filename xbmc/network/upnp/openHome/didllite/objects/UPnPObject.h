#pragma once
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

#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPClass.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPDateTime.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPEnums.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPrice.h"

class CUPnPActor;
class CUPnPAlbumArt;
class CUPnPArtist;
class CUPnPAuthor;
class CUPnPGenre;
class CUPnPRating;
class CUPnPResource;

class CUPnPObject : public IFileItemElement
{
public:
  virtual ~CUPnPObject();

  // implementations of IDidlLiteElement
  virtual std::string GetIdentifier() const { return "Object"; }
  virtual std::set<std::string> Extends() const { return { }; }

  // implementations of IFileItemElement
  virtual std::string GetType() const { return m_class.GetType(); }
  virtual bool CanHandleFileItem(const CFileItem& item) const override { return true; }
  virtual bool ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const override;
  virtual bool FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context) override;

  const std::string& GetId() const { return m_id; }
  void SetId(const std::string& id);
  const std::string& GetParentId() const { return m_parentID; }
  void SetParentId(const std::string& parentId);
  bool IsRestricted() const { return m_restricted; }
  void SetRestricted(bool restricted);
  const std::string& GetTitle() const { return m_title; }
  void SetTitle(const std::string& title);
  const CUPnPClass& GetClass() const { return m_class; }

  CUPnPClass& GetClass() { return m_class; }

protected:
  CUPnPObject(const std::string& elementName, const std::string& classType, const std::string& className = "");
  CUPnPObject(const CUPnPObject& obj);

  void SetClass(const std::string& classType, const std::string& className = "");
  void SetClass(const CUPnPClass& upnpClass);
  const std::string& GetCreator() const { return m_creator; }
  void SetCreator(const std::string& creator);
  const std::string& GetWriteStatus() const { return m_writeStatus; }
  void SetWriteStatus(const std::string& writeStatus);
  std::vector<const CUPnPResource*> GetResources() const { return getMultiValueProperty(m_res); }
  void SetResources(const std::vector<CUPnPResource*>& resources);
  void AddResource(CUPnPResource* resource);
  // TODO: resExt, @id, ::isSyncAnchor, ::componentInfo, ..., ::segmentInfo, ...
  std::vector<const CUPnPArtist*> GetArtists() const { return getMultiValueProperty(m_artist); }
  void SetArtists(const std::vector<CUPnPArtist*>& artists);
  void SetArtists(const std::vector<std::string>& artists);
  std::vector<const CUPnPActor*> GetActors() const { return getMultiValueProperty(m_actor); }
  void SetActors(const std::vector<CUPnPActor*>& actors);
  std::vector<const CUPnPAuthor*> GetAuthors() const { return getMultiValueProperty(m_author); }
  void SetAuthors(const std::vector<CUPnPAuthor*>& authors);
  void SetAuthors(const std::vector<std::string>& authors);
  const std::vector<std::string>& GetProducers() const { return m_producer; }
  void SetProducers(const std::vector<std::string>& producers);
  const std::vector<std::string>& GetDirectors() const { return m_director; }
  void SetDirectors(const std::vector<std::string>& directors);
  const std::vector<std::string>& GetPublishers() const { return m_publisher; }
  void SetPublishers(const std::vector<std::string>& publishers);
  const std::vector<std::string>& GetContributors() const { return m_contributor; }
  void SetContributors(const std::vector<std::string>& contributors);
  std::vector<const CUPnPGenre*> GetGenres() const { return getMultiValueProperty(m_genre); }
  void SetGenres(const std::vector<CUPnPGenre*>& genres);
  void SetGenres(const std::vector<std::string>& genres);
  const std::vector<std::string>& GetAlbums() const { return m_album; }
  void SetAlbums(const std::vector<std::string>& albums);
  const std::vector<std::string>& GetPlaylists() const { return m_playlist; }
  void SetPlaylists(const std::vector<std::string>& playlists);
  std::vector<const CUPnPAlbumArt*> GetAlbumArt() const { return getMultiValueProperty(m_albumArtURI); }
  void SetAlbumArt(const std::vector<CUPnPAlbumArt*>& albumArt);
  void SetAlbumArt(const std::vector<std::string>& albumArt);
  const std::string& GetArtistDiscographyURI() const { return m_artistDiscographyURI; }
  void SetArtistDiscographyURI(const std::string& artistDiscographyURI);
  const std::string& GetLyricsURI() const { return m_lyricsURI; }
  void SetLyricsURI(const std::string& lyricsURI);
  const std::vector<std::string>& GetRelations() const { return m_relation; }
  void SetRelations(const std::vector<std::string>& relations);
  const std::string& GetStorageMedium() const { return m_storageMedium; }
  void SetStorageMedium(const std::string& storageMedium);
  const std::string& GetDescription() const { return m_description; }
  void SetDescription(const std::string& description);
  const std::string& GetLongDescription() const { return m_longDescription; }
  void SetLongDescription(const std::string& longDescription);
  const std::string& GetIcon() const { return m_icon; }
  void SetIcon(const std::string& icon);
  const std::vector<std::string>& GetRegions() const { return m_region; }
  void SetRegions(const std::vector<std::string>& regions);
  const std::vector<std::string>& GetRights() const { return m_rights; }
  void SetRights(const std::vector<std::string>& rights);
  const CDateTime& GetDate() const { return m_date.GetDate(); }
  void SetDate(const CDateTime& date);
  const std::vector<std::string>& GetLanguages() const { return m_language; }
  void SetLanguages(const std::vector<std::string>& languages);
  int32_t GetPlaybackCount() const { return m_playbackCount; }
  void SetPlaybackCount(int32_t playbackCount);
  const CDateTime& GetLastPlaybackTime() const { return m_lastPlaybackTime.GetDateTime(); }
  void SetLastPlaybackTime(const CDateTime& lastPlaybackTime);
  int64_t GetLastPlaybackPosition() const { return DidlLiteUtils::GetDurationInSeconds(m_lastPlaybackPosition); }
  void SetLastPlaybackPosition(int64_t lastPlaybackPosition);
  const CDateTime& GetRecordedStartDateTime() const { return m_recordedStartDateTime.GetDateTime(); }
  void SetRecordedStartDateTime(const CDateTime& recordedStartDateTime);
  const CDateTime& GetRecordedEndDateTime() const { return m_recordedEndDateTime.GetDateTime(); }
  void SetRecordedEndDateTime(const CDateTime& recordedEndDateTime);
  int64_t GetRecordedDuration() const { return DidlLiteUtils::GetDurationInSeconds(m_recordedDuration); }
  void SetRecordedDuration(int64_t recordedDuration);
  const std::string& GetRecordedDayOfWeekString() const { return m_recordedDayOfWeek; }
  void SetRecordedDayOfWeek(const std::string& recordedDayOfWeek);
  void SetRecordedDayOfWeek(UPnPDayOfWeek recordedDayOfWeek);
  const std::string& GetSrsRecordScheduleID() const { return m_srsRecordScheduleID; }
  void SetSrsRecordScheduleID(const std::string& srsRecordScheduleID);
  const std::string& GetSrsRecordTaskID() const { return m_srsRecordTaskID; }
  void SetSrsRecordTaskID(const std::string& srsRecordTaskID);
  bool IsRecordable() const { return m_recordable; }
  void SetRecordable(bool recordable);
  const std::string& GetProgramTitle() const { return m_programTitle; }
  void SetProgramTitle(const std::string& programTitle);
  const std::string& GetSeriesTitle() const { return m_seriesTitle; }
  void SetSeriesTitle(const std::string& seriesTitle);
  // TODO: upnp:programID, @type
  // TODO: upnp:seriesID, @type
  // TODO: upnp:channelID, @type, @distriNetworkName (min: 3), @distriNetworkID (min: 3)
  const std::string& GetEpisodeTypeString() const { return m_episodeType; }
  void SetEpisodeType(const std::string& episodeType);
  void SetEpisodeType(UPnPRecordedEpisodeType episodeType);
  uint32_t GetEpisodeCount() const { return m_episodeCount; }
  void SetEpisodeCount(uint32_t episodeCount);
  uint32_t GetEpisodeNumber() const { return m_episodeNumber; }
  void SetEpisodeNumber(uint32_t episodeNumber);
  uint32_t GetEpisodeSeason() const { return m_episodeSeason; }
  void SetEpisodeSeason(uint32_t episodeSeason);
  // TODO: upnp:programCode, @type
  std::vector<const CUPnPRating*> GetRatings() const { return getMultiValueProperty(m_rating); }
  void SetRatings(const std::vector<CUPnPRating*>& ratings);
  void SetRatings(const std::vector<std::string>& ratings);
  // TODO: upnp:channelGroupName, @id
  const std::string& GetCallSign() const { return m_callSign; }
  void SetCallSign(const std::string& callSign);
  const std::string& GetNetworkAffiliation() const { return m_networkAffiliation; }
  void SetNetworkAffiliation(const std::string& networkAffiliation);
  const std::string& GetServiceProvider() const { return m_serviceProvider; }
  void SetServiceProvider(const std::string& serviceProvider);
  const CUPnPPrice& GetPrice() const { return m_price; }
  void SetPrice(float price);
  void SetPrice(float price, const std::string& currency);
  bool IsPayPerView() const { return m_payPerView; }
  void SetPayPerView(bool payPerView);
  const std::string& SetEpgProviderName() const { return m_epgProviderName; }
  void SetEpgProviderName(const std::string& epgProviderName);
  // TODO: upnp:dateTimeRange, @daylightSaving (min: 3)
  // TODO: upnp:programPreserved, @startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
  // TODO: upnp:preservedTimeRange, e@startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
  // TODO: upnp:programList, ::program, ::program@preserved
  const std::string& GetRadioCallSign() const { return m_radioCallSign; }
  void SetRadioCallSign(const std::string& radioCallSign);
  const std::string& GetRadioStationID() const { return m_radioStationID; }
  void SetRadioStationID(const std::string& radioStationID);
  UPnPRadioBand GetRadioBand() const { return UPnPEnums::RadioBandFromString(m_radioBand); }
  const std::string& GetRadioBandString() const { return m_radioBand; }
  void SetRadioBand(const std::string& radioBand);
  void SetRadioBand(UPnPRadioBand radioBand);
  int32_t GetChannelNr() const { return m_channelNr; }
  void SetChannelNr(int32_t channelNr);
  const std::string& GetChannelName() const { return m_channelName; }
  void SetChannelName(const std::string& channelName);
  // TODO: upnp:scheduledStartTime, @usage (min: 3), @daylightSaving (min: 3)
  const CDateTime& GetScheduledEndTime() const { return m_scheduledEndTime.GetDateTime(); }
  void SetScheduledEndTime(const CDateTime& scheduledEndTime);
  int64_t GetScheduledDuration() const { return DidlLiteUtils::GetDurationInSeconds(m_scheduledDuration); }
  void SetScheduledDuration(int64_t scheduledDuration);
  int32_t GetSignalStrength() const { return m_signalStrength; }
  void SetSignalStrength(int32_t signalStrength);
  bool IsSignalLocked() const { return m_signalLocked; }
  void SetSignalLocked(bool signalLocked);
  bool IsTuned() const { return m_tuned; }
  void SetTuned(bool tuned);
  bool IsNeverPlayable() const { return m_neverPlayable; }
  void SetNeverPlayable(bool neverPlayable);
  const std::vector<std::string>& GetSegmentIDs() const { return m_segmentIDs; }
  void SetSegmentIDs(const std::vector<std::string>& segmentIDs);
  const std::vector<std::string>& GetBookmarkIDs() const { return m_bookmarkIDs; }
  void SetBookmarkIDs(const std::vector<std::string>& bookmarkIDs);
  const std::string& GetBookmarkedObjectID() const { return m_bookmarkedObjectID; }
  void SetBookmarkedObjectID(const std::string& bookmarkedObjectID);
  // TODO: upnp:deviceUDN, @serviceType, @serviceID
  // TODO: upnp:stateVariableCollection, @serviceName, @rcsInstanceType, ::stateVariable, ::stateVariable@variableName
  int32_t GetDVDRegionCode() const { return m_DVDRegionCode; }
  void SetDVDRegionCode(int32_t DVDRegionCode);
  int32_t GetOriginalTrackNumber() const { return m_originalTrackNumber; }
  void SetOriginalTrackNumber(int32_t originalTrackNumber);
  const std::string& GetTableOfContents() const { return m_toc; }
  void SetTableOfContents(const std::string& toc);
  const std::string& GetUserAnnotation() const { return m_userAnnotation; }
  void SetUserAnnotation(const std::string& userAnnotation);
  uint32_t GetObjectUpdateID() const { return m_objectUpdateID; }
  void SetObjectUpdateID(uint32_t objectUpdateID);
  // TODO: upnp:inclusionControl, ::role
  // TODO: upnp:objectOwner, @lock, ::role
  // TODO: upnp:objectLink, @groupID, @headObjID, @nextObjID, @prevObjID, ::title, ::startObject, ::mode, ::relatedInfo, ..., ::startInfo, ..., @endAction, @endAction@action, ::endAction, ...
  // TODO: upnp:objectLinkRef, ....
  // TODO: upnp:foreignMetadata, ...

  template<class TDidlLiteElement>
  static std::vector<const TDidlLiteElement*> getMultiValueProperty(const std::vector<TDidlLiteElement*>& prop)
  {
    std::vector<const TDidlLiteElement*> props;
    for (const auto& value : prop)
      props.push_back(value);

    return props;
  }

private:
  void initializeProperties();

  std::string m_id;
  std::string m_parentID;
  bool m_restricted;
  CUPnPClass m_class;
  std::string m_title;
  std::string m_creator;
  std::string m_writeStatus;
  std::vector<CUPnPResource*> m_res;
  // TODO: resExt (min: 4), @id, ::isSyncAnchor, ::componentInfo, ..., ::segmentInfo, ...
  std::vector<CUPnPArtist*> m_artist;
  std::vector<CUPnPActor*> m_actor;
  std::vector<CUPnPAuthor*> m_author;
  std::vector<std::string> m_producer;
  std::vector<std::string> m_director;
  std::vector<std::string> m_publisher;
  std::vector<std::string> m_contributor;
  std::vector<CUPnPGenre*> m_genre;
  std::vector<std::string> m_album;
  std::vector<std::string> m_playlist;
  std::vector<CUPnPAlbumArt*> m_albumArtURI;
  std::string m_artistDiscographyURI;
  std::string m_lyricsURI;
  std::vector<std::string> m_relation;
  std::string m_storageMedium;
  std::string m_description;
  std::string m_longDescription;
  std::string m_icon;
  std::vector<std::string> m_region;
  std::vector<std::string> m_rights;
  CUPnPDate m_date;
  std::vector<std::string> m_language;
  int32_t m_playbackCount;
  CUPnPDateTime m_lastPlaybackTime;
  std::string m_lastPlaybackPosition;
  CUPnPDateTime m_recordedStartDateTime;
  CUPnPDateTime m_recordedEndDateTime;
  std::string m_recordedDuration;
  std::string m_recordedDayOfWeek;
  std::string m_srsRecordScheduleID;
  std::string m_srsRecordTaskID;
  bool m_recordable;
  std::string m_programTitle;
  std::string m_seriesTitle;
  // TODO: upnp:programID (min: 2), @type
  // TODO: upnp:seriesID (min: 2), @type
  // TODO: upnp:channelID (min: 2), @type, @distriNetworkName, @distriNetworkID
  std::string m_episodeType;
  uint32_t m_episodeCount;
  uint32_t m_episodeNumber;
  uint32_t m_episodeSeason;
  // TODO: upnp:programCode (min: 2), @type
  std::vector<CUPnPRating*> m_rating;
  // TODO: upnp:recommendationID (min: 4), @type
  // TODO: upnp:channelGroupName (min: 2), @id
  std::string m_callSign;
  std::string m_networkAffiliation;
  std::string m_serviceProvider;
  CUPnPPrice m_price;
  bool m_payPerView;
  std::string m_epgProviderName;
  // TODO: upnp:dateTimeRange (min: 2), @daylightSaving
  // TODO: upnp:programPreserved (min: 4), @startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
  // TODO: upnp:preservedTimeRange (min: 4), e@startTime, @startTimeDaylightSaving, @endTime, @endTimeDaylightSaving
  // TODO: upnp:programList (min: 4), ::program, ::program@preserved
  std::string m_radioCallSign;
  std::string m_radioStationID;
  std::string m_radioBand;
  int32_t m_channelNr;
  std::string m_channelName;
  // TODO: upnp:scheduledStartTime, @usage, @daylightSaving
  CUPnPDateTime m_scheduledEndTime;
  std::string m_scheduledDuration;
  int32_t m_signalStrength;
  bool m_signalLocked;
  bool m_tuned;
  bool m_neverPlayable;
  std::vector<std::string> m_segmentIDs;
  std::vector<std::string> m_bookmarkIDs;
  std::string m_bookmarkedObjectID;
  // TODO: upnp:deviceUDN (min: 2), @serviceType, @serviceID
  // TODO: upnp:stateVariableCollection (min: 2), @serviceName, @rcsInstanceType, ::stateVariable, ::stateVariable@variableName
  int32_t m_DVDRegionCode;
  int m_originalTrackNumber;
  std::string m_toc;
  std::string m_userAnnotation;
  uint32_t m_objectUpdateID;
  // TODO: upnp:inclusionControl (min: 4), ::role
  // TODO: upnp:objectOwner (min: 4), @lock, ::role
  // TODO: upnp:objectLink (min: 4), @groupID, @headObjID, @nextObjID, @prevObjID, ::title, ::startObject, ::mode, ::relatedInfo, ..., ::startInfo, ..., @endAction, @endAction@action, ::endAction, ...
  // TODO: upnp:objectLinkRef (min: 4), ....
  // TODO: upnp:foreignMetadata (min: 3), ...
};
