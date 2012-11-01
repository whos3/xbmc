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
    : m_operation(op), m_negated(neg)
  { }

  bool operator==(const CFilterOperator &other) const { return m_operation == other.m_operation && m_negated == other.m_negated; }
  bool operator!=(const CFilterOperator &other) const { return !(*this == other); }
  
  virtual bool Load(const CVariant &obj);
  virtual bool Save(CVariant &obj) const;

  FilterOperation GetOperation() const { return m_operation; }
  void SetOperation(FilterOperation op) { m_operation = op; }
  bool IsNegated() const { return m_negated; }
  void Negate(bool negate) { m_negated = negate; }

  int ToInt() const { return (int)m_operation | (int)m_negated << 31; }
  void FromInt(int op)
  {
    m_operation = (FilterOperation)(op & 0x7FFFFFFF);
    m_negated = (bool)(op >> 31);
  }

private:
  friend class CFilterRule;

  FilterOperation m_operation;
  bool m_negated;
};

class CFilterRule
{
public:
  CFilterRule();
  virtual ~CFilterRule() { }

  virtual bool Load(const CVariant &obj);
  virtual bool Save(CVariant &obj) const;

  Field GetField() const { return m_field; }
  void SetField(Field field) { m_field = field; }
  FilterFieldType GetType() const { return m_type; }
  void SetType(FilterFieldType type) { m_type = type; }
  const CFilterOperator& GetOperator() const { return m_operator; }
  CFilterOperator& GetOperator() { return m_operator; }
  void SetOperator(const CFilterOperator& op) { m_operator = op; }
  bool IsBrowseable() const { return m_browseable; }
  void SetBrowseable(bool browseable) { m_browseable = browseable; }

  std::string GetValue() const;
  const std::vector<std::string>& GetValues() const { return m_value; }
  void SetValue(const std::string &value);
  void SetValue(const std::vector<std::string> &values);

private:
  Field m_field;
  FilterFieldType m_type;
  CFilterOperator m_operator;
  std::vector<std::string> m_value;
  bool m_browseable;
};
