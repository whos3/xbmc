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
