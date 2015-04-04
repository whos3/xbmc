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
#include <algorithm>

#include "UPnPIcon.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPIcon::CUPnPIcon()
  : CUPnPIcon("", "", 0, 0, 0)
{ }

CUPnPIcon::CUPnPIcon(const std::string &url, const std::string &mimetype, uint32_t width, uint32_t height, uint32_t depth)
  : IDidlLiteElement("", "icon"),
    m_url(url),
    m_mimetype(mimetype),
    m_width(width),
    m_height(height),
    m_depth(depth)
{
  initializeProperties();

  setPropertyValidity("url", !m_url.empty());
  setPropertyValidity("mimetype", !m_mimetype.empty());
  setPropertyValidity("width", m_width > 0);
  setPropertyValidity("height", m_height > 0);
  setPropertyValidity("depth", m_depth > 0);
}

CUPnPIcon::CUPnPIcon(const CUPnPIcon& upnpIcon)
  : IDidlLiteElement(upnpIcon),
    m_url(upnpIcon.m_url),
    m_mimetype(upnpIcon.m_mimetype),
    m_width(upnpIcon.m_width),
    m_height(upnpIcon.m_height),
    m_depth(upnpIcon.m_depth)
{
  initializeProperties();
  copyPropertyValidity(&upnpIcon);
}

CUPnPIcon::~CUPnPIcon()
{ }

bool CUPnPIcon::operator<(const CUPnPIcon &other) const
{
  if (m_width < other.m_width)
    return true;
  if (m_height < other.m_height)
    return true;
  if (m_depth < other.m_depth)
    return true;

  if (m_mimetype != "image/png" && other.m_mimetype == "image/png")
    return true;

  return false;
}

void CUPnPIcon::initializeProperties()
{
  addStringProperty("url", &m_url).SetRequired();
  addStringProperty("mimetype", &m_mimetype).SetOptional();
  addUnsignedIntegerProperty("width", &m_width).SetOptional();
  addUnsignedIntegerProperty("height", &m_height).SetOptional();
  addUnsignedIntegerProperty("depth", &m_depth).SetOptional();
}

CUPnPIconList::CUPnPIconList()
  : CUPnPIconList(std::vector<CUPnPIcon>())
{ }

CUPnPIconList::CUPnPIconList(const std::vector<CUPnPIcon>& icons)
  : IDidlLiteElement("", "iconList")
{
  initializeProperties();

  for (const auto& icon : icons)
    m_icons.push_back(icon.Clone());

  setPropertyValidity("icon", !m_icons.empty());
}

CUPnPIconList::CUPnPIconList(const CUPnPIconList& upnpIcons)
  : IDidlLiteElement(upnpIcons)
{
  copyElementProperty(m_icons, upnpIcons.m_icons);

  initializeProperties();
  copyPropertyValidity(&upnpIcons);
}

CUPnPIconList::~CUPnPIconList()
{
  // TODO[crash]: clearElementProperty(m_icons);
}

template<class TPointer>
static bool ComparePointers(const TPointer* lhs, const TPointer* rhs)
{
  return *lhs < *rhs;
}

std::vector<const CUPnPIcon*> CUPnPIconList::GetIcons() const
{
  std::vector<const CUPnPIcon*> icons;
  std::copy(m_icons.cbegin(), m_icons.cend(), std::back_inserter(icons));

  // sort by desencing icon quality
  std::sort(icons.begin(), icons.end(), ComparePointers<CUPnPIcon>);
  std::reverse(icons.begin(), icons.end());

  return icons;
}

void CUPnPIconList::initializeProperties()
{
  addElementProperty("", "icon", &m_icons).SetRequired().SupportMultipleValues().SetGenerator(std::make_shared<CUPnPIcon>());
}
