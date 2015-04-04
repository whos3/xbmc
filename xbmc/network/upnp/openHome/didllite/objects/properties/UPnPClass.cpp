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

#include "UPnPClass.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPClass::CUPnPClass(const std::string& classType /* = "" */, const std::string& className /* = "" */)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, "class"),
    m_classType(classType),
    m_className(className)
{
  initializeProperties();

  setPropertyValidity(!m_classType.empty());
  setPropertyValidity("@name", !m_className.empty());
}

CUPnPClass::CUPnPClass(const CUPnPClass& upnpClass)
  : IDidlLiteElement(upnpClass),
    m_classType(upnpClass.m_classType),
    m_className(upnpClass.m_className)
{
  initializeProperties();
  copyPropertyValidity(&upnpClass);
}

CUPnPClass::~CUPnPClass()
{ }

bool CUPnPClass::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (m_classType.empty())
    return false;

  setPropertyValidity(!m_classType.empty());
  setPropertyValidity("@name", !m_className.empty());

  return true;
}

void CUPnPClass::initializeProperties()
{
  addStringProperty(&m_classType).SetRequired();
  addStringProperty("@name", &m_className).AsAttribute().SetOptional();
}
