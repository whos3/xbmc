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

#include <memory>
#include <string>
#include <vector>

#include <OpenHome/Net/Cpp/CpUpnpOrgAVTransport2.h>

#include "network/upnp/openHome/controlpoints/ohUPnPControlPoint.h"
#include "network/upnp/openHome/utils/ohUPnPAVTransport.h"
#include "threads/CriticalSection.h"

class CDateTimeSpan;

class COhUPnPAVTransportControlPoint;

class IOhUPnPAVTransportControlPointAsync
{
public:
  virtual ~IOhUPnPAVTransportControlPointAsync() = default;

  virtual void SetAVTransportURIResult(bool success) { }
  virtual void SetNextAVTransportURIResult(bool success) { }
  virtual void GetMediaInfoResult(bool success, uint32_t numberOfTracks, const CDateTimeSpan& duration, const std::string& currentUri, const std::string& currentUriMetadata,
    const std::string& nextUri, const std::string& nextUriMetadata, const COhUPnPAVTransportStorageMedium& playbackMedium,
    const COhUPnPAVTransportStorageMedium& recordMedium, const COhUPnPAVTransportWriteStatus& writeStatus) { }
  virtual void GetTransportInfoResult(bool success, const COhUPnPAVTransportTransportState& transportState,
    const COhUPnPAVTransportTransportStatus& transportStatus, const std::string& speed) { }
  virtual void GetPositionInfoResult(bool success, uint32_t track, const CDateTimeSpan& duration, const std::string& metadata, const std::string& uri,
    const CDateTimeSpan& relativeTime, const CDateTimeSpan& absoluteTime, int32_t relativeCount, int32_t absoluteCount) { }
  virtual void GetDeviceCapabilitiesResult(bool success, const std::vector<COhUPnPAVTransportStorageMedium>& playbackMedia,
    const std::vector<COhUPnPAVTransportStorageMedium>& recordingMedia, const std::vector<COhUPnPAVTransportRecordQualityMode>& recordingQualityModes) { };
  virtual void GetTransportSettingsResult(bool success, const COhUPnPAVTransportPlayMode& playMode, const COhUPnPAVTransportRecordQualityMode& recordingQualityMode) { }
  virtual void StopResult(bool success) { }
  virtual void PlayResult(bool success) { }
  virtual void PauseResult(bool success) { }
  virtual void SeekResult(bool success) { }
  virtual void NextResult(bool success) { }
  virtual void PreviousResult(bool success) { }
  virtual void SetPlayModeResult(bool success) { }
  virtual void GetCurrentTransportActionsResult(bool success, const std::vector<COhUPnPAVTransportTransportAction>& actions) { }
  // TODO: virtual void SetStaticPlaylistResult(bool success, TODO) { }
  // TODO: virtual void GetPlaylistInfoResult(bool success, TODO) { }

protected:
  IOhUPnPAVTransportControlPointAsync() = default;
};

