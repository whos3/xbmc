#pragma once
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

#include <map>
#include <set>
#include <string>
#include <vector>

#include "network/upnp/openHome/didllite/DidlLiteElementProperty.h"

struct OhUPnPControlPointContext;
struct OhUPnPRootDeviceContext;
class TiXmlNode;

class IDidlLiteElement
{
public:
  virtual ~IDidlLiteElement();

  /*!
   * \brief Creates a new DIDL-Lite element instance.
   */
  virtual IDidlLiteElement* Create() const = 0;

  /*!
  * \brief Clones the DIDL-Lite element instance into a new DIDL-Lite element instance.
  */
  virtual IDidlLiteElement* Clone() const = 0;

  /*!
   * \brief Get the DIDL-Lite element's identifier.
   */
  virtual std::string GetIdentifier() const = 0;

  /*!
   * \brief Get a list of DIDL-Lite elements that this implementation extends.
   */
  virtual std::set<std::string> Extends() const = 0;

  /*!
  * \brief Serializes the DIDL-Lite element into the given parent XML element.
  *
  * \param[out] node Parent XML node to serialize the DIDL-Lite element into
  * \param[in] context Context of the call
  * \return True if the serialization was successful otherwise false
  */
  virtual bool Serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const;

  /*!
  * \brief Deserializes the DIDL-Lite element from the given parent XML element.
  *
  * \param[in] node Parent XML node to deserialize the DIDL-Lite element from
  * \param[in] context Context of the call
  * \return True if the deserialization was successful otherwise false
  */
  virtual bool Deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context);

  const std::string& GetName() const { return m_name; }

protected:
  IDidlLiteElement();
  IDidlLiteElement(const std::string& name);
  IDidlLiteElement(const std::string& ns, const std::string& name);
  IDidlLiteElement(const IDidlLiteElement& element);

  /*!
  * \brief Serializes the DIDL-Lite element into the given XML element.
  *
  * \param[out] node XML node to serialize the DIDL-Lite element into
  * \param[in] context Context of the call
  * \return True if the serialization was successful otherwise false
  */
  virtual bool serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const { return true; }

  /*!
  * \brief Deserializes the DIDL-Lite element from the given XML element.
  *
  * \param[in] node XML node to deserialize the DIDL-Lite element from
  * \param[in] context Context of the call
  * \return True if the deserialization was successful otherwise false
  */
  virtual bool deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context) { return true; }

  CDidlLiteElementProperty& addElementProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addElementProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addStringProperty(void* propertyValue);
  CDidlLiteElementProperty& addStringProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addStringProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addBooleanProperty(void* propertyValue);
  CDidlLiteElementProperty& addBooleanProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addBooleanProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addIntegerProperty(void* propertyValue);
  CDidlLiteElementProperty& addIntegerProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addUnsignedIntegerProperty(void* propertyValue);
  CDidlLiteElementProperty& addUnsignedIntegerProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addUnsignedIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addLongProperty(void* propertyValue);
  CDidlLiteElementProperty& addLongProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addLongProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addUnsignedLongProperty(void* propertyValue);
  CDidlLiteElementProperty& addUnsignedLongProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addUnsignedLongProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addNumberProperty(void* propertyValue);
  CDidlLiteElementProperty& addNumberProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addNumberProperty(const std::string& ns, const std::string& name, void* propertyValue);

  CDidlLiteElementProperty& getProperty();
  CDidlLiteElementProperty& getProperty(const std::string& name);
  CDidlLiteElementProperty& getProperty(const std::string& ns, const std::string& name);

  bool isPropertyValid() const;
  bool isPropertyValid(const std::string& name) const;
  bool isPropertyValid(const std::string& ns, const std::string& name) const;

  void setPropertyValid();
  void setPropertyValid(const std::string& name);
  void setPropertyValid(const std::string& ns, const std::string& name);
  void setPropertyValidity(bool valid);
  void setPropertyValidity(const std::string& name, bool valid);
  void setPropertyValidity(const std::string& ns, const std::string& name, bool valid);

  void copyPropertyValidity(const IDidlLiteElement* otherElement);

  template<class TDidlLiteElement>
  static void copyElementProperty(TDidlLiteElement*& dstElement, const TDidlLiteElement* srcElement)
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

  template<class TDidlLiteElement>
  static void copyElementProperty(std::vector<TDidlLiteElement*>& dstElements, const std::vector<TDidlLiteElement*>& srcElements)
  {
    // first clean up the destination
    if (!dstElements.empty())
    {
      for (const auto& elem : dstElements)
        delete elem;

      dstElements.clear();
    }

    for (const auto& elem : srcElements)
      dstElements.push_back(reinterpret_cast<TDidlLiteElement*>(elem->Clone()));
  }

  template<class TDidlLiteElement>
  static void clearElementProperty(std::vector<TDidlLiteElement*>& elements)
  {
    for (const auto& element : elements)
      delete element;
    elements.clear();
  }

  template<class TDidlLiteElement>
  static void setMultiValueProperty(std::vector<TDidlLiteElement*>& prop, const std::vector<TDidlLiteElement*>& values)
  {
    clearElementProperty(prop);
    for (const auto& value : values)
    {
      if (value == nullptr)
        continue;

      prop.push_back(value);
    }
  }

  template<class TDidlLiteElement>
  void setAndValidateMultiValueProperty(std::vector<TDidlLiteElement*>& prop, const std::vector<TDidlLiteElement*>& values, const std::string& element)
  {
    setAndValidateMultiValueProperty(prop, values, "", element);
  }

  template<class TDidlLiteElement>
  void setAndValidateMultiValueProperty(std::vector<TDidlLiteElement*>& prop, const std::vector<TDidlLiteElement*>& values, const std::string& ns, const std::string& element)
  {
    setMultiValueProperty(prop, values);
    setPropertyValidity(ns, element, !prop.empty());
  }

  template<class TDidlLiteElement>
  static void setMultiValueProperty(std::vector<TDidlLiteElement*>& prop, const std::vector<std::string>& values)
  {
    clearElementProperty(prop);
    for (const auto& value : values)
      prop.push_back(new TDidlLiteElement(value));
  }

  template<class TDidlLiteElement>
  void setAndValidateMultiValueProperty(std::vector<TDidlLiteElement*>& prop, const std::vector<std::string>& values, const std::string& element)
  {
    setAndValidateMultiValueProperty(prop, values, "", element);
  }

  template<class TDidlLiteElement>
  void setAndValidateMultiValueProperty(std::vector<TDidlLiteElement*>& prop, const std::vector<std::string>& values, const std::string& ns, const std::string& element)
  {
    setMultiValueProperty(prop, values);
    setPropertyValidity(ns, element, !prop.empty());
  }

private:
  CDidlLiteElementProperty& addProperty(const std::string& ns, const std::string& name, CDidlLiteElementProperty::Type type, void* propertyValue);

  std::string m_name;

  std::map<std::string, CDidlLiteElementProperty> m_properties;
};
