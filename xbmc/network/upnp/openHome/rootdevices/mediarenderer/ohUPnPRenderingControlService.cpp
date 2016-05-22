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

#include "ohUPnPRenderingControlService.h"
#include "Application.h"
#include "interfaces/AnnouncementManager.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/rootdevices/ohUPnPClientDevice.h"
#include "network/upnp/openHome/utils/ohUPnPRenderingControl.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

COhUPnPRenderingControlService::COhUPnPRenderingControlService(COhUPnPRootDevice& device)
  : IOhUPnPService(UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_RENDERINGCONTROL, 2)
  , m_device(device)
  , m_renderingControl(nullptr)
{ }

COhUPnPRenderingControlService::~COhUPnPRenderingControlService()
{
  Stop();
}

void COhUPnPRenderingControlService::Start()
{
  if (m_renderingControl != nullptr)
    return;

  // create a ContentDirectory service
  m_renderingControl.reset(new RenderingControl(*this, *m_device.GetDevice()));
}

void COhUPnPRenderingControlService::Stop()
{
  if (!IsRunning())
    return;

  // then destroy the ContentDirectory service
  m_renderingControl.reset();
}

bool COhUPnPRenderingControlService::IsRunning() const
{
  return m_renderingControl != nullptr;
}

COhUPnPRenderingControlService::RenderingControl::RenderingControl(COhUPnPRenderingControlService& service, OpenHome::Net::DvDeviceStd& device)
  : OpenHome::Net::DvProviderUpnpOrgRenderingControl2Cpp(device)
  , m_service(service)
  , m_presetNameList({ "FactoryDefaults" })
  , m_muted(false)
  , m_volume(100)
  , m_volumeDB(0)
{
  // enable required actions
  EnableActionListPresets();
  EnableActionSelectPreset();

  // enable optional actions
  EnableActionGetMute();
  EnableActionSetMute();
  EnableActionGetVolume();
  EnableActionSetVolume();
  EnableActionGetVolumeDB();
  EnableActionSetVolumeDB();
  EnableActionGetVolumeDBRange();

  EnableActionGetStateVariables();

  ANNOUNCEMENT::CAnnouncementManager::GetInstance().AddAnnouncer(this);
}

COhUPnPRenderingControlService::RenderingControl::~RenderingControl()
{
  ANNOUNCEMENT::CAnnouncementManager::GetInstance().RemoveAnnouncer(this);
}

void COhUPnPRenderingControlService::RenderingControl::ListPresets(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& currentPresetNameList)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- ListPresets(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: ListPresets() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  currentPresetNameList = COhUtils::ToCSV(m_presetNameList);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> ListPresets(%u): current presets = %s", instanceID, currentPresetNameList.c_str());
}

void COhUPnPRenderingControlService::RenderingControl::SelectPreset(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& presetName)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- SelectPreset(%u, %s)", instanceID, presetName.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: SelectPreset() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  if (std::find(m_presetNameList.cbegin(), m_presetNameList.cend(), presetName) == m_presetNameList.cend())
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: trying to set invalid preset \"%s\"", presetName.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_PRESET_NAME, "Invalid Name");
    return;
  }

  // TODO
}

void COhUPnPRenderingControlService::RenderingControl::GetMute(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, bool& currentMute)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetMute(%u, %s)", instanceID, channel.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: GetMute() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPRenderingControlChannel channelValue(channel);
  if (channelValue != OhUPnPRenderingControlChannel::Master)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid channel \"%s\" referenced", channel.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_CHANNEL, "Invalid Channel");
    return;
  }

  currentMute = g_application.IsMuted();

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetMute(%u, %s): current mute = %s", instanceID, channel.c_str(), (currentMute ? "true" : "false"));
}

void COhUPnPRenderingControlService::RenderingControl::SetMute(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, bool desiredMute)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- SetMute(%u, %s, %s)", instanceID, channel.c_str(), (desiredMute ? "true" : "false"));

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: SetMute() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPRenderingControlChannel channelValue(channel);
  if (channelValue != OhUPnPRenderingControlChannel::Master)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid channel \"%s\" referenced", channel.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_CHANNEL, "Invalid Channel");
    return;
  }

  g_application.SetMute(desiredMute);
}

void COhUPnPRenderingControlService::RenderingControl::GetVolume(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, uint32_t& currentVolume)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolume(%u, %s)", instanceID, channel.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: GetVolume() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPRenderingControlChannel channelValue(channel);
  if (channelValue != OhUPnPRenderingControlChannel::Master)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid channel \"%s\" referenced", channel.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_CHANNEL, "Invalid Channel");
    return;
  }

  currentVolume = static_cast<uint32_t>(g_application.GetVolume(true));

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetVolume(%u, %s): current volume = %u", instanceID, channel.c_str(), currentVolume);
}

