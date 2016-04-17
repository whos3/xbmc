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

#include <openHome/Net/Cpp/CpProxy.h>

#include "ohUPnPConnectionManagerControlPoint.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

COhUPnPConnectionManagerControlPointManager::COhUPnPConnectionManagerControlPointManager(const std::string& deviceType /* = "" */)
  : IOhUPnPControlPointManager(deviceType, UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_CONNECTIONMANAGER, 2 /* TODO: 3 */, { 1 /* TODO: , 2 */ })
{ }

COhUPnPConnectionManagerControlPointManager::~COhUPnPConnectionManagerControlPointManager()
{ }

bool COhUPnPConnectionManagerControlPointManager::GetProtocolInfoSync(const std::string& uuid, std::string& sourceInfo, std::string& sinkInfo) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPConnectionManagerControlPointManager: trying to get protocol info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetProtocolInfo()");

    service.service->SyncGetProtocolInfo(sourceInfo, sinkInfo);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetProtocolInfo(): source = %s; sink = %s", sourceInfo.c_str(), sinkInfo.c_str());
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncGetProtocolInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncGetProtocolInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPConnectionManagerControlPointManager::GetProtocolInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPConnectionManagerControlPointManager: trying to get protocol info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetProtocolInfoAsync()");

    service.service->BeginGetProtocolInfo(callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling BeginGetProtocolInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling BeginGetProtocolInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPConnectionManagerControlPointManager::GetProtocolInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, std::string& sourceInfo, std::string& sinkInfo) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPConnectionManagerControlPointManager: trying to get protocol info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetProtocolInfoAsyncResult()");

    service.service->EndGetProtocolInfo(result, sourceInfo, sinkInfo);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetProtocolInfoAsyncResult(): source = %s; sink = %s", sourceInfo.c_str(), sinkInfo.c_str());
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling EndGetProtocolInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling EndGetProtocolInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

COhUPnPConnectionManagerControlPoint::COhUPnPConnectionManagerControlPoint(const COhUPnPConnectionManagerControlPointManager* manager,
  const std::string& uuid, IOhUPnPConnectionManagerControlPointAsync* callback)
  : IOhUPnPControlPointInstance(manager, uuid, callback)
  , m_profile()
{
  if (m_manager != nullptr)
  {
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(GetDevice(), m_profile))
      CLog::Log(LOGINFO, "COhUPnPAVTransportControlPoint: connection manager %s (%s) doesn't match any profiles", GetDevice().GetFriendlyName().c_str(), GetDevice().GetUuid().c_str());
    else
      CLog::Log(LOGINFO, "COhUPnPAVTransportControlPoint: connection manager %s (%s) matches profile %s", GetDevice().GetFriendlyName().c_str(), GetDevice().GetUuid().c_str(), m_profile.GetName().c_str());
  }
}

bool COhUPnPConnectionManagerControlPoint::GetProtocolInfoSync(std::string& sourceInfo, std::string& sinkInfo) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetProtocolInfoSync(m_uuid, sourceInfo, sinkInfo);
}

bool COhUPnPConnectionManagerControlPoint::GetProtocolInfoAsync()
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetProtocolInfoAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPConnectionManagerControlPoint::GetProtocolInfoCallback), m_uuid);
}

void COhUPnPConnectionManagerControlPoint::GetProtocolInfoCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  std::string sourceInfo, sinkInfo;
  bool success = m_manager->GetProtocolInfoAsyncResult(result, m_uuid, sourceInfo, sinkInfo);
  m_callback->GetProtocolInfoResult(success, sourceInfo, sinkInfo);
}

