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

#include "IPropertyTreeElement.h"
#include "utils/propertytree/PropertyTreePath.h"

IPropertyTreeElement::IPropertyTreeElement()
  : IPropertyTreeElement("")
{ }

IPropertyTreeElement::IPropertyTreeElement(const std::string& name)
  : m_name(name)
{ }

IPropertyTreeElement::IPropertyTreeElement(const IPropertyTreeElement& element)
  : m_name(element.m_name)
{
  // we don't copy m_properties on purpose because every property contains a pointer to the class'
  // member variable where the actual value is stored and we can't copy/map that pointer
}

bool IPropertyTreeElement::HasProperty(const std::string& name) const
{
  CPropertyTreePath path(name);
  const auto& it = m_properties.find(path.GetCurrentElement());
  if (it == m_properties.cend())
    return false;

  path = path.Next();
  if (path.IsEmpty())
    return true;

  if (!it->second.IsElement())
    return false;

  return it->second.GetValue<IPropertyTreeElement*>()->HasProperty(path.ToString());
}

bool IPropertyTreeElement::GetProperty(const std::string& name, CPropertyTreeElementProperty& prop) const
{
  CPropertyTreePath path(name);
  const auto& it = m_properties.find(path.GetCurrentElement());
  if (it == m_properties.cend())
    return false;

  path = path.Next();
  if (path.IsEmpty())
  {
    prop = it->second;
    return true;
  }

  if (!it->second.IsElement())
    return false;

  return it->second.GetValue<IPropertyTreeElement*>()->GetProperty(path.ToString(), prop);
}

bool IPropertyTreeElement::SetProperty(const std::string& name, const CPropertyTreeElementProperty& prop)
{
  CPropertyTreePath path(name);
  auto& it = m_properties.find(path.GetCurrentElement());
  if (it == m_properties.cend())
    return false;

  path = path.Next();
  if (path.IsEmpty())
  {
    it->second = prop;
    return true;
  }

  if (!it->second.IsElement())
    return false;

  return it->second.GetValue<IPropertyTreeElement*>()->SetProperty(path.ToString(), prop);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addElementProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::Element, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addStringProperty(void* propertyValue)
{
  return addStringProperty("", propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addStringProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::String, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addBooleanProperty(void* propertyValue)
{
  return addBooleanProperty("", propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addBooleanProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::Boolean, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addIntegerProperty(void* propertyValue)
{
  return addIntegerProperty("", propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addIntegerProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::Integer, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addUnsignedIntegerProperty(void* propertyValue)
{
  return addUnsignedIntegerProperty("", propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addUnsignedIntegerProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::UnsignedInteger, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addLongProperty(void* propertyValue)
{
  return addLongProperty("", propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addLongProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::Long, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addUnsignedLongProperty(void* propertyValue)
{
  return addUnsignedLongProperty("", propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addUnsignedLongProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::UnsignedLong, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addNumberProperty(void* propertyValue)
{
  return addNumberProperty("", propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::addNumberProperty(const std::string& name, void* propertyValue)
{
  return addProperty(name, CPropertyTreeElementProperty::Type::Number, propertyValue);
}

CPropertyTreeElementProperty& IPropertyTreeElement::getProperty()
{
  return getProperty("");
}

CPropertyTreeElementProperty& IPropertyTreeElement::getProperty(const std::string& name)
{
  auto& propertyIt = m_properties.find(name);
  assert(propertyIt != m_properties.end());

  return propertyIt->second;
}

bool IPropertyTreeElement::isPropertyValid() const
{
  return isPropertyValid("");
}

bool IPropertyTreeElement::isPropertyValid(const std::string& name) const
{
  auto& attribute = m_properties.find(name);
  if (attribute == m_properties.end())
    return false;

  return attribute->second.IsValid();
}

void IPropertyTreeElement::setPropertyValid()
{
  setPropertyValidity("");
}

void IPropertyTreeElement::setPropertyValid(const std::string& name)
{
  setPropertyValidity(name, true);
}

void IPropertyTreeElement::setPropertyValidity(bool valid)
{
  setPropertyValidity("", valid);
}

void IPropertyTreeElement::setPropertyValidity(const std::string& name, bool valid)
{
  auto& attribute = m_properties.find(name);
  if (attribute == m_properties.end())
    return;

  attribute->second.SetValid(valid);
}

void IPropertyTreeElement::copyPropertyValidity(const IPropertyTreeElement* otherElement)
{
  if (otherElement == nullptr)
    return;

  for (const auto& otherProp : otherElement->m_properties)
    setPropertyValidity(otherProp.first, otherProp.second.IsValid());
}

CPropertyTreeElementProperty& IPropertyTreeElement::addProperty(const std::string& name, CPropertyTreeElementProperty::Type type, void* propertyValue)
{
  // create the property
  CPropertyTreeElementProperty elementProperty(name, type, propertyValue);

  // add it to the properties map
  return m_properties.insert(std::make_pair(name, elementProperty)).first->second;
}
