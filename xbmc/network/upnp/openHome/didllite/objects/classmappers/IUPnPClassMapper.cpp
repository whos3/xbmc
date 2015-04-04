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

#include "IUPnPClassMapper.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "utils/StringUtils.h"

IUPnPClassMapper::IUPnPClassMapper(const std::string& classType, const std::string& mediaType)
  : m_classType(classType),
    m_mediaType(mediaType)
{ }

IUPnPClassMapper::~IUPnPClassMapper()
{ }

bool IUPnPClassMapper::Matches(const IFileItemElement* fileItem) const
{
  if (fileItem == nullptr)
    return false;

  if (StringUtils::EqualsNoCase(m_classType, fileItem->GetType()))
    return true;

  return matches(fileItem);
}
