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

#include "ohUPnPAVTransportControlPoint.h"
#include "cores/PlayerCoreFactory/PlayerCoreFactory.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "settings/AdvancedSettings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

COhUPnPAVTransportControlPointManager::COhUPnPAVTransportControlPointManager(const std::string& deviceType)
  : IOhUPnPControlPointManager(deviceType, UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_AVTRANSPORT, 3, { 1, 2 })
{ }

COhUPnPAVTransportControlPointManager::~COhUPnPAVTransportControlPointManager()
{
  // unregister all renderers
  std::vector<COhUPnPControlPointDevice> devices = GetDevices();
  for (const auto& device : devices)
    UnregisterRenderer(device.GetUuid());
}

void COhUPnPAVTransportControlPointManager::OnServiceAdded(const UPnPControlPointService &service)
{
  // register the renderer instance with the player core factory
  CPlayerCoreFactory::GetInstance().OnPlayerDiscovered(service.device.GetUuid(), service.device.GetFriendlyName());

  IOhUPnPControlPointManager::OnServiceAdded(service);
}

void COhUPnPAVTransportControlPointManager::OnServiceRemoved(const UPnPControlPointService &service)
{
  // unregister the renderer
  UnregisterRenderer(service.device.GetUuid());

  IOhUPnPControlPointManager::OnServiceRemoved(service);
}