void COhUPnPRenderingControlService::RenderingControl::SetVolume(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, uint32_t desiredVolume)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- SetVolume(%u, %s, %u)", instanceID, channel.c_str(), desiredVolume);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: SetVolume() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPRenderingControlChannel channelValue(channel);
  if (channelValue != OhUPnPRenderingControlChannel::Master)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid channel \"%s\" referenced", channel.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_CHANNEL, "Invalid Channel");
    return;
  }

  g_application.SetVolume(static_cast<float>(desiredVolume), true);
}

void COhUPnPRenderingControlService::RenderingControl::GetVolumeDB(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, int32_t& currentVolume)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDB(%u, %s)", instanceID, channel.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: GetVolumeDB() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPRenderingControlChannel channelValue(channel);
  if (channelValue != OhUPnPRenderingControlChannel::Master)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid channel \"%s\" referenced", channel.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_CHANNEL, "Invalid Channel");
    return;
  }

  currentVolume = static_cast<int32_t>(g_application.GetVolume(false));

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetVolumeDB(%u, %s): current vollume = %d dB", instanceID, channel.c_str(), currentVolume);
}

void COhUPnPRenderingControlService::RenderingControl::SetVolumeDB(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, int32_t desiredVolume) 
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- SetVolumeDB(%u, %s, %d)", instanceID, channel.c_str(), desiredVolume);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: SetVolumeDB() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPRenderingControlChannel channelValue(channel);
  if (channelValue != OhUPnPRenderingControlChannel::Master)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid channel \"%s\" referenced", channel.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_CHANNEL, "Invalid Channel");
    return;
  }

  g_application.SetVolume(static_cast<float>(desiredVolume), false);
}

void COhUPnPRenderingControlService::RenderingControl::GetVolumeDBRange(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& channel, int32_t& minValue, int32_t& maxValue)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDBRange(%u, %s)", instanceID, channel.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: GetVolumeDBRange() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPRenderingControlChannel channelValue(channel);
  if (channelValue != OhUPnPRenderingControlChannel::Master)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid channel \"%s\" referenced", channel.c_str());
    invocation.ReportError(UPNP_ERROR_RC_INVALID_CHANNEL, "Invalid Channel");
    return;
  }

  minValue = -60; // TODO
  maxValue = 0;

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetVolumeDBRange(%u, %s): min value = %d dB; max value = %d dB", instanceID, channel.c_str(), minValue, maxValue);
}

void COhUPnPRenderingControlService::RenderingControl::GetStateVariables(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& stateVariableList,
  std::string& stateVariableValuePairs)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetStateVariables(%u, %s)", instanceID, stateVariableList.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: GetStateVariables() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  CSingleLock lock(m_critical);

  const auto& stateVariables = COhUtils::SplitCSV(stateVariableList);
  std::vector<std::string> stateVariableValues;
  for (const auto& stateVariable : stateVariables)
  {
    std::string value;
    if (stateVariable == "PresetNameList")
      value = COhUtils::ToCSV(m_presetNameList); // TODO
    else if (stateVariable == "Mute")
      value = m_muted ? "true" : "false";
    else if (stateVariable == "Volume")
      value = StringUtils::Format("%hu", m_volume);
    else if (stateVariable == "VolumeDB")
      value = StringUtils::Format("%hd", m_volumeDB);
    else
    {
      CLog::Log(LOGDEBUG, "COhUPnPRenderingControlService: unknown state variable \"%s\" requested", stateVariable.c_str());
      continue;
    }

    stateVariableValues.push_back(StringUtils::Format("%s=%s", stateVariable.c_str(), value.c_str()));
  }

  stateVariableValuePairs = COhUtils::ToCSV(stateVariableValues);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetStateVariables(%u, %s): values = %s", instanceID, stateVariableList.c_str(), stateVariableValuePairs.c_str());
}

void COhUPnPRenderingControlService::RenderingControl::Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data)
{
  // ignore non-xbmc announcements
  if (strcmp(sender, "xbmc") != 0)
    return;

  CSingleLock lock(m_critical);

  if (flag == ANNOUNCEMENT::Application)
  {
    if (strcmp(message, "OnVolumeChanged") == 0)
    {
      m_volume = static_cast<uint16_t>(data["volume"].asUnsignedInteger());
      m_volumeDB = 256 * (m_volume * 60 - 60) / 100;
      m_muted = data["muted"].asBoolean();
    }
  }
}
