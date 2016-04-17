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
#include <vector>

#include "network/upnp/openHome/didllite/objects/properties/UPnPIcon.h"

namespace OpenHome
{
  namespace Net {
    class CpDeviceCpp;
  }
}

class COhUPnPDevice
{
public:
  virtual ~COhUPnPDevice();

  bool IsValid() const { return m_valid; }

  const std::string& GetDescriptionUrl() const { return m_descriptionUrl; }
  const std::string& GetBaseUrl() const { return m_baseUrl; }

  const std::string& GetUuid() const { return m_uuid; }
  const std::string& GetFriendlyName() const { return m_friendlyName; }
  const std::string& GetDeviceTypeExtended() const { return m_deviceTypeString; }
  const std::string& GetDeviceType() const { return m_deviceType; }
  uint8_t GetDeviceTypeVersion() const { return m_deviceTypeVersion; }
  const std::string& GetManufacturer() const { return m_manufacturer; }
  const std::string& GetManufacturerUrl() const { return m_manufacturerUrl; }
  const std::string& GetModelName() const { return m_modelName; }
  const std::string& GetModelNumber() const { return m_modelNumber; }
  const std::string& GetModelUrl() const { return m_modelUrl; }
  const std::string& GetModelDescription() const { return m_modelDescription; }
  const std::string& GetSerialNumber() const { return m_serialNumber; }
  const std::string& GetUniversalProductCode() const { return m_universalProductCode; }
  const std::string& GetPresentationUrl() const { return m_presentationUrl; }

  const CUPnPIconList& GetIcons() const { return m_icons; }
  std::string GetIconUrl() const;
  std::string GetIconUrl(const std::string &mimetype) const;

  std::set<std::pair<std::string, uint8_t>> GetServices() const;
  bool HasService(const std::string& serviceType) const;
  bool GetServiceVersion(const std::string& serviceType, uint8_t& serviceVersion);

  const std::string& GetUserAgent() const { return m_userAgent; }

protected:
  COhUPnPDevice();

  void SetValid(bool valid) { m_valid = valid; }

  static bool ParseDeviceType(const std::string &deviceTypeString, std::string &deviceType, uint8_t &version);
  static bool ParseServiceType(const std::string &serviceTypeString, std::string &serviceType, uint8_t &version);

  std::string m_descriptionUrl;
  std::string m_baseUrl;
  std::string m_uuid;
  std::string m_friendlyName;
  std::string m_deviceTypeString;
  std::string m_deviceType;
  uint8_t m_deviceTypeVersion;
  std::string m_manufacturer;
  std::string m_manufacturerUrl;
  std::string m_modelDescription;
  std::string m_modelName;
  std::string m_modelNumber;
  std::string m_modelUrl;
  std::string m_serialNumber;
  std::string m_universalProductCode;
  std::string m_presentationUrl;

  CUPnPIconList m_icons;

  std::map<std::string, uint8_t> m_services;

  std::string m_userAgent;

private:
  static bool ParseType(const std::string &typeString, const std::string& typeName, std::string &type, uint8_t &version);

  bool m_valid;
};
