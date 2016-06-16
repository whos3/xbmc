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

#include "XmlPropertyTreeDeserializer.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"
#include "utils/XMLUtils.h"
#include "utils/propertytree/xml/XmlPropertyTree.h"

static const std::string NamespacePrefix = "xmlns"; // TODO: combine with XmlPropertyTreeSerializer

static bool DeserializePropertyValue(CPropertyTreeElementProperty::Type type, const std::string& value, void* propertyValue)
{
  switch (type)
  {
  case CPropertyTreeElementProperty::Type::String:
    *reinterpret_cast<std::string*>(propertyValue) = value;
    break;

  case CPropertyTreeElementProperty::Type::Boolean:
    if (value.compare("0") == 0)
      *reinterpret_cast<bool*>(propertyValue) = false;
    else if (value.compare("1") == 0)
      *reinterpret_cast<bool*>(propertyValue) = true;
    else if (StringUtils::EqualsNoCase(value, "false") || StringUtils::EqualsNoCase(value, "no"))
      *reinterpret_cast<bool*>(propertyValue) = false;
    else if (StringUtils::EqualsNoCase(value, "true") || StringUtils::EqualsNoCase(value, "yes"))
      *reinterpret_cast<bool*>(propertyValue) = true;
    else
      return false;

    break;

  case CPropertyTreeElementProperty::Type::Integer:
  {
    char* end = nullptr;
    *reinterpret_cast<int32_t*>(propertyValue) = static_cast<int32_t>(strtol(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CPropertyTreeElementProperty::Type::UnsignedInteger:
  {
    char* end = nullptr;
    *reinterpret_cast<uint32_t*>(propertyValue) = static_cast<uint32_t>(strtoul(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CPropertyTreeElementProperty::Type::Long:
  {
    char* end = nullptr;
    *reinterpret_cast<int64_t*>(propertyValue) = static_cast<int64_t>(strtoll(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CPropertyTreeElementProperty::Type::UnsignedLong:
  {
    char* end = nullptr;
    *reinterpret_cast<uint64_t*>(propertyValue) = static_cast<uint64_t>(strtoull(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CPropertyTreeElementProperty::Type::Number:
  {
    char* end = nullptr;
    *reinterpret_cast<double*>(propertyValue) = static_cast<double>(strtod(value.c_str(), &end));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  default:
    return false;
  }

  return true;
}

template<typename TType>
static bool DeserializeElementValues(CPropertyTreeElementProperty::Type type, const TiXmlNode* parent, const std::string& elementName, size_t elementCount, std::vector<TType>& values)
{
  const TiXmlElement* propertyElement = parent->FirstChildElement(elementName);
  for (size_t i = 0; i < elementCount; ++i)
  {
    if (propertyElement == nullptr)
      break;

    TType value;
    if (propertyElement->FirstChild() == nullptr ||
        !DeserializePropertyValue(type, propertyElement->FirstChild()->ValueStr(), &value))
      return false;

    values.push_back(value);

    propertyElement = propertyElement->NextSiblingElement(elementName);
  }

  return true;
}

bool CXmlPropertyTreeDeserializer::RegisterElement(const IPropertyTreeElement* element)
{
  if (element == nullptr)
    return false;

  std::shared_ptr<const IPropertyTreeElement> elementPtr = std::shared_ptr<const IPropertyTreeElement>(element);

  // check if the element already exists
  const auto& it = FindElementByName(m_elements, element->GetName());
  if (it != m_elements.cend())
    return false;

  m_elements.push_back(std::make_pair(element->GetName(), elementPtr));

  return true;
}

bool CXmlPropertyTreeDeserializer::Deserialize(const std::string& data, CXmlPropertyTree& tree) const
{
  TiXmlDocument doc;
  doc.Parse(data.c_str());

  TiXmlElement* root = doc.RootElement();
  if (root == nullptr)
    return false;

  const std::string& elementName = root->ValueStr();
  if (elementName != tree.GetName())
    return false;

  // look for XML namespaces
  const TiXmlAttribute* attr = root->FirstAttribute();
  while (attr != nullptr)
  {
    auto attrName = attr->NameTStr();
    const auto& attrValue = attr->ValueStr();
    if (!attrValue.empty() && StringUtils::StartsWith(attrName, NamespacePrefix))
    {
      const auto& attrValue = attr->ValueStr();
      attrName = StringUtils::Mid(attrName, NamespacePrefix.size());
      if (attrName.empty())
        tree.AddNamespace(attrValue);
      else if (StringUtils::StartsWith(attrName, ":") && attrName.size() > 1)
        tree.AddNamespace(StringUtils::Mid(attrName, 1), attrValue);
    }

    attr = attr->Next();
  }

  // loop over all children and deserialize them
  const TiXmlElement* child = root->FirstChildElement();
  while (child != nullptr)
  {
    auto element = FindElementByName(m_elements, child->ValueStr());
    if (element == m_elements.end())
      return false;

    auto ptElement = element->second->Clone();
    if (!DeserializeElement(child, ptElement))
    {
      delete ptElement;
      return false;
    }

    tree.AddElement(ptElement);

    child = child->NextSiblingElement();
  }

  return false;
}

bool CXmlPropertyTreeDeserializer::DeserializeElement(const TiXmlNode* node, IPropertyTreeElement* element)
{
  if (node == nullptr || element == nullptr)
    return false;

  // deserialize all properties
  for (auto prop : element->GetProperties())
  {
    if (DeserializeProperty(node, prop.second))
      element->SetProperty(prop.first, prop.second);
    else if (prop.second.IsRequired())
      return false;
  }

  return true;
}

bool CXmlPropertyTreeDeserializer::DeserializeProperty(const TiXmlNode* node, CPropertyTreeElementProperty& prop)
{
  if (node == nullptr)
    return false;

  // reset the validity flag
  prop.SetInvalid();

  // the property is stored as an attribute
  if (prop.IsAttribute())
    return DeserializeAttribute(node, prop);

  const auto& name = prop.GetName();
  const auto& supportsMultipleValues = prop.SupportsMultipleValues();
  const auto& type = prop.GetType();
  if (name.empty())
  {
    if (supportsMultipleValues && type == CPropertyTreeElementProperty::Type::Element)
      return false;

    const TiXmlNode* valueNode = node->FirstChild();
    std::string value;
    if (valueNode != nullptr)
    {
      if (valueNode->Type() != TiXmlNode::TINYXML_TEXT)
        return false;

      value = valueNode->ValueStr();
    }

    prop.SetValid(DeserializePropertyValue(type, value, prop.GetRawValue()));
    return prop.IsValid();
  }

  // count all elements matching the property's name
  size_t elementCount = 0;
  const TiXmlElement* propertyElement = node->FirstChildElement(name);
  while (propertyElement != nullptr)
  {
    ++elementCount;

    propertyElement = propertyElement->NextSiblingElement(name);
  }

  // we couldn't find any matching elements
  if (elementCount == 0)
  {
    // a required property is missing
    if (prop.IsRequired())
      return false;

    // an optional property is not defined
    return true;
  }

  // the property doesn't support multiple values but we found more than one matching element
  if (elementCount > 1 && !supportsMultipleValues)
    return false;

  // deserialize the attribute value
  switch (type)
  {
  case CPropertyTreeElementProperty::Type::Element:
  {
    auto value = prop.GetRawValue();
    // to support multiple or optional values we need a generator
    if (!prop.HasGenerator() && (supportsMultipleValues || value == nullptr))
      return false;

    std::vector<IPropertyTreeElement*> values;
    bool valuesGenerated = false;
    if (!supportsMultipleValues)
    {
      // get a new element using the generator if necessary
      if (value == nullptr)
      {
        prop.SetValue(prop.GenerateValue());
        valuesGenerated = true;
      }

      values.push_back(reinterpret_cast<IPropertyTreeElement*>(value));
    }
    else
    {
      for (size_t i = 0; i < elementCount; ++i)
        values.push_back(prop.GenerateValue());
      valuesGenerated = true;
    }

    bool result = true;
    if (name.empty())
      result = false;
    else
    {
      propertyElement = node->FirstChildElement(name);
      for (auto& value : values)
      {
        if (DeserializeElement(propertyElement, value))
        {
          result = false;
          break;
        }

        propertyElement = propertyElement->NextSiblingElement(name);
      }
    }

    if (!result)
    {
      if (valuesGenerated)
      {
        // cleanup generated values
        for (auto& value : values)
          delete value;

        if (!supportsMultipleValues)
          value = nullptr;
      }

      return false;
    }

    prop.SetValues<IPropertyTreeElement*>(values);
    break;
  }

  case CPropertyTreeElementProperty::Type::Boolean:
  {
    std::vector<bool> values;
    if (!DeserializeElementValues<bool>(type, node, name, elementCount, values))
      return false;

    prop.SetValues<bool>(values);
    break;
  }

  case CPropertyTreeElementProperty::Type::String:
  {
    std::vector<std::string> values;
    if (!DeserializeElementValues<std::string>(type, node, name, elementCount, values))
      return false;

    prop.SetValues<std::string>(values);
    break;
  }

  case CPropertyTreeElementProperty::Type::Integer:
  {
    std::vector<int32_t> values;
    if (!DeserializeElementValues<int32_t>(type, node, name, elementCount, values))
      return false;

    prop.SetValues<int32_t>(values);
    break;
  }

  case CPropertyTreeElementProperty::Type::UnsignedInteger:
  {
    std::vector<uint32_t> values;
    if (!DeserializeElementValues<uint32_t>(type, node, name, elementCount, values))
      return false;

    prop.SetValues<uint32_t>(values);
    break;
  }

  case CPropertyTreeElementProperty::Type::Long:
  {
    std::vector<int64_t> values;
    if (!DeserializeElementValues<int64_t>(type, node, name, elementCount, values))
      return false;

    prop.SetValues<int64_t>(values);
    break;
  }

  case CPropertyTreeElementProperty::Type::UnsignedLong:
  {
    std::vector<uint64_t> values;
    if (!DeserializeElementValues<uint64_t>(type, node, name, elementCount, values))
      return false;

    prop.SetValues<uint64_t>(values);
    break;
  }

  case CPropertyTreeElementProperty::Type::Number:
  {
    std::vector<double> values;
    if (!DeserializeElementValues<double>(type, node, name, elementCount, values))
      return false;

    prop.SetValues<double>(values);
    break;
  }

  default:
    return false;
  }

  prop.SetValid();
  return true;
}

bool CXmlPropertyTreeDeserializer::DeserializeAttribute(const TiXmlNode* node, CPropertyTreeElementProperty& prop)
{
  if (node == nullptr)
    return false;

  const TiXmlElement* element = node->ToElement();
  if (element == nullptr)
    return false;

  const std::string* attributeValue = element->Attribute(prop.GetName());
  if (attributeValue == nullptr)
  {
    // a required attribute is missing
    if (prop.IsRequired())
      return false;

    // an optional attribute is not defined
    return true;
  }

  // deserialize the property value
  prop.SetValid(DeserializePropertyValue(prop.GetType(), *attributeValue, prop.GetRawValue()));
  return prop.IsValid();
}


CXmlPropertyTreeDeserializer::IPropertyTreeElementPairs::const_iterator CXmlPropertyTreeDeserializer::FindElementByName(const IPropertyTreeElementPairs& elements, const std::string& elementName)
{
  return std::find_if(elements.cbegin(), elements.cend(),
    [&elementName](const IPropertyTreeElementPair& pair)
  {
    return pair.first == elementName;
  });
}
