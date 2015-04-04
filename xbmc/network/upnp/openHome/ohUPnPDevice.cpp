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

bool COhUPnPDevice::ParseDeviceType(const std::string &deviceTypeString, std::string &deviceType, uint8_t &version)
{
  // the deviceType value must be of the form "urn:<domain-name>:device:<deviceType>:<version>"
  if (!StringUtils::StartsWith(deviceTypeString, "urn:"))
    return false;

  std::string tmp = StringUtils::Mid(deviceTypeString, 4);
  size_t pos = tmp.find(':');
  if (pos == std::string::npos)
    return false;

  // we don't care for the domain-name part
  tmp = StringUtils::Mid(tmp, pos + 1);

  // make sure the next part is "device:"
  if (!StringUtils::StartsWith(tmp, "device:"))
    return false;
  tmp = StringUtils::Mid(tmp, 7);

  // now split the deviceType and the version
  pos = tmp.find(":");
  if (pos == std::string::npos)
    return false;

  // extract the deviceType
  deviceType = StringUtils::Left(tmp, pos);
  tmp = StringUtils::Mid(tmp, pos + 1);

  char *endptr = NULL;
  version = static_cast<uint8_t>(strtoul(tmp.c_str(), &endptr, 0));
  if (endptr != NULL && *endptr != '\0')
    return false;

  return true;
}
