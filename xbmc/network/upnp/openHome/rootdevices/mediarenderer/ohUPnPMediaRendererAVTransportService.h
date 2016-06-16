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

#include <OpenHome/Net/Cpp/DvUpnpOrgAVTransport2.h>

#include "interfaces/IAnnouncer.h"
#include "network/upnp/openHome/ohUPnPService.h"
#include "network/upnp/openHome/rootdevices/ohUPnPRootDevice.h"
#include "network/upnp/openHome/utils/ohUPnPAVTransport.h"
#include "network/upnp/openHome/utils/ohUPnPAVTransportLastChange.h"

class CFileItem;
class COhUPnPClientDevice;

class COhUPnPMediaRendererAVTransportService : public IOhUPnPService
{
public:
  COhUPnPMediaRendererAVTransportService(COhUPnPRootDevice& device,
    const CFileItemElementFactory& fileItemElementFactory);
  virtual ~COhUPnPMediaRendererAVTransportService();

  void Start();
  void Stop();
  bool IsRunning() const;

  void UpdateState();

private:
  class AVTransport : public OpenHome::Net::DvProviderUpnpOrgAVTransport2Cpp, public ANNOUNCEMENT::IAnnouncer
  {
  public:
    AVTransport(COhUPnPMediaRendererAVTransportService& service, OpenHome::Net::DvDeviceStd& device);
    virtual ~AVTransport();

    void UpdateState();

  protected:
    // implementation of DvProviderUpnpOrgAVTransport2Cpp
    void SetAVTransportURI(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& currentURI, const std::string& currentURIMetaData) override;
    void SetNextAVTransportURI(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& nextURI, const std::string& nextURIMetaData) override;
    void GetMediaInfo(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, uint32_t& nrTracks, std::string& mediaDuration, std::string& currentURI,
      std::string& currentURIMetaData, std::string& nextURI, std::string& nextURIMetaData, std::string& playMedium, std::string& recordMedium, std::string& writeStatus) override;
    void GetTransportInfo(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& currentTransportState, std::string& currentTransportStatus, std::string& currentSpeed) override;
    void GetPositionInfo(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, uint32_t& track, std::string& trackDuration, std::string& trackMetaData, std::string& trackURI,
      std::string& relTime, std::string& absTime, int32_t& relCount, int32_t& absCount) override;
    void GetDeviceCapabilities(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& playMedia, std::string& recMedia, std::string& recQualityModes) override;
    void GetTransportSettings(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& playMode, std::string& recQualityMode) override;
    void Stop(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID) override;
    void Play(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& speed) override;
    void Pause(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID) override;
    void Seek(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& unit, const std::string& target) override;
    void Next(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID) override;
    void Previous(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID) override;
    void SetPlayMode(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& newPlayMode) override;
    void GetCurrentTransportActions(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& actions) override;
    void GetStateVariables(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& stateVariableList, std::string& stateVariableValuePairs) override;

    // implementation of IAnnouncer
    void Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data) override;

  private:
    void Play(const std::string& uri, const std::string& metadata, const COhUPnPClientDevice& device);
    bool GetFileItemFromMetadata(const std::string& uri, const std::string& metadata, const COhUPnPClientDevice& device, CFileItem& item) const;
    bool GetMetadataFromFileItem(const CFileItem& item, const COhUPnPClientDevice& device, std::string& metadata) const;

    void UpdateLastChange();

    COhUPnPMediaRendererAVTransportService& m_service;

    COhUPnPAVTransportLastChange m_variables;
  };

  friend class AVTransport;

  COhUPnPRootDevice& m_device;
  std::unique_ptr<AVTransport> m_avTransport;

  const CFileItemElementFactory& m_elementFactory;
};
