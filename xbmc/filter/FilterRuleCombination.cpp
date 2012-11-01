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

#include "FilterRuleCombination.h"
#include "FilterRule.h"
#include "utils/Variant.h"

using namespace std;

CFilterRuleCombination::CFilterRuleCombination()
: m_type(CombinationAnd)
{ }

bool CFilterRuleCombination::Load(const CVariant &obj)
{
  if (!obj.isObject() && !obj.isArray())
    return false;
  
  CVariant child;
  if (obj.isObject())
  {
    if (obj.isMember("and") && obj["and"].isArray())
    {
      m_type = CombinationAnd;
      child = obj["and"];
    }
    else if (obj.isMember("or") && obj["or"].isArray())
    {
      m_type = CombinationOr;
      child = obj["or"];
    }
    else
      return false;
  }
  else
    child = obj;
  
  for (CVariant::const_iterator_array it = child.begin_array(); it != child.end_array(); it++)
  {
    if (!it->isObject())
      continue;
    
    if (it->isMember("and") || it->isMember("or"))
    {
      CFilterRuleCombination combo;
      if (combo.Load(*it))
        m_combinations.push_back(combo);
    }
    else
    {
      CFilterRule rule;
      if (rule.Load(*it))
        m_rules.push_back(rule);
    }
  }
  
  return true;
}

bool CFilterRuleCombination::Save(CVariant &obj) const
{
  if (!obj.isObject() || (m_combinations.empty() && m_rules.empty()))
    return false;
  
  CVariant comboArray(CVariant::VariantTypeArray);
  if (!m_combinations.empty())
  {
    for (CFilterRuleCombinations::const_iterator combo = m_combinations.begin(); combo != m_combinations.end(); combo++)
    {
      CVariant comboObj(CVariant::VariantTypeObject);
      if (combo->Save(comboObj))
        comboArray.push_back(comboObj);
    }
    
  }
  if (!m_rules.empty())
  {
    for (CFilterRules::const_iterator rule = m_rules.begin(); rule != m_rules.end(); rule++)
    {
      CVariant ruleObj(CVariant::VariantTypeObject);
      if (rule->Save(ruleObj))
        comboArray.push_back(ruleObj);
    }
  }
  
  obj[TranslateCombinationType()] = comboArray;
  
  return true;
}

std::string CFilterRuleCombination::TranslateCombinationType() const
{
  return m_type == CombinationAnd ? "and" : "or";
}

void CFilterRuleCombination::AddRule(const CFilterRule &rule)
{
  m_rules.push_back(rule);
}

void CFilterRuleCombination::AddCombination(const CFilterRuleCombination &combination)
{
  m_combinations.push_back(combination);
}