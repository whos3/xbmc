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
#include "ohUPnPDevice.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

COhUPnPDevice::COhUPnPDevice()
  : m_valid(false),
    m_deviceTypeVersion(1)
{ }

COhUPnPDevice::~COhUPnPDevice()
{ }

std::string COhUPnPDevice::GetIconUrl() const
{
  if (!m_valid)
    return "";

  const auto& icons = m_icons.GetIcons();
  if (icons.empty())
    return "";

  return URIUtils::AddFileToFolder(m_baseUrl, icons.front()->GetUrl());
}

std::string COhUPnPDevice::GetIconUrl(const std::string &mimetype) const
{
  if (!m_valid)
    return "";

  const auto& icons = m_icons.GetIcons();
  if (icons.empty())
    return "";

  if (mimetype.empty())
    return GetIconUrl();

  for (const auto& icon : icons)
  {
    if (StringUtils::EqualsNoCase(icon->GetMimeType(), mimetype))
      return URIUtils::AddFileToFolder(m_baseUrl, icon->GetUrl());
  }

  return "";
}

std::set<std::pair<std::string, uint8_t>> COhUPnPDevice::GetServices() const
{
  return std::set<std::pair<std::string, uint8_t>>(m_services.cbegin(), m_services.cend());
}

bool COhUPnPDevice::HasService(const std::string& serviceType) const
{
  if (serviceType.empty())
    return false;

  return m_services.find(serviceType) != m_services.cend();
}

bool COhUPnPDevice::GetServiceVersion(const std::string& serviceType, uint8_t& serviceVersion)
{
  if (serviceType.empty())
    return false;

  const auto& service = m_services.find(serviceType);
  if (service == m_services.cend())
    return false;

  serviceVersion = service->second;
  return true;
}

bool COhUPnPDevice::ParseDeviceType(const std::string &deviceTypeString, std::string &deviceType, uint8_t &version)
{
  return ParseType(deviceTypeString, "device", deviceType, version);
}

bool COhUPnPDevice::ParseServiceType(const std::string &serviceTypeString, std::string &serviceType, uint8_t &version)
{
  return ParseType(serviceTypeString, "service", serviceType, version);
}

bool COhUPnPDevice::ParseType(const std::string &typeString, const std::string& typeName, std::string &type, uint8_t &version)
{
  if (typeName.empty())
    return false;

  // the deviceType value must be of the form "urn:<domain-name>:device:<deviceType>:<version>"
  if (!StringUtils::StartsWith(typeString, "urn:"))
    return false;

  std::string tmp = StringUtils::Mid(typeString, 4);
  size_t pos = tmp.find(':');
  if (pos == std::string::npos)
    return false;

  // we don't care for the domain-name part
  tmp = StringUtils::Mid(tmp, pos + 1);

  // make sure the next part matches the type name
  if (!StringUtils::StartsWith(tmp, typeName + ":"))
    return false;
  tmp = StringUtils::Mid(tmp, typeName.size() + 1);

  // now split the deviceType and the version
  pos = tmp.find(":");
  if (pos == std::string::npos)
    return false;

  // extract the deviceType
  type = StringUtils::Left(tmp, pos);
  tmp = StringUtils::Mid(tmp, pos + 1);

  char *endptr = NULL;
  version = static_cast<uint8_t>(strtoul(tmp.c_str(), &endptr, 0));
  if (endptr != NULL && *endptr != '\0')
    return false;

  return true;
}
