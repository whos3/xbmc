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

#include "IPropertyHandler.h"
#include "Variant.h"
#include "XBDateTime.h"
#include "log.h"

IPropertyHandler::~IPropertyHandler()
{
  m_fieldMap.clear();
  m_nameMap.clear();
}

void IPropertyHandler::SetHandledPropertyMap(HandledProperty propertyMap[], size_t size)
{
  if (propertyMap == NULL || size <= 0)
    return;

  m_propertyMap = propertyMap;
  
  for (size_t i = 0; i < size; i++)
  {
    HandledProperty *prop = &propertyMap[i];
    if (prop == NULL)
      continue;

    if (prop->m_field != FieldNone)
      m_fieldMap.insert(std::make_pair(prop->m_field, prop));
    if (!prop->m_name.empty())
      m_nameMap.insert(std::make_pair(prop->m_name, prop));
  }
}

std::set<Field> IPropertyHandler::GetHandledPropertyFields() const
{
  std::set<Field> fields;
  for (std::map<Field, HandledProperty*>::const_iterator field = m_fieldMap.begin(); field != m_fieldMap.end(); field++)
    fields.insert(field->first);

  return fields;
}

std::set<std::string> IPropertyHandler::GetHandledPropertyNames() const
{
  std::set<std::string> names;
  for (std::map<std::string, HandledProperty*>::const_iterator field = m_nameMap.begin(); field != m_nameMap.end(); field++)
    names.insert(field->first);
  
  return names;
}

bool IPropertyHandler::GetHandledPropertyValue(Field propertyField, CVariant &value) const
{
  return getHandledPropertyValue(getHandledProperty(propertyField), value);
}

bool IPropertyHandler::GetHandledPropertyValue(const std::string &propertyName, CVariant &value) const
{
  return getHandledPropertyValue(getHandledProperty(propertyName), value);
}

bool IPropertyHandler::SetHandledPropertyValue(Field propertyField, const CVariant &value)
{
  return setHandledPropertyValue(getHandledProperty(propertyField), value);
}

bool IPropertyHandler::SetHandledPropertyValue(const std::string &propertyName, const CVariant &value)
{
  return setHandledPropertyValue(getHandledProperty(propertyName), value);
}

const HandledProperty* IPropertyHandler::getHandledProperty(Field propertyField) const
{
  if (propertyField == FieldNone)
    return NULL;
  
  std::map<Field, HandledProperty*>::const_iterator it = m_fieldMap.find(propertyField);
  if (it == m_fieldMap.end())
  {
    CLog::Log(LOGDEBUG, "IPropertyHandler: unknown field-based property (%d)", propertyField);
    return NULL;
  }

  return it->second;
}

const HandledProperty* IPropertyHandler::getHandledProperty(const std::string &propertyName) const
{
  if (propertyName.empty())
    return NULL;
  
  std::map<std::string, HandledProperty*>::const_iterator it = m_nameMap.find(propertyName);
  if (it == m_nameMap.end())
  {
    CLog::Log(LOGDEBUG, "IPropertyHandler: unknown name-based property (%s)", propertyName.c_str());
    return NULL;
  }

  return it->second;
}

