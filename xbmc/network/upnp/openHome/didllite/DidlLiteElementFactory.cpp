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

#include <algorithm>

#include "DidlLiteElementFactory.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/didllite/IDidlLiteElement.h"
#include "network/upnp/openHome/didllite/objects/UPnPObject.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"
#include "utils/XMLUtils.h"

CDidlLiteElementFactory::CDidlLiteElementFactory()
{ }

CDidlLiteElementFactory::~CDidlLiteElementFactory()
{ }

IDidlLiteElement* CDidlLiteElementFactory::GetElement(const TiXmlNode* xmlNode) const
{
  if (xmlNode == nullptr)
    return nullptr;

  std::string elementName;
  // try to find a upnp:class element
  if (XMLUtils::GetString(xmlNode, DidlLiteUtils::GetElementName(UPNP_DIDL_UPNP_NAMESPACE, "class").c_str(), elementName) && !elementName.empty())
  {
    const auto& upnpObjectIt = FindElementByName(m_elements, elementName);
    if (upnpObjectIt != m_elements.cend())
      return upnpObjectIt->second->Clone();

    // try to find the best partial match
    std::vector<std::string> upnpClassTypeParts = StringUtils::Split(elementName, ".");
    std::shared_ptr<const CUPnPObject> bestMatchingUPnPObject = nullptr;
    size_t bestMatchingCount = 0;
    for (auto& it : m_elements)
    {
      // ignore non UPnP objects
      std::shared_ptr<const CUPnPObject> upnpObject = std::dynamic_pointer_cast<const CUPnPObject>(it.second);
      if (upnpObject == nullptr)
        continue;

      size_t count = 0;
      std::vector<std::string> parts = StringUtils::Split(it.first, ".");
      for (; count < upnpClassTypeParts.size() && count < parts.size(); ++count)
      {
        // abort if a part doesn't match
        if (upnpClassTypeParts.at(count).compare(parts.at(count)) != 0)
          break;
      }

      if (count > bestMatchingCount)
      {
        bestMatchingUPnPObject = upnpObject;
        bestMatchingCount = count;
      }
    }

    // return the best matching element if we found one
    if (bestMatchingUPnPObject != nullptr)
    {
      CUPnPObject* upnpObject = dynamic_cast<CUPnPObject*>(bestMatchingUPnPObject->Clone());

      // overwrite the UPnP class type for proper deserialization
      upnpObject->GetClass().SetType(elementName);

      return upnpObject;
    }
  }

  // use the XML element name
  elementName = xmlNode->ValueStr();

  const auto& elementIt = FindElementByName(m_elements, elementName);
  if (elementIt != m_elements.cend())
    return elementIt->second->Clone();

  return nullptr;
}

bool CDidlLiteElementFactory::RegisterElement(const IDidlLiteElement* element)
{
  if (element == nullptr)
    return false;

  std::shared_ptr<const IDidlLiteElement> elementPtr = std::shared_ptr<const IDidlLiteElement>(element);

  // determine the element name
  std::string elementName = element->GetName();

  // check if the item is an object
  const CUPnPObject* upnpObject = dynamic_cast<const CUPnPObject*>(element);
  if (upnpObject != nullptr && !upnpObject->GetClass().GetType().empty())
  {
    // now determine the upnp object's class type
    elementName = upnpObject->GetClass().GetType();
  }

  const auto& it = FindElementByName(m_elements, elementName);
  if (it != m_elements.cend())
    return false;

  m_elements.push_back(std::make_pair(elementName, elementPtr));

  return true;
}

CDidlLiteElementFactory::IDidlLiteElementPairs::const_iterator CDidlLiteElementFactory::FindElementByName(const IDidlLiteElementPairs& elements, const std::string& elementName)
{
  return std::find_if(elements.cbegin(), elements.cend(),
    [&elementName](const IDidlLiteElementPair& pair)
    {
      return pair.first == elementName;
    });
}