class COhUPnPAVTransportControlPointManager
  : public IOhUPnPControlPointManager<OpenHome::Net::CpProxyUpnpOrgAVTransport2Cpp, COhUPnPAVTransportControlPointManager, COhUPnPAVTransportControlPoint, IOhUPnPAVTransportControlPointAsync>
{
public:
  COhUPnPAVTransportControlPointManager(const std::string& deviceType);
  virtual ~COhUPnPAVTransportControlPointManager();

protected:
  friend class COhUPnPAVTransportControlPoint;

  // specialization of IOhUPnPControlPoint
  void OnServiceAdded(const UPnPControlPointService &service) override;
  void OnServiceRemoved(const UPnPControlPointService &service) override;

  bool SetAVTransportURISync(const std::string& uuid, const std::string& uri, const std::string& uriMetadata, uint32_t instanceId = 0) const;
  bool SetAVTransportURIAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const std::string& uri, const std::string& uriMetadata, uint32_t instanceId = 0) const;
  bool SetAVTransportURIAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool SetNextAVTransportURISync(const std::string& uuid, const std::string& nextUri, const std::string& nextUriMetadata, uint32_t instanceId = 0) const;
  bool SetNextAVTransportURIAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const std::string& nextUri, const std::string& nextUriMetadata, uint32_t instanceId = 0) const;
  bool SetNextAVTransportURIAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool GetMediaInfoSync(const std::string& uuid, uint32_t& numberOfTracks, CDateTimeSpan& duration, std::string& currentUri, std::string& currentUriMetadata,
    std::string& nextUri, std::string& nextUriMetadata, COhUPnPAVTransportStorageMedium& playbackMedium,
    COhUPnPAVTransportStorageMedium& recordMedium, COhUPnPAVTransportWriteStatus& writeStatus, uint32_t instanceId = 0) const;
  bool GetMediaInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool GetMediaInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, uint32_t& numberOfTracks, CDateTimeSpan& duration,
    std::string& currentUri, std::string& currentUriMetadata, std::string& nextUri, std::string& nextUriMetadata,
    COhUPnPAVTransportStorageMedium& playbackMedium, COhUPnPAVTransportStorageMedium& recordMedium, COhUPnPAVTransportWriteStatus& writeStatus) const;

  bool GetTransportInfoSync(const std::string& uuid, COhUPnPAVTransportTransportState& transportState, COhUPnPAVTransportTransportStatus& transportStatus,
    std::string& speed, uint32_t instanceId = 0) const;
  bool GetTransportInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool GetTransportInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, COhUPnPAVTransportTransportState& transportState,
    COhUPnPAVTransportTransportStatus& transportStatus, std::string& speed) const;

  bool GetPositionInfoSync(const std::string& uuid, uint32_t& track, CDateTimeSpan& duration, std::string& metadata, std::string& uri,
    CDateTimeSpan& relativeTime, CDateTimeSpan& absoluteTime, int32_t& relativeCount, int32_t& absoluteCount, uint32_t instanceId = 0) const;
  bool GetPositionInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool GetPositionInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, uint32_t& track, CDateTimeSpan& duration, std::string& metadata,
    std::string& uri, CDateTimeSpan& relativeTime, CDateTimeSpan& absoluteTime, int32_t& relativeCount, int32_t& absoluteCount) const;

  bool GetDeviceCapabilitiesSync(const std::string& uuid, std::vector<COhUPnPAVTransportStorageMedium>& playbackMedia,
    std::vector<COhUPnPAVTransportStorageMedium>& recordingMedia, std::vector<COhUPnPAVTransportRecordQualityMode>& recordingQualityModes, uint32_t instanceId = 0) const;
  bool GetDeviceCapabilitiesAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool GetDeviceCapabilitiesAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, std::vector<COhUPnPAVTransportStorageMedium>& playbackMedia,
    std::vector<COhUPnPAVTransportStorageMedium>& recordingMedia, std::vector<COhUPnPAVTransportRecordQualityMode>& recordingQualityModes) const;

  bool GetTransportSettingsSync(const std::string& uuid, COhUPnPAVTransportPlayMode& playMode, COhUPnPAVTransportRecordQualityMode& recordingQualityMode, uint32_t instanceId = 0) const;
  bool GetTransportSettingsAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool GetTransportSettingsAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, COhUPnPAVTransportPlayMode& playMode,
    COhUPnPAVTransportRecordQualityMode& recordingQualityMode) const;

  bool StopSync(const std::string& uuid, uint32_t instanceId = 0) const;
  bool StopAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool StopAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool PlaySync(const std::string& uuid, const std::string& speed = "1", uint32_t instanceId = 0) const;
  bool PlayAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const std::string& speed = "1", uint32_t instanceId = 0) const;
  bool PlayAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool PauseSync(const std::string& uuid, uint32_t instanceId = 0) const;
  bool PauseAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool PauseAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool SeekSync(const std::string& uuid, const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId = 0) const;
  bool SeekAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId = 0) const;
  bool SeekAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool NextSync(const std::string& uuid, uint32_t instanceId = 0) const;
  bool NextAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool NextAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool PreviousSync(const std::string& uuid, uint32_t instanceId = 0) const;
  bool PreviousAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool PreviousAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool SetPlayModeSync(const std::string& uuid, const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId = 0) const;
  bool SetPlayModeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId = 0) const;
  bool SetPlayModeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool GetCurrentTransportActionsSync(const std::string& uuid, std::vector<COhUPnPAVTransportTransportAction>& actions, uint32_t instanceId = 0) const;
  bool GetCurrentTransportActionsAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId = 0) const;
  bool GetCurrentTransportActionsAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, std::vector<COhUPnPAVTransportTransportAction>& actions) const;

  // TODO: bool SetStaticPlaylistSync(const std::string& uuid, TODO) const;
  // TODO: bool SetStaticPlaylistAsync(const std::string& uuid, TODO) const;
  // TODO: bool SetStaticPlaylistAsyncResult(const std::string& uuid, TODO) const;

  // TODO: bool GetPlaylistInfoSync(const std::string& uuid, TODO) const;
  // TODO: bool GetPlaylistInfoAsync(const std::string& uuid, TODO) const;
  // TODO: bool GetPlaylistInfoAsyncResult(const std::string& uuid, TODO) const;

