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

#include "DidlLiteDocument.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/didllite/DidlLiteElementFactory.h"
#include "utils/XBMCTinyXml.h"

#define XML_NAMESPACE_PREFIX  "xmlns"

CDidlLiteDocument::CDidlLiteDocument(const CDidlLiteElementFactory& factory)
  : IDidlLiteElement("DIDL-Lite"),
    m_factory(factory)
{
  m_namespaces.insert(std::make_pair("", UPNP_DIDL_LITE_NAMESPACE_URI));
}

CDidlLiteDocument::~CDidlLiteDocument()
{
  clear();
}

void CDidlLiteDocument::AddNamespace(const std::string& uri)
{
  AddNamespace("", uri);
}

void CDidlLiteDocument::AddNamespace(const std::string& name, const std::string& uri)
{
  if (uri.empty())
    return;

  auto& ns = m_namespaces.find(name);
  if (ns == m_namespaces.end())
    m_namespaces.insert(std::make_pair(name, uri));
  else
    ns->second = uri;
}

void CDidlLiteDocument::AddElement(IDidlLiteElement* element)
{
  if (element == nullptr)
    return;

  m_elements.push_back(element);
}

bool CDidlLiteDocument::Serialize(std::string& result, const OhUPnPRootDeviceContext& context) const
{
  TiXmlDocument doc;
  if (!IDidlLiteElement::Serialize(&doc, context))
    return false;

  TiXmlPrinter printer;
  doc.Accept(&printer);
  result = printer.Str();

  return !result.empty();
}

bool CDidlLiteDocument::Deserialize(const std::string& data, const OhUPnPControlPointContext& context)
{
  TiXmlDocument doc;
  doc.Parse(data.c_str());

  TiXmlElement* root = doc.RootElement();
  if (root == nullptr)
    return false;

  return IDidlLiteElement::Deserialize(root, context);
}

std::vector<const IDidlLiteElement*> CDidlLiteDocument::GetElements() const
{
  std::vector<const IDidlLiteElement*> elements;
  for (const auto& element : m_elements)
    elements.push_back(element);

  return elements;
}

bool CDidlLiteDocument::serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const
{
  if (node == nullptr)
    return false;

  TiXmlElement* element = node->ToElement();
  if (element == nullptr)
    return false;

  // add all namespaces
  for (const auto& it : m_namespaces)
  {
    std::string ns = XML_NAMESPACE_PREFIX;
    if (!it.first.empty())
      ns += ":" + it.first;

    element->SetAttribute(ns, it.second);
  }

  // loop over all known elements and serialize them
  for (auto& elementIt : m_elements)
  {
    if (!elementIt->Serialize(node, context))
      return false;
  }

  return true;
}

bool CDidlLiteDocument::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (node == nullptr)
    return false;

  clear();
  // loop over all children and deserialize them
  const TiXmlElement* child = node->FirstChildElement();
  while (child != nullptr)
  {
    IDidlLiteElement* didlLiteElement = m_factory.GetElement(child);
    if (didlLiteElement == nullptr)
      return false;

    if (!didlLiteElement->Deserialize(child, context))
    {
      delete didlLiteElement;
      return false;
    }

    m_elements.push_back(didlLiteElement);
    child = child->NextSiblingElement();
  }

  return true;
}

void CDidlLiteDocument::clear()
{
  for (const auto& element : m_elements)
    delete element;

  m_elements.clear();
}
