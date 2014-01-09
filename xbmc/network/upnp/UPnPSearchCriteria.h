#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include "utils/BooleanLogic.h"
#include "utils/ExpressionParser.h"

typedef enum {
  UPnPSearchCriteriaOperatorNone = 0,
  UPnPSearchCriteriaOperatorEquals,
  UPnPSearchCriteriaOperatorNotEquals,
  UPnPSearchCriteriaOperatorLessThan,
  UPnPSearchCriteriaOperatorLessThanOrEqual,
  UPnPSearchCriteriaOperatorGreaterThan,
  UPnPSearchCriteriaOperatorGreaterThanOrEqual,
  UPnPSearchCriteriaOperatorContains,
  UPnPSearchCriteriaOperatorDoesNotContain,
  UPnPSearchCriteriaOperatorStartsWith,
  UPnPSearchCriteriaOperatorDerivedFrom,
  UPnPSearchCriteriaOperatorExists,
  UPnPSearchCriteriaOperatorDoesNotExist,
  UPnPSearchCriteriaOperatorAll
} UPnPSearchCriteriaOperator;

class IUPnPSearchCriteria
{
public:
  virtual ~IUPnPSearchCriteria() { }

  virtual bool Deserialize(const IExpressionPtr &expression) = 0;

protected:
  IUPnPSearchCriteria() { }
};

class CUPnPSearchCriteriaCondition : public IUPnPSearchCriteria, public CBooleanLogicValue
{
public:
  CUPnPSearchCriteriaCondition() { }
  virtual ~CUPnPSearchCriteriaCondition() { }

  const std::string& GetProperty() const { return m_property; }
  UPnPSearchCriteriaOperator GetOperator() const { return m_operator; }

  // implementation of IUPnPSearchCriteria
  virtual bool Deserialize(const IExpressionPtr &expression);

  // implementation of CBooleanLogic
  virtual bool Deserialize(const TiXmlNode *node) { return false; }

private:
  bool SetOperator(const std::string &strOperator);

  std::string m_property;
  UPnPSearchCriteriaOperator m_operator;
};

class CUPnPSearchCriteriaCombination : public IUPnPSearchCriteria, public CBooleanLogicOperation
{
public:
  CUPnPSearchCriteriaCombination(BooleanLogicOperation op = BooleanLogicOperationAnd)
    : CBooleanLogicOperation(op)
  { }
  virtual ~CUPnPSearchCriteriaCombination() { }
  
  // implementation of IUPnPSearchCriteria
  virtual bool Deserialize(const IExpressionPtr &expression);

  // implementation of CBooleanLogic
  virtual bool Deserialize(const TiXmlNode *node) { return false; }

protected:
  virtual CBooleanLogicOperation* newOperation() { return new CUPnPSearchCriteriaCombination(); }
  virtual CBooleanLogicValue* newValue() { return new CUPnPSearchCriteriaCondition(); }

private:
  bool handleSearchExpression(const IExpressionPtr &expression);
};

class CUPnPSearchCriteria : public CBooleanLogic
{
public:
  CUPnPSearchCriteria();
  virtual ~CUPnPSearchCriteria() { }

  virtual bool Deserialize(const std::string &searchCriteria);

  // implementation of CBooleanLogic
  virtual bool Deserialize(const TiXmlNode *node) { return false; }
};