bool COhUPnPAVTransportControlPointManager::SetAVTransportURISync(const std::string& uuid, const std::string& uri, const std::string& uriMetadata, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set AVTransport URI on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetAVTransportURI(%u, %s, %s)", instanceId, uri.c_str(), uriMetadata.c_str());

    service.service->SyncSetAVTransportURI(instanceId, uri, uriMetadata);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncSetAVTransportURI(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncSetAVTransportURI(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetAVTransportURIAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const std::string& uri, const std::string& uriMetadata, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set AVTransport URI on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetAVTransportURIAsync(%u, %s, %s)", instanceId, uri.c_str(), uriMetadata.c_str());

    service.service->BeginSetAVTransportURI(instanceId, uri, uriMetadata, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginSetAVTransportURI(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginSetAVTransportURI(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetAVTransportURIAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set AVTransport URI on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- SetAVTransportURIAsyncResult()");

    service.service->EndSetAVTransportURI(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndSetAVTransportURI(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndSetAVTransportURI(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetNextAVTransportURISync(const std::string& uuid, const std::string& nextUri, const std::string& nextUriMetadata, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set next AVTransport URI on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetNextAVTransportURI(%u, %s, %s)", instanceId, nextUri.c_str(), nextUriMetadata.c_str());

    service.service->SyncSetNextAVTransportURI(instanceId, nextUri, nextUriMetadata);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncSetNextAVTransportURI(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncSetNextAVTransportURI(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetNextAVTransportURIAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const std::string& uri, const std::string& uriMetadata, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set next AVTransport URI on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetNextAVTransportURIAsync(%u, %s, %s)", instanceId, uri.c_str(), uriMetadata.c_str());

    service.service->BeginSetNextAVTransportURI(instanceId, uri, uriMetadata, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginSetNextAVTransportURI(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginSetNextAVTransportURI(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetNextAVTransportURIAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set next AVTransport URI on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- SetNextAVTransportURIAsyncResult()");

    service.service->EndSetNextAVTransportURI(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndSetNextAVTransportURI(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndSetNextAVTransportURI(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetMediaInfoSync(const std::string& uuid, uint32_t& numberOfTracks, CDateTimeSpan& duration, std::string& currentUri, std::string& currentUriMetadata,
  std::string& nextUri, std::string& nextUriMetadata, COhUPnPAVTransportStorageMedium& playbackMedium, COhUPnPAVTransportStorageMedium& recordMedium, COhUPnPAVTransportWriteStatus& writeStatus, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get media info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetMediaInfo(%u)", instanceId);

    std::string strDuration, strPlaybackMedium, strRecordMedium, strWriteStatus;
    service.service->SyncGetMediaInfo(instanceId, numberOfTracks, strDuration, currentUri, currentUriMetadata,
      nextUri, nextUriMetadata, strPlaybackMedium, strRecordMedium, strWriteStatus);

    duration = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strDuration)));
    playbackMedium.SetFromString(strPlaybackMedium);
    recordMedium.SetFromString(strRecordMedium);
    writeStatus.SetFromString(strWriteStatus);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetMediaInfo(%u): # tracks = %u; duration = %s; URI = %s; URI metadata = %s; next URI = %s; next URI metadata = %s; playback medium = %s; record medium = %s; write status = %s",
        instanceId, numberOfTracks, COhUtils::GetDurationFromSeconds(duration.GetSecondsTotal()).c_str(), currentUri.c_str(), currentUriMetadata.c_str(),
        nextUri.c_str(), nextUriMetadata.c_str(), playbackMedium.GetAsString().c_str(), recordMedium.GetAsString().c_str(), writeStatus.GetAsString().c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncGetMediaInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncGetMediaInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetMediaInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get media info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetMediaInfoAsync(%u)", instanceId);

    service.service->BeginGetMediaInfo(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginGetMediaInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginGetMediaInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetMediaInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, uint32_t& numberOfTracks, CDateTimeSpan& duration, std::string& currentUri, std::string& currentUriMetadata,
  std::string& nextUri, std::string& nextUriMetadata, COhUPnPAVTransportStorageMedium& playbackMedium, COhUPnPAVTransportStorageMedium& recordMedium, COhUPnPAVTransportWriteStatus& writeStatus) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get media info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetMediaInfoAsyncResult()");

    std::string strDuration, strPlaybackMedium, strRecordMedium, strWriteStatus;
    service.service->EndGetMediaInfo(result, numberOfTracks, strDuration, currentUri, currentUriMetadata,
      nextUri, nextUriMetadata, strPlaybackMedium, strRecordMedium, strWriteStatus);

    duration = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strDuration)));
    playbackMedium.SetFromString(strPlaybackMedium);
    recordMedium.SetFromString(strRecordMedium);
    writeStatus.SetFromString(strWriteStatus);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetMediaInfoAsyncResult(): # tracks = %u; duration = %s; URI = %s; URI metadata = %s; next URI = %s; next URI metadata = %s; playback medium = %s; record medium = %s; write status = %s",
        numberOfTracks, COhUtils::GetDurationFromSeconds(duration.GetSecondsTotal()).c_str(), currentUri.c_str(), currentUriMetadata.c_str(),
        nextUri.c_str(), nextUriMetadata.c_str(), playbackMedium.GetAsString().c_str(), recordMedium.GetAsString().c_str(), writeStatus.GetAsString().c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndGetMediaInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndGetMediaInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetTransportInfoSync(const std::string& uuid, COhUPnPAVTransportTransportState& transportState,
  COhUPnPAVTransportTransportStatus& transportStatus, std::string& speed, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get transport info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetTransportInfo(%u)", instanceId);

    std::string strTransportState, strTransportStatus;
    service.service->SyncGetTransportInfo(instanceId, strTransportState, strTransportStatus, speed);

    transportState.SetFromString(strTransportState);
    transportStatus.SetFromString(strTransportStatus);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportInfo(%u): state = %s; status = %s; speed = %s",
        instanceId, transportState.GetAsString().c_str(), transportStatus.GetAsString().c_str(), speed.c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncGetTransportInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncGetTransportInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetTransportInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get transport info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetTransportInfoAsync(%u)", instanceId);

    service.service->BeginGetTransportInfo(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginGetTransportInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginGetTransportInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetTransportInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, COhUPnPAVTransportTransportState& transportState,
  COhUPnPAVTransportTransportStatus& transportStatus, std::string& speed) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get transport info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportInfoAsyncResult()");

    std::string strTransportState, strTransportStatus;
    service.service->EndGetTransportInfo(result, strTransportState, strTransportStatus, speed);

    transportState.SetFromString(strTransportState);
    transportStatus.SetFromString(strTransportStatus);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportInfoAsyncResult(): state = %s; status = %s; speed = %s",
        transportState.GetAsString().c_str(), transportStatus.GetAsString().c_str(), speed.c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndGetTransportInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndGetTransportInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetPositionInfoSync(const std::string& uuid, uint32_t& track, CDateTimeSpan& duration, std::string& metadata, std::string& uri,
  CDateTimeSpan& relativeTime, CDateTimeSpan& absoluteTime, int32_t& relativeCount, int32_t& absoluteCount, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get position info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetPositionInfo(%u)", instanceId);

    std::string strDuration, strRelativeTime, strAbsoluteTime;
    service.service->SyncGetPositionInfo(instanceId, track, strDuration, metadata, uri, strRelativeTime, strAbsoluteTime, relativeCount, absoluteCount);

    duration = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strDuration)));
    relativeTime = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strRelativeTime)));
    absoluteTime = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strAbsoluteTime)));

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetPositionInfo(%u): track = %u; duration = %s; metadata = %s; URI = %s; relative time = %s; absolute time = %s; relative count = %d; absolute count = %d",
        instanceId, track, COhUtils::GetDurationFromSeconds(duration.GetSecondsTotal()).c_str(), metadata.c_str(), uri.c_str(),
        COhUtils::GetDurationFromSeconds(relativeTime.GetSecondsTotal()).c_str(), COhUtils::GetDurationFromSeconds(absoluteTime.GetSecondsTotal()).c_str(), relativeCount, absoluteCount);
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncGetPositionInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncGetPositionInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetPositionInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get position info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetPositionInfoAsync(%u)", instanceId);

    service.service->BeginGetPositionInfo(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginGetPositionInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginGetPositionInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetPositionInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, uint32_t& track, CDateTimeSpan& duration,
  std::string& metadata, std::string& uri, CDateTimeSpan& relativeTime, CDateTimeSpan& absoluteTime, int32_t& relativeCount, int32_t& absoluteCount) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get position info on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetPositionInfoAsyncResult()");

    std::string strDuration, strRelativeTime, strAbsoluteTime;
    service.service->EndGetPositionInfo(result, track, strDuration, metadata, uri, strRelativeTime, strAbsoluteTime, relativeCount, absoluteCount);

    duration = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strDuration)));
    relativeTime = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strRelativeTime)));
    absoluteTime = CDateTimeSpan(0, 0, 0, static_cast<int>(COhUtils::GetDurationInSeconds(strAbsoluteTime)));

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetPositionInfoAsyncResult(): track = %u; duration = %s; metadata = %s; URI = %s; relative time = %s; absolute time = %s; relative count = %d; absolute count = %d",
        track, COhUtils::GetDurationFromSeconds(duration.GetSecondsTotal()).c_str(), metadata.c_str(), uri.c_str(),
        COhUtils::GetDurationFromSeconds(relativeTime.GetSecondsTotal()).c_str(), COhUtils::GetDurationFromSeconds(absoluteTime.GetSecondsTotal()).c_str(), relativeCount, absoluteCount);
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndGetPositionInfo(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndGetPositionInfo(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetDeviceCapabilitiesSync(const std::string& uuid, std::vector<COhUPnPAVTransportStorageMedium>& playbackMedia,
  std::vector<COhUPnPAVTransportStorageMedium>& recordingMedia, std::vector<COhUPnPAVTransportRecordQualityMode>& recordingQualityModes, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get device capabilities on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetDeviceCapabilities(%u)", instanceId);

    std::string strPlaybackMedia, strRecordMedia, strRecordingQualityModes;
    service.service->SyncGetDeviceCapabilities(instanceId, strPlaybackMedia, strRecordMedia, strRecordingQualityModes);

    std::vector<std::string> vecPlaybackMedia = StringUtils::Split(strPlaybackMedia, ",");
    for (const auto& playbackMedium : vecPlaybackMedia)
      playbackMedia.emplace_back(playbackMedium);
    std::vector<std::string> vecRecordMedia = StringUtils::Split(strRecordMedia, ",");
    for (const auto& recordMedium : vecRecordMedia)
      playbackMedia.emplace_back(recordMedium);
    std::vector<std::string> vecRecordingQualityModes = StringUtils::Split(strRecordingQualityModes, ",");
    for (const auto& recordingQualityMode : vecRecordingQualityModes)
      playbackMedia.emplace_back(recordingQualityMode);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetDeviceCapabilities(%u): playback media = %s; recording media = %s; recording quality modes = %s",
        instanceId, strPlaybackMedia.c_str(), strRecordMedia.c_str(), strRecordingQualityModes.c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncGetDeviceCapabilities(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncGetDeviceCapabilities(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetDeviceCapabilitiesAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get device capabilities on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetDeviceCapabilitiesAsync(%u)", instanceId);

    service.service->BeginGetDeviceCapabilities(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginGetDeviceCapabilities(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginGetDeviceCapabilities(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetDeviceCapabilitiesAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid,
  std::vector<COhUPnPAVTransportStorageMedium>& playbackMedia, std::vector<COhUPnPAVTransportStorageMedium>& recordingMedia,
  std::vector<COhUPnPAVTransportRecordQualityMode>& recordingQualityModes) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get device capabilities on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetDeviceCapabilitiesAsyncResult()");

    std::string strPlaybackMedia, strRecordMedia, strRecordingQualityModes;
    service.service->EndGetDeviceCapabilities(result, strPlaybackMedia, strRecordMedia, strRecordingQualityModes);

    std::vector<std::string> vecPlaybackMedia = StringUtils::Split(strPlaybackMedia, ",");
    for (const auto& playbackMedium : vecPlaybackMedia)
      playbackMedia.emplace_back(playbackMedium);
    std::vector<std::string> vecRecordMedia = StringUtils::Split(strRecordMedia, ",");
    for (const auto& recordMedium : vecRecordMedia)
      playbackMedia.emplace_back(recordMedium);
    std::vector<std::string> vecRecordingQualityModes = StringUtils::Split(strRecordingQualityModes, ",");
    for (const auto& recordingQualityMode : vecRecordingQualityModes)
      playbackMedia.emplace_back(recordingQualityMode);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetDeviceCapabilitiesAsyncResult(): playback media = %s; recording media = %s; recording quality modes = %s",
        strPlaybackMedia.c_str(), strRecordMedia.c_str(), strRecordingQualityModes.c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndGetDeviceCapabilities(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndGetDeviceCapabilities(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetTransportSettingsSync(const std::string& uuid, COhUPnPAVTransportPlayMode& playMode,
  COhUPnPAVTransportRecordQualityMode& recordingQualityMode, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get transport settings on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetTransportSettings(%u)", instanceId);

    std::string strPlayMode, strRecordQualityMode;
    service.service->SyncGetTransportSettings(instanceId, strPlayMode, strRecordQualityMode);

    playMode.SetFromString(strPlayMode);
    recordingQualityMode.SetFromString(strRecordQualityMode);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportSettings(%u): play mode = %s; recording quality mode = %s", instanceId, playMode.GetAsString().c_str(), recordingQualityMode.GetAsString().c_str());
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncGetTransportSettings(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncGetTransportSettings(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetTransportSettingsAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get transport settings on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetTransportSettingsAsync(%u)", instanceId);

    service.service->BeginGetTransportSettings(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginGetTransportSettings(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginGetTransportSettings(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetTransportSettingsAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid,
  COhUPnPAVTransportPlayMode& playMode, COhUPnPAVTransportRecordQualityMode& recordingQualityMode) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get transport settings on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportSettingsAsyncResult()");

    std::string strPlayMode, strRecordQualityMode;
    service.service->EndGetTransportSettings(result, strPlayMode, strRecordQualityMode);

    playMode.SetFromString(strPlayMode);
    recordingQualityMode.SetFromString(strRecordQualityMode);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportSettingsAsyncResult(): play mode = %s; recording quality mode = %s", playMode.GetAsString().c_str(), recordingQualityMode.GetAsString().c_str());
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndGetTransportSettings(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndGetTransportSettings(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::StopSync(const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to stop playback on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> Stop(%u)", instanceId);

    service.service->SyncStop(instanceId);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncStop(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncStop(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::StopAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to stop playback on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> StopAsync(%u)", instanceId);

    service.service->BeginStop(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginStop(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginStop(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::StopAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to stop playback on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- StopAsyncResult()");

    service.service->EndStop(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndStop(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndStop(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PlaySync(const std::string& uuid, const std::string& speed /* = "1" */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  if (speed == "0")
    return false;

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> Play(%u, %s)", instanceId, speed.c_str());

    service.service->SyncPlay(instanceId, speed);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncPlay(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncPlay(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PlayAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const std::string& speed /* = "1" */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  if (speed == "0")
    return false;

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> PlayAsync(%u, %s)", instanceId, speed.c_str());

    service.service->BeginPlay(instanceId, speed, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginPlay(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginPlay(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PlayAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- PlayAsyncResult()");

    service.service->EndPlay(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndPlay(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndPlay(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PauseSync(const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to pause playback on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> Pause(%u)", instanceId);

    service.service->SyncPause(instanceId);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncPause(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncPause(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PauseAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to pause playback on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> PauseAsync(%u)", instanceId);

    service.service->BeginPause(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginPause(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginPause(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PauseAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to pause playback on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- PauseAsyncResult()");

    service.service->EndPause(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndPause(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndPause(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SeekSync(const std::string& uuid, const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to seek on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  if (unit.Get() == OhUPnPAVTransportSeekUnit::Unknown || unit.GetAsString().empty())
    return false;

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> Seek(%u, %s, %s)", instanceId, unit.GetAsString().c_str(), target.c_str());

    service.service->SyncSeek(instanceId, unit.GetAsString(), target);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncSeek(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncSeek(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SeekAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to seek on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  if (unit.Get() == OhUPnPAVTransportSeekUnit::Unknown || unit.GetAsString().empty())
    return false;

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SeekAsync(%u, %s, %s)", instanceId, unit.GetAsString().c_str(), target.c_str());

    service.service->BeginSeek(instanceId, unit.GetAsString(), target, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginSeek(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginSeek(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SeekAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to seek on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- SeekAsyncResult()");

    service.service->EndSeek(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndSeek(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndSeek(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::NextSync(const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play the next uri on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> Next(%u)", instanceId);

    service.service->SyncNext(instanceId);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncNext(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncNext(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::NextAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play the next uri on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> NextAsync(%u)", instanceId);

    service.service->BeginNext(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginNext(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginNext(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::NextAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play the next uri on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- NextAsyncResult()");

    service.service->EndNext(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndNext(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndNext(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PreviousSync(const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play the previous uri on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> Previous(%u)", instanceId);

    service.service->SyncPrevious(instanceId);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncPrevious(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncPrevious(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PreviousAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play the previous uri on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> PreviousAsync(%u)", instanceId);

    service.service->BeginPrevious(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginPrevious(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginPrevious(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::PreviousAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to play the previous uri on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- PreviousAsyncResult()");

    service.service->EndPrevious(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndPrevious(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndPrevious(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetPlayModeSync(const std::string& uuid, const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set play mode on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetPlayMode(%u, %s)", instanceId, newPlayMode.GetAsString().c_str());

    service.service->SyncSetPlayMode(instanceId, newPlayMode.GetAsString());
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncSetPlayMode(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncSetPlayMode(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetPlayModeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid,
  const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set play mode on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetPlayModeAsync(%u, %s)", instanceId, newPlayMode.GetAsString().c_str());

    service.service->BeginSetPlayMode(instanceId, newPlayMode.GetAsString(), callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginSetPlayMode(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginSetPlayMode(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::SetPlayModeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to set play mode on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- SetPlayModeAsyncResult()");

    service.service->EndSetPlayMode(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndSetPlayMode(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndSetPlayMode(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetCurrentTransportActionsSync(const std::string& uuid, std::vector<COhUPnPAVTransportTransportAction>& actions, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get current transport actions on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetCurrentTransportActions(%u)", instanceId);

    std::string strActions;
    service.service->SyncGetCurrentTransportActions(instanceId, strActions);

    const auto vecActions = StringUtils::Split(strActions, ",");
    for (const auto& action : vecActions)
      actions.push_back(COhUPnPAVTransportTransportAction(action));

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetCurrentTransportActions(%u): actions = %s", instanceId, strActions.c_str());
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling SyncGetCurrentTransportActions(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling SyncGetCurrentTransportActions(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetCurrentTransportActionsAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get current transport actions on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetCurrentTransportActionsAsync(%u)", instanceId);

    service.service->BeginGetCurrentTransportActions(instanceId, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginGetCurrentTransportActions(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginGetCurrentTransportActions(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPAVTransportControlPointManager::GetCurrentTransportActionsAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid,
  std::vector<COhUPnPAVTransportTransportAction>& actions) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPAVTransportControlPointManager: trying to get current transport actions on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetCurrentTransportActionsAsyncResult()");

    std::string strActions;
    service.service->EndGetCurrentTransportActions(result, strActions);

    const auto vecActions = StringUtils::Split(strActions, ",");
    for (const auto& action : vecActions)
      actions.push_back(COhUPnPAVTransportTransportAction(action));

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetCurrentTransportActionsAsyncResult(): actions = %s", strActions.c_str());
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndGetCurrentTransportActions(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndGetCurrentTransportActions(): %s", ex.Message());
    return false;
  }

  return true;
}

void COhUPnPAVTransportControlPointManager::UnregisterRenderer(const std::string& rendererUuid)
{
  // unregister the renderer instance from the player core factory
  CPlayerCoreFactory::GetInstance().OnPlayerRemoved(rendererUuid);
}

COhUPnPAVTransportControlPoint::COhUPnPAVTransportControlPoint(const COhUPnPAVTransportControlPointManager* manager,
  const std::string& uuid, IOhUPnPAVTransportControlPointAsync* callback)
  : IOhUPnPControlPointInstance<COhUPnPAVTransportControlPointManager, IOhUPnPAVTransportControlPointAsync>(manager, uuid, callback)
  , m_profile()
{
  if (m_manager != nullptr)
  {
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(GetDevice(), m_profile))
      CLog::Log(LOGINFO, "COhUPnPAVTransportControlPoint: renderer %s (%s) doesn't match any profiles", GetDevice().GetFriendlyName().c_str(), GetDevice().GetUuid().c_str());
    else
      CLog::Log(LOGINFO, "COhUPnPAVTransportControlPoint: renderer %s (%s) matches profile %s", GetDevice().GetFriendlyName().c_str(), GetDevice().GetUuid().c_str(), m_profile.GetName().c_str());
  }
}

bool COhUPnPAVTransportControlPoint::SetAVTransportURISync(const std::string& uri, const std::string& uriMetadata, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->SetAVTransportURISync(m_uuid, uri, uriMetadata, instanceId);
}

bool COhUPnPAVTransportControlPoint::SetAVTransportURIAsync(const std::string& uri, const std::string& uriMetadata, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->SetAVTransportURIAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::SetAVTransportURICallback), m_uuid, uri, uriMetadata, instanceId);
}

void COhUPnPAVTransportControlPoint::SetAVTransportURICallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->SetAVTransportURIAsyncResult(result, m_uuid);
  m_callback->SetAVTransportURIResult(success);
}

bool COhUPnPAVTransportControlPoint::SetNextAVTransportURISync(const std::string& nextUri, const std::string& nextUriMetadata, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->SetNextAVTransportURISync(m_uuid, nextUri, nextUriMetadata, instanceId);
}

bool COhUPnPAVTransportControlPoint::SetNextAVTransportURIAsync(const std::string& nextUri, const std::string& nextUriMetadata, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->SetNextAVTransportURIAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::SetNextAVTransportURICallback), m_uuid, nextUri, nextUriMetadata, instanceId);
}

void COhUPnPAVTransportControlPoint::SetNextAVTransportURICallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->SetNextAVTransportURIAsyncResult(result, m_uuid);
  m_callback->SetNextAVTransportURIResult(success);
}

bool COhUPnPAVTransportControlPoint::GetMediaInfoSync(uint32_t& numberOfTracks, CDateTimeSpan& duration, std::string& currentUri, std::string& currentUriMetadata,
  std::string& nextUri, std::string& nextUriMetadata, COhUPnPAVTransportStorageMedium& playbackMedium, COhUPnPAVTransportStorageMedium& recordMedium,
  COhUPnPAVTransportWriteStatus& writeStatus, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetMediaInfoSync(m_uuid, numberOfTracks, duration, currentUri, currentUriMetadata, nextUri, nextUriMetadata, playbackMedium, recordMedium, writeStatus, instanceId);
}

bool COhUPnPAVTransportControlPoint::GetMediaInfoAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetMediaInfoAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::GetMediaInfoCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::GetMediaInfoCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  uint32_t numberOfTracks;
  CDateTimeSpan duration;
  std::string currentUri, currentUriMetadata, nextUri, nextUriMetadata;
  COhUPnPAVTransportStorageMedium playbackMedium;
  COhUPnPAVTransportStorageMedium recordMedium;
  COhUPnPAVTransportWriteStatus writeStatus;
  bool success = m_manager->GetMediaInfoAsyncResult(result, m_uuid, numberOfTracks, duration, currentUri, currentUriMetadata,
    nextUri, nextUriMetadata, playbackMedium, recordMedium, writeStatus);
  m_callback->GetMediaInfoResult(success, numberOfTracks, duration, currentUri, currentUriMetadata,
    nextUri, nextUriMetadata, playbackMedium, recordMedium, writeStatus);
}

bool COhUPnPAVTransportControlPoint::GetTransportInfoSync(COhUPnPAVTransportTransportState& transportState,
  COhUPnPAVTransportTransportStatus& transportStatus, std::string& speed, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetTransportInfoSync(m_uuid, transportState, transportStatus, speed, instanceId);
}

bool COhUPnPAVTransportControlPoint::GetTransportInfoAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetTransportInfoAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::GetTransportInfoCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::GetTransportInfoCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  COhUPnPAVTransportTransportState transportState;
  COhUPnPAVTransportTransportStatus transportStatus;
  std::string speed;
  bool success = m_manager->GetTransportInfoAsyncResult(result, m_uuid, transportState, transportStatus, speed);
  m_callback->GetTransportInfoResult(success, transportState, transportStatus, speed);
}

bool COhUPnPAVTransportControlPoint::GetPositionInfoSync(uint32_t& track, CDateTimeSpan& duration, std::string& metadata, std::string& uri,
  CDateTimeSpan& relativeTime, CDateTimeSpan& absoluteTime, int32_t& relativeCount, int32_t& absoluteCount, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetPositionInfoSync(m_uuid, track, duration, metadata, uri, relativeTime, absoluteTime, relativeCount, absoluteCount, instanceId);
}

bool COhUPnPAVTransportControlPoint::GetPositionInfoAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetPositionInfoAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::GetPositionInfoCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::GetPositionInfoCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  uint32_t track;
  CDateTimeSpan duration, relativeTime, absoluteTime;
  std::string metadata, uri;
  int32_t relativeCount, absoluteCount;
  bool success = m_manager->GetPositionInfoAsyncResult(result, m_uuid, track, duration, metadata, uri, relativeTime, absoluteTime, relativeCount, absoluteCount);
  m_callback->GetPositionInfoResult(success, track, duration, metadata, uri, relativeTime, absoluteTime, relativeCount, absoluteCount);
}

bool COhUPnPAVTransportControlPoint::GetDeviceCapabilitiesSync(std::vector<COhUPnPAVTransportStorageMedium>& playbackMedia,
  std::vector<COhUPnPAVTransportStorageMedium>& recordingMedia, std::vector<COhUPnPAVTransportRecordQualityMode>& recordingQualityModes,
  uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetDeviceCapabilitiesSync(m_uuid, playbackMedia, recordingMedia, recordingQualityModes, instanceId);
}

bool COhUPnPAVTransportControlPoint::GetDeviceCapabilitiesAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetDeviceCapabilitiesAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::GetDeviceCapabilitiesCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::GetDeviceCapabilitiesCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  std::vector<COhUPnPAVTransportStorageMedium> playbackMedia, recordingMedia;
  std::vector<COhUPnPAVTransportRecordQualityMode> recordingQualityModes;
  bool success = m_manager->GetDeviceCapabilitiesAsyncResult(result, m_uuid, playbackMedia, recordingMedia, recordingQualityModes);
  m_callback->GetDeviceCapabilitiesResult(success, playbackMedia, recordingMedia, recordingQualityModes);
}

bool COhUPnPAVTransportControlPoint::GetTransportSettingsSync(COhUPnPAVTransportPlayMode& playMode,
  COhUPnPAVTransportRecordQualityMode& recordingQualityMode, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetTransportSettingsSync(m_uuid, playMode, recordingQualityMode, instanceId);
}

bool COhUPnPAVTransportControlPoint::GetTransportSettingsAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetTransportSettingsAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::GetTransportSettingsCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::GetTransportSettingsCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  COhUPnPAVTransportPlayMode playMode;
  COhUPnPAVTransportRecordQualityMode recordingQualityMode;
  bool success = m_manager->GetTransportSettingsAsyncResult(result, m_uuid, playMode, recordingQualityMode);
  m_callback->GetTransportSettingsResult(success, playMode, recordingQualityMode);
}

bool COhUPnPAVTransportControlPoint::StopSync(uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->StopSync(m_uuid, instanceId);
}

bool COhUPnPAVTransportControlPoint::StopAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->StopAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::StopCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::StopCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->StopAsyncResult(result, m_uuid);
  m_callback->StopResult(success);
}

bool COhUPnPAVTransportControlPoint::PlaySync(const std::string& speed /* = "1" */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->PlaySync(m_uuid, speed, instanceId);
}

bool COhUPnPAVTransportControlPoint::PlayAsync(const std::string& speed /* = "1" */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->PlayAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::PlayCallback), m_uuid, speed, instanceId);
}

void COhUPnPAVTransportControlPoint::PlayCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->PlayAsyncResult(result, m_uuid);
  m_callback->PlayResult(success);
}

bool COhUPnPAVTransportControlPoint::PauseSync(uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->PauseSync(m_uuid, instanceId);
}

bool COhUPnPAVTransportControlPoint::PauseAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->PauseAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::PauseCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::PauseCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->PauseAsyncResult(result, m_uuid);
  m_callback->PauseResult(success);
}

bool COhUPnPAVTransportControlPoint::SeekSync(const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->SeekSync(m_uuid, unit, target, instanceId);
}

bool COhUPnPAVTransportControlPoint::SeekAsync(const COhUPnPAVTransportSeekUnit& unit, const std::string& target, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->SeekAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::SeekCallback), m_uuid, unit, target, instanceId);
}

void COhUPnPAVTransportControlPoint::SeekCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->SeekAsyncResult(result, m_uuid);
  m_callback->SeekResult(success);
}

bool COhUPnPAVTransportControlPoint::NextSync(uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->NextSync(m_uuid, instanceId);
}

bool COhUPnPAVTransportControlPoint::NextAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->NextAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::NextCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::NextCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->NextAsyncResult(result, m_uuid);
  m_callback->NextResult(success);
}

bool COhUPnPAVTransportControlPoint::PreviousSync(uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->PreviousSync(m_uuid, instanceId);
}

bool COhUPnPAVTransportControlPoint::PreviousAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->PreviousAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::PreviousCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::PreviousCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->PreviousAsyncResult(result, m_uuid);
  m_callback->PreviousResult(success);
}

bool COhUPnPAVTransportControlPoint::SetPlayModeSync(const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->SetPlayModeSync(m_uuid, newPlayMode, instanceId);
}

bool COhUPnPAVTransportControlPoint::SetPlayModeAsync(const COhUPnPAVTransportPlayMode& newPlayMode, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->SetPlayModeAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::SetPlayModeCallback), m_uuid, newPlayMode, instanceId);
}

void COhUPnPAVTransportControlPoint::SetPlayModeCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->SetPlayModeAsyncResult(result, m_uuid);
  m_callback->SetPlayModeResult(success);
}

bool COhUPnPAVTransportControlPoint::GetCurrentTransportActionsSync(std::vector<COhUPnPAVTransportTransportAction>& actions, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetCurrentTransportActionsSync(m_uuid, actions, instanceId);
}

bool COhUPnPAVTransportControlPoint::GetCurrentTransportActionsAsync(uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetCurrentTransportActionsAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPAVTransportControlPoint::GetCurrentTransportActionsCallback), m_uuid, instanceId);
}

void COhUPnPAVTransportControlPoint::GetCurrentTransportActionsCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  std::vector<COhUPnPAVTransportTransportAction> actions;
  bool success = m_manager->GetCurrentTransportActionsAsyncResult(result, m_uuid, actions);
  m_callback->GetCurrentTransportActionsResult(success, actions);
}
