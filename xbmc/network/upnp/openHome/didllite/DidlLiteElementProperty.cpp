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

#include "DidlLiteElementProperty.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/didllite/IDidlLiteElement.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"
#include "utils/XMLUtils.h"

static size_t countElements(const TiXmlElement* propertyElement, const std::string& name)
{
  size_t count = 0;
  while (propertyElement != nullptr)
  {
    ++count;

    propertyElement = propertyElement->NextSiblingElement(name);
  }

  return count;
}

static bool serializePropertyValue(CDidlLiteElementProperty::Type type, const void* propertyValue, std::string& value)
{
  switch (type)
  {
  case CDidlLiteElementProperty::Type::String:
    value = *reinterpret_cast<const std::string*>(propertyValue);
    break;

  case CDidlLiteElementProperty::Type::Boolean:
    value = *reinterpret_cast<const bool*>(propertyValue) ? "1" : "0";
    break;

  case CDidlLiteElementProperty::Type::Integer:
    value = StringUtils::Format("%d", *reinterpret_cast<const int32_t*>(propertyValue));
    break;

  case CDidlLiteElementProperty::Type::UnsignedInteger:
    value = StringUtils::Format("%u", *reinterpret_cast<const uint32_t*>(propertyValue));
    break;

  case CDidlLiteElementProperty::Type::Long:
    value = StringUtils::Format("%lld", *reinterpret_cast<const int64_t*>(propertyValue));
    break;

  case CDidlLiteElementProperty::Type::UnsignedLong:
    value = StringUtils::Format("%llu", *reinterpret_cast<const uint64_t*>(propertyValue));
    break;

  case CDidlLiteElementProperty::Type::Number:
    value = StringUtils::Format("%.2f", *reinterpret_cast<const double*>(propertyValue));
    break;

  default:
    return false;
  }

  return true;
}

