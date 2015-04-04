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

#include "UPnPRating.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPRating::CUPnPRating()
  : CUPnPRating("")
{ }

CUPnPRating::CUPnPRating(const std::string& rating, const std::string& type /* = "" */, const std::string& advice /* = "" */, const std::string& equivalentAge /* = "" */)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, "rating"),
    m_rating(rating),
    m_ratingType(type),
    m_ratingAdvice(advice),
    m_ratingEquivalentAge(equivalentAge)
{
  initializeProperties();

  setPropertyValidity(!m_rating.empty());
  setPropertyValidity("@type", !m_ratingType.empty());
  setPropertyValidity("@advice", !m_ratingAdvice.empty());
  setPropertyValidity("@equivalentAge", !m_ratingEquivalentAge.empty());
}

CUPnPRating::CUPnPRating(const CUPnPRating& upnpRating)
  : IDidlLiteElement(upnpRating),
    m_rating(upnpRating.m_rating),
    m_ratingType(upnpRating.m_ratingType),
    m_ratingAdvice(upnpRating.m_ratingAdvice),
    m_ratingEquivalentAge(upnpRating.m_ratingEquivalentAge)
{
  initializeProperties();
  copyPropertyValidity(&upnpRating);
}

CUPnPRating::~CUPnPRating()
{ }

bool CUPnPRating::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  return !m_rating.empty();
}

void CUPnPRating::initializeProperties()
{
  addStringProperty(&m_rating).SetRequired();
  addStringProperty("@type", &m_ratingType).AsAttribute().SetOptional();
  addStringProperty("@advice", &m_ratingAdvice).AsAttribute().SetOptional();
  addStringProperty("@equivalentAge", &m_ratingEquivalentAge).AsAttribute().SetOptional();
}
