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
#include <set>

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
#include "utils/StringUtils.h"

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
      for (const auto& device : m_devices)
        delete device;
      m_devices.clear();
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
  IOhUPnPControlPoint(const std::string &domain, const std::string &serviceName, uint8_t version, std::set<uint8_t> additionalVersions = std::set<uint8_t>())
    : IOhUPnPService(domain, serviceName, version),
      m_devices()
  {
    CSingleLock lock(m_devicesSection);

    additionalVersions.insert(version);
    for (const auto& additionalVersion : additionalVersions)
    {
      m_devices.emplace(new OpenHome::Net::CpDeviceListCppUpnpServiceType(
        domain, serviceName, additionalVersion,
        OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &IOhUPnPControlPoint::added),
        OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &IOhUPnPControlPoint::removed)));
    }

  }

  /*!
  * \brief Callback for every service that has been discovered to check if it matches for the control point
  */
  virtual bool IsMatchingService(const UPnPControlPointService &service) const { return true; }

  /*!
   * \brief Callback for every service that has been discovered
   */
  virtual void OnServiceAdded(const UPnPControlPointService &service) { }

  /*!
   * \brief Callback for every previously discovered service that disappeared
   */
  virtual void OnServiceRemoved(const UPnPControlPointService &service) { }

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

    CSingleLock lock(m_servicesSection);
    if (m_services.find(device.Udn()) != m_services.end())
      return;

    CLog::Log(LOGDEBUG, "%s: service \"%s:%hhu\" detected: %s (%s)", getLogPrefix().c_str(),
      upnpDevice.GetDeviceType().c_str(), upnpDevice.GetDeviceTypeVersion(), upnpDevice.GetFriendlyName().c_str(), device.Udn().c_str());

    // try to find a matching device profile
    COhUPnPDeviceProfile profile;
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(upnpDevice, profile))
      CLog::Log(LOGINFO, "%s: service %s (%s) doesn't match any profiles", getLogPrefix().c_str(), upnpDevice.GetFriendlyName().c_str(), device.Udn().c_str());
    else
      CLog::Log(LOGINFO, "%s: service %s (%s) matches profile %s", getLogPrefix().c_str(), upnpDevice.GetFriendlyName().c_str(), device.Udn().c_str(), profile.GetName().c_str());

    UPnPControlPointService service = { upnpDevice, profile, new TControlPointService(device) };

    if (!IsMatchingService(service))
    {
      CLog::Log(LOGDEBUG, "%s: ignoring service \"%s:%hhu\" from %s (%s)", getLogPrefix().c_str(),
        upnpDevice.GetDeviceType().c_str(), upnpDevice.GetDeviceTypeVersion(), upnpDevice.GetFriendlyName().c_str(), device.Udn());

      delete service.service;
      return;
    }

    m_services.insert(std::make_pair(device.Udn(), service)).second;
    lock.Leave();

    OnServiceAdded(service);
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

    OnServiceRemoved(service->second);

    delete service->second.service;
    m_services.erase(service);
  }

  CCriticalSection m_servicesSection;
  std::map<std::string, UPnPControlPointService> m_services;

  CCriticalSection m_devicesSection;
  std::set<OpenHome::Net::CpDeviceListCppUpnpServiceType*> m_devices;
};

template<class TControlPointService, class TControlPointManager, class TControlPointInstance, class TControlPointAsync>
class IOhUPnPControlPointManager : public IOhUPnPControlPoint<TControlPointService>
{
private:
  using ControlPointPtr = std::shared_ptr<TControlPointInstance>;
public:
  virtual ~IOhUPnPControlPointManager()
  {
    // disable and destroy all control points
    std::set<ControlPointPtr> controlPoints;
    {
      CSingleLock lock(m_criticalControlPoints);
      controlPoints = m_controlPoints;
    }

    for (const auto& controlPoint : controlPoints)
      DestroyControlPoint(controlPoint);
    controlPoints.clear();
  }

