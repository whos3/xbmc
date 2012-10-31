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

#include "utils/DatabaseUtils.h"

typedef enum {
  FilterFieldTypeNone         = 0,
  FilterFieldTypeBoolean      = 1,
  FilterFieldTypeString       = 2,
  FilterFieldTypeInteger      = 3,
  FilterFieldTypeNumber       = 4,
  FilterFieldTypeDate         = 5,
  FilterFieldTypeTime         = 6,
  FilterFieldTypePlaylist     = 7
} FilterFieldType;

typedef enum {
  FilterOperationNone         =  0,
  FilterOperationContains     =  1,
  FilterOperationEquals       =  2,
  FilterOperationStartsWith   =  3,
  FilterOperationEndsWith     =  4,
  FilterOperationGreaterThan  =  5,
  FilterOperationLessThan     =  6,
  FilterOperationAfter        =  7,
  FilterOperationBefore       =  8,
  FilterOperationInTheLast    =  9,
  FilterOperationTrue         = 10,
  FilterOperationBetween      = 11
} FilterOperation;

class CFilterOperator
{
public:
  CFilterOperator(FilterOperation op = FilterOperationNone, bool neg = false)
    : operation(op), negated(neg)
  { }

  bool operator==(const CFilterOperator &other) const { return operation == other.operation && negated == other.negated; }
  bool operator!=(const CFilterOperator &other) const { return !(*this == other); }

  int ToInt() const { return (int)operation | (int)negated << 31; }
  void FromInt(int op)
  {
    operation = (FilterOperation)(op & 0x7FFFFFFF);
    negated = (bool)(op >> 31);
  }

  FilterOperation operation;
  bool negated;
};

class CFilterRule
{
public:
  CFilterRule();
  virtual ~CFilterRule() { }

  virtual bool Load(const CVariant &obj);
  virtual bool Save(CVariant &obj) const;

  std::string GetValue() const;
  void SetValue(const std::string &value);
  void SetValue(const std::vector<std::string> &values);

  Field m_field;
  FilterFieldType m_type;
  CFilterOperator m_operator;
  std::vector<std::string> m_value;
  bool m_browseable;
};

class CFilterRuleCombination;

typedef std::vector<CFilterRule> CFilterRules;
typedef std::vector<CFilterRuleCombination> CFilterRuleCombinations;

class CFilterRuleCombination
{
public:
  CFilterRuleCombination();

  typedef enum {
    CombinationOr = 0,
    CombinationAnd
  } Combination;

  virtual bool Load(const CVariant &obj);
  virtual bool Save(CVariant &obj) const;

  std::string TranslateCombinationType() const;

  Combination GetType() const { return m_type; }
  void SetType(Combination combination) { m_type = combination; }

  void AddRule(const CFilterRule &rule);
  void AddCombination(const CFilterRuleCombination &rule);

  Combination m_type;
  CFilterRuleCombinations m_combinations;
  CFilterRules m_rules;
};

class CFilter
{
public:
  CFilter();
  virtual ~CFilter() { }
  
  virtual void Reset();
  
  virtual bool Load(const CVariant &obj);
  virtual bool Save(CVariant &obj) const;
  
  MediaType GetType() const { return m_type; }
  virtual void SetType(MediaType type) { m_type = type; }

  void SetMatchAllRules(bool matchAll) { m_ruleCombination.SetType(matchAll ? CFilterRuleCombination::CombinationAnd : CFilterRuleCombination::CombinationOr); }
  bool GetMatchAllRules() const { return m_ruleCombination.GetType() == CFilterRuleCombination::CombinationAnd; }
  
  virtual bool IsEmpty() const { return m_ruleCombination.m_rules.empty() && m_ruleCombination.m_combinations.empty(); }

protected:
  CFilterRuleCombination m_ruleCombination;
  MediaType m_type;
};

class FilterUtils
{
public:
  static void GetAvailableFields(std::vector<std::string> &fieldList);
  static const std::string& TranslateField(Field field);
  static Field TranslateField(const std::string &field);
  static std::string GetLocalizedField(Field field);
  
  static void GetAvailableOperators(std::vector<std::string> &operatorList);
  static const std::string& TranslateOperator(const CFilterOperator &op);
  static const CFilterOperator& TranslateOperator(const std::string &op);
  static std::string GetLocalizedOperator(const CFilterOperator &op);
};
