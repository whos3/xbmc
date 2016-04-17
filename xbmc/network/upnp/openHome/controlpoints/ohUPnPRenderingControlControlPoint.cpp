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

#include "ohUPnPRenderingControlControlPoint.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"

COhUPnPRenderingControlControlPointManager::COhUPnPRenderingControlControlPointManager(const std::string& deviceType)
  : IOhUPnPControlPointManager(deviceType, UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_RENDERINGCONTROL, 2 /* TODO: 3 */, { 1 /* TODO: , 2 */ })
{ }

COhUPnPRenderingControlControlPointManager::~COhUPnPRenderingControlControlPointManager()
{ }

bool COhUPnPRenderingControlControlPointManager::GetMuteSync(const std::string& uuid, bool& mute, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get mute on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetMute(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->SyncGetMute(instanceId, channel.GetAsString(), mute);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetMute(%u, %s): mute = %s", instanceId, channel.GetAsString().c_str(), (mute ? "true" : "false"));
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncGetMute(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncGetMute(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetMuteAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get mute on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetMuteAsync(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->BeginGetMute(instanceId, channel.GetAsString(), callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling BeginGetMute(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling BeginGetMute(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetMuteAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, bool& mute) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get mute on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetMuteAsyncResult()");

    service.service->EndGetMute(result, mute);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetMuteAsyncResult(): mute = %s", (mute ? "true" : "false"));
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: error %u calling EndGetMute(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPAVTransportControlPointManager: exception calling EndGetMute(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetMuteSync(const std::string& uuid, bool mute, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set mute on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetMute(%u, %s, %s)", instanceId, channel.GetAsString().c_str(), (mute ? "mute" : "unmute"));

    service.service->SyncSetMute(instanceId, channel.GetAsString(), mute);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncSetMute(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncSetMute(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetMuteAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, bool mute, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set mute on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetMuteAsync(%u, %s, %s)", instanceId, channel.GetAsString().c_str(), (mute ? "mute" : "unmute"));

    service.service->BeginSetMute(instanceId, channel.GetAsString(), mute, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling BeginSetMute(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling BeginSetMute(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetMuteAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set mute on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- SetMuteAsyncResult()");

    service.service->EndSetMute(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling EndSetMute(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling EndSetMute(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeSync(const std::string& uuid, uint32_t& volume, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetVolume(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->SyncGetVolume(instanceId, channel.GetAsString(), volume);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolume(%s, %u): volume = %u", instanceId, channel.GetAsString().c_str(), volume);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncGetVolume(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncGetVolume(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetVolumeAsync(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->BeginGetVolume(instanceId, channel.GetAsString(), callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling BeginGetVolume(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling BeginGetVolume(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, uint32_t& volume) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeAsyncResult()");

    service.service->EndGetVolume(result, volume);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeAsyncResult(): volume = %u", volume);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling EndGetVolume(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling EndGetVolume(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetVolumeSync(const std::string& uuid, uint32_t volume, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set volume on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetVolume(%u, %s, %u)", instanceId, channel.GetAsString().c_str(), volume);

    service.service->SyncSetVolume(instanceId, channel.GetAsString(), volume);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncSetVolume(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncSetVolume(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetVolumeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t volume, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set volume on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetVolumeAsync(%u, %s, %u)", instanceId, channel.GetAsString().c_str(), volume);

    service.service->BeginSetVolume(instanceId, channel.GetAsString(), volume, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling BeginSetVolume(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling BeginSetVolume(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetVolumeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set volume on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- SetVolumeAsyncResult()");

    service.service->EndSetVolume(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling EndSetVolume(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling EndSetVolume(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeDBSync(const std::string& uuid, int32_t& volumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetVolumeDB(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->SyncGetVolumeDB(instanceId, channel.GetAsString(), volumeDB);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDB(%u, %s): volume = %d dB", instanceId, channel.GetAsString().c_str(), volumeDB);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncGetVolumeDB(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncGetVolumeDB(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeDBAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetVolumeDBAsync(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->BeginGetVolumeDB(instanceId, channel.GetAsString(), callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling BeginGetVolumeDB(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling BeginGetVolumeDB(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeDBAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, int32_t& volumeDB) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDBAsyncResult()");

    service.service->EndGetVolumeDB(result, volumeDB);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDBAsyncResult(): volume = %d dB", volumeDB);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling EndGetVolumeDB(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling EndGetVolumeDB(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeDBRangeSync(const std::string& uuid, int32_t& minVolumeDB, int32_t& maxVolumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume range in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetVolumeDBRange(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->SyncGetVolumeDBRange(instanceId, channel.GetAsString(), minVolumeDB, maxVolumeDB);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDBRange(%u, %s): min = %d dB; max = %d dB", instanceId, channel.GetAsString().c_str(), minVolumeDB, maxVolumeDB);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncGetVolumeDBRange(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncGetVolumeDBRange(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeDBRangeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPoint: trying to get volume range in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetVolumeDBRangeAsync(%u, %s)", instanceId, channel.GetAsString().c_str());

    service.service->BeginGetVolumeDBRange(instanceId, channel.GetAsString(), callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPoint: error %u calling BeginGetVolumeDBRange(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPoint: exception calling BeginGetVolumeDBRange(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::GetVolumeDBRangeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, int32_t& minVolumeDB, int32_t& maxVolumeDB) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to get volume range in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDBRangeAsyncResult()");

    service.service->EndGetVolumeDBRange(result, minVolumeDB, maxVolumeDB);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetVolumeDBRangeAsyncResult(): min = %d dB; max = %d dB", minVolumeDB, maxVolumeDB);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling EndGetVolumeDBRange(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling EndGetVolumeDBRange(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetVolumeDBSync(const std::string& uuid, int32_t volumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set volume in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetVolumeDB(%u, %s, %d)", instanceId, channel.GetAsString().c_str(), volumeDB);

    service.service->SyncSetVolumeDB(instanceId, channel.GetAsString(), volumeDB);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling SyncSetVolumeDB(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling SyncSetVolumeDB(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetVolumeDBAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, int32_t volumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set volume in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SetVolumeDBAsync(%u, %s, %d)", instanceId, channel.GetAsString().c_str(), volumeDB);

    service.service->BeginSetVolumeDB(instanceId, channel.GetAsString(), volumeDB, callback);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling BeginSetVolumeDB(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling BeginSetVolumeDB(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPRenderingControlControlPointManager::SetVolumeDBAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPRenderingControlControlPointManager: trying to set volume in dB on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- SetVolumeDBAsyncResult()");

    service.service->EndSetVolumeDB(result);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: error %u calling EndSetVolumeDB(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPRenderingControlControlPointManager: exception calling EndSetVolumeDB(): %s", ex.Message());
    return false;
  }

  return true;
}

COhUPnPRenderingControlControlPoint::COhUPnPRenderingControlControlPoint(const COhUPnPRenderingControlControlPointManager* manager,
  const std::string& uuid, IOhUPnPRenderingControlControlPointAsync* callback)
  : IOhUPnPControlPointInstance(manager, uuid, callback)
  , m_profile()
{
  if (m_manager != nullptr)
  {
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(GetDevice(), m_profile))
      CLog::Log(LOGINFO, "COhUPnPAVTransportControlPoint: rendering controller %s (%s) doesn't match any profiles", GetDevice().GetFriendlyName().c_str(), GetDevice().GetUuid().c_str());
    else
      CLog::Log(LOGINFO, "COhUPnPAVTransportControlPoint: rendering controller %s (%s) matches profile %s", GetDevice().GetFriendlyName().c_str(), GetDevice().GetUuid().c_str(), m_profile.GetName().c_str());
  }
}

bool COhUPnPRenderingControlControlPoint::GetMuteSync(bool& mute, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetMuteSync(m_uuid, mute, channel, instanceId);
}

bool COhUPnPRenderingControlControlPoint::GetMuteAsync(COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetMuteAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPRenderingControlControlPoint::GetMuteCallback), m_uuid, channel, instanceId);
}

void COhUPnPRenderingControlControlPoint::GetMuteCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool mute;
  bool success = m_manager->GetMuteAsyncResult(result, m_uuid, mute);
  m_callback->GetMuteResult(success, mute);
}

bool COhUPnPRenderingControlControlPoint::SetMuteSync(bool mute, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->SetMuteSync(m_uuid, mute, channel, instanceId);
}

bool COhUPnPRenderingControlControlPoint::SetMuteAsync(bool mute, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->SetMuteAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPRenderingControlControlPoint::GetMuteCallback), m_uuid, mute, channel, instanceId);
}

void COhUPnPRenderingControlControlPoint::SetMuteCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->SetMuteAsyncResult(result, m_uuid);
  m_callback->SetMuteResult(success);
}

bool COhUPnPRenderingControlControlPoint::GetVolumeSync(uint32_t& volume, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetVolumeSync(m_uuid, volume, channel, instanceId);
}

bool COhUPnPRenderingControlControlPoint::GetVolumeAsync(COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetVolumeAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPRenderingControlControlPoint::GetVolumeCallback), m_uuid, channel, instanceId);
}

void COhUPnPRenderingControlControlPoint::GetVolumeCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  uint32_t volume;
  bool success = m_manager->GetVolumeAsyncResult(result, m_uuid, volume);
  m_callback->GetVolumeResult(success, volume);
}

bool COhUPnPRenderingControlControlPoint::SetVolumeSync(uint32_t volume, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->SetVolumeSync(m_uuid, volume, channel, instanceId);
}

bool COhUPnPRenderingControlControlPoint::SetVolumeAsync(uint32_t volume, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->SetVolumeAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPRenderingControlControlPoint::SetVolumeCallback), m_uuid, volume, channel, instanceId);
}

void COhUPnPRenderingControlControlPoint::SetVolumeCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->SetVolumeAsyncResult(result, m_uuid);
  m_callback->SetVolumeResult(success);
}

bool COhUPnPRenderingControlControlPoint::GetVolumeDBSync(int32_t& volumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetVolumeDBSync(m_uuid, volumeDB, channel, instanceId);
}

bool COhUPnPRenderingControlControlPoint::GetVolumeDBAsync(COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetVolumeDBAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPRenderingControlControlPoint::GetVolumeDBCallback), m_uuid, channel, instanceId);
}

void COhUPnPRenderingControlControlPoint::GetVolumeDBCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  int32_t volumeDB;
  bool success = m_manager->GetVolumeDBAsyncResult(result, m_uuid, volumeDB);
  m_callback->GetVolumeDBResult(success, volumeDB);
}

bool COhUPnPRenderingControlControlPoint::GetVolumeDBRangeSync(int32_t& minVolumeDB, int32_t& maxVolumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->GetVolumeDBRangeSync(m_uuid, minVolumeDB, maxVolumeDB, channel, instanceId);
}

bool COhUPnPRenderingControlControlPoint::GetVolumeDBRangeAsync(COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->GetVolumeDBRangeAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPRenderingControlControlPoint::GetVolumeDBRangeCallback), m_uuid, channel, instanceId);
}

void COhUPnPRenderingControlControlPoint::GetVolumeDBRangeCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  int32_t minVolumeDB, maxVolumeDB;
  bool success = m_manager->GetVolumeDBRangeAsyncResult(result, m_uuid, minVolumeDB, maxVolumeDB);
  m_callback->GetVolumeDBRangeResult(success, minVolumeDB, maxVolumeDB);
}

bool COhUPnPRenderingControlControlPoint::SetVolumeDBSync(int32_t volumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */) const
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr)
    return false;

  return m_manager->SetVolumeDBSync(m_uuid, volumeDB, channel, instanceId);
}

bool COhUPnPRenderingControlControlPoint::SetVolumeDBAsync(int32_t volumeDB, COhUPnPRenderingControlChannel channel /* = OhUPnPRenderingControlChannel::Master */, uint32_t instanceId /* = 0 */)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return false;

  return m_manager->SetVolumeDBAsync(OpenHome::Net::MakeFunctorAsync(*this, &COhUPnPRenderingControlControlPoint::SetVolumeDBCallback), m_uuid, volumeDB, channel, instanceId);
}

void COhUPnPRenderingControlControlPoint::SetVolumeDBCallback(OpenHome::Net::IAsync& result)
{
  CSingleLock lock(m_criticalManager);
  if (m_manager == nullptr || m_callback == nullptr)
    return;

  bool success = m_manager->SetVolumeDBAsyncResult(result, m_uuid);
  m_callback->SetVolumeDBResult(success);
}
