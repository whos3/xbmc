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

#include "UPnPGenre.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

static bool IsGenreValid(const std::string& genre)
{
  return !genre.empty() && genre != "Unknown";
}

CUPnPGenre::CUPnPGenre()
  : CUPnPGenre("")
{ }

CUPnPGenre::CUPnPGenre(const std::string& name)
  : CUPnPGenre(name, "", "")
{ }

CUPnPGenre::CUPnPGenre(const std::string& name, const std::string& id, const std::string& extended)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, "genre"),
    m_genreName(name),
    m_genreId(id),
    m_genreExtended(extended)
{
  initializeProperties();

  setPropertyValidity(IsGenreValid(m_genreName));
  setPropertyValidity("@id", !m_genreId.empty());
  setPropertyValidity("@extended", !m_genreExtended.empty());
}

CUPnPGenre::CUPnPGenre(const CUPnPGenre& upnpGenre)
  : IDidlLiteElement(upnpGenre),
    m_genreName(upnpGenre.m_genreName),
    m_genreId(upnpGenre.m_genreId),
    m_genreExtended(upnpGenre.m_genreExtended)
{
  initializeProperties();
  copyPropertyValidity(&upnpGenre);
}

CUPnPGenre::~CUPnPGenre()
{ }

bool CUPnPGenre::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (!IsGenreValid(m_genreName))
    return false;

  setPropertyValid();

  // @id must be present if @extended is present
  if (!m_genreExtended.empty() && m_genreId.empty())
    return false;

  setPropertyValidity("@id", !m_genreId.empty());
  setPropertyValidity("@extended", !m_genreExtended.empty());

  return true;
}

void CUPnPGenre::initializeProperties()
{
  addStringProperty(&m_genreName).SetRequired();
  addStringProperty("@id", &m_genreId).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@extended", &m_genreExtended).AsAttribute().SetOptional().SetMinimumVersion(2);
}
