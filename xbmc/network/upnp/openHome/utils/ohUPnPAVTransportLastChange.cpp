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

#include "ohUPnPAVTransportLastChange.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "threads/SingleLock.h"

static const std::string UPnPAVTransportNamespace = "urn:schemas-upnp-org:metadata-1-0/AVT/";

static const std::string StateVariableTransportState = "TransportState";
static const std::string StateVariableTransportStatus = "TransportStatus";
static const std::string StateVariableCurrentMediaCategory = "CurrentMediaCategory";
static const std::string StateVariablePlaybackStorageMedium = "PlaybackStorageMedium";
static const std::string StateVariableRecordStorageMedium = "RecordStorageMedium";
static const std::string StateVariablePossiblePlaybackStorageMedia = "PossiblePlaybackStorageMedia";
static const std::string StateVariablePossibleRecordStorageMedia = "PossibleRecordStorageMedia";
static const std::string StateVariableCurrentPlayMode = "CurrentPlayMode";
static const std::string StateVariableTransportPlaySpeed = "TransportPlaySpeed";
static const std::string StateVariableRecordMediumWriteStatus = "RecordMediumWriteStatus";
static const std::string StateVariableCurrentRecordQualityMode = "CurrentRecordQualityMode";
static const std::string StateVariablePossibleRecordQualityMode = "PossibleRecordQualityMode";
static const std::string StateVariableNumberOfTracks = "NumberOfTracks";
static const std::string StateVariableCurrentTrack = "CurrentTrack";
static const std::string StateVariableCurrentTrackDuration = "CurrentTrackDuration";
static const std::string StateVariableCurrentMediaDuration = "CurrentMediaDuration";
static const std::string StateVariableRelativeTimePosition = "RelativeTimePosition";
static const std::string StateVariableAbsoluteTimePosition = "AbsoluteTimePosition";
static const std::string StateVariableCurrentTrackMetadata = "CurrentTrackMetadata";
static const std::string StateVariableCurrentTrackURI = "CurrentTrackURI";
static const std::string StateVariableAVTransportURI = "AVTransportURI";
static const std::string StateVariableNextAVTransportURI = "NextAVTransportURI";
static const std::string StateVariableCurrentTransportActions = "CurrentTransportActions";
static const std::string StateVariableRelativeCounterPosition = "RelativeCounterPosition";
static const std::string StateVariableAbsoluteCounterPosition = "AbsoluteCounterPosition";
static const std::string StateVariableDRMState = "DRMState";

template<typename TEnum>
static std::vector<TEnum> EnumArrayFromCSVString(const std::string& value)
{
  std::vector<TEnum> enumValues;
  const auto values = COhUtils::SplitCSV(value);
  for (const auto& val : values)
    enumValues.push_back(TEnum(val));

  return enumValues;
}

template<typename TEnum>
static std::string CSVStringFromEnumArray(const std::vector<TEnum>& enumValues)
{
  std::vector<std::string> values;
  for (const auto& val : enumValues)
    values.push_back(val.GetAsString());

  return COhUtils::ToCSV(values);
}

COhUPnPAVTransportLastChangeDocument::COhUPnPAVTransportLastChangeDocument()
  : CXmlPropertyTree("Event")
{
  AddNamespace(UPnPAVTransportNamespace);
}

COhUPnPAVTransportLastChange::COhUPnPAVTransportLastChange(uint32_t instanceID /* = 0 */)
  : COhUPnPStateVariable("InstanceID")
  , m_transportState(StateVariableTransportState)
  , m_transportStatus(StateVariableTransportStatus)
  , m_currentMediaCategory(StateVariableCurrentMediaCategory)
  , m_playbackStorageMedium(StateVariablePlaybackStorageMedium)
  , m_recordStorageMedium(StateVariableRecordStorageMedium)
  , m_possiblePlaybackStorageMedia(StateVariablePossiblePlaybackStorageMedia)
  , m_possibleRecordStorageMedia(StateVariablePossibleRecordStorageMedia)
  , m_currentPlayMode(StateVariableCurrentPlayMode)
  , m_transportPlaySpeed(StateVariableTransportPlaySpeed)
  , m_recordMediumWriteStatus(StateVariableRecordMediumWriteStatus)
  , m_currentRecordQualityMode(StateVariableCurrentRecordQualityMode)
  , m_possibleRecordQualityMode(StateVariablePossibleRecordQualityMode)
  , m_numberOfTracks(StateVariableNumberOfTracks)
  , m_currentTrack(StateVariableCurrentTrack)
  , m_currentTrackDuration(StateVariableCurrentTrackDuration)
  , m_currentMediaDuration(StateVariableCurrentMediaDuration)
  , m_relativeTimePosition(StateVariableRelativeTimePosition)
  , m_absoluteTimePosition(StateVariableAbsoluteTimePosition)
  , m_currentTrackMetadata(StateVariableCurrentTrackMetadata)
  , m_currentTrackURI(StateVariableCurrentTrackURI)
  , m_avTransportURI(StateVariableAVTransportURI)
  , m_nextAVTransportURI(StateVariableNextAVTransportURI)
  , m_currentTransportActions(StateVariableCurrentTransportActions)
  , m_relativeCounterPosition(StateVariableRelativeCounterPosition)
  , m_absoluteCounterPosition(StateVariableAbsoluteCounterPosition)
  , m_drmState(StateVariableDRMState)
{
  initializeProperties();

  SetValue(instanceID);
  setPropertyValid(ValueProperty);
}

