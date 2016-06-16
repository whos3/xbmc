#pragma once
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

#include <string>

#include "network/upnp/openHome/utils/ohUPnPAVTransport.h"
#include "network/upnp/openHome/utils/ohUPnPStateVariables.h"
#include "utils/propertytree/IPropertyTreeElement.h"
#include "utils/propertytree/xml/XmlPropertyTree.h"
#include "threads/CriticalSection.h"

class COhUPnPAVTransportLastChangeDocument : public CXmlPropertyTree
{
public:
  COhUPnPAVTransportLastChangeDocument();
  ~COhUPnPAVTransportLastChangeDocument() = default;
};

class COhUPnPAVTransportLastChange : public COhUPnPStateVariable<uint32_t>
{
public:
  COhUPnPAVTransportLastChange(uint32_t instanceID = 0);
  COhUPnPAVTransportLastChange(const COhUPnPAVTransportLastChange& other);
  ~COhUPnPAVTransportLastChange() = default;

  // implementations of IPropertyTreeElement
  COhUPnPAVTransportLastChange* Create() const override { return new COhUPnPAVTransportLastChange(); }
  COhUPnPAVTransportLastChange* Clone() const override { return new COhUPnPAVTransportLastChange(*this); }

  // specialisations of COhUPnPAVTransportLastChangeStateVariable<>
  bool HasChanged() const override;
  void Fix() override;

  inline void Lock() { m_critical.lock(); }
  inline void Unlock() { m_critical.unlock(); }

  COhUPnPAVTransportTransportState GetTransportState() const { return COhUPnPAVTransportTransportState(m_transportState.GetValue()); }
  COhUPnPAVTransportTransportStatus GetTransportStatus() const { return COhUPnPAVTransportTransportStatus(m_transportStatus.GetValue()); }
  COhUPnPAVTransportMediaCategory GetCurrentMediaCategory() const { return COhUPnPAVTransportMediaCategory(m_currentMediaCategory.GetValue()); }
  COhUPnPAVTransportStorageMedium GetPlaybackStorageMedium() const { return COhUPnPAVTransportStorageMedium(m_playbackStorageMedium.GetValue()); }
  COhUPnPAVTransportStorageMedium GetRecordStorageMedium() const { return COhUPnPAVTransportStorageMedium(m_recordStorageMedium.GetValue()); }
  std::vector<COhUPnPAVTransportStorageMedium> GetPossiblePlaybackStorageMedia() const;
  std::vector<COhUPnPAVTransportStorageMedium> GetPossibleRecordStorageMedia() const;
  COhUPnPAVTransportPlayMode GetCurrentPlayMode() const { return COhUPnPAVTransportPlayMode(m_currentPlayMode.GetValue()); }
  float GetTransportPlaySpeed() const { return m_transportPlaySpeed.GetValue(); }
  COhUPnPAVTransportWriteStatus GetRecordMediumWriteStatus() const { return COhUPnPAVTransportWriteStatus(m_recordMediumWriteStatus.GetValue()); }
  COhUPnPAVTransportRecordQualityMode GetCurrentRecordQualityMode() const { return COhUPnPAVTransportRecordQualityMode(m_currentRecordQualityMode.GetValue()); }
  std::vector<COhUPnPAVTransportRecordQualityMode> GetPossibleRecordQualityModes() const;
  uint32_t GetNumberOfTracks() const { return m_numberOfTracks.GetValue(); }
  uint32_t GetCurrentTrack() const { return m_currentTrack.GetValue(); }
  std::string GetCurrentTrackDuration() const { return m_currentTrackDuration.GetValue(); }
  std::string GetCurrentMediaDuration() const { return m_currentMediaDuration.GetValue(); }
  std::string GetCurrentTrackURI() const { return m_currentTrackURI.GetValue(); }
  std::string GetCurrentTrackMetaData() const { return m_currentTrackMetadata.GetValue(); }
  std::string GetAVTransportURI() const { return m_avTransportURI.GetValue(); }
  std::string GetAVTransportURIMetaData() const { return m_avTransportURIMetadata; }
  std::string GetNextAVTransportURI() const { return m_nextAVTransportURI.GetValue(); }
  std::string GetNextAVTransportURIMetaData() const { return m_nextAVTransportURIMetadata; }
  std::string GetRelativeTimePosition() const { return m_relativeTimePosition.GetValue(); }
  std::string GetAbsoluteTimePosition() const { return m_absoluteTimePosition.GetValue(); }
  std::vector<COhUPnPAVTransportTransportAction> GetCurrentTransportActions() const;
  int32_t GetRelativeCounterPosition() const { return m_relativeCounterPosition.GetValue(); }
  uint32_t GetAbsoluteCounterPosition() const { return m_absoluteCounterPosition.GetValue(); }
  std::string GetDRMState() const { return m_drmState.GetValue(); }