static bool deserializePropertyValue(CDidlLiteElementProperty::Type type, const std::string& value, void* propertyValue)
{
  switch (type)
  {
  case CDidlLiteElementProperty::Type::String:
    *reinterpret_cast<std::string*>(propertyValue) = value;
    break;

  case CDidlLiteElementProperty::Type::Boolean:
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

  case CDidlLiteElementProperty::Type::Integer:
  {
    char* end = nullptr;
    *reinterpret_cast<int32_t*>(propertyValue) = static_cast<int32_t>(strtol(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CDidlLiteElementProperty::Type::UnsignedInteger:
  {
    char* end = nullptr;
    *reinterpret_cast<uint32_t*>(propertyValue) = static_cast<uint32_t>(strtoul(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CDidlLiteElementProperty::Type::Long:
  {
    char* end = nullptr;
    *reinterpret_cast<int64_t*>(propertyValue) = static_cast<int64_t>(strtoll(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CDidlLiteElementProperty::Type::UnsignedLong:
  {
    char* end = nullptr;
    *reinterpret_cast<uint64_t*>(propertyValue) = static_cast<uint64_t>(strtoull(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;

    break;
  }

  case CDidlLiteElementProperty::Type::Number:
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
static std::vector<TType> getValues(const void* value, bool multipleValues, typename std::enable_if_t<!std::is_pointer<TType>::value, void*> t = nullptr)
{
  std::vector<TType> values;
  if (!multipleValues)
    values.push_back(*static_cast<const TType*>(value));
  else
  {
    const std::vector<TType>* vecValues = static_cast<const std::vector<TType>*>(value);
    for (const auto& it : *vecValues)
      values.push_back(it);
  }

  return values;
}

template<typename TType>
static std::vector<TType> getValues(const void* value, bool multipleValues, typename std::enable_if_t<std::is_pointer<TType>::value, void*> t = nullptr)
{
  std::vector<TType> values;
  if (!multipleValues)
    values.push_back(static_cast<const TType*>(value));
  else
  {
    const std::vector<TType>* vecValues = static_cast<const std::vector<TType>*>(value);
    for (const auto& it : *vecValues)
      values.push_back(it);
  }

  return values;
}

template<typename TType>
static void setValues(std::vector<TType> values, void* value, bool multipleValues)
{
  if (!multipleValues)
    *static_cast<TType*>(value) = values.at(0);
  else
  {
    std::vector<TType>* propertyValues = static_cast<std::vector<TType>*>(value);
    propertyValues->assign(values.begin(), values.end());
  }
}

template<typename TType>
static bool deserializeElementValues(CDidlLiteElementProperty::Type type, const TiXmlNode* parent, const std::string& elementName, size_t elementCount, std::vector<TType>& values)
{
  const TiXmlElement* propertyElement = parent->FirstChildElement(elementName);
  for (size_t i = 0; i < elementCount; ++i)
  {
    if (propertyElement == nullptr)
      break;

    TType value;
    if (propertyElement->FirstChild() == nullptr ||
      !deserializePropertyValue(type, propertyElement->FirstChild()->ValueStr(), &value))
      return false;

    values.push_back(value);

    propertyElement = propertyElement->NextSiblingElement(elementName);
  }

  return true;
}

CDidlLiteElementProperty::CDidlLiteElementProperty(Type type, void* value)
  : CDidlLiteElementProperty("", type, value)
{ }

CDidlLiteElementProperty::CDidlLiteElementProperty(const std::string& name, Type type, void* value)
  : CDidlLiteElementProperty("", name, type, value)
{ }

CDidlLiteElementProperty::CDidlLiteElementProperty(const std::string& ns, const std::string& name, Type type, void* value)
  : m_value(value),
    m_type(type),
    m_isElement(true),
    m_required(false),
    m_multipleValues(false),
    m_valid(false),
    m_valueGenerator(nullptr),
    m_valueGenerated(false),
    m_minimumVersion(1)
{
  m_isElement = !DidlLiteUtils::IsAttributeName(name);
  if (m_isElement)
    m_name = DidlLiteUtils::GetElementName(ns, name);
  else
    m_name = DidlLiteUtils::GetAttributeName(name);
}

CDidlLiteElementProperty::~CDidlLiteElementProperty()
{
  m_value = nullptr;
}

bool CDidlLiteElementProperty::Serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const
{
  if (node == nullptr)
    return false;

  // any required property needs to be set
  if (!IsValid())
    return !IsRequired(context.device.GetDeviceTypeVersion());

  // don't serialize properties from newer versions
  if (context.profile.GetContentDirectoryVersion() < GetMinimumVersion())
    return true;

  // the property is stored as an attribute
  if (IsAttribute())
    return serializeAttribute(node, context);

  // don't serialize properties that are not part of the requested properties
  if (IsOptional() && !context.filters.Contains(m_name))
    return true;

  return serializeElement(node, context);
}

bool CDidlLiteElementProperty::Deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (node == nullptr)
    return false;

  // reset the valid flag
  m_valid = false;

  // the property is stored as an element
  if (IsElement())
    return deserializeElement(node, context);

  // the property is stored as an attribute
  return deserializeAttribute(node, context);
}

bool CDidlLiteElementProperty::serializeElement(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const
{
  if (m_name.empty())
  {
    if (m_multipleValues && m_type == Type::Element)
      return false;
    
    std::string value;
    if (!serializePropertyValue(m_type, m_value, value))
      return false;

    TiXmlText valueNode(value);
    node->InsertEndChild(valueNode);

    return true;
  }

  switch (m_type)
  {
  case Type::Element:
  {
    std::vector<const IDidlLiteElement*> values;
    if (!m_multipleValues)
      values.push_back(static_cast<const IDidlLiteElement*>(m_value));
    else
    {
      const std::vector<IDidlLiteElement*>* vecValues = static_cast<const std::vector<IDidlLiteElement*>*>(m_value);
      for (const auto& it : *vecValues)
        values.push_back(it);
    }

    for (const auto& value : values)
    {
      if (!value->Serialize(node, context))
        return false;
    }
    break;
  }

  case Type::Boolean:
  {
    std::vector<bool> values = getValues<bool>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetBoolean(node, m_name.c_str(), value);
    break;
  }

  case Type::String:
  {
    std::vector<std::string> values = getValues<std::string>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetString(node, m_name.c_str(), value);
    break;
  }

  case Type::Integer:
  {
    std::vector<int32_t> values = getValues<int32_t>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetInt(node, m_name.c_str(), value);
    break;
  }

  case Type::UnsignedInteger:
  {
    std::vector<uint32_t> values = getValues<uint32_t>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetInt(node, m_name.c_str(), value);
    break;
  }

  case Type::Long:
  {
    std::vector<int64_t> values = getValues<int64_t>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetInt64(node, m_name.c_str(), value);
    break;
  }

  case Type::UnsignedLong:
  {
    std::vector<uint64_t> values = getValues<uint64_t>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetUInt64(node, m_name.c_str(), value);
    break;
  }

  case Type::Number:
  {
    std::vector<double> values = getValues<double>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetFloat(node, m_name.c_str(), static_cast<float>(value));
    break;
  }

  default:
    return false;
  }

  return true;
}

bool CDidlLiteElementProperty::serializeAttribute(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const
{
  TiXmlElement* element = node->ToElement();
  if (element == nullptr)
    return false;

  std::string propertyValue;
  // try to serialize the property's value into a string
  if (!serializePropertyValue(m_type, m_value, propertyValue))
    return false;

  // set the attribute
  element->SetAttribute(m_name, propertyValue);

  return true;
}

bool CDidlLiteElementProperty::deserializeElement(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (m_name.empty())
  {
    if (m_multipleValues && m_type == Type::Element)
      return false;

    const TiXmlNode* valueNode = node->FirstChild();
    std::string value;
    if (valueNode != nullptr)
    {
      if (valueNode->Type() != TiXmlNode::TINYXML_TEXT)
        return false;

      value = valueNode->ValueStr();
    }

    m_valid = deserializePropertyValue(m_type, value, m_value);
    return m_valid;
  }

  // count all elements matching the property's name
  size_t elementCount = 0;
  const TiXmlElement* propertyElement = node->FirstChildElement(m_name);
  while (propertyElement != nullptr)
  {
    ++elementCount;

    propertyElement = propertyElement->NextSiblingElement(m_name);
  }

  // we couldn't find any matching elements
  if (elementCount == 0)
  {
    // a required property is missing
    if (IsRequired(context.device.GetDeviceTypeVersion()))
      return false;

    // an optional property is not defined
    return true;
  }

  // the property doesn't support multiple values but we found more than one matching element
  if (elementCount > 1 && !m_multipleValues)
    return false;

  // deserialize the attribute value
  switch (m_type)
  {
  case Type::Element:
  {
    // to support multiple or optional values we need a generator
    if (m_valueGenerator == nullptr && (m_multipleValues || m_value == nullptr))
      return false;

    std::vector<IDidlLiteElement*> values;
    if (!m_multipleValues)
    {
      // get a new element using the generator if necessary
      if (m_value == nullptr)
      {
        m_value = m_valueGenerator->Clone();
        m_valueGenerated = true;
      }

      values.push_back(reinterpret_cast<IDidlLiteElement*>(m_value));
    }
    else
    {
      for (size_t i = 0; i < elementCount; ++i)
        values.push_back(m_valueGenerator->Clone());
      m_valueGenerated = true;
    }

    bool result = true;
    if (m_name.empty())
      result = false;
    else
    {
      propertyElement = node->FirstChildElement(m_name);
      for (auto& value : values)
      {
        if (!value->Deserialize(propertyElement, context))
        {
          result = false;
          break;
        }

        propertyElement = propertyElement->NextSiblingElement(m_name);
      }
    }

    if (!result)
    {
      if (m_valueGenerated)
      {
        // cleanup generated values
        for (auto& value : values)
          delete value;

        if (!m_multipleValues)
          m_value = nullptr;
      }

      return false;
    }

    setValues<IDidlLiteElement*>(values, m_value, m_multipleValues);
    break;
  }

  case Type::Boolean:
  {
    std::vector<bool> values;
    if (!deserializeElementValues<bool>(m_type, node, m_name, elementCount, values))
      return false;

    setValues<bool>(values, m_value, m_multipleValues);
    break;
  }

  case Type::String:
  {
    std::vector<std::string> values;
    if (!deserializeElementValues<std::string>(m_type, node, m_name, elementCount, values))
      return false;

    setValues<std::string>(values, m_value, m_multipleValues);
    break;
  }

  case Type::Integer:
  {
    std::vector<int32_t> values;
    if (!deserializeElementValues<int32_t>(m_type, node, m_name, elementCount, values))
      return false;

    setValues<int32_t>(values, m_value, m_multipleValues);
    break;
  }

  case Type::UnsignedInteger:
  {
    std::vector<uint32_t> values;
    if (!deserializeElementValues<uint32_t>(m_type, node, m_name, elementCount, values))
      return false;

    setValues<uint32_t>(values, m_value, m_multipleValues);
    break;
  }

  case Type::Long:
  {
    std::vector<int64_t> values;
    if (!deserializeElementValues<int64_t>(m_type, node, m_name, elementCount, values))
      return false;

    setValues<int64_t>(values, m_value, m_multipleValues);
    break;
  }

  case Type::UnsignedLong:
  {
    std::vector<uint64_t> values;
    if (!deserializeElementValues<uint64_t>(m_type, node, m_name, elementCount, values))
      return false;

    setValues<uint64_t>(values, m_value, m_multipleValues);
    break;
  }

  case Type::Number:
  {
    std::vector<double> values;
    if (!deserializeElementValues<double>(m_type, node, m_name, elementCount, values))
      return false;

    setValues<double>(values, m_value, m_multipleValues);
    break;
  }

  default:
    return false;
  }

  m_valid = true;
  return true;
}

bool CDidlLiteElementProperty::deserializeAttribute(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  const TiXmlElement* element = node->ToElement();
  if (element == nullptr)
    return false;

  // try to retrieve the value of the attribute
  const std::string* attributeValue = element->Attribute(m_name);
  if (attributeValue == nullptr)
  {
    // a required attribute is missing
    if (IsRequired(context.device.GetDeviceTypeVersion()))
      return false;

    // an optional attribute is not defined
    return true;
  }

  // deserialize the property value
  m_valid = deserializePropertyValue(m_type, *attributeValue, m_value);
  return m_valid;
}