COhUPnPAVTransportLastChange::COhUPnPAVTransportLastChange(const COhUPnPAVTransportLastChange& other)
  : COhUPnPStateVariable(other)
  , m_transportState(other.m_transportState)
  , m_transportStatus(other.m_transportStatus)
  , m_currentMediaCategory(other.m_currentMediaCategory)
  , m_playbackStorageMedium(other.m_playbackStorageMedium)
  , m_recordStorageMedium(other.m_recordStorageMedium)
  , m_possiblePlaybackStorageMedia(other.m_possiblePlaybackStorageMedia)
  , m_possibleRecordStorageMedia(other.m_possibleRecordStorageMedia)
  , m_currentPlayMode(other.m_currentPlayMode)
  , m_transportPlaySpeed(other.m_transportPlaySpeed)
  , m_recordMediumWriteStatus(other.m_recordMediumWriteStatus)
  , m_currentRecordQualityMode(other.m_currentRecordQualityMode)
  , m_possibleRecordQualityMode(other.m_possibleRecordQualityMode)
  , m_numberOfTracks(other.m_numberOfTracks)
  , m_currentTrack(other.m_currentTrack)
  , m_currentTrackDuration(other.m_currentTrackDuration)
  , m_currentMediaDuration(other.m_currentMediaDuration)
  , m_relativeTimePosition(other.m_relativeTimePosition)
  , m_absoluteTimePosition(other.m_absoluteTimePosition)
  , m_currentTrackMetadata(other.m_currentTrackMetadata)
  , m_currentTrackURI(other.m_currentTrackURI)
  , m_avTransportURI(other.m_avTransportURI)
  , m_nextAVTransportURI(other.m_nextAVTransportURI)
  , m_currentTransportActions(other.m_currentTransportActions)
  , m_relativeCounterPosition(other.m_relativeCounterPosition)
  , m_absoluteCounterPosition(other.m_absoluteCounterPosition)
  , m_drmState(other.m_drmState)
{
  initializeProperties();
  copyPropertyValidity(&other);
}

bool COhUPnPAVTransportLastChange::HasChanged() const
{
  CSingleLock lock(m_critical);

  bool changed = false;
  changed |= m_transportState.HasChanged();
  changed |= m_transportStatus.HasChanged();
  changed |= m_currentMediaCategory.HasChanged();
  changed |= m_playbackStorageMedium.HasChanged();
  changed |= m_recordStorageMedium.HasChanged();
  changed |= m_possiblePlaybackStorageMedia.HasChanged();
  changed |= m_possibleRecordStorageMedia.HasChanged();
  changed |= m_currentPlayMode.HasChanged();
  changed |= m_transportPlaySpeed.HasChanged();
  changed |= m_recordMediumWriteStatus.HasChanged();
  changed |= m_currentRecordQualityMode.HasChanged();
  changed |= m_possibleRecordQualityMode.HasChanged();
  changed |= m_numberOfTracks.HasChanged();
  changed |= m_currentTrack.HasChanged();
  changed |= m_currentTrackDuration.HasChanged();
  changed |= m_currentMediaDuration.HasChanged();
  changed |= m_relativeTimePosition.HasChanged();
  changed |= m_absoluteTimePosition.HasChanged();
  changed |= m_currentTrackMetadata.HasChanged();
  changed |= m_currentTrackURI.HasChanged();
  changed |= m_avTransportURI.HasChanged();
  changed |= m_nextAVTransportURI.HasChanged();
  changed |= m_currentTransportActions.HasChanged();
  changed |= m_relativeCounterPosition.HasChanged();
  changed |= m_absoluteCounterPosition.HasChanged();
  changed |= m_drmState.HasChanged();

  return changed;
}

