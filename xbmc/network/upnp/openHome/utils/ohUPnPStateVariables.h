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

#include <string>

#include "utils/propertytree/IPropertyTreeElement.h"

template<typename TType>
class COhUPnPStateVariable : public IPropertyTreeElement
{
protected:
  static const std::string ValueProperty;

public:
  COhUPnPStateVariable(const std::string& name, const TType& defaultValue = TType())
    : IPropertyTreeElement(name)
  {
    initializeProperties<TType>();
  }

  COhUPnPStateVariable(const COhUPnPStateVariable& other)
    : IPropertyTreeElement(other)
    , m_current(other.m_current)
    , m_old(other.m_old)
  {
    initializeProperties<TType>();
    copyPropertyValidity(&other);
  }

  virtual ~COhUPnPStateVariable() = default;

  // implementations of IPropertyTreeElement
  IPropertyTreeElement* Create() const override { return new COhUPnPStateVariable(GetName()); }
  IPropertyTreeElement* Clone() const override { return new COhUPnPStateVariable(*this); }

  virtual const TType& GetValue() const { return m_current; }
  virtual bool SetValue(const TType& value)
  {
    m_current = value;

    bool changed = m_current != m_old;
    setPropertyValidity(ValueProperty, changed);

    return changed;
  }

  virtual bool HasChanged() const { return m_current != m_old; }

  virtual void Fix()
  {
    m_old = m_current;
    setPropertyValidity(ValueProperty, false);
  }

private:
  template<typename TValueType>
  void initializeProperties() { static_assert("Unsupported state variable type"); }

  template<>
  void initializeProperties<bool>()
  {
    addBooleanProperty("@val", &m_current).AsAttribute().SetRequired();
  }

  template<>
  void initializeProperties<int32_t>()
  {
    addIntegerProperty("@val", &m_current).AsAttribute().SetRequired();
  }

  template<>
  void initializeProperties<uint32_t>()
  {
    addUnsignedIntegerProperty("@val", &m_current).AsAttribute().SetRequired();
  }

  template<>
  void initializeProperties<int64_t>()
  {
    addLongProperty("@val", &m_current).AsAttribute().SetRequired();
  }

  template<>
  void initializeProperties<uint64_t>()
  {
    addUnsignedLongProperty("@val", &m_current).AsAttribute().SetRequired();
  }

  template<>
  void initializeProperties<float>()
  {
    addNumberProperty("@val", &m_current).AsAttribute().SetRequired();
  }

  template<>
  void initializeProperties<std::string>()
  {
    addStringProperty("@val", &m_current).AsAttribute().SetRequired();
  }

  TType m_current;
  TType m_old;
};

template<typename TType>
const std::string COhUPnPStateVariable<TType>::ValueProperty = "@val";
