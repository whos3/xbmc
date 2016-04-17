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
#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/didllite/objects/FileItemElementFactory.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferManager.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"
#include "utils/XBMCTinyXml.h"

static const size_t MaximumAttributeValueLength = 256;

bool COhUPnPRootDevice::m_deviceStackStarted = false;

COhUPnPRootDevice::COhUPnPRootDevice(const std::string& uuid,
  const std::string& deviceDomain, const std::string& deviceType, uint8_t deviceVersion,
  const CFileItemElementFactory& fileItemElementFactory,
  COhUPnPTransferManager& transferManager,
  COhUPnPResourceManager& resourceManager)
  : COhUPnPDevice(),
    IOhUPnPService(deviceDomain, deviceType, deviceVersion),
    m_device(nullptr),
    m_elementFactory(fileItemElementFactory),
    m_transferManager(transferManager),
    m_resourceManager(resourceManager),
    m_lastIpAddress(0)
{
  m_uuid = uuid;
  SetValid(!m_uuid.empty());
}

COhUPnPRootDevice::~COhUPnPRootDevice()
{
  Stop();
}

bool COhUPnPRootDevice::Start(TIpAddress ipAddress)
{
  if (IsRunning())
  {
    // if the IP address is the same there's nothing to do
    if (ipAddress == m_lastIpAddress)
      return true;

    // otherwise we need to stop the device
    Stop();
  }

  m_lastIpAddress = ipAddress;

  if (!m_deviceStackStarted)
  {
    CLog::Log(LOGINFO, "COhUPnPRootDevice: starting device stack...");
    OpenHome::Net::UpnpLibrary::StartDv();

    m_deviceStackStarted = true;
  }

  CLog::Log(LOGINFO, "COhUPnPRootDevice: starting MediaServer device...");

  // setup device
  m_device = std::make_shared<OpenHome::Net::DvDeviceStdStandard>(m_uuid, m_resourceManager);
  SetDomain(m_domain);
  SetDeviceType(m_serviceName);
  SetDeviceTypeVersion(m_serviceVersion);
  SetFriendlyName(CSysInfo::GetDeviceName());
  SetModelName(CSysInfo::GetAppName());
  SetModelNumber(CSysInfo::GetVersion());
  SetModelUrl("http://kodi.tv/");
  SetManufacturer("XBMC Foundation");
  SetManufacturerUrl("http://kodi.tv/");
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_WEBSERVER))
    SetPresentationUrl(StringUtils::Format("http://%s:%d", COhUtils::TIpAddressToString(m_lastIpAddress).c_str(), CSettings::GetInstance().GetInt(CSettings::SETTING_SERVICES_WEBSERVERPORT)));
  SetIcons({
    CUPnPIcon(m_resourceManager.AddSmallResource(*this, "special://xbmc/media/icon256x256.png", "icon256x256.png"), "image/png", 256, 256, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*this, "special://xbmc/media/icon120x120.png", "icon120x120.png"), "image/png", 120, 120, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*this, "special://xbmc/media/icon48x48.png", "icon48x48.png"), "image/png", 48, 48, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*this, "special://xbmc/media/icon32x32.png", "icon32x32.png"), "image/png", 32, 32, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*this, "special://xbmc/media/icon16x16.png", "icon16x16.png"), "image/png", 16, 16, 8)
  });

  // allow additional configuration of the device
  SetupDevice(GetDevice());

  // start all services
  if (!StartServices())
  {
    CLog::Log(LOGERROR, "COhUPnPRootDevice: failed to start services");
    m_device.reset();

    return false;
  }

  // enable the device
  m_device->SetEnabled();

  return true;
}

bool COhUPnPRootDevice::IsRunning() const
{
  return m_device != nullptr;
}

void COhUPnPRootDevice::Stop()
{
  if (!IsRunning())
    return;

  CLog::Log(LOGINFO, "COhUPnPRootDevice: stopping MediaServer device...");

  // disable the device
  m_device->SetDisabled(OpenHome::MakeFunctor(*this, &COhUPnPRootDevice::OnDeviceDisabled));

  // wait for the device to be disabled
  if (!m_deviceDisabledEvent.Wait())
    CLog::Log(LOGWARNING, "COhUPnPRootDevice: device didn't stop in time");

  // stop all services
  if (!StopServices())
    CLog::Log(LOGERROR, "COhUPnPRootDevice: failed to stop services");

  // and finally destroy the device
  m_device.reset();
}

void COhUPnPRootDevice::Restart()
{
  Stop();
  Start(m_lastIpAddress);
}

void COhUPnPRootDevice::OnDeviceDisabled()
{
  m_deviceDisabledEvent.Set();
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
  if (m_device == nullptr || m_device->Enabled())
    return false;

  m_device->SetAttribute(name.c_str(), value.c_str());
  return true;
}

