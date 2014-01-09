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

#include "UPnPSearchCriteria.h"
#include "utils/StringUtils.h"

using namespace std;

bool CUPnPSearchCriteriaCondition::Deserialize(const IExpressionPtr &expression)
{
  if (expression == NULL || expression->GetValue().empty())
    return false;

  if (expression->GetName() == "asterisk" && !expression->GetValue().empty())
  {
    m_operator = UPnPSearchCriteriaOperatorAll;
    return true;
  }

  if (expression->GetName() != "relExp")
    return false;

  boost::shared_ptr<CAlternativeExpression> relExp = boost::static_pointer_cast<CAlternativeExpression>(expression);
  if (relExp->GetResult() == NULL)
    return false;

  boost::shared_ptr<CSequenceExpression> exp = boost::static_pointer_cast<CSequenceExpression>(relExp->GetResult());
  const std::vector<IExpressionPtr>& expressions = exp->GetResult();
  if (expressions.size() != 5)
    return false;
  
  if (!SetOperator(expressions.at(2)->GetValue()))
    return false;

  m_property = expressions.at(0)->GetValue();
  if (m_operator == UPnPSearchCriteriaOperatorExists)
  {
    if (expressions.at(4)->GetValue() == "false")
      m_operator = UPnPSearchCriteriaOperatorDoesNotExist;
  }
  else
  {
    if (expressions.at(4)->GetName() != "quotedVal")
      return false;

    boost::shared_ptr<CSequenceExpression> quotedValExp = boost::static_pointer_cast<CSequenceExpression>(expressions.at(4));
    const std::vector<IExpressionPtr>& quotedValExps = quotedValExp->GetResult();
    if (quotedValExps.size() != 3)
      return false;

    if (quotedValExps.at(1)->GetName() != "escapedQuote")
      return false;

    m_value = StringUtils::Unescape(quotedValExps.at(1)->GetValue());
  }

  return true;
}

bool CUPnPSearchCriteriaCondition::SetOperator(const std::string &strOperator)
{
  if (strOperator == "=")
    m_operator = UPnPSearchCriteriaOperatorEquals;
  else if (strOperator == "!=")
    m_operator = UPnPSearchCriteriaOperatorNotEquals;
  else if (strOperator == "<")
    m_operator = UPnPSearchCriteriaOperatorLessThan;
  else if (strOperator == "<=")
    m_operator = UPnPSearchCriteriaOperatorLessThanOrEqual;
  else if (strOperator == ">")
    m_operator = UPnPSearchCriteriaOperatorGreaterThan;
  else if (strOperator == ">=")
    m_operator = UPnPSearchCriteriaOperatorGreaterThanOrEqual;
  else if (StringUtils::EqualsNoCase(strOperator, "contains"))
    m_operator = UPnPSearchCriteriaOperatorContains;
  else if (StringUtils::EqualsNoCase(strOperator, "doesNotContain"))
    m_operator = UPnPSearchCriteriaOperatorDoesNotContain;
  else if (StringUtils::EqualsNoCase(strOperator, "startsWith"))
    m_operator = UPnPSearchCriteriaOperatorStartsWith;
  else if (StringUtils::EqualsNoCase(strOperator, "derivedfrom"))
    m_operator = UPnPSearchCriteriaOperatorDerivedFrom;
  else if (StringUtils::EqualsNoCase(strOperator, "exists"))
    m_operator = UPnPSearchCriteriaOperatorExists;
  else if (strOperator == "*")
    m_operator = UPnPSearchCriteriaOperatorAll;
  else
    return false;

  return true;
}

bool CUPnPSearchCriteriaCombination::Deserialize(const IExpressionPtr &expression)
{
  if (expression == NULL || expression->GetValue().empty())
    return false;

  if (expression->GetName() == "asterisk" || expression->GetName() == "relExp")
  {
    CUPnPSearchCriteriaCondition *condition = static_cast<CUPnPSearchCriteriaCondition*>(newValue());
    if (condition == NULL || !condition->Deserialize(expression))
      return false;

    m_values.push_back(CBooleanLogicValuePtr(condition));
    return true;
  }

  if (expression->GetName() == "searchExp")
  {
    boost::shared_ptr<CAlternativeExpression> searchExp = boost::static_pointer_cast<CAlternativeExpression>(expression);
    if (searchExp->GetResult() == NULL)
      return false;

    return Deserialize(searchExp->GetResult());
  }

  if (expression->GetName() == "searchExpLogOp" || expression->GetName() == "searchExpBraces")
  {
    boost::shared_ptr<CSequenceExpression> searchExpResult = boost::static_pointer_cast<CSequenceExpression>(expression);
    const std::vector<IExpressionPtr> &results = searchExpResult->GetResult();
    if (results.size() != 5)
      return false;

    if (expression->GetName() == "searchExpLogOp")
    {
      boost::shared_ptr<CAlternativeExpression> logOp = boost::static_pointer_cast<CAlternativeExpression>(results.at(2));
      if (logOp == NULL || logOp->GetName() != "logOp" || logOp->GetResult() == NULL)
        return false;

      if (logOp->GetResult()->GetName() == "and")
        m_operation = BooleanLogicOperationAnd;
      else if (logOp->GetResult()->GetName() == "or")
        m_operation = BooleanLogicOperationOr;
      else
        return false;

      if (!handleSearchExpression(results.at(0)) ||
          !handleSearchExpression(results.at(4)))
        return false;

      return true;
    }
    else
      return Deserialize(results.at(2));
  }

  return false;
}

