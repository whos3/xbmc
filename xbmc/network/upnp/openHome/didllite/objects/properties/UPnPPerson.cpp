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

#include "UPnPPerson.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPPerson::CUPnPPerson(const std::string& elementName)
  : CUPnPPerson(elementName, "")
{ }

CUPnPPerson::CUPnPPerson(const std::string& elementName, const std::string& person, const std::string& role)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, elementName),
    m_personName(person),
    m_personRole(role)
{
  initializeProperties();

  setPropertyValidity(!m_personName.empty());
  setPropertyValidity("@role", !m_personRole.empty());
}

CUPnPPerson::CUPnPPerson(const CUPnPPerson& upnpPerson)
  : IDidlLiteElement(upnpPerson),
    m_personName(upnpPerson.m_personName),
    m_personRole(upnpPerson.m_personRole)
{
  initializeProperties();
  copyPropertyValidity(&upnpPerson);
}

CUPnPPerson::~CUPnPPerson()
{ }

bool CUPnPPerson::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (m_personName.empty())
    return false;

  setPropertyValidity(!m_personName.empty());
  setPropertyValidity("@role", !m_personRole.empty());

  return true;
}

void CUPnPPerson::initializeProperties()
{
  addStringProperty(&m_personName).SetRequired();
  addStringProperty("@role", &m_personRole).AsAttribute().SetOptional();
}

CUPnPActor::CUPnPActor()
  : CUPnPActor("")
{ }

CUPnPActor::CUPnPActor(const std::string& person, const std::string& role /* = "" */)
  : CUPnPPerson("actor", person, role)
{ }

CUPnPActor::CUPnPActor(const CUPnPActor& upnpActor)
  : CUPnPPerson(upnpActor)
{ }

CUPnPActor::~CUPnPActor()
{ }

CUPnPArtist::CUPnPArtist()
  : CUPnPArtist("")
{ }

CUPnPArtist::CUPnPArtist(const std::string& person, const std::string& role /* = "" */)
  : CUPnPPerson("artist", person, role)
{ }

CUPnPArtist::CUPnPArtist(const CUPnPArtist& upnpArtist)
  : CUPnPPerson(upnpArtist)
{ }

CUPnPArtist::~CUPnPArtist()
{ }

CUPnPAuthor::CUPnPAuthor()
  : CUPnPAuthor("")
{ }

CUPnPAuthor::CUPnPAuthor(const std::string& person, const std::string& role /* = "" */)
  : CUPnPPerson("author", person, role)
{ }

CUPnPAuthor::CUPnPAuthor(const CUPnPAuthor& upnpAuthor)
  : CUPnPPerson(upnpAuthor)
{ }

CUPnPAuthor::~CUPnPAuthor()
{ }