bool IPropertyHandler::getHandledPropertyValue(const HandledProperty *prop, CVariant &value) const
{
  if (prop == NULL || value == CVariant::ConstNullVariant)
    return false;

  if (!value.empty())
    value.clear();
  
  switch (prop->m_type)
  {
    case HandledPropertyTypeBool:
      value = *(bool*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeInt32:
      value = *(int32_t*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeUInt32:
      value = *(uint32_t*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeInt64:
      value = *(int64_t*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeUInt64:
      value = *(uint64_t*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeFloat:
      value = *(float*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeDouble:
      value = *(double*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeString:
      value = *(std::string*)(((char*)this) + prop->m_member);
      break;
      
    case HandledPropertyTypeArrayString:
      value = *(std::vector<std::string>*)(((char*)this) + prop->m_member);
      break;
    
    case HandledPropertyTypeMapString:
      value = *(std::map<std::string, std::string>*)(((char*)this) + prop->m_member);
      break;
    
    case HandledPropertyTypeMapVariant:
      value = *(std::map<std::string, CVariant>*)(((char*)this) + prop->m_member);
      break;
    
    case HandledPropertyTypeDate:
    {
      CDateTime *date = (CDateTime*)(((char*)this) + prop->m_member);
      value = date->IsValid() ? date->GetAsDBDate() : "";
      break;
    }
      
    case HandledPropertyTypeDateTime:
    {
      CDateTime *date = (CDateTime*)(((char*)this) + prop->m_member);
      value = date->IsValid() ? date->GetAsDBDateTime() : "";
      break;
    }
      
    case HandledPropertyTypeVariant:
      value = *(CVariant*)(((char*)this) + prop->m_member);
      break;
      
    default:
      return false;
  }

  return true;
}

bool IPropertyHandler::setHandledPropertyValue(const HandledProperty *prop, const CVariant &value)
{
  if (prop == NULL)
    return false;

  switch (prop->m_type)
  {
    case HandledPropertyTypeBool:
      if (!value.isBoolean())
        return false;
      
      *(bool*)(((char*)this) + prop->m_member) = value.asBoolean();
      break;
      
    case HandledPropertyTypeInt32:
      if (!value.isInteger())
        return false;
      
      *(int32_t*)(((char*)this) + prop->m_member) = (int32_t)value.asInteger();
      break;
      
    case HandledPropertyTypeUInt32:
      if (!value.isUnsignedInteger())
        return false;
      
      *(uint32_t*)(((char*)this) + prop->m_member) = (uint32_t)value.asUnsignedInteger();
      break;
      
    case HandledPropertyTypeInt64:
      if (!value.isInteger())
        return false;
      
      *(int64_t*)(((char*)this) + prop->m_member) = value.asInteger();
      break;
      
    case HandledPropertyTypeUInt64:
      if (!value.isUnsignedInteger())
        return false;
      
      *(uint64_t*)(((char*)this) + prop->m_member) = value.asUnsignedInteger();
      break;
      
    case HandledPropertyTypeFloat:
      if (!value.isDouble())
        return false;
      
      *(float*)(((char*)this) + prop->m_member) = value.asFloat();
      break;
      
    case HandledPropertyTypeDouble:
      if (!value.isDouble())
        return false;
      
      *(double*)(((char*)this) + prop->m_member) = value.asDouble();
      break;
      
    case HandledPropertyTypeString:
      if (!value.isString())
        return false;
      
      *(std::string*)(((char*)this) + prop->m_member) = value.asString();
      break;
      
    case HandledPropertyTypeArrayString:
    {
      if (!value.isArray())
        return false;

      std::vector<std::string> *member = (std::vector<std::string>*)(((char*)this) + prop->m_member);
      member->clear();
      
      for (CVariant::const_iterator_array it = value.begin_array(); it != value.end_array(); it++)
      {
        if (!it->isString())
        {
          member->clear();
          return false;
        }

        member->push_back(it->asString());
      }
      break;
    }
      
    case HandledPropertyTypeMapString:
    {
      if (!value.isObject())
        return false;
      
      std::map<std::string, std::string> *member = (std::map<std::string, std::string>*)(((char*)this) + prop->m_member);
      member->clear();
      
      for (CVariant::const_iterator_map it = value.begin_map(); it != value.end_map(); it++)
      {
        if (!it->second.isString())
        {
          member->clear();
          return false;
        }
        
        member->insert(std::make_pair(it->first, it->second.asString()));
      }
      break;
    }
    
    case HandledPropertyTypeMapVariant:
    {
      if (!value.isObject())
        return false;
      
      std::map<std::string, CVariant> *member = (std::map<std::string, CVariant>*)(((char*)this) + prop->m_member);
      member->clear();
      
      for (CVariant::const_iterator_map it = value.begin_map(); it != value.end_map(); it++)
        member->insert(std::make_pair(it->first, it->second));
      break;
    }
      
    case HandledPropertyTypeDate:
    case HandledPropertyTypeDateTime:
      if (value.isString())
      {
        if (prop->m_type == HandledPropertyTypeDate)
          ((CDateTime*)(((char*)this) + prop->m_member))->SetFromDBDate(value.asString());
        else
          ((CDateTime*)(((char*)this) + prop->m_member))->SetFromDBDateTime(value.asString());
      }
      else if (value.isInteger())
        ((CDateTime*)(((char*)this) + prop->m_member))->SetFromUTCDateTime((time_t)value.asInteger());
      else
        return false;
      break;
      
    case HandledPropertyTypeVariant:
      *(CVariant*)(((char*)this) + prop->m_member) = value;
      break;

    default:
      return false;
  }

  return true;
}