private:
  void UnregisterRenderer(const std::string& rendererUuid);
};

class COhUPnPAVTransportControlPoint : public IOhUPnPControlPointInstance<COhUPnPAVTransportControlPointManager, IOhUPnPAVTransportControlPointAsync>
{
public:
  virtual ~COhUPnPAVTransportControlPoint() = default;

  // implementation of IOhUPnPControlPointInstance
  const COhUPnPDeviceProfile& GetProfile() const override { return m_profile; }

  bool SetAVTransportURISync(const std::string& uri, const std::string& uriMetadata, uint32_t instanceId = 0) const;
  bool SetAVTransportURIAsync(const std::string& uri, const std::string& uriMetadata, uint32_t instanceId = 0);

  bool SetNextAVTransportURISync(const std::string& nextUri, const std::string& nextUriMetadata, uint32_t instanceId = 0) const;
  bool SetNextAVTransportURIAsync(const std::string& nextUri, const std::string& nextUriMetadata, uint32_t instanceId = 0);

  bool GetMediaInfoSync(uint32_t& numberOfTracks, CDateTimeSpan& duration, std::string& currentUri, std::string& currentUriMetadata,
    std::string& nextUri, std::string& nextUriMetadata, COhUPnPAVTransportStorageMedium& playbackMedium, COhUPnPAVTransportStorageMedium& recordMedium,
    COhUPnPAVTransportWriteStatus& writeStatus, uint32_t instanceId = 0) const;
  bool GetMediaInfoAsync(uint32_t instanceId = 0);

  bool GetTransportInfoSync(COhUPnPAVTransportTransportState& transportState, COhUPnPAVTransportTransportStatus& transportStatus,
    std::string& speed, uint32_t instanceId = 0) const;
  bool GetTransportInfoAsync(uint32_t instanceId = 0);

  bool GetPositionInfoSync(uint32_t& track, CDateTimeSpan& duration, std::string& metadata, std::string& uri,
    CDateTimeSpan& relativeTime, CDateTimeSpan& absoluteTime, int32_t& relativeCount, int32_t& absoluteCount, uint32_t instanceId = 0) const;
  bool GetPositionInfoAsync(uint32_t instanceId = 0);

