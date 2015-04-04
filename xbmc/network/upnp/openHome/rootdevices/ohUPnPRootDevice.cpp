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
#include <cstring>

#include "ohUPnPRootDevice.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"

static const size_t MaximumAttributeValueLength = 256;

COhUPnPRootDevice::COhUPnPRootDevice(const std::string& udn, OpenHome::Net::IResourceManagerStd& resourceManager)
  : COhUPnPDevice(),
    m_device(udn, resourceManager)
{
  m_uuid = udn;
  SetValid(!m_uuid.empty());
}

COhUPnPRootDevice::~COhUPnPRootDevice()
{ }

void COhUPnPRootDevice::Enable()
{
  m_device.SetEnabled();
}

void COhUPnPRootDevice::Disable(OpenHome::Functor completedCallback)
{
  m_device.SetDisabled(completedCallback);
}

void COhUPnPRootDevice::SetDomain(const std::string& domain)
{
  SetAttribute("Upnp.Domain", domain);
}

void COhUPnPRootDevice::SetFriendlyName(const std::string& friendlyName)
{
  if (!SetAttribute(UPNP_DEVICE_ATTRIBUTE_FRIENDLY_NAME, friendlyName))
    return;

  m_friendlyName = friendlyName;
}

/* TODO
void COhUPnPRootDevice::SetDeviceTypeExtended(const std::string& deviceTypeExtended)
{
  if (!SetAttribute("" /* TODO *//*, deviceTypeExtended))
    return;

  COhUPnPDevice::SetDeviceTypeExtended(deviceTypeExtended);
}
*/

void COhUPnPRootDevice::SetDeviceType(const std::string& deviceType)
{
  if (!SetAttribute("Upnp.Type", deviceType))
    return;

  m_deviceType = deviceType;
}

void COhUPnPRootDevice::SetDeviceTypeVersion(uint8_t deviceTypeVersion)
{
  if (!SetAttribute("Upnp.Version", StringUtils::Format("%hhu", deviceTypeVersion)))
    return;

  m_deviceTypeVersion = deviceTypeVersion;
}

void COhUPnPRootDevice::SetManufacturer(const std::string& manufacturer)
{
  if (!SetAttribute("Upnp.Manufacturer", manufacturer))
    return;

  m_manufacturer = manufacturer;
}

void COhUPnPRootDevice::SetManufacturerUrl(const std::string& manufacturerUrl)
{
  if (!SetAttribute("Upnp.ManufacturerUrl", manufacturerUrl))
    return;

  m_manufacturerUrl = manufacturerUrl;
}

void COhUPnPRootDevice::SetModelName(const std::string& modelName)
{
  if (!SetAttribute("Upnp.ModelName", modelName))
    return;

  m_modelName = modelName;
}

void COhUPnPRootDevice::SetModelNumber(const std::string& modelNumber)
{
  if (!SetAttribute("Upnp.ModelNumber", modelNumber))
    return;

  m_modelNumber = modelNumber;
}

void COhUPnPRootDevice::SetModelUrl(const std::string& modelUrl)
{
  if (!SetAttribute("Upnp.ModelUrl", modelUrl))
    return;

  m_modelUrl = modelUrl;
}

void COhUPnPRootDevice::SetModelDescription(const std::string& modelDescription)
{
  if (!SetAttribute("Upnp.ModelDescription", modelDescription))
    return;

  m_modelDescription = modelDescription;
}

void COhUPnPRootDevice::SetSerialNumber(const std::string& serialNumber)
{
  if (!SetAttribute("Upnp.SerialNumber", serialNumber))
    return;

  m_serialNumber = serialNumber;
}

void COhUPnPRootDevice::SetUniversalProductCode(const std::string& universalProductCode)
{
  if (!SetAttribute("Upnp.Upc", universalProductCode))
    return;

  m_universalProductCode = universalProductCode;
}

void COhUPnPRootDevice::SetPresentationUrl(const std::string& presentationUrl)
{
  if (!SetAttribute("Upnp.PresentationUrl", presentationUrl))
    return;

  m_presentationUrl = presentationUrl;
}

void COhUPnPRootDevice::SetIcons(const std::vector<CUPnPIcon>& icons)
{
  // serialize iconList
  COhUPnPDeviceProfile tmpProfile;
  OhUPnPRootDeviceContext context = { *this, tmpProfile };
  TiXmlDocument doc;
  for (const auto& icon : icons)
  {
    if (!icon.Serialize(&doc, context))
      return;
  }

  TiXmlPrinter printer;
  doc.Accept(&printer);
  if (!SetAttribute("Upnp.IconList", printer.Str()))
    return;

  m_icons = CUPnPIconList(icons);
}

bool COhUPnPRootDevice::SetAttribute(const std::string& name, const std::string& value)
{
  if (IsEnabled())
    return false;

  m_device.SetAttribute(name.c_str(), value.c_str());
  return true;
}

