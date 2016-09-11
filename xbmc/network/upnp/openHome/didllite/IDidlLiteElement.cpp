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

#include "IDidlLiteElement.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"
#include "utils/XMLUtils.h"

IDidlLiteElement::IDidlLiteElement()
  : IDidlLiteElement("")
{ }

IDidlLiteElement::IDidlLiteElement(const std::string& name)
  : m_name(name)
{ }

IDidlLiteElement::IDidlLiteElement(const std::string& ns, const std::string& name)
  : IDidlLiteElement(DidlLiteUtils::GetElementName(ns, name))
{ }

IDidlLiteElement::IDidlLiteElement(const IDidlLiteElement& element)
  : m_name(element.m_name)
{
  // we don't copy m_properties on purpose because every property contains a pointer to the class'
  // member variable where the actual value is stored and we can't copy/map that pointer
}

IDidlLiteElement::~IDidlLiteElement()
{ }

bool IDidlLiteElement::Serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const
{
  if (node == nullptr)
    return false;

  // create the element
  TiXmlElement newElement(m_name);

  // add all specified attributes
  for (const auto& attr : m_properties)
  {
    if (!attr.second.Serialize(&newElement, context))
    {
      if (attr.second.IsElement())
        CLog::Log(LOGDEBUG, "IDidlLiteElement: failed to serialize <%s><%s>", m_name.c_str(), attr.first.c_str());
      else
        CLog::Log(LOGDEBUG, "IDidlLiteElement: failed to serialize %s attribute of <%s>", attr.first.c_str(), m_name.c_str());

      // only fail for required properties
      if (attr.second.IsRequired(context.device.GetDeviceTypeVersion()))
        return false;
    }
  }

  // perform any custom serialization
  if (!serialize(&newElement, context))
    return false;

  // insert the element at the end
  return node->InsertEndChild(newElement) != nullptr;
}

bool IDidlLiteElement::Deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  if (node == nullptr)
    return false;

  std::string elementName = node->ValueStr();
  if (elementName.empty())
    return false;

  // check if the deserialized namespace and name match the predefined ones
  if (!m_name.empty() && m_name.compare(elementName) != 0)
      return false;

  // deserialize the specified attributes
  for (auto& attr : m_properties)
  {
    if (!attr.second.Deserialize(node, context))
    {
      if (attr.second.IsElement())
        CLog::Log(LOGDEBUG, "IDidlLiteElement: failed to deserialize <%s><%s>", elementName.c_str(), attr.first.c_str());
      else
        CLog::Log(LOGDEBUG, "IDidlLiteElement: failed to deserialize %s attribute of <%s>", attr.first.c_str(), elementName.c_str());

      // only fail for required properties
      if (attr.second.IsRequired(context.device.GetDeviceTypeVersion()))
        return false;
    }
  }

  // perform any custom deserialization
  return deserialize(node, context);
}

CDidlLiteElementProperty& IDidlLiteElement::addElementProperty(const std::string& name, void* propertyValue)
{
  return addElementProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addElementProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Element, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addStringProperty(void* propertyValue)
{
  return addStringProperty("", propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addStringProperty(const std::string& name, void* propertyValue)
{
  return addStringProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addStringProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::String, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addBooleanProperty(void* propertyValue)
{
  return addBooleanProperty("", propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addBooleanProperty(const std::string& name, void* propertyValue)
{
  return addBooleanProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addBooleanProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Boolean, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addIntegerProperty(void* propertyValue)
{
  return addIntegerProperty("", propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addIntegerProperty(const std::string& name, void* propertyValue)
{
  return addIntegerProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Integer, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedIntegerProperty(void* propertyValue)
{
  return addUnsignedIntegerProperty("", propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedIntegerProperty(const std::string& name, void* propertyValue)
{
  return addUnsignedIntegerProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::UnsignedInteger, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addLongProperty(void* propertyValue)
{
  return addLongProperty("", propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addLongProperty(const std::string& name, void* propertyValue)
{
  return addLongProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addLongProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Long, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedLongProperty(void* propertyValue)
{
  return addUnsignedLongProperty("", propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedLongProperty(const std::string& name, void* propertyValue)
{
  return addUnsignedLongProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedLongProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::UnsignedLong, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addNumberProperty(void* propertyValue)
{
  return addNumberProperty("", propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addNumberProperty(const std::string& name, void* propertyValue)
{
  return addNumberProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addNumberProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Number, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::getProperty()
{
  return getProperty("");
}

CDidlLiteElementProperty& IDidlLiteElement::getProperty(const std::string& name)
{
  return getProperty("", name);
}

CDidlLiteElementProperty& IDidlLiteElement::getProperty(const std::string& ns, const std::string& name)
{
  std::string propertyName = DidlLiteUtils::GetElementName(ns, name);

  auto& propertyIt = m_properties.find(propertyName);
  assert(propertyIt != m_properties.end());

  return propertyIt->second;
}

bool IDidlLiteElement::isPropertyValid() const
{
  return isPropertyValid("");
}

bool IDidlLiteElement::isPropertyValid(const std::string& name) const
{
  return isPropertyValid("", name);
}

bool IDidlLiteElement::isPropertyValid(const std::string& ns, const std::string& name) const
{
  std::string attributeName = DidlLiteUtils::GetElementName(ns, name);

  auto& attribute = m_properties.find(attributeName);
  if (attribute == m_properties.end())
    return false;

  return attribute->second.IsValid();
}

void IDidlLiteElement::setPropertyValid()
{
  setPropertyValidity("");
}

void IDidlLiteElement::setPropertyValid(const std::string& name)
{
  setPropertyValid("", name);
}

void IDidlLiteElement::setPropertyValid(const std::string& ns, const std::string& name)
{
  setPropertyValidity(ns, name, true);
}

void IDidlLiteElement::setPropertyValidity(bool valid)
{
  setPropertyValidity("", valid);
}

void IDidlLiteElement::setPropertyValidity(const std::string& name, bool valid)
{
  setPropertyValidity("", name, valid);
}

void IDidlLiteElement::setPropertyValidity(const std::string& ns, const std::string& name, bool valid)
{
  std::string attributeName = DidlLiteUtils::GetElementName(ns, name);

  auto& attribute = m_properties.find(attributeName);
  if (attribute == m_properties.end())
    return;

  attribute->second.SetValid(valid);
}

void IDidlLiteElement::copyPropertyValidity(const IDidlLiteElement* otherElement)
{
  if (otherElement == nullptr)
    return;

  for (const auto& otherProp : otherElement->m_properties)
    setPropertyValidity(otherProp.first, otherProp.second.IsValid());
}

CDidlLiteElementProperty& IDidlLiteElement::addProperty(const std::string& ns, const std::string& name, CDidlLiteElementProperty::Type type, void* propertyValue)
{
  std::string propertyName = DidlLiteUtils::GetElementName(ns, name);

  // create the property
  CDidlLiteElementProperty elementProperty(ns, name, type, propertyValue);

  // add it to the properties map
  return m_properties.insert(std::make_pair(propertyName, elementProperty)).first->second;
}