  bool GetDeviceCapabilitiesSync(std::vector<COhUPnPAVTransportStorageMedium>& playbackMedia, std::vector<COhUPnPAVTransportStorageMedium>& recordingMedia,
    std::vector<COhUPnPAVTransportRecordQualityMode>& recordingQualityModes, uint32_t instanceId = 0) const;
  bool GetDeviceCapabilitiesAsync(uint32_t instanceId = 0);

  bool GetTransportSettingsSync(COhUPnPAVTransportPlayMode& playMode, COhUPnPAVTransportRecordQualityMode& recordingQualityMode, uint32_t instanceId = 0) const;
  bool GetTransportSettingsAsync(uint32_t instanceId = 0);

  bool StopSync(uint32_t instanceId = 0) const;
  bool StopAsync(uint32_t instanceId = 0);

  bool PlaySync(const std::string& speed = "1", uint32_t instanceId = 0) const;
  bool PlayAsync(const std::string& speed = "1", uint32_t instanceId = 0);

  bool PauseSync(uint32_t instanceId = 0) const;
  bool PauseAsync(uint32_t instanceId = 0);

  bool SeekSync(const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId = 0) const;
  bool SeekAsync(const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId = 0);

  bool NextSync(uint32_t instanceId = 0) const;
  bool NextAsync(uint32_t instanceId = 0);

  bool PreviousSync(uint32_t instanceId = 0) const;
  bool PreviousAsync(uint32_t instanceId = 0);

  bool SetPlayModeSync(const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId = 0) const;
  bool SetPlayModeAsync(const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId = 0);

  bool GetCurrentTransportActionsSync(std::vector<COhUPnPAVTransportTransportAction>& actions, uint32_t instanceId = 0) const;
  bool GetCurrentTransportActionsAsync(uint32_t instanceId = 0);

  // TODO: bool SetStaticPlaylistSync(TODO, uint32_t instanceId = 0) const;
  // TODO: bool SetStaticPlaylistAsync(TODO, uint32_t instanceId = 0);

  // TODO: bool GetPlaylistInfoSync(TODO, uint32_t instanceId = 0) const;
  // TODO: bool GetPlaylistInfoAsync(TODO, uint32_t instanceId = 0);

protected:
  friend class IOhUPnPControlPointManager<OpenHome::Net::CpProxyUpnpOrgAVTransport2Cpp, COhUPnPAVTransportControlPointManager, COhUPnPAVTransportControlPoint, IOhUPnPAVTransportControlPointAsync>;

  COhUPnPAVTransportControlPoint(const COhUPnPAVTransportControlPointManager* manager,
    const std::string& uuid, IOhUPnPAVTransportControlPointAsync* callback);

  void SetAVTransportURICallback(OpenHome::Net::IAsync& result);
  void SetNextAVTransportURICallback(OpenHome::Net::IAsync& result);
  void GetMediaInfoCallback(OpenHome::Net::IAsync& result);
  void GetTransportInfoCallback(OpenHome::Net::IAsync& result);
  void GetPositionInfoCallback(OpenHome::Net::IAsync& result);
  void GetDeviceCapabilitiesCallback(OpenHome::Net::IAsync& result);
  void GetTransportSettingsCallback(OpenHome::Net::IAsync& result);
  void StopCallback(OpenHome::Net::IAsync& result);
  void PlayCallback(OpenHome::Net::IAsync& result);
  void PauseCallback(OpenHome::Net::IAsync& result);
  void SeekCallback(OpenHome::Net::IAsync& result);
  void NextCallback(OpenHome::Net::IAsync& result);
  void PreviousCallback(OpenHome::Net::IAsync& result);
  void SetPlayModeCallback(OpenHome::Net::IAsync& result);
  void GetCurrentTransportActionsCallback(OpenHome::Net::IAsync& result);
  // TODO: void SetStaticPlaylistCallback(OpenHome::Net::IAsync& result);
  // TODO: void GetPlaylistInfoCallback(OpenHome::Net::IAsync& result);

private:
  COhUPnPDeviceProfile m_profile;
};