  void SetTransportState(const COhUPnPAVTransportTransportState& value);
  void SetTransportStatus(const COhUPnPAVTransportTransportStatus& value);
  void SetCurrentMediaCategory(const COhUPnPAVTransportMediaCategory& value);
  void SetPlaybackStorageMedium(const COhUPnPAVTransportStorageMedium& value);
  void SetRecordStorageMedium(const COhUPnPAVTransportStorageMedium& value);
  void SetPossiblePlaybackStorageMedia(const std::vector<COhUPnPAVTransportStorageMedium>& value);
  void SetPossibleRecordStorageMedia(const std::vector<COhUPnPAVTransportStorageMedium>& value);
  void SetCurrentPlayMode(const COhUPnPAVTransportPlayMode& value);
  void SetTransportPlaySpeed(float value);
  void SetRecordMediumWriteStatus(const COhUPnPAVTransportWriteStatus& value);
  void SetCurrentRecordQualityMode(const COhUPnPAVTransportRecordQualityMode& value);
  void SetPossibleRecordQualityMode(const std::vector<COhUPnPAVTransportRecordQualityMode>& value);
  void SetNumberOfTracks(uint32_t value);
  void SetCurrentTrack(uint32_t value);
  void SetCurrentTrackDuration(uint32_t value);
  void SetCurrentTrackDuration(const std::string& value);
  void SetCurrentMediaDuration(uint32_t value);
  void SetCurrentMediaDuration(const std::string& value);
  void SetRelativeTimePosition(uint32_t value);
  void SetRelativeTimePosition(const std::string& value);
  void SetAbsoluteTimePosition(uint32_t value);
  void SetAbsoluteTimePosition(const std::string& value);
  void SetCurrentTrackMetadata(const std::string& value);
  void SetCurrentTrackURI(const std::string& value);
  void SetAVTransportURI(const std::string& value);
  void SetAVTransportURIMetadata(const std::string& value);
  void SetNextAVTransportURI(const std::string& value);
  void SetNextAVTransportURIMetadata(const std::string& value);
  void SetCurrentTransportActions(const std::vector<COhUPnPAVTransportTransportAction>& value);
  void SetRelativeCounterPosition(int32_t value);
  void SetAbsoluteCounterPosition(uint32_t value);
  void SetDRMState(const std::string& value);

private:
  void initializeProperties();

  CCriticalSection m_critical;

  COhUPnPStateVariable<std::string> m_transportState;
  COhUPnPStateVariable<std::string> m_transportStatus;
  COhUPnPStateVariable<std::string> m_currentMediaCategory;
  COhUPnPStateVariable<std::string> m_playbackStorageMedium;
  COhUPnPStateVariable<std::string> m_recordStorageMedium;
  COhUPnPStateVariable<std::string> m_possiblePlaybackStorageMedia;
  COhUPnPStateVariable<std::string> m_possibleRecordStorageMedia;
  COhUPnPStateVariable<std::string> m_currentPlayMode;
  COhUPnPStateVariable<float> m_transportPlaySpeed;
  COhUPnPStateVariable<std::string> m_recordMediumWriteStatus;
  COhUPnPStateVariable<std::string> m_currentRecordQualityMode;
  COhUPnPStateVariable<std::string> m_possibleRecordQualityMode;
  COhUPnPStateVariable<uint32_t> m_numberOfTracks;
  COhUPnPStateVariable<uint32_t> m_currentTrack;
  COhUPnPStateVariable<std::string> m_currentTrackDuration;
  COhUPnPStateVariable<std::string> m_currentMediaDuration;
  COhUPnPStateVariable<std::string> m_relativeTimePosition;
  COhUPnPStateVariable<std::string> m_absoluteTimePosition;
  COhUPnPStateVariable<std::string> m_currentTrackMetadata;
  COhUPnPStateVariable<std::string> m_currentTrackURI;
  COhUPnPStateVariable<std::string> m_avTransportURI;
  std::string m_avTransportURIMetadata; // not serialized
  COhUPnPStateVariable<std::string> m_nextAVTransportURI;
  std::string m_nextAVTransportURIMetadata; // not serialized
  COhUPnPStateVariable<std::string> m_currentTransportActions;
  COhUPnPStateVariable<int32_t> m_relativeCounterPosition;
  COhUPnPStateVariable<uint32_t> m_absoluteCounterPosition;
  COhUPnPStateVariable<std::string> m_drmState;
};

class COhUPnPAVTransportLastChangeLock
{
public:
  explicit COhUPnPAVTransportLastChangeLock(COhUPnPAVTransportLastChange& lastChange)
    : m_lastChange(lastChange)
  {
    m_lastChange.Lock();
  }

  COhUPnPAVTransportLastChangeLock(const COhUPnPAVTransportLastChangeLock& other) = delete;

  ~COhUPnPAVTransportLastChangeLock()
  {
    m_lastChange.Unlock();
  }

  void operator=(COhUPnPAVTransportLastChangeLock const&) = delete;

private:
  COhUPnPAVTransportLastChange& m_lastChange;
};
