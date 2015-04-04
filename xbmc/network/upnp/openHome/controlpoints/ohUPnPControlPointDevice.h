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

#include "network/upnp/openHome/ohUPnPDevice.h"

namespace OpenHome
{
  namespace Net {
    class CpDeviceCpp;
  }
}

class COhUPnPControlPointDevice : public COhUPnPDevice
{
public:
  enum class SupportsMethod
  {
    Unknown = -1,
    No = 0,
    Yes = 1
  };

public:
  COhUPnPControlPointDevice();
  explicit COhUPnPControlPointDevice(const std::string& friendlyName);
  explicit COhUPnPControlPointDevice(OpenHome::Net::CpDeviceCpp& device);
  virtual ~COhUPnPControlPointDevice();

  SupportsMethod SupportsSearch() const { return m_supportsSearch; }
  void SetSupportsSearch(bool supportsSearch) { m_supportsSearch = supportsSearch ? SupportsMethod::Yes : SupportsMethod::No; }
  SupportsMethod SupportsCreateObject() const { return m_supportsCreateObject; }
  void SetSupportsCreateObject(bool supportsCreateObject) { m_supportsCreateObject = supportsCreateObject ? SupportsMethod::Yes : SupportsMethod::No; }
  SupportsMethod SupportsImportResource() const { return m_supportsImportResource; }
  void SetSupportsImportResource(bool supportsImportResource) { m_supportsImportResource = supportsImportResource ? SupportsMethod::Yes : SupportsMethod::No; }

private:
  void Initialize(const std::string& uuid, const std::string& descriptionUrl, const std::string& descriptionXml);
  bool Deserialize(const std::string &descriptionXml);
  bool SetBaseUrl(const std::string &descriptionUrl);

  SupportsMethod m_supportsSearch;
  SupportsMethod m_supportsCreateObject;
  SupportsMethod m_supportsImportResource;
};
