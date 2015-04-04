/*
 *      Copyright (C) 2015 Team XBMC
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

#include "UPnPClassMapping.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "network/upnp/openHome/didllite/objects/classmappers/IUPnPClassMapper.h"
#include "utils/StringUtils.h"

CUPnPClassMapping::CUPnPClassMapping(const std::string& defaultMediaType /* = "" */)
  : m_defaultMediaType(defaultMediaType)
{ }

CUPnPClassMapping::~CUPnPClassMapping()
{
  m_mappers.clear();
}

CUPnPClassMapping& CUPnPClassMapping::Get()
{
  static CUPnPClassMapping s_instance;
  return s_instance;
}

const std::string& CUPnPClassMapping::GetMediaType(const IFileItemElement* fileItem) const
{
  if (fileItem == nullptr)
    return m_defaultMediaType;

  std::string upnpClassType = fileItem->GetType();
  if (upnpClassType.empty())
    return m_defaultMediaType;

  StringUtils::ToLower(upnpClassType);

  // check if there is an exact match on the upnp:class
  const auto& it = m_mappers.find(upnpClassType);
  if (it != m_mappers.end())
    return it->second->GetMediaType();

  // try all mappers manually
  for (const auto& mapper : m_mappers)
  {
    if (mapper.second->Matches(fileItem))
      return mapper.second->GetMediaType();
  }

  // no match found
  return m_defaultMediaType;
}

void CUPnPClassMapping::RegisterMapper(const IUPnPClassMapper* mapper)
{
  if (mapper == nullptr)
    return;

  std::string upnpClassType = mapper->GetClass();
  if (upnpClassType.empty())
    return;

  StringUtils::ToLower(upnpClassType);

  const auto& it = m_mappers.find(upnpClassType);
  if (it != m_mappers.end())
    return;

  m_mappers.insert(std::make_pair(upnpClassType, std::shared_ptr<const IUPnPClassMapper>(mapper)));
}
