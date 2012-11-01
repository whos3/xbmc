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

#include <string>
#include <vector>

class CVariant;

class CFilterRule;
class CFilterRuleCombination;

typedef std::vector<CFilterRule> CFilterRules;
typedef std::vector<CFilterRuleCombination> CFilterRuleCombinations;

class CFilterRuleCombination
{
public:
  CFilterRuleCombination();
  virtual ~CFilterRuleCombination() { }

  typedef enum {
    CombinationOr = 0,
    CombinationAnd
  } Combination;

  virtual void Reset();

  virtual bool Load(const CVariant &obj);
  virtual bool Save(CVariant &obj) const;

  std::string TranslateCombinationType() const;

  Combination GetType() const { return m_type; }
  void SetType(Combination combination) { m_type = combination; }

  const CFilterRules& GetRules() const { return m_rules; }
  CFilterRules& GetRules() { return m_rules; }
  void AddRule(const CFilterRule &rule);
  const CFilterRuleCombinations& GetCombinations() const { return m_combinations; }
  CFilterRuleCombinations& GetCombinations() { return m_combinations; }
  void AddCombination(const CFilterRuleCombination &rule);

private:
  Combination m_type;
  CFilterRuleCombinations m_combinations;
  CFilterRules m_rules;
};