  virtual ControlPointPtr CreateControlPoint(const std::string& uuid, TControlPointAsync* callback)
  {
    UPnPControlPointService service;
    if (!findService(uuid, service))
    {
      CLog::Log(LOGERROR, "IOhUPnPControlPointManager: trying to create a control point for an unknown service with UUID \"%s\"", uuid.c_str());
      return false;
    }

    ControlPointPtr controlPoint(new TControlPointInstance(reinterpret_cast<TControlPointManager*>(this), uuid, callback));

    // remember the control point
    CSingleLock lock(m_criticalControlPoints);
    m_controlPoints.insert(controlPoint);

    return controlPoint;
  }

  virtual void DestroyControlPoint(ControlPointPtr controlPoint)
  {
    if (controlPoint == nullptr)
      return;

    // make sure the control point is disabled
    controlPoint->Disable();

    // remove the control point
    CSingleLock lock(m_criticalControlPoints);
    m_controlPoints.erase(controlPoint);
  }

protected:
  IOhUPnPControlPointManager(const std::string& deviceType, const std::string &domain, const std::string &serviceName, uint8_t version, std::set<uint8_t> additionalVersions = std::set<uint8_t>())
    : IOhUPnPControlPoint<TControlPointService>(domain, serviceName, version, additionalVersions)
    , m_deviceType(deviceType)
  { }

  // specialization of IOhUPnPControlPoint
  virtual bool IsMatchingService(const UPnPControlPointService &service) const override
  {
    if (m_deviceType.empty())
      return true;

    // only handle AVTransport services from a matching device
    return service.device.GetDeviceType() == m_deviceType;
  }

  virtual void OnServiceAdded(const UPnPControlPointService &service) override
  {
    // enable all existing control points matching the detected device
    CSingleLock lock(m_criticalControlPoints);
    for (const auto& controlPoint : m_controlPoints)
    {
      if (controlPoint->GetUuid() == service.device.GetUuid())
        controlPoint->Enable(reinterpret_cast<TControlPointManager*>(this));
    }
  }

  virtual void OnServiceRemoved(const UPnPControlPointService &service) override
  {
    // disable all existing control points matching the removed device
    CSingleLock lock(m_criticalControlPoints);
    for (const auto& controlPoint : m_controlPoints)
    {
      if (controlPoint->GetUuid() == service.device.GetUuid())
        controlPoint->Disable();
    }
  }

private:
  const std::string m_deviceType;

  CCriticalSection m_criticalControlPoints;
  std::set<ControlPointPtr> m_controlPoints;
};

template<class TControlPointManager, class TControlPointAsync>
class IOhUPnPControlPointInstance
{
public:
  virtual ~IOhUPnPControlPointInstance()
  {
    m_manager = nullptr;
    m_callback = nullptr;
  }

  const std::string& GetUuid() const { return m_uuid; }

  const COhUPnPControlPointDevice& GetDevice() const { return m_device; }

  virtual const COhUPnPDeviceProfile& GetProfile() const = 0;

protected:
  IOhUPnPControlPointInstance(const TControlPointManager* manager, const std::string& uuid, TControlPointAsync* callback)
    : m_uuid(uuid)
    , m_manager(manager)
    , m_callback(callback)
    , m_device()
  {
    if (m_manager == nullptr || !m_manager->GetDevice(m_uuid, m_device))
      CLog::Log(LOGERROR, "IOhUPnPControlPointInstance: failed to determine device %s", m_uuid.c_str());
  }

  void Enable(const TControlPointManager* manager)
  {
    CSingleLock lock(m_criticalManager);
    m_manager = manager;
  }

  void Disable()
  {
    CSingleLock lock(m_criticalManager);
    m_manager = nullptr;
  }

  const std::string m_uuid;
  CCriticalSection m_criticalManager;
  const TControlPointManager* m_manager;
  TControlPointAsync* m_callback;

private:
  COhUPnPControlPointDevice m_device;
};
