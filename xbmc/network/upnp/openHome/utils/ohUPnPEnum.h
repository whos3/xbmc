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

#include <algorithm>
#include <set>
#include <string>

template<typename TEnum>
using OhUPnPEnumValueDefinition = std::pair<TEnum, std::string>;

template<typename TEnum>
using OhUPnPEnumValueDefinitions = std::set<OhUPnPEnumValueDefinition<TEnum>>;

template<typename TEnum, TEnum Default>
class COhUPnPEnum
{
public:
  ~COhUPnPEnum() = default;

  TEnum Get() const { return m_value; }
  const std::string& GetAsString() const { return m_string; }
  bool IsVendorSpecific() const { return m_isVendorSpecific; }

  operator TEnum() { return m_value; }

  bool operator==(TEnum value) const
  {
    if (m_isVendorSpecific)
      return false;

    return m_value == value;
  }

  bool operator!=(TEnum value)
  {
    return !(*this == value);
  }

  virtual void Set(TEnum value)
  {
    const auto& it = std::find_if(m_definition.cbegin(), m_definition.cend(),
      [&value](const OhUPnPEnumValueDefinition<TEnum>& enumValueDefinition)
      {
        return std::get<0>(enumValueDefinition) == value;
      });

    // if the value couldn't be found in the definitions
    // it's something vendor specific
    m_isVendorSpecific = it == m_definition.cend();
    if (m_isVendorSpecific)
    {
      m_value = Default;
      m_string.clear();
    }
    else
    {
      m_value = std::get<0>(*it);
      m_string = std::get<1>(*it);
    }
  }

  virtual void SetFromString(const std::string& value)
  {
    const auto& it = std::find_if(m_definition.cbegin(), m_definition.cend(),
      [&value](const OhUPnPEnumValueDefinition<TEnum>& enumValueDefinition)
      {
        return std::get<1>(enumValueDefinition) == value;
      });

    // if the value couldn't be found in the definitions
    // it's something vendor specific
    m_isVendorSpecific = it == m_definition.cend();
    if (m_isVendorSpecific)
    {
      m_value = Default;
      m_string = value;
    }
    else
    {
      m_value = std::get<0>(*it);
      m_string = std::get<1>(*it);
    }
  }

protected:
  COhUPnPEnum(const OhUPnPEnumValueDefinitions<TEnum>& definition)
    : COhUPnPEnum(definition, Default)
  { }

  COhUPnPEnum(const OhUPnPEnumValueDefinitions<TEnum>& definition, TEnum value)
    : m_value(value)
    , m_isVendorSpecific(false)
    , m_definition(definition)
  {
    Set(value);
  }

  COhUPnPEnum(const OhUPnPEnumValueDefinitions<TEnum>& definition, const std::string& value)
    : m_value(Default)
    , m_isVendorSpecific(false)
    , m_definition(definition)
  {
    SetFromString(value);
  }

  TEnum m_value;
  std::string m_string;
  bool m_isVendorSpecific;

  OhUPnPEnumValueDefinitions<TEnum> m_definition;
};
