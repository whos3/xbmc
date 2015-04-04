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

#include "DidlLitePropertyList.h"
#include "utils/StringUtils.h"

static const std::string UPnPAllProperties = "*";
static const std::string UPnPPropertiesSeparator = ",";

CDidlLitePropertyList::CDidlLitePropertyList()
{ }

CDidlLitePropertyList::CDidlLitePropertyList(const std::string &properties)
  : CDidlLitePropertyList(StringUtils::Split(properties, UPnPPropertiesSeparator))
{ }

CDidlLitePropertyList::CDidlLitePropertyList(const std::vector<std::string> &properties)
  : CDidlLitePropertyList(std::set<std::string>(properties.begin(), properties.end()))
{ }

CDidlLitePropertyList::CDidlLitePropertyList(const std::set<std::string> &properties)
{
  for (const auto& prop : properties)
    AddProperty(prop);
}

std::string CDidlLitePropertyList::ToString() const
{
  if (m_properties.empty())
    return UPnPAllProperties;

  return StringUtils::Join(std::vector<std::string>(m_properties.begin(), m_properties.end()), UPnPPropertiesSeparator);
}

bool CDidlLitePropertyList::Contains(const std::string& prop) const
{
  if (prop.empty())
    return false;

  if (m_properties.empty())
    return true;

  return m_properties.find(prop) != m_properties.cend();
}

bool CDidlLitePropertyList::Contains(const std::vector<std::string>& properties) const
{
  return Contains(std::set<std::string>(properties.begin(), properties.end()));
}

bool CDidlLitePropertyList::Contains(const std::set<std::string>& properties) const
{
  if (properties.empty())
    return false;

  for (const auto& prop : properties)
  {
    if (!Contains(prop))
      return false;
  }

  return true;
}

void CDidlLitePropertyList::AddProperty(const std::string& prop)
{
  if (prop.empty())
    return;

  if (prop == UPnPAllProperties)
  {
    m_properties.clear();
    return;
  }

  m_properties.insert(prop);
}