void COhUPnPAVTransportLastChange::Fix()
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableTransportState, false);
  setPropertyValidity(StateVariableTransportStatus, false);
  setPropertyValidity(StateVariableCurrentMediaCategory, false);
  setPropertyValidity(StateVariablePlaybackStorageMedium, false);
  setPropertyValidity(StateVariableRecordStorageMedium, false);
  setPropertyValidity(StateVariablePossiblePlaybackStorageMedia, false);
  setPropertyValidity(StateVariablePossibleRecordStorageMedia, false);
  setPropertyValidity(StateVariableCurrentPlayMode, false);
  setPropertyValidity(StateVariableTransportPlaySpeed, false);
  setPropertyValidity(StateVariableRecordMediumWriteStatus, false);
  setPropertyValidity(StateVariableCurrentRecordQualityMode, false);
  setPropertyValidity(StateVariablePossibleRecordQualityMode, false);
  setPropertyValidity(StateVariableNumberOfTracks, false);
  setPropertyValidity(StateVariableCurrentTrack, false);
  setPropertyValidity(StateVariableCurrentTrackDuration, false);
  setPropertyValidity(StateVariableCurrentMediaDuration, false);
  setPropertyValidity(StateVariableRelativeTimePosition, false);
  setPropertyValidity(StateVariableAbsoluteTimePosition, false);
  setPropertyValidity(StateVariableCurrentTrackMetadata, false);
  setPropertyValidity(StateVariableCurrentTrackURI, false);
  setPropertyValidity(StateVariableAVTransportURI, false);
  setPropertyValidity(StateVariableNextAVTransportURI, false);
  setPropertyValidity(StateVariableCurrentTransportActions, false);
  setPropertyValidity(StateVariableRelativeCounterPosition, false);
  setPropertyValidity(StateVariableAbsoluteCounterPosition, false);
  setPropertyValidity(StateVariableDRMState, false);

  m_transportState.Fix();
  m_transportStatus.Fix();
  m_currentMediaCategory.Fix();
  m_playbackStorageMedium.Fix();
  m_recordStorageMedium.Fix();
  m_possiblePlaybackStorageMedia.Fix();
  m_possibleRecordStorageMedia.Fix();
  m_currentPlayMode.Fix();
  m_transportPlaySpeed.Fix();
  m_recordMediumWriteStatus.Fix();
  m_currentRecordQualityMode.Fix();
  m_possibleRecordQualityMode.Fix();
  m_numberOfTracks.Fix();
  m_currentTrack.Fix();
  m_currentTrackDuration.Fix();
  m_currentMediaDuration.Fix();
  m_relativeTimePosition.Fix();
  m_absoluteTimePosition.Fix();
  m_currentTrackMetadata.Fix();
  m_currentTrackURI.Fix();
  m_avTransportURI.Fix();
  m_nextAVTransportURI.Fix();
  m_currentTransportActions.Fix();
  m_relativeCounterPosition.Fix();
  m_absoluteCounterPosition.Fix();
  m_drmState.Fix();
}

std::vector<COhUPnPAVTransportStorageMedium> COhUPnPAVTransportLastChange::GetPossiblePlaybackStorageMedia() const
{
  return EnumArrayFromCSVString<COhUPnPAVTransportStorageMedium>(m_possiblePlaybackStorageMedia.GetValue());
}

std::vector<COhUPnPAVTransportStorageMedium> COhUPnPAVTransportLastChange::GetPossibleRecordStorageMedia() const
{
  return EnumArrayFromCSVString<COhUPnPAVTransportStorageMedium>(m_possibleRecordStorageMedia.GetValue());
}

std::vector<COhUPnPAVTransportRecordQualityMode> COhUPnPAVTransportLastChange::GetPossibleRecordQualityModes() const
{
  return EnumArrayFromCSVString<COhUPnPAVTransportRecordQualityMode>(m_possibleRecordQualityMode.GetValue());
}

std::vector<COhUPnPAVTransportTransportAction> COhUPnPAVTransportLastChange::GetCurrentTransportActions() const
{
  return EnumArrayFromCSVString<COhUPnPAVTransportTransportAction>(m_currentTransportActions.GetValue());
}

