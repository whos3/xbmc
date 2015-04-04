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

#include <string>

#include <OpenHome/Net/Cpp/DvDevice.h>

#include "network/upnp/openHome/ohUPnPDevice.h"

class COhUPnPRootDevice : public COhUPnPDevice
{
public:
  explicit COhUPnPRootDevice(const std::string& udn, OpenHome::Net::IResourceManagerStd& resourceManager);
  virtual ~COhUPnPRootDevice();

  const OpenHome::Net::DvDeviceStdStandard& GetDevice() const { return m_device; }
  OpenHome::Net::DvDeviceStdStandard& GetDevice() { return m_device; }

  void Enable();
  void Disable(OpenHome::Functor completedCallback);
  bool IsEnabled() const { return m_device.Enabled(); }

  void SetBaseUrl(const std::string& baseUrl) { m_baseUrl = baseUrl; }
  void SetDomain(const std::string& domain);
  void SetFriendlyName(const std::string& friendlyName);
  // TODO: void SetDeviceTypeExtended(const std::string& deviceTypeExtended);
  void SetDeviceType(const std::string& deviceType);
  void SetDeviceTypeVersion(uint8_t deviceTypeVersion);
  void SetManufacturer(const std::string& manufacturer);
  void SetManufacturerUrl(const std::string& manufacturerUrl);
  void SetModelName(const std::string& modelName);
  void SetModelNumber(const std::string& modelNumber);
  void SetModelUrl(const std::string& modelUrl);
  void SetModelDescription(const std::string& modelDescription);
  void SetSerialNumber(const std::string& serialNumber);
  void SetUniversalProductCode(const std::string& universalProductCode);
  void SetPresentationUrl(const std::string& presentationUrl);
  void SetIcons(const std::vector<CUPnPIcon>& icons);

private:
  bool SetAttribute(const std::string& name, const std::string& value);

  OpenHome::Net::DvDeviceStdStandard m_device;
};
