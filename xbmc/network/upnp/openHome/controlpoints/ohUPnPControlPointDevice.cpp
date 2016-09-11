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
#include <algorithm>

#include <OpenHome/Net/Cpp/CpDevice.h>

#include "ohUPnPControlPointDevice.h"
#include "URL.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/StringUtils.h"
#include "utils/XMLUtils.h"

static const char* UPnPDeviceElementRoot = "root";
static const char* UPnPDeviceElementDevice = "device";
static const char* UPnPDeviceElementDeviceType = "deviceType";
static const char* UPnPDeviceElementFriendlyName = "friendlyName";
static const char* UPnPDeviceElementManufacturer = "manufacturer";
static const char* UPnPDeviceElementManufacturerUrl = "manufacturerURL";
static const char* UPnPDeviceElementModelDescription = "modelDescription";
static const char* UPnPDeviceElementModelName = "modelName";
static const char* UPnPDeviceElementModelNumber = "modelNumber";
static const char* UPnPDeviceElementModelUrl = "modelURL";
static const char* UPnPDeviceElementSerialNumber = "serialNumber";
static const char* UPnPDeviceElementUdn = "UDN";
static const char* UPnPDeviceElementUpc = "UPC";
static const char* UPnPDeviceElementPresentationUrl = "presentationURL";

static const char* UPnPDeviceElementServiceList = "serviceList";
static const char* UPnPDeviceElementService = "service";
static const char* UPnPDeviceElementServiceType = "serviceType";

COhUPnPControlPointDevice::COhUPnPControlPointDevice()
  : m_supportsSearch(SupportsMethod::Unknown),
    m_supportsCreateObject(SupportsMethod::Unknown),
    m_supportsImportResource(SupportsMethod::Unknown)
{ }

COhUPnPControlPointDevice::COhUPnPControlPointDevice(const std::string& friendlyName)
  : COhUPnPControlPointDevice()
{
  m_friendlyName = friendlyName;
  SetValid(!m_friendlyName.empty());
}

COhUPnPControlPointDevice::COhUPnPControlPointDevice(OpenHome::Net::CpDeviceCpp& device)
  : COhUPnPControlPointDevice()
{
  const std::string& udn = device.Udn();
  if (udn.empty())
    return;

  std::string descriptionUrl;
  if (!device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_LOCATION, descriptionUrl) || descriptionUrl.empty())
    return;

  std::string descriptionXml;
  if (!device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_DEVICE_XML, descriptionXml) || descriptionXml.empty())
    return;

  Initialize(udn, descriptionUrl, descriptionXml);
}

COhUPnPControlPointDevice::~COhUPnPControlPointDevice()
{ }

void COhUPnPControlPointDevice::Initialize(const std::string& uuid, const std::string& descriptionUrl, const std::string& descriptionXml)
{
  if (uuid.empty() || descriptionUrl.empty() || descriptionXml.empty())
    return;

  m_uuid = uuid;

  m_descriptionUrl = descriptionUrl;
  if (!SetBaseUrl(m_descriptionUrl))
    return;

  SetValid(Deserialize(descriptionXml));
}

bool COhUPnPControlPointDevice::Deserialize(const std::string &deviceDescriptionXml)
{
  if (deviceDescriptionXml.empty())
    return false;

  CXBMCTinyXML doc;
  if (!doc.Parse(deviceDescriptionXml) || doc.RootElement() == NULL)
    return false;

  TiXmlElement *rootElement = doc.RootElement();
  if (!StringUtils::EqualsNoCase(rootElement->ValueStr(), UPnPDeviceElementRoot))
    return false;

  TiXmlNode *deviceNode = rootElement->FirstChild(UPnPDeviceElementDevice);
  if (deviceNode == nullptr)
    return false;

  if (!XMLUtils::GetString(deviceNode, UPnPDeviceElementDeviceType, m_deviceTypeString) ||
      !ParseDeviceType(m_deviceTypeString, m_deviceType, m_deviceTypeVersion))
    return false;

  if (!XMLUtils::GetString(deviceNode, UPnPDeviceElementFriendlyName, m_friendlyName) ||
      m_friendlyName.empty())
    return false;

  if (!XMLUtils::GetString(deviceNode, UPnPDeviceElementManufacturer, m_manufacturer) ||
      m_manufacturer.empty())
    return false;

  XMLUtils::GetString(deviceNode, UPnPDeviceElementManufacturerUrl, m_manufacturerUrl);
  XMLUtils::GetString(deviceNode, UPnPDeviceElementModelDescription, m_modelDescription);

  if (!XMLUtils::GetString(deviceNode, UPnPDeviceElementModelName, m_modelName) ||
      m_modelName.empty())
    return false;

  XMLUtils::GetString(deviceNode, UPnPDeviceElementModelNumber, m_modelNumber);
  XMLUtils::GetString(deviceNode, UPnPDeviceElementModelUrl, m_modelUrl);
  XMLUtils::GetString(deviceNode, UPnPDeviceElementSerialNumber, m_serialNumber);
  XMLUtils::GetString(deviceNode, UPnPDeviceElementUpc, m_universalProductCode);
  XMLUtils::GetString(deviceNode, UPnPDeviceElementPresentationUrl, m_presentationUrl);

  // parse <iconList>
  m_icons.Deserialize(deviceNode->FirstChildElement(m_icons.GetName()), { *this });

  // parse <serviceList>
  const TiXmlNode* serviceListNode = deviceNode->FirstChild(UPnPDeviceElementServiceList);
  if (serviceListNode != nullptr)
  {
    const TiXmlNode* serviceNode = serviceListNode->FirstChild(UPnPDeviceElementService);
    while (serviceNode != nullptr)
    {
      const TiXmlNode* serviceTypeNode = serviceNode->FirstChild(UPnPDeviceElementServiceType);
      if (serviceTypeNode != nullptr && serviceTypeNode->FirstChild() != nullptr && serviceTypeNode->FirstChild()->Type() == TiXmlNode::TINYXML_TEXT)
      {
        const std::string fullServiceType = serviceTypeNode->FirstChild()->ValueStr();
        std::string serviceType;
        uint8_t serviceTypeVersion;

        if (ParseServiceType(fullServiceType, serviceType, serviceTypeVersion))
          m_services.insert({ serviceType, serviceTypeVersion });
      }

      serviceNode = serviceNode->NextSibling(UPnPDeviceElementService);
    }
  }

  return true;
}

bool COhUPnPControlPointDevice::SetBaseUrl(const std::string &descriptionUrl)
{
  if (descriptionUrl.empty())
    return false;

  CURL url(descriptionUrl);
  // remove any options
  url.SetOptions("");
  // remove the path
  url.SetFileName("");

  m_baseUrl = url.Get();

  return !m_baseUrl.empty();
}
