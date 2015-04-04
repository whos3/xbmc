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
#include <map>
#include <string>
#include <stdint.h>

#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>

#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/ohUPnPService.h"
#include "network/upnp/openHome/controlpoints/ohUPnPControlPointDevice.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfile.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfilesManager.h"
#include "threads/CriticalSection.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

template<class TControlPointService>
class IOhUPnPControlPoint : public IOhUPnPService
{
public:
  virtual ~IOhUPnPControlPoint()
  {
    {
      CSingleLock lock(m_servicesSection);
      for (auto& service : m_services)
        delete service.second.service;

      m_services.clear();
    }

    {
      CSingleLock lock(m_devicesSection);
      delete m_devices;
      m_devices = nullptr;
    }
  }

  typedef struct UPnPControlPointService
  {
    COhUPnPControlPointDevice device;
    COhUPnPDeviceProfile profile;
    TControlPointService* service;
  } UPnPControlPointService;

  std::vector<COhUPnPControlPointDevice> GetDevices() const
  {
    std::vector<COhUPnPControlPointDevice> devices;
    {
      CSingleLock lock(m_servicesSection);
      for (const auto& service : m_services)
        devices.push_back(service.second.device);
    }

    return devices;
  }

  bool GetDevice(const std::string &uuid, COhUPnPControlPointDevice &device) const
  {
    if (uuid.empty())
      return false;

    {
      CSingleLock lock(m_servicesSection);
      const auto& service = m_services.find(uuid);
      if (service == m_services.end())
        return false;

      device = service->second.device;
    }
    return true;
  }

  bool GetService(const std::string &uuid, TControlPointService* &controlPointService) const
  {
    if (uuid.empty())
      return nullptr;

    CSingleLock lock(m_servicesSection);
    const auto& service = m_services.find(uuid);
    if (service == m_services.end())
      return nullptr;

    controlPointService = service->second.service;
    return controlPointService != nullptr;
  }

protected:
  IOhUPnPControlPoint(const std::string &domain, const std::string &serviceName, uint8_t version)
    : IOhUPnPService(domain, serviceName, version),
      m_devices(nullptr)
  {
    CSingleLock lock(m_devicesSection);
    m_devices = new OpenHome::Net::CpDeviceListCppUpnpServiceType(
      domain, serviceName, version,
      OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &IOhUPnPControlPoint::added),
      OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &IOhUPnPControlPoint::removed));
  }

  /*!
   * \brief Callback for every service that has been discovered
   */
  virtual void onServiceAdded(const UPnPControlPointService &service) { };

  /*!
   * \brief Callback for every previously discovered service that disappeared
   */
  virtual void onServiceRemoved(const UPnPControlPointService &service) { };

  bool findService(const std::string& uuid, UPnPControlPointService& service) const
  {
    if (uuid.empty())
      return false;

    const auto& serviceIt = m_services.find(uuid);
    if (serviceIt == m_services.end())
      return false;

    service = serviceIt->second;
    return true;
  }

  std::string getLogPrefix() const
  {
    return StringUtils::Format("UPnPControlPoint[%s:%hhu]", m_serviceName.c_str(), m_serviceVersion);
  }

private:
  void added(OpenHome::Net::CpDeviceCpp& device)
  {
    COhUPnPControlPointDevice upnpDevice(device);
    if (!upnpDevice.IsValid())
    {
      std::string friendlyName;
      device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_FRIENDLY_NAME, friendlyName);
      CLog::Log(LOGWARNING, "%s: service %s is invalid", getLogPrefix().c_str(), friendlyName.c_str());
      return;
    }

    CLog::Log(LOGDEBUG, "%s: service \"%s:%hhu\" detected: %s (%s)", getLogPrefix().c_str(),
      upnpDevice.GetDeviceType().c_str(), upnpDevice.GetDeviceTypeVersion(), upnpDevice.GetFriendlyName().c_str(), device.Udn().c_str());

    // try to find a matching device profile
    COhUPnPDeviceProfile profile;
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(upnpDevice, profile))
      CLog::Log(LOGINFO, "%s: service %s (%s) doesn't match any profiles", getLogPrefix().c_str(), upnpDevice.GetFriendlyName().c_str(), device.Udn().c_str());
    else
      CLog::Log(LOGINFO, "%s: service %s (%s) matches profile %s", getLogPrefix().c_str(), upnpDevice.GetFriendlyName().c_str(), device.Udn().c_str(), profile.GetName().c_str());

    UPnPControlPointService service = { upnpDevice, profile, new TControlPointService(device) };
    {
      CSingleLock lock(m_servicesSection);
      m_services.insert(std::make_pair(device.Udn(), service));
    }

    onServiceAdded(service);
  }

  void removed(OpenHome::Net::CpDeviceCpp& device)
  {
    CSingleLock lock(m_servicesSection);
    auto& service = m_services.find(device.Udn());
    if (service == m_services.end())
    {
      std::string friendlyName;
      device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_FRIENDLY_NAME, friendlyName);
      CLog::Log(LOGDEBUG, "%s: unknown service removed: %s (%s)", getLogPrefix().c_str(), friendlyName.c_str(), device.Udn().c_str());
      return;
    }

    CLog::Log(LOGDEBUG, "%s: service removed: %s (%s)", getLogPrefix().c_str(), service->second.device.GetFriendlyName().c_str(), device.Udn().c_str());

    onServiceRemoved(service->second);

    delete service->second.service;
    m_services.erase(service);
  }

  CCriticalSection m_servicesSection;
  std::map<std::string, UPnPControlPointService> m_services;

  CCriticalSection m_devicesSection;
  OpenHome::Net::CpDeviceListCppUpnpServiceType *m_devices;
};
