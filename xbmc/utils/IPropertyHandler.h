#pragma once
/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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

#include "DatabaseUtils.h"

#ifndef my_offsetof
#ifndef _LINUX
#define my_offsetof(TYPE, MEMBER) offsetof(TYPE, MEMBER)
#else
/*
 C us*tom version of standard offsetof() macro which can be used to get
 offsets of members in class for non-POD types (according to the current
 version of C++ standard offsetof() macro can't be used in such cases and
 attempt to do so causes warnings to be emitted, OTOH in many cases it is
 still OK to assume that all instances of the class has the same offsets
 for the same members).
   */
#define my_offsetof(TYPE, MEMBER) \
((size_t)((char *)&(((TYPE *)0x10)->MEMBER) - (char*)0x10))
#endif
#endif

class CVariant;

typedef enum {
  HandledPropertyTypeBool,
  HandledPropertyTypeInt32,
  HandledPropertyTypeUInt32,
  HandledPropertyTypeInt64,
  HandledPropertyTypeUInt64,
  HandledPropertyTypeFloat,
  HandledPropertyTypeDouble,
  HandledPropertyTypeString,
  HandledPropertyTypeArrayString,
  HandledPropertyTypeMapString,
  HandledPropertyTypeMapVariant,
  HandledPropertyTypeDate,
  HandledPropertyTypeDateTime,
  HandledPropertyTypeVariant
} HandledPropertyType;

typedef struct {
  Field m_field;
  std::string m_name;
  HandledPropertyType m_type;
  size_t m_member;
} HandledProperty;

class IPropertyHandler
{
public:
  virtual ~IPropertyHandler();

protected:
  void SetHandledPropertyMap(HandledProperty propertyMap[]);

  std::set<Field> GetHandledPropertyFields() const;
  std::set<std::string> GetHandledPropertyNames() const;

  bool GetHandledPropertyValue(Field propertyField, CVariant &value) const;
  bool GetHandledPropertyValue(const std::string &propertyName, CVariant &value) const;
  
  bool SetHandledPropertyValue(Field propertyField, const CVariant &value);
  bool SetHandledPropertyValue(const std::string &propertyName, const CVariant &value);

private:
  const HandledProperty* getHandledProperty(Field propertyField) const;
  const HandledProperty* getHandledProperty(const std::string &propertyName) const;
  
  bool getHandledPropertyValue(const HandledProperty *prop, CVariant &value) const;
  bool setHandledPropertyValue(const HandledProperty *prop, const CVariant &value);
  
  HandledProperty *m_propertyMap;
  std::map<Field, HandledProperty*> m_fieldMap;
  std::map<std::string, HandledProperty*> m_nameMap;
};