bool CUPnPSearchCriteriaCombination::handleSearchExpression(const IExpressionPtr &expression)
{
  if (expression == NULL || expression->GetName() != "searchExp")
    return false;

  CUPnPSearchCriteriaCombination *combinationLeft = static_cast<CUPnPSearchCriteriaCombination*>(newOperation());
  if (combinationLeft == NULL || !combinationLeft->Deserialize(expression))
    return false;

  if (combinationLeft->GetOperations().empty() && combinationLeft->GetValues().size() == 1)
  {
    m_values.push_back(combinationLeft->GetValues().at(0));
    delete combinationLeft;
  }
  else
    m_operations.push_back(CBooleanLogicOperationPtr(combinationLeft));

  return true;
}

CUPnPSearchCriteria::CUPnPSearchCriteria()
{
  m_operation = CBooleanLogicOperationPtr(new CUPnPSearchCriteriaCombination());
}

bool CUPnPSearchCriteria::Deserialize(const std::string &searchCriteria)
{
  if (m_operation == NULL)
    return false;

  // asterisk = *
  IExpressionPtr expCAsterisk(new CCharacterExpression("asterisk", '*'));
  // dQuote = "
  IExpressionPtr expCDoubleQuotes(new CCharacterExpression("dQuote", '"'));
  // escapedQuote = (\\.|[^"])*
  IExpressionPtr expREscapedQuote(new CRegexpExpression("escapedQuote", "(\\\\.|[^\"])*"));
  // TODO: property = @?[a-zA-Z]+|[a-zA-Z]+(:[a-zA-Z]+)?(@[a-zA-Z]+)?
  IExpressionPtr expRProperty(new CRegexpExpression("property", "@[a-z]+|[a-z]+(:[a-z]+)?(@[a-z]+)?", false));
  // wChar+
  IExpressionPtr expSWhitespace(new CStringExpression("wChar+", " \f\n\r\t\v", 1, -1));
  // wChar*
  IExpressionPtr expSWhitespaceOptional(new CStringExpression("wChar*", " \f\n\r\t\v", 0, -1));
  // quotedVal = dQuote escapedQuote dQuote
  std::vector<IExpressionPtr> quotedValueExps;
  quotedValueExps.push_back(expCDoubleQuotes);
  quotedValueExps.push_back(expREscapedQuote);
  quotedValueExps.push_back(expCDoubleQuotes);
  IExpressionPtr expSBooleanValue(new CSequenceExpression("quotedVal", quotedValueExps));
  // boolVal = true | false
  std::vector<IExpressionPtr> booleanValueExps;
  booleanValueExps.push_back(IExpressionPtr(new CFixedStringExpression("true", "true")));
  booleanValueExps.push_back(IExpressionPtr(new CFixedStringExpression("false", "false")));
  IExpressionPtr expABooleanValue(new CAlternativeExpression("boolVal", booleanValueExps));
  // existsOp = exists
  IExpressionPtr expFExistsOperation(new CFixedStringExpression("existsOp", "exists"));
  // stringOp = contains | doesNotContain | startsWith | derivedFrom
  std::vector<IExpressionPtr> stringOperationExps;
  stringOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("contains", "contains")));
  stringOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("doesNotContain", "doesNotContain")));
  stringOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("startsWith", "startsWith")));
  stringOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("derivedFrom", "derivedFrom")));
  IExpressionPtr expAStringOperations(new CAlternativeExpression("stringOp", stringOperationExps));
  // relOp = = | != | < | <= | > | >=
  std::vector<IExpressionPtr> relativeOperationExps;
  relativeOperationExps.push_back(IExpressionPtr(new CCharacterExpression("=", '=')));
  relativeOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("!=", "!=")));
  relativeOperationExps.push_back(IExpressionPtr(new CCharacterExpression("<", '<')));
  relativeOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("<=", "<=")));
  relativeOperationExps.push_back(IExpressionPtr(new CCharacterExpression(">", '>')));
  relativeOperationExps.push_back(IExpressionPtr(new CFixedStringExpression(">=", ">=")));
  IExpressionPtr expARelativeOperations(new CAlternativeExpression("relOp", relativeOperationExps));
  // binOp = relOp | stringOp
  std::vector<IExpressionPtr> binaryOperationExps;
  binaryOperationExps.push_back(expARelativeOperations);
  binaryOperationExps.push_back(expAStringOperations);
  IExpressionPtr expABinaryOperations(new CAlternativeExpression("binOp", binaryOperationExps));
  // relExp = property wChar+ binOp wChar+ quotedVal | property wChar+ existsOp wChar+ boolVal
  IExpressionPtr expARelativeExpression(new CAlternativeExpression("relExp"));
  std::vector<IExpressionPtr> relBinOpExps;
  relBinOpExps.push_back(expRProperty);
  relBinOpExps.push_back(expSWhitespace);
  relBinOpExps.push_back(expABinaryOperations);
  relBinOpExps.push_back(expSWhitespace);
  relBinOpExps.push_back(expSBooleanValue);
  boost::static_pointer_cast<CAlternativeExpression>(expARelativeExpression)->Add(IExpressionPtr(new CSequenceExpression("relExpBinOp", relBinOpExps)));
  std::vector<IExpressionPtr> relExistsOpExps;
  relExistsOpExps.push_back(expRProperty);
  relExistsOpExps.push_back(expSWhitespace);
  relExistsOpExps.push_back(expFExistsOperation);
  relExistsOpExps.push_back(expSWhitespace);
  relExistsOpExps.push_back(expABooleanValue);
  boost::static_pointer_cast<CAlternativeExpression>(expARelativeExpression)->Add(IExpressionPtr(new CSequenceExpression("relExpExistsOp", relExistsOpExps)));
  // logOp = and | or
  std::vector<IExpressionPtr> logicalOperationExps;
  logicalOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("and", "and")));
  logicalOperationExps.push_back(IExpressionPtr(new CFixedStringExpression("or", "or")));
  IExpressionPtr expALogicalOperations(new CAlternativeExpression("logOp", logicalOperationExps));
  // searchExp = relExp | searchExp wChar+ logOp wChar+ searchExp | ( wChar* searchExp wChar* )
  IExpressionPtr expASearchExpression(new CAlternativeExpression("searchExp"));
  boost::static_pointer_cast<CAlternativeExpression>(expASearchExpression)->Add(expARelativeExpression);
  std::vector<IExpressionPtr> searchLogOpExps;
  searchLogOpExps.push_back(expASearchExpression);
  searchLogOpExps.push_back(expSWhitespace);
  searchLogOpExps.push_back(expALogicalOperations);
  searchLogOpExps.push_back(expSWhitespace);
  searchLogOpExps.push_back(expASearchExpression);
  boost::static_pointer_cast<CAlternativeExpression>(expASearchExpression)->Add(IExpressionPtr(new CSequenceExpression("searchExpLogOp", searchLogOpExps)));
  std::vector<IExpressionPtr> searchBracesExps;
  searchBracesExps.push_back(IExpressionPtr(new CCharacterExpression("openingBrace", '(')));
  searchBracesExps.push_back(expSWhitespaceOptional);
  searchBracesExps.push_back(expASearchExpression);
  searchBracesExps.push_back(expSWhitespaceOptional);
  searchBracesExps.push_back(IExpressionPtr(new CCharacterExpression("closingBrace", ')')));
  boost::static_pointer_cast<CAlternativeExpression>(expASearchExpression)->Add(IExpressionPtr(new CSequenceExpression("searchExpBraces", searchBracesExps)));
  // TsearchCrit = searchExp | asterisk
  std::vector<IExpressionPtr> searchCriteriaExps;
  searchCriteriaExps.push_back(expASearchExpression);
  searchCriteriaExps.push_back(expCAsterisk);
  IExpressionPtr expASearchCriteria(new CAlternativeExpression("searchCrit", searchCriteriaExps));

  CExpressionParser expParser;
  if (!expParser.Define(expASearchCriteria))
    return false;

  if (!expParser.Parse(searchCriteria) || expParser.GetResult() == NULL)
    return false;

  boost::shared_ptr<CAlternativeExpression> searchCritExp = boost::static_pointer_cast<CAlternativeExpression>(expParser.GetResult());
  return boost::static_pointer_cast<CUPnPSearchCriteriaCombination>(m_operation)->Deserialize(searchCritExp->GetResult());
}
