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

#include "ohUPnPMediaRendererConnectionManagerService.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfilesManager.h"
#include "network/upnp/openHome/rootdevices/ohUPnPClientDevice.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

COhUPnPMediaRendererConnectionManagerService::COhUPnPMediaRendererConnectionManagerService(COhUPnPRootDevice& device)
  : IOhUPnPService(UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_CONNECTIONMANAGER, 2),
    m_device(device),
    m_connectionManager(nullptr)
{ }

COhUPnPMediaRendererConnectionManagerService::~COhUPnPMediaRendererConnectionManagerService()
{
  Stop();
}

void COhUPnPMediaRendererConnectionManagerService::Start()
{
  if (m_connectionManager != nullptr)
    return;

  // create a ConnectionManager service
  m_connectionManager.reset(new ConnectionManager(*this, *m_device.GetDevice()));
}

void COhUPnPMediaRendererConnectionManagerService::Stop()
{
  if (!IsRunning())
    return;

  // then destroy the ConnectionManager service
  m_connectionManager.reset();
}

bool COhUPnPMediaRendererConnectionManagerService::IsRunning() const
{
  return m_connectionManager != nullptr;
}

COhUPnPMediaRendererConnectionManagerService::ConnectionManager::ConnectionManager(COhUPnPMediaRendererConnectionManagerService& service, OpenHome::Net::DvDeviceStd& device)
  : OpenHome::Net::DvProviderUpnpOrgConnectionManager2Cpp(device),
    m_service(service)
{
  COhUPnPDeviceProfile profile;
  COhUPnPDeviceProfilesManager::GetInstance().FindProfile(service.m_device, profile);

  EnablePropertySourceProtocolInfo();
  SetPropertySourceProtocolInfo("");
  EnablePropertySinkProtocolInfo();
  SetPropertySinkProtocolInfo(profile.GetProtocolInfo());
  EnablePropertyCurrentConnectionIDs();
  SetPropertyCurrentConnectionIDs("0");

  EnableActionGetProtocolInfo();
  EnableActionGetCurrentConnectionIDs();
  EnableActionGetCurrentConnectionInfo();
}

COhUPnPMediaRendererConnectionManagerService::ConnectionManager::~ConnectionManager()
{ }

void COhUPnPMediaRendererConnectionManagerService::ConnectionManager::GetProtocolInfo(OpenHome::Net::IDvInvocationStd& invocation, std::string& source, std::string& sink)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetProtocolInfo()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: GetProtocolInfo() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  // get a (matching) profile
  COhUPnPDeviceProfile profile;
  if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(clientDevice, profile))
    CLog::Log(LOGINFO, "COhUPnPMediaRendererConnectionManagerService::GetProtocolInfo: client doesn't match any profiles");
  else
    CLog::Log(LOGINFO, "COhUPnPMediaRendererConnectionManagerService::GetProtocolInfo: client matches profile %s", profile.GetName().c_str());

  GetPropertySourceProtocolInfo(source);
  sink = profile.GetProtocolInfo();

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetProtocolInfo(): source = %s; sink = %s", source.c_str(), sink.c_str());
}

void COhUPnPMediaRendererConnectionManagerService::ConnectionManager::PrepareForConnection(OpenHome::Net::IDvInvocationStd& invocation, const std::string& remoteProtocolInfo,
  const std::string& peerConnectionManager, int32_t peerConnectionID, const std::string& direction, int32_t& connectionID, int32_t& avTransportID, int32_t& rcsID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] <-- PrepareForConnection(%s, %s, %d, %s)",
      remoteProtocolInfo.c_str(), peerConnectionManager.c_str(), peerConnectionID, direction);
  }

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: PrepareForConnection(%s, %s, %d, %s) from %s (version %u; user-agent: %s)",
    remoteProtocolInfo.c_str(), peerConnectionManager.c_str(), peerConnectionID, direction,
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: PrepareForConnection is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> PrepareForConnection(%s, %s, %d, %s): connection ID = %d; AVTransport ID = %d; RCS ID = %d",
      remoteProtocolInfo.c_str(), peerConnectionManager.c_str(), peerConnectionID, direction, connectionID, avTransportID, rcsID);
  }
}

void COhUPnPMediaRendererConnectionManagerService::ConnectionManager::ConnectionComplete(OpenHome::Net::IDvInvocationStd& invocation, int32_t connectionID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- ConnectionComplete(%d)", connectionID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: ConnectionComplete(%d) from %s (version %u; user-agent: %s)",
    connectionID, COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: ConnectionComplete is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> ConnectionComplete(%d)", connectionID);
}

void COhUPnPMediaRendererConnectionManagerService::ConnectionManager::GetCurrentConnectionIDs(OpenHome::Net::IDvInvocationStd& invocation, std::string& connectionIDs)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetCurrentConnectionIDs()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: GetCurrentConnectionIDs() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  GetPropertyCurrentConnectionIDs(connectionIDs);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetCurrentConnectionIDs(): connection IDs = %s", connectionIDs.c_str());
}

void COhUPnPMediaRendererConnectionManagerService::ConnectionManager::GetCurrentConnectionInfo(OpenHome::Net::IDvInvocationStd& invocation, int32_t connectionID, int32_t& rcsID,
  int32_t& avTransportID, std::string& protocolInfo, std::string& peerConnectionManager, int32_t& peerConnectionID, std::string& direction, std::string& status)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetCurrentConnectionInfo(%d)", connectionID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: GetCurrentConnectionInfo(%d) from %s (version %u; user-agent: %s)",
    connectionID, COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  std::string connectionIDs;
  GetPropertyCurrentConnectionIDs(connectionIDs);

  // we only support a single connection
  if (connectionID != 0 || connectionIDs.empty())
  {
    CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice::GetCurrentConnectionInfo: invalid connection ID %d", connectionID);
    invocation.ReportError(UPNP_ERROR_CM_INVALID_CONNECTION_REFERENCE, "Invalid connection reference");
    return;
  }

  rcsID = 0;
  avTransportID = 0;
  GetPropertySourceProtocolInfo(protocolInfo);
  peerConnectionManager = "";
  peerConnectionID = -1;
  direction = "Input";
  status = "Unknown";

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> GetCurrentConnectionInfo(%d): RCS ID = %d; AVTransport ID = %d; protocol info = %s; peer connection manager = %s; peer connection ID = %d; direction = %s; status = %s",
      connectionID, rcsID, avTransportID, protocolInfo.c_str(), peerConnectionManager.c_str(), peerConnectionID, direction.c_str(), status.c_str());
  }
}
