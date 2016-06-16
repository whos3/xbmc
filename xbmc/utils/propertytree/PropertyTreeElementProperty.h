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

#include <memory>
#include <string>

class IPropertyTreeElement;

class CPropertyTreeElementProperty
{
public:
  enum class Type {
    Element,
    String,
    Boolean,
    Integer,
    UnsignedInteger,
    Long,
    UnsignedLong,
    Number
  };

  using Generator = std::shared_ptr<const IPropertyTreeElement>;

  static const std::string PropertyNameSeparator;

public:
  CPropertyTreeElementProperty(Type type, void* value);
  CPropertyTreeElementProperty(const std::string& name, Type type, void* value);
  virtual ~CPropertyTreeElementProperty();

  CPropertyTreeElementProperty& AsAttribute() { m_isElement = false; return *this; }
  CPropertyTreeElementProperty& SetOptional(bool optional = true) { m_required = !optional; return *this; }
  CPropertyTreeElementProperty& SetRequired(bool required = true) { m_required = required; return *this; }
  CPropertyTreeElementProperty& SupportMultipleValues() { m_multipleValues = true; return *this; }

  template<class TGenerator, typename... TArgs>
  CPropertyTreeElementProperty& SetGenerator(TArgs... args)
  {
    assert(m_type == Type::Element);

    m_valueGenerator = std::make_shared<TGenerator>(std::forward<TArgs>(args)...);
    return *this;
  }

  CPropertyTreeElementProperty& SetValid(bool valid = true) { m_valid = valid; return *this; }
  CPropertyTreeElementProperty& SetInvalid(bool invalid = true) { m_valid = !invalid; return *this; }

  const std::string& GetName() const { return m_name; }

  Type GetType() const { return m_type; }
  bool IsElement() const { return m_isElement; }
  bool IsAttribute() const { return !m_isElement; }
  bool IsRequired() const { return m_required; }
  bool IsOptional() const { return !m_required; }
  bool SupportsMultipleValues() const { return m_multipleValues; }
  bool IsValid() const { return m_valid; }
  bool HasGenerator() const { return m_valueGenerator != nullptr; }

  void* const GetRawValue() { return m_value; }
  const void* const GetRawValue() const { return m_value; }

  template<typename TType>
  inline const TType GetValue() const { return GetValueInternal<TType>(); }

  template<typename TType>
  std::vector<TType> GetValues() const
  {
    return GetValuesInternal<TType>();
  }

  template<typename TType>
  void SetValue(const TType& value)
  {
    // TODO
  }

  template<typename TType>
  void SetValues(const std::vector<TType>& values)
  {
    if (!m_multipleValues)
      *static_cast<TType*>(m_value) = values.at(0);
    else
    {
      std::vector<TType>* propertyValues = static_cast<std::vector<TType>*>(m_value);
      propertyValues->assign(values.begin(), values.end());
    }
  }

  IPropertyTreeElement* const GenerateValue();
  bool IsValueGenerated() const { return m_valueGenerated; }
  void SetValueGenerated(bool generated) { m_valueGenerated = generated; }

private:
  template<typename TType>
  inline const TType GetValueInternal(typename std::enable_if_t<!std::is_pointer<TType>::value, void*> t = nullptr) const
  {
    return *reinterpret_cast<const TType*>(m_value);
  }

  template<typename TType>
  inline const TType GetValueInternal(typename std::enable_if_t<std::is_pointer<TType>::value, void*> t = nullptr) const
  {
    return reinterpret_cast<const TType>(m_value);
  }

  template<typename TType>
  std::vector<TType> GetValuesInternal(typename std::enable_if_t<!std::is_pointer<TType>::value, void*> t = nullptr) const
  {
    std::vector<TType> values;
    if (!m_multipleValues)
      values.push_back(*static_cast<const TType*>(m_value));
    else
    {
      const std::vector<TType>* vecValues = static_cast<const std::vector<TType>*>(m_value);
      for (const auto& it : *vecValues)
        values.push_back(it);
    }

    return values;
  }

  template<typename TType>
  std::vector<TType> GetValuesInternal(typename std::enable_if_t<std::is_pointer<TType>::value, void*> t = nullptr) const
  {
    std::vector<TType> values;
    if (!m_multipleValues)
      values.push_back(static_cast<const TType>(m_value));
    else
    {
      const std::vector<TType>* vecValues = static_cast<const std::vector<TType>*>(m_value);
      for (const auto& it : *vecValues)
        values.push_back(it);
    }

    return values;
  }

  std::string m_name;
  void* m_value;
  Type m_type;

  bool m_isElement;
  bool m_required;
  bool m_multipleValues;
  bool m_valid;
  Generator m_valueGenerator;
  bool m_valueGenerated;
};
