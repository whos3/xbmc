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

#include "UPnPDateTime.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "utils/XBMCTinyXml.h"

static const std::string DaylightSavingAttribute = "daylightSaving";
static const std::string UPnPDaylightSavingAttribute = "upnp:" + DaylightSavingAttribute;

static const std::string DaylightSavingUnknown = "UNKNOWN";
static const std::string DaylightSavingStandard = "STANDARD";
static const std::string DaylightSavingDaylightSaving = "DAYLIGHTSAVING";

CUPnPDate::CUPnPDate()
  : CUPnPDate("")
{ }

CUPnPDate::CUPnPDate(const std::string& date, UPnPDaylightSaving daylightSaving /* = UPnPDaylightSaving::Unknown */)
  : CUPnPDate(DidlLiteUtils::GetDate(date))
{
  m_daylightSaving = daylightSaving;
}

CUPnPDate::CUPnPDate(const CDateTime& date)
  : IDidlLiteElement(UPNP_DIDL_DC_NAMESPACE, "date"),
    m_date(date),
    m_daylightSaving(UPnPDaylightSaving::Unknown)
{ }

CUPnPDate::CUPnPDate(const CUPnPDate& upnpDate)
  : IDidlLiteElement(upnpDate),
    m_date(upnpDate.m_date),
    m_daylightSaving(upnpDate.m_daylightSaving)
{
  copyPropertyValidity(&upnpDate);
}

CUPnPDate::~CUPnPDate()
{ }

bool CUPnPDate::serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const
{
  if (node == nullptr)
    return false;

  TiXmlText value(DidlLiteUtils::GetDate(m_date));
  node->InsertEndChild(value);

  // @daylightSaving
  if (context.profile.GetContentDirectoryVersion() >= 3)
  {
    std::string daylightSaving;
    switch (m_daylightSaving)
    {
      case UPnPDaylightSaving::Standard:
        daylightSaving = DaylightSavingStandard;
        break;

      case UPnPDaylightSaving::DaylightSaving:
        daylightSaving = DaylightSavingDaylightSaving;
        break;

      case UPnPDaylightSaving::Unknown:
      default:
        break;
    }

    if (!daylightSaving.empty())
    {
      TiXmlElement* element = node->ToElement();
      if (element == nullptr)
        return false;

      element->SetAttribute(UPnPDaylightSavingAttribute, DaylightSavingStandard);
    }
  }

  return true;
}

bool CUPnPDate::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (node->FirstChild() == nullptr)
    return false;

  m_date = DidlLiteUtils::GetDate(node->FirstChild()->ValueStr());
  if (!m_date.IsValid())
    return false;

  m_daylightSaving = UPnPDaylightSaving::Unknown;

  const TiXmlElement* element = node->ToElement();
  const std::string* daylightSaving = element->Attribute(UPnPDaylightSavingAttribute);
  if (daylightSaving != nullptr)
  {
    if (*daylightSaving == DaylightSavingUnknown)
      m_daylightSaving = UPnPDaylightSaving::Unknown;
    else if (*daylightSaving == DaylightSavingStandard)
      m_daylightSaving = UPnPDaylightSaving::Standard;
    else if (*daylightSaving == DaylightSavingDaylightSaving)
      m_daylightSaving = UPnPDaylightSaving::DaylightSaving;
  }

  return true;
}

CUPnPDateTime::CUPnPDateTime(const std::string& elementName)
  : CUPnPDateTime(elementName, "")
{ }

CUPnPDateTime::CUPnPDateTime(const std::string& elementName, const std::string& dateTime, UPnPDaylightSaving daylightSaving /* = UPnPDaylightSaving::Unknown */)
  : CUPnPDateTime(elementName, DidlLiteUtils::GetDateTime(dateTime))
{
  m_daylightSaving = daylightSaving;
}

CUPnPDateTime::CUPnPDateTime(const std::string& elementName, const CDateTime& dateTime)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, elementName),
    m_dateTime(dateTime),
    m_daylightSaving(UPnPDaylightSaving::Unknown)
{ }

CUPnPDateTime::CUPnPDateTime(const CUPnPDateTime& upnpDateTime)
  : IDidlLiteElement(upnpDateTime),
    m_dateTime(upnpDateTime.m_dateTime),
    m_daylightSaving(upnpDateTime.m_daylightSaving)
{
  copyPropertyValidity(&upnpDateTime);
}

CUPnPDateTime::~CUPnPDateTime()
{ }

bool CUPnPDateTime::serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const
{
  if (node == nullptr)
    return false;

  TiXmlText value(DidlLiteUtils::GetDateTime(m_dateTime));
  node->InsertEndChild(value);

  // @daylightSaving
  if (context.profile.GetContentDirectoryVersion() >= 3)
  {
    std::string daylightSaving;
    switch (m_daylightSaving)
    {
    case UPnPDaylightSaving::Standard:
      daylightSaving = DaylightSavingStandard;
      break;

    case UPnPDaylightSaving::DaylightSaving:
      daylightSaving = DaylightSavingDaylightSaving;
      break;

    case UPnPDaylightSaving::Unknown:
    default:
      break;
    }

    if (!daylightSaving.empty())
    {
      TiXmlElement* element = node->ToElement();
      if (element == nullptr)
        return false;

      element->SetAttribute(UPnPDaylightSavingAttribute, DaylightSavingStandard);
    }
  }

  return true;
}

bool CUPnPDateTime::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (node->FirstChild() == nullptr)
    return false;

  m_dateTime = DidlLiteUtils::GetDateTime(node->FirstChild()->ValueStr());
  if (!m_dateTime.IsValid())
    return false;

  m_daylightSaving = UPnPDaylightSaving::Unknown;

  const TiXmlElement* element = node->ToElement();
  const std::string* daylightSaving = element->Attribute(DaylightSavingAttribute);
  if (daylightSaving != nullptr)
  {
    if (*daylightSaving == DaylightSavingUnknown)
      m_daylightSaving = UPnPDaylightSaving::Unknown;
    else if (*daylightSaving == DaylightSavingStandard)
      m_daylightSaving = UPnPDaylightSaving::Standard;
    else if (*daylightSaving == DaylightSavingDaylightSaving)
      m_daylightSaving = UPnPDaylightSaving::DaylightSaving;
  }

  return true;
}
