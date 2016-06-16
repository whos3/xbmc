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

#include "XmlPropertyTreeSerializer.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"
#include "utils/XMLUtils.h"
#include "utils/propertytree/xml/XmlPropertyTree.h"

static const std::string NamespacePrefix = "xmlns";

static bool SerializePropertyValue(CPropertyTreeElementProperty::Type type, const void* propertyValue, std::string& value)
{
  switch (type)
  {
  case CPropertyTreeElementProperty::Type::String:
    value = *reinterpret_cast<const std::string*>(propertyValue);
    break;

  case CPropertyTreeElementProperty::Type::Boolean:
    value = *reinterpret_cast<const bool*>(propertyValue) ? "1" : "0";
    break;

  case CPropertyTreeElementProperty::Type::Integer:
    value = StringUtils::Format("%d", *reinterpret_cast<const int32_t*>(propertyValue));
    break;

  case CPropertyTreeElementProperty::Type::UnsignedInteger:
    value = StringUtils::Format("%u", *reinterpret_cast<const uint32_t*>(propertyValue));
    break;

  case CPropertyTreeElementProperty::Type::Long:
    value = StringUtils::Format("%lld", *reinterpret_cast<const int64_t*>(propertyValue));
    break;

  case CPropertyTreeElementProperty::Type::UnsignedLong:
    value = StringUtils::Format("%llu", *reinterpret_cast<const uint64_t*>(propertyValue));
    break;

  case CPropertyTreeElementProperty::Type::Number:
    value = StringUtils::Format("%.2f", *reinterpret_cast<const double*>(propertyValue));
    break;

  default:
    return false;
  }

  return true;
}

bool CXmlPropertyTreeSerializer::Serialize(const CXmlPropertyTree& tree, std::string& result) const
{
  TiXmlDocument doc;
  if (!Serialize(&tree, &doc))
    return false;

  // get the root element
  TiXmlElement* root = doc.RootElement();
  if (root == nullptr)
    return false;

  // add all namespaces to the root element
  for (const auto& it : tree.GetNamespaces())
  {
    std::string ns = NamespacePrefix;
    if (!it.first.empty())
      ns += ":" + it.first;

    root->SetAttribute(ns, it.second);
  }

  // loop over all known elements and serialize them
  for (auto& elementIt : tree.GetElements())
  {
    if (!Serialize(elementIt, root))
      return false;
  }

  TiXmlPrinter printer;
  doc.Accept(&printer);
  result = printer.Str();

  return !result.empty();
}

bool CXmlPropertyTreeSerializer::Serialize(const IPropertyTreeElement* element, TiXmlNode* node)
{
  if (element == nullptr || node == nullptr)
    return false;

  // create the element
  TiXmlElement newElement(element->GetName());

  // add all specified properties
  for (const auto& prop : element->GetProperties())
  {
    if (!Serialize(prop.second, &newElement))
    {
      // only fail for required properties
      if (prop.second.IsRequired())
        return false;
    }
  }

  // insert the element at the end
  return node->InsertEndChild(newElement) != nullptr;
}

bool CXmlPropertyTreeSerializer::Serialize(const CPropertyTreeElementProperty& prop, TiXmlNode* node)
{
  if (node == nullptr)
    return false;

  // any required property needs to be set
  if (!prop.IsValid())
    return false;

  // the property is stored as an attribute
  if (prop.IsAttribute())
    return SerializeAttribute(prop, node);

  const auto& name = prop.GetName();
  const auto& type = prop.GetType();
  const bool supportsMultipleValues = prop.SupportsMultipleValues();
  if (name.empty())
  {
    if (supportsMultipleValues && type == CPropertyTreeElementProperty::Type::Element)
      return false;

    std::string value;
    if (!SerializePropertyValue(type, prop.GetRawValue(), value))
      return false;

    TiXmlText valueNode(value);
    node->InsertEndChild(valueNode);

    return true;
  }

  switch (type)
  {
  case CPropertyTreeElementProperty::Type::Element:
  {

    std::vector<const IPropertyTreeElement*> values = prop.GetValues<const IPropertyTreeElement*>();
    for (const auto& value : values)
    {
      if (!Serialize(value, node))
        return false;
    }
    break;
  }

  case CPropertyTreeElementProperty::Type::Boolean:
  {
    std::vector<bool> values = prop.GetValues<bool>();
    for (const auto& value : values)
      XMLUtils::SetBoolean(node, name.c_str(), value);
    break;
  }

  case CPropertyTreeElementProperty::Type::String:
  {
    std::vector<std::string> values = prop.GetValues<std::string>();
    for (const auto& value : values)
      XMLUtils::SetString(node, name.c_str(), value);
    break;
  }

  case CPropertyTreeElementProperty::Type::Integer:
  {
    std::vector<int32_t> values = prop.GetValues<int32_t>();
    for (const auto& value : values)
      XMLUtils::SetInt(node, name.c_str(), value);
    break;
  }

  case CPropertyTreeElementProperty::Type::UnsignedInteger:
  {
    std::vector<uint32_t> values = prop.GetValues<uint32_t>();
    for (const auto& value : values)
      XMLUtils::SetInt(node, name.c_str(), value);
    break;
  }

  case CPropertyTreeElementProperty::Type::Long:
  {
    std::vector<int64_t> values = prop.GetValues<int64_t>();
    for (const auto& value : values)
      XMLUtils::SetInt64(node, name.c_str(), value);
    break;
  }

  case CPropertyTreeElementProperty::Type::UnsignedLong:
  {
    std::vector<uint64_t> values = prop.GetValues<uint64_t>();
    for (const auto& value : values)
      XMLUtils::SetUInt64(node, name.c_str(), value);
    break;
  }

  case CPropertyTreeElementProperty::Type::Number:
  {
    std::vector<double> values = prop.GetValues<double>();
    for (const auto& value : values)
      XMLUtils::SetFloat(node, name.c_str(), static_cast<float>(value));
    break;
  }

  default:
    return false;
  }

  return true;
}

bool CXmlPropertyTreeSerializer::SerializeAttribute(const CPropertyTreeElementProperty& prop, TiXmlNode* node)
{
  if (node == nullptr)
    return false;

  TiXmlElement* element = node->ToElement();
  if (element == nullptr)
    return false;

  std::string value;
  // try to serialize the property's value into a string
  if (!SerializePropertyValue(prop.GetType(), prop.GetRawValue(), value))
    return false;

  // set the attribute
  element->SetAttribute(prop.GetName(), value);

  return true;
}
