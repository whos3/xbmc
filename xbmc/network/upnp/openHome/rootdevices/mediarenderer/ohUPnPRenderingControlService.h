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

#include <OpenHome/Net/Cpp/DvUpnpOrgRenderingControl2.h>

#include "interfaces/IAnnouncer.h"
#include "network/upnp/openHome/ohUPnPService.h"
#include "network/upnp/openHome/rootdevices/ohUPnPRootDevice.h"
#include "threads/CriticalSection.h"

class COhUPnPRenderingControlService : public IOhUPnPService
{
public:
  COhUPnPRenderingControlService(COhUPnPRootDevice& device);
  virtual ~COhUPnPRenderingControlService();

  void Start();
  void Stop();
  bool IsRunning() const;

private:
  class RenderingControl : public OpenHome::Net::DvProviderUpnpOrgRenderingControl2Cpp, public ANNOUNCEMENT::IAnnouncer
  {
  public:
    RenderingControl(COhUPnPRenderingControlService& service, OpenHome::Net::DvDeviceStd& device);
    virtual ~RenderingControl();

  protected:
    // required actions
    void ListPresets(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& currentPresetNameList) override;
    void SelectPreset(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& presetName) override;

    // optional actions
    void GetMute(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, bool& currentMute) override;
    void SetMute(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, bool desiredMute) override;
    void GetVolume(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, uint32_t& currentVolume) override;
    void SetVolume(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, uint32_t desiredVolume) override;
    void GetVolumeDB(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, int32_t& currentVolume) override;
    void SetVolumeDB(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, int32_t desiredVolume) override;
    void GetVolumeDBRange(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, int32_t& minValue, int32_t& maxValue) override;

    void GetStateVariables(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& stateVariableList, std::string& stateVariableValuePairs) override;

    // implementation of IAnnouncer
    void Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data) override;

  private:
    COhUPnPRenderingControlService& m_service;

    // state variables
    // TODO: std::string m_lastChange;
    std::vector<std::string> m_presetNameList;
    bool m_muted;
    uint16_t m_volume;
    int16_t m_volumeDB;

    CCriticalSection m_critical;
  };

  friend class RenderingControl;

  COhUPnPRootDevice& m_device;
  std::unique_ptr<RenderingControl> m_renderingControl;
};