void COhUPnPAVTransportLastChange::SetTransportState(const COhUPnPAVTransportTransportState& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableTransportState, m_transportState.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetTransportStatus(const COhUPnPAVTransportTransportStatus& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableTransportStatus, m_transportStatus.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetCurrentMediaCategory(const COhUPnPAVTransportMediaCategory& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentMediaCategory, m_currentMediaCategory.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetPlaybackStorageMedium(const COhUPnPAVTransportStorageMedium& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariablePlaybackStorageMedium, m_playbackStorageMedium.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetRecordStorageMedium(const COhUPnPAVTransportStorageMedium& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableRecordStorageMedium, m_recordStorageMedium.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetPossiblePlaybackStorageMedia(const std::vector<COhUPnPAVTransportStorageMedium>& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariablePossiblePlaybackStorageMedia, m_possiblePlaybackStorageMedia.SetValue(CSVStringFromEnumArray(value)));
}

void COhUPnPAVTransportLastChange::SetPossibleRecordStorageMedia(const std::vector<COhUPnPAVTransportStorageMedium>& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariablePossibleRecordStorageMedia, m_possibleRecordStorageMedia.SetValue(CSVStringFromEnumArray(value)));
}

void COhUPnPAVTransportLastChange::SetCurrentPlayMode(const COhUPnPAVTransportPlayMode& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentPlayMode, m_currentPlayMode.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetTransportPlaySpeed(float value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableTransportPlaySpeed, m_transportPlaySpeed.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetRecordMediumWriteStatus(const COhUPnPAVTransportWriteStatus& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableRecordMediumWriteStatus, m_recordMediumWriteStatus.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetCurrentRecordQualityMode(const COhUPnPAVTransportRecordQualityMode& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentRecordQualityMode, m_currentRecordQualityMode.SetValue(value.GetAsString()));
}

void COhUPnPAVTransportLastChange::SetPossibleRecordQualityMode(const std::vector<COhUPnPAVTransportRecordQualityMode>& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariablePossibleRecordQualityMode, m_possibleRecordQualityMode.SetValue(CSVStringFromEnumArray(value)));
}

void COhUPnPAVTransportLastChange::SetNumberOfTracks(uint32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableNumberOfTracks, m_numberOfTracks.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetCurrentTrack(uint32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrack, m_currentTrack.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetCurrentTrackDuration(uint32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrackDuration, m_currentTrackDuration.SetValue(COhUtils::GetDurationFromSeconds(value)));
}

void COhUPnPAVTransportLastChange::SetCurrentTrackDuration(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrackDuration, m_currentTrackDuration.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetCurrentMediaDuration(uint32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentMediaDuration, m_currentMediaDuration.SetValue(COhUtils::GetDurationFromSeconds(value)));
}

void COhUPnPAVTransportLastChange::SetCurrentMediaDuration(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrackDuration, m_currentMediaDuration.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetRelativeTimePosition(uint32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableRelativeTimePosition, m_relativeTimePosition.SetValue(COhUtils::GetDurationFromSeconds(value)));
}

void COhUPnPAVTransportLastChange::SetRelativeTimePosition(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrackDuration, m_relativeTimePosition.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetAbsoluteTimePosition(uint32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableAbsoluteTimePosition, m_absoluteTimePosition.SetValue(COhUtils::GetDurationFromSeconds(value)));
}

void COhUPnPAVTransportLastChange::SetAbsoluteTimePosition(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrackDuration, m_absoluteTimePosition.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetCurrentTrackMetadata(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrackMetadata, m_currentTrackMetadata.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetCurrentTrackURI(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTrackURI, m_currentTrackURI.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetAVTransportURI(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableAVTransportURI, m_avTransportURI.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetAVTransportURIMetadata(const std::string& value)
{
  CSingleLock lock(m_critical);
  m_avTransportURIMetadata = value;
}

void COhUPnPAVTransportLastChange::SetNextAVTransportURI(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableNextAVTransportURI, m_nextAVTransportURI.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetNextAVTransportURIMetadata(const std::string& value)
{
  CSingleLock lock(m_critical);
  m_nextAVTransportURIMetadata = value;
}

void COhUPnPAVTransportLastChange::SetCurrentTransportActions(const std::vector<COhUPnPAVTransportTransportAction>& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableCurrentTransportActions, m_currentTransportActions.SetValue(CSVStringFromEnumArray(value)));
}

void COhUPnPAVTransportLastChange::SetRelativeCounterPosition(int32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableRelativeCounterPosition, m_relativeCounterPosition.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetAbsoluteCounterPosition(uint32_t value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableAbsoluteCounterPosition, m_absoluteCounterPosition.SetValue(value));
}

void COhUPnPAVTransportLastChange::SetDRMState(const std::string& value)
{
  CSingleLock lock(m_critical);
  setPropertyValidity(StateVariableDRMState, m_drmState.SetValue(value));
}

void COhUPnPAVTransportLastChange::initializeProperties()
{
  addElementProperty(StateVariableTransportState, &m_transportState).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableTransportState);
  addElementProperty(StateVariableTransportStatus, &m_transportStatus).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableTransportStatus);
  addElementProperty(StateVariableCurrentMediaCategory, &m_currentMediaCategory).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentMediaCategory);
  addElementProperty(StateVariablePlaybackStorageMedium, &m_playbackStorageMedium).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariablePlaybackStorageMedium);
  addElementProperty(StateVariableRecordStorageMedium, &m_recordStorageMedium).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableRecordStorageMedium);
  addElementProperty(StateVariablePossiblePlaybackStorageMedia, &m_possiblePlaybackStorageMedia).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariablePossiblePlaybackStorageMedia);
  addElementProperty(StateVariablePossibleRecordStorageMedia, &m_possibleRecordStorageMedia).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariablePossibleRecordStorageMedia);
  addElementProperty(StateVariableCurrentPlayMode, &m_currentPlayMode).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentPlayMode);
  addElementProperty(StateVariableTransportPlaySpeed, &m_transportPlaySpeed).SetOptional().SetGenerator<COhUPnPStateVariable<float>>(StateVariableTransportPlaySpeed);
  addElementProperty(StateVariableRecordMediumWriteStatus, &m_recordMediumWriteStatus).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableRecordMediumWriteStatus);
  addElementProperty(StateVariableCurrentRecordQualityMode, &m_currentRecordQualityMode).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentRecordQualityMode);
  addElementProperty(StateVariablePossibleRecordQualityMode, &m_possibleRecordQualityMode).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariablePossibleRecordQualityMode);
  addElementProperty(StateVariableNumberOfTracks, &m_numberOfTracks).SetOptional().SetGenerator<COhUPnPStateVariable<uint32_t>>(StateVariableNumberOfTracks);
  addElementProperty(StateVariableCurrentTrack, &m_currentTrack).SetOptional().SetGenerator<COhUPnPStateVariable<uint32_t>>(StateVariableCurrentTrack);
  addElementProperty(StateVariableCurrentTrackDuration, &m_currentTrackDuration).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentTrackDuration);
  addElementProperty(StateVariableCurrentMediaDuration, &m_currentMediaDuration).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentMediaDuration);
  addElementProperty(StateVariableRelativeTimePosition, &m_relativeTimePosition).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableRelativeTimePosition);
  addElementProperty(StateVariableAbsoluteTimePosition, &m_absoluteTimePosition).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableAbsoluteTimePosition);
  addElementProperty(StateVariableCurrentTrackMetadata, &m_currentTrackMetadata).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentTrackMetadata);
  addElementProperty(StateVariableCurrentTrackURI, &m_currentTrackURI).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentTrackURI);
  addElementProperty(StateVariableAVTransportURI, &m_avTransportURI).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableAVTransportURI);
  addElementProperty(StateVariableNextAVTransportURI, &m_nextAVTransportURI).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableNextAVTransportURI);
  addElementProperty(StateVariableCurrentTransportActions, &m_currentTransportActions).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableCurrentTransportActions);
  addElementProperty(StateVariableRelativeCounterPosition, &m_relativeCounterPosition).SetOptional().SetGenerator<COhUPnPStateVariable<int32_t>>(StateVariableRelativeCounterPosition);
  addElementProperty(StateVariableAbsoluteCounterPosition, &m_absoluteCounterPosition).SetOptional().SetGenerator<COhUPnPStateVariable<uint32_t>>(StateVariableAbsoluteCounterPosition);
  addElementProperty(StateVariableDRMState, &m_drmState).SetOptional().SetGenerator<COhUPnPStateVariable<std::string>>(StateVariableDRMState);
}
