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

#include <string>

struct OhUPnPControlPointContext;
struct OhUPnPRootDeviceContext;
class IDidlLiteElement;
class TiXmlNode;

class CDidlLiteElementProperty
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

  using Generator = std::shared_ptr<const IDidlLiteElement>;

public:
  CDidlLiteElementProperty(Type type, void* value);
  CDidlLiteElementProperty(const std::string& name, Type type, void* value);
  CDidlLiteElementProperty(const std::string& ns, const std::string& name, Type type, void* value);
  virtual ~CDidlLiteElementProperty();

  bool Serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const;
  bool Deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context);

  CDidlLiteElementProperty& AsAttribute() { m_isElement = false; return *this; }
  CDidlLiteElementProperty& SetOptional(bool optional = true) { m_required = !optional; return *this; }
  CDidlLiteElementProperty& SetRequired(bool required = true) { m_required = required; return *this; }
  CDidlLiteElementProperty& SupportMultipleValues() { m_multipleValues = true; return *this; }
  CDidlLiteElementProperty& SetGenerator(Generator valueGenerator);
  CDidlLiteElementProperty& SetValid(bool valid = true) { m_valid = valid; return *this; }
  CDidlLiteElementProperty& SetInvalid(bool invalid = true) { m_valid = !invalid; return *this; }
  CDidlLiteElementProperty& SetMinimumVersion(uint8_t version = 1) { m_minimumVersion = version; return *this; }

  const std::string& GetName() const { return m_name; }

  const void* GetValue() const { return m_value; }
  void* GetValue() { return m_value; }
  template<typename TType> const TType* GetValue() const { return reinterpret_cast<TType*>(m_value); }
  template<typename TType> TType* GetValue() { return reinterpret_cast<TType*>(m_value); }

  Type GetType() const { return m_type; }
  bool IsElement() const { return m_isElement; }
  bool IsAttribute() const { return !m_isElement; }
  bool IsRequired() const { return m_required; }
  bool IsRequired(uint8_t version) const { return m_required && version >= m_minimumVersion; }
  bool IsOptional() const { return !m_required; }
  bool SupportsMultipleValues() const { return m_multipleValues; }
  bool IsValid() const { return m_valid; }
  uint8_t GetMinimumVersion() const { return m_minimumVersion; }

private:
  bool serializeElement(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const;
  bool serializeAttribute(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const;
  bool deserializeElement(const TiXmlNode* node, const OhUPnPControlPointContext& context);
  bool deserializeAttribute(const TiXmlNode* node, const OhUPnPControlPointContext& context);

  std::string m_name;
  void* m_value;
  Type m_type;

  bool m_isElement;
  bool m_required;
  bool m_multipleValues;
  bool m_valid;
  Generator m_valueGenerator;
  bool m_valueGenerated;
  uint8_t m_minimumVersion;
};
