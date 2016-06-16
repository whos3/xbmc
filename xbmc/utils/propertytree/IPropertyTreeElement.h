#pragma once
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

#include <map>
#include <set>
#include <string>
#include <vector>

#include "utils/propertytree/PropertyTreeElementProperty.h"

class IPropertyTreeElement
{
public:
  virtual ~IPropertyTreeElement() = default;

  /*!
   * \brief Creates a new property tree element instance.
   */
  virtual IPropertyTreeElement* Create() const = 0;

  /*!
  * \brief Clones the property tree element instance into a new property tree element instance.
  */
  virtual IPropertyTreeElement* Clone() const = 0;

  const std::string& GetName() const { return m_name; }

  const std::map<std::string, CPropertyTreeElementProperty>& GetProperties() const { return m_properties; }
  bool HasProperty(const std::string& name) const;
  bool GetProperty(const std::string& name, CPropertyTreeElementProperty& prop) const;
  bool SetProperty(const std::string& name, const CPropertyTreeElementProperty& prop);

  template<typename TType>
  const TType GetValue() const
  {
    return GetValue("");
  }

  template<typename TType>
  std::vector<TType> GetValues() const
  {
    return GetValues("");
  }

  template<typename TType>
  const TType GetValue(const std::string& path) const
  {
    CPropertyTreeElementProperty prop;
    GetProperty(path, prop);

    return prop.GetValue<TType>();
  }

  template<typename TType>
  std::vector<TType> GetValues(const std::string& path) const
  {
    CPropertyTreeElementProperty prop;
    GetProperty(path, prop);

    return prop.GetValues<TType>();
  }

protected:
  IPropertyTreeElement();
  explicit IPropertyTreeElement(const std::string& name);
  IPropertyTreeElement(const IPropertyTreeElement& element);

  CPropertyTreeElementProperty& addElementProperty(const std::string& name, void* propertyValue);
  CPropertyTreeElementProperty& addStringProperty(void* propertyValue);
  CPropertyTreeElementProperty& addStringProperty(const std::string& name, void* propertyValue);
  CPropertyTreeElementProperty& addBooleanProperty(void* propertyValue);
  CPropertyTreeElementProperty& addBooleanProperty(const std::string& name, void* propertyValue);
  CPropertyTreeElementProperty& addIntegerProperty(void* propertyValue);
  CPropertyTreeElementProperty& addIntegerProperty(const std::string& name, void* propertyValue);
  CPropertyTreeElementProperty& addUnsignedIntegerProperty(void* propertyValue);
  CPropertyTreeElementProperty& addUnsignedIntegerProperty(const std::string& name, void* propertyValue);
  CPropertyTreeElementProperty& addLongProperty(void* propertyValue);
  CPropertyTreeElementProperty& addLongProperty(const std::string& name, void* propertyValue);
  CPropertyTreeElementProperty& addUnsignedLongProperty(void* propertyValue);
  CPropertyTreeElementProperty& addUnsignedLongProperty(const std::string& name, void* propertyValue);
  CPropertyTreeElementProperty& addNumberProperty(void* propertyValue);
  CPropertyTreeElementProperty& addNumberProperty(const std::string& name, void* propertyValue);

  CPropertyTreeElementProperty& getProperty();
  CPropertyTreeElementProperty& getProperty(const std::string& name);

  bool isPropertyValid() const;
  bool isPropertyValid(const std::string& name) const;

  void setPropertyValid();
  void setPropertyValid(const std::string& name);
  void setPropertyValidity(bool valid);
  void setPropertyValidity(const std::string& name, bool valid);

  void copyPropertyValidity(const IPropertyTreeElement* otherElement);

  template<class TPropertyTreeElement>
  static void copyElementProperty(TPropertyTreeElement*& dstElement, const TPropertyTreeElement* srcElement)
  {
    if (dstElement != NULL)
    {
      delete dstElement;
      dstElement = NULL;
    }

    if (srcElement == NULL)
      return;

    dstElement = srcElement->Clone();
  }

  template<class TPropertyTreeElement>
  static void copyElementProperty(std::vector<TPropertyTreeElement*>& dstElements, const std::vector<TPropertyTreeElement*>& srcElements)
  {
    // first clean up the destination
    if (!dstElements.empty())
    {
      for (const auto& elem : dstElements)
        delete elem;

      dstElements.clear();
    }

    for (const auto& elem : srcElements)
      dstElements.push_back(reinterpret_cast<TPropertyTreeElement*>(elem->Clone()));
  }

  template<class TPropertyTreeElement>
  static void clearElementProperty(std::vector<TPropertyTreeElement*>& elements)
  {
    for (const auto& element : elements)
      delete element;
    elements.clear();
  }

  template<class TPropertyTreeElement>
  static void setMultiValueProperty(std::vector<TPropertyTreeElement*>& prop, const std::vector<TPropertyTreeElement*>& values)
  {
    clearElementProperty(prop);
    for (const auto& value : values)
    {
      if (value == nullptr)
        continue;

      prop.push_back(value);
    }
  }

  template<class TPropertyTreeElement>
  void setAndValidateMultiValueProperty(std::vector<TPropertyTreeElement*>& prop, const std::vector<TPropertyTreeElement*>& values, const std::string& element)
  {
    setMultiValueProperty(prop, values);
    setPropertyValidity(element, !prop.empty());
  }

  template<class TPropertyTreeElement>
  static void setMultiValueProperty(std::vector<TPropertyTreeElement*>& prop, const std::vector<std::string>& values)
  {
    clearElementProperty(prop);
    for (const auto& value : values)
      prop.push_back(new TPropertyTreeElement(value));
  }

  template<class TPropertyTreeElement>
  void setAndValidateMultiValueProperty(std::vector<TPropertyTreeElement*>& prop, const std::vector<std::string>& values, const std::string& element)
  {
    setMultiValueProperty(prop, values);
    setPropertyValidity(element, !prop.empty());
  }

private:
  CPropertyTreeElementProperty& addProperty(const std::string& name, CPropertyTreeElementProperty::Type type, void* propertyValue);

  std::string m_name;
  std::map<std::string, CPropertyTreeElementProperty> m_properties;
};
