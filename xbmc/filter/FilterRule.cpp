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

#include "FilterRule.h"
#include "Filter.h"
#include "utils/StringUtils.h"
#include "utils/Variant.h"

using namespace std;

bool CFilterOperator::Load(const CVariant &obj)
{
  if (!obj.isObject() ||
      !obj.isMember("operation") || !obj["operation"].isString())
    return false;

  *this = CFilter::TranslateOperator(obj["operation"].asString());
  if (obj.isMember("negated") && obj["negated"].isBoolean())
    m_negated = obj["negated"].asBoolean();

  return true;
}

bool CFilterOperator::Save(CVariant &obj) const
{
  if (obj.isNull() || m_operation == FilterOperationNone)
    return false;

  obj["operation"] = CFilter::TranslateOperator(*this);
  if (m_negated)
    obj["negated"] = true;

  return true;
}

CFilterRule::CFilterRule()
  : m_field(FieldNone), m_type(FilterFieldTypeNone), m_browseable(false)
{ }

bool CFilterRule::Load(const CVariant &obj)
{
  if (!obj.isObject() ||
      !obj.isMember("field") || !obj["field"].isString() ||
      !obj.isMember("operator") || !obj["operator"].isObject() ||
      !m_operator.Load(obj["operator"]))
    return false;
  
  m_field = CFilter::TranslateField(obj["field"].asString().c_str());  
  if (m_operator.m_operation == FilterOperationTrue)
    return true;
  
  if (!obj.isMember("value") || (!obj["value"].isString() && !obj["value"].isArray()))
    return false;
  
  const CVariant &value = obj["value"];
  if (value.isString() && !value.asString().empty())
    m_value.push_back(value.asString());
  else if (value.isArray())
  {
    for (CVariant::const_iterator_array val = value.begin_array(); val != value.end_array(); val++)
    {
      if (val->isString() && !val->asString().empty())
        m_value.push_back(val->asString());
    }
  }
  else
    return false;
  
  return true;
}

bool CFilterRule::Save(CVariant &obj) const
{
  if (obj.isNull() ||
     (m_value.empty() && m_operator.m_operation != FilterOperationTrue) ||
     !m_operator.Save(obj["operator"]))
    return false;
  
  obj["field"] = CFilter::TranslateField(m_field);  
  obj["value"] = CVariant(CVariant::VariantTypeArray);
  for (vector<std::string>::const_iterator it = m_value.begin(); it != m_value.end(); it++)
    obj["value"].push_back(*it);
  
  return true;
}

std::string CFilterRule::GetValue() const
{
  return StringUtils::Join(m_value, " / ");
}

void CFilterRule::SetValue(const std::string &value)
{
  m_value.clear();
  m_value = StringUtils::Split(value, " / ");
}

void CFilterRule::SetValue(const std::vector<std::string> &values)
{
  m_value.assign(values.begin(), values.end());
}
