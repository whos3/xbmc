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

#include "ExpressionParser.h"
#include "utils/RegExp.h"
#include "utils/StringUtils.h"

#define MAX_RECURSIVE_CALLS   10

IExpression::IExpression(const std::string &name)
  : m_name(name),
    m_recursiveCalls(0)
{ }

bool IExpression::Parse(const std::string &value, size_t &processedCharacters)
{
  processedCharacters = 0;

  if (!BeginParse())
    return false;

  bool ret = parse(value, processedCharacters);

  EndParse();

  return ret;
}

bool IExpression::BeginParse()
{
  if (m_recursiveCalls + 1 > MAX_RECURSIVE_CALLS)
    return false;

  m_recursiveCalls += 1;
  return true;
}

void IExpression::EndParse()
{
  if (m_recursiveCalls > 0)
    m_recursiveCalls -= 1;
}

void IExpression::clone(IExpression *clonedExpression)
{
  if (clonedExpression == NULL)
    return;

  clonedExpression->m_value = m_value;
  clonedExpression->m_recursiveCalls = m_recursiveCalls;
}

CCharacterExpression::CCharacterExpression(const std::string &name, char character, int minimum /* = 1 */, int maximum /* = 1 */)
  : IExpression(name),
    m_character(character),
    m_minimum(minimum),
    m_maximum(maximum)
{
  if (m_minimum < 0)
    m_minimum = 0;
  if (m_maximum == 0)
    m_maximum = 1;

  if (m_minimum > m_maximum)
    m_maximum = m_minimum;
}

IExpression* CCharacterExpression::Clone()
{
  CCharacterExpression *clone = new CCharacterExpression(m_name, m_character, m_minimum, m_maximum);
  if (clone == NULL)
    return NULL;

  IExpression::clone(clone);
  return clone;
}

bool CCharacterExpression::parse(const std::string &value, size_t &processedCharacters)
{
  size_t pos = value.find_first_not_of(m_character);
  if (pos == std::string::npos)
    pos = value.size();

  if (pos < (size_t)m_minimum)
    return false;

  if (m_maximum > 0 && pos > (size_t)m_maximum)
    pos = (size_t)m_maximum;

  m_value = value.substr(0, pos);
  processedCharacters = pos;
  return true;
}

CStringExpression::CStringExpression(const std::string &name, const std::string &characters, int minimum /* = 1 */, int maximum /* = 1 */)
  : IExpression(name),
    m_characters(characters),
    m_minimum(minimum),
    m_maximum(maximum)
{
  if (m_minimum < 0)
    m_minimum = 0;
  if (m_maximum == 0)
    m_maximum = 1;

  if (m_minimum > m_maximum)
    m_maximum = m_minimum;
}

IExpression* CStringExpression::Clone()
{
  CStringExpression *clone = new CStringExpression(m_name, m_characters, m_minimum, m_maximum);
  if (clone == NULL)
    return NULL;
  
  IExpression::clone(clone);
  return clone;
}

bool CStringExpression::parse(const std::string &value, size_t &processedCharacters)
{
  size_t pos = value.find_first_not_of(m_characters.c_str());
  if (pos == std::string::npos)
    pos = value.size();

  if (pos < (size_t)m_minimum)
    return false;

  if (m_maximum > 0 && pos > (size_t)m_maximum)
    pos = (size_t)m_maximum;

  m_value = value.substr(0, pos);
  processedCharacters = pos;
  return true;
}

CFixedStringExpression::CFixedStringExpression(const std::string &name, const std::string &expression, bool caseSensitive /* = true */)
  : IExpression(name),
    m_expression(expression),
    m_caseSensitive(caseSensitive)
{ }

IExpression* CFixedStringExpression::Clone()
{
  CFixedStringExpression *clone = new CFixedStringExpression(m_name, m_expression, m_caseSensitive);
  if (clone == NULL)
    return NULL;
  
  IExpression::clone(clone);
  return clone;
}

bool CFixedStringExpression::parse(const std::string &value, size_t &processedCharacters)
{
  if (value.size() < m_expression.size())
    return false;

  std::string tmp = value.substr(0, m_expression.size());
  if ((m_caseSensitive && m_expression.compare(tmp) != 0) ||
      (!m_caseSensitive && !StringUtils::EqualsNoCase(m_expression, tmp)))
    return false;

  m_value = tmp;
  processedCharacters = m_value.size();
  return true;
}

CRegexpExpression::CRegexpExpression(const std::string &name, const std::string &regexp, bool caseSensitive /* = true */)
  : IExpression(name),
    m_regexp(regexp),
    m_caseSensitive(caseSensitive)
{ }

IExpression* CRegexpExpression::Clone()
{
  CRegexpExpression *clone = new CRegexpExpression(m_name, m_regexp, m_caseSensitive);
  if (clone == NULL)
    return NULL;
  
  IExpression::clone(clone);
  return clone;
}

bool CRegexpExpression::parse(const std::string &value, size_t &processedCharacters)
{
  if (m_regexp.empty())
    return true;

  CRegExp regExp(!m_caseSensitive, CRegExp::autoUtf8);
  if (!regExp.RegComp(m_regexp))
    return false;

  if (regExp.RegFind(value) != 0)
    return false;

  processedCharacters = (size_t)regExp.GetFindLen();
  m_value = regExp.GetMatch();
  return true;
}

CQuantificationExpression::CQuantificationExpression(const std::string &name, const IExpressionPtr &expression, int minimum /* = 1 */, int maximum /* = 1 */)
  : IExpression(name),
    m_expression(expression),
    m_minimum(minimum),
    m_maximum(maximum)
{
  if (m_minimum < 0)
    m_minimum = 0;
  if (m_maximum == 0)
    m_maximum = 1;

  if (m_minimum > m_maximum)
    m_maximum = m_minimum;
}

IExpression* CQuantificationExpression::Clone()
{
  CQuantificationExpression *clone = new CQuantificationExpression(m_name, m_expression, m_minimum, m_maximum);
  if (clone == NULL)
    return NULL;
  
  IExpression::clone(clone);
  return clone;
}

bool CQuantificationExpression::parse(const std::string &value, size_t &processedCharacters)
{
  m_result.reset();

  if (m_expression == NULL)
    return false;

  std::string processedValue;
  int count = 0;
  IExpressionPtr clonedExp(m_expression->Clone());
  while (count < m_maximum || m_maximum < 0)
  {
    size_t processed;
    bool result = m_expression->BeginParse() && clonedExp->Parse(value.substr(processedCharacters), processed);
    m_expression->EndParse();
    if (!result)
    {
      if (count < m_minimum ||
         (count > m_maximum && m_maximum > 0))
        return false;

      break;
    }

    processedCharacters += processed;
    processedValue += m_expression->GetValue();
    count += 1;
  }

  processedValue = value;
  m_result = clonedExp;
  return true;
}

CSequenceExpression::CSequenceExpression(const std::string &name, const std::vector<IExpressionPtr> &expressions /* = std::vector<IExpressionPtr>() */)
  : IExpression(name),
    m_expressions(expressions)
{ }

void CSequenceExpression::Add(const IExpressionPtr &expression)
{
  if (expression == NULL)
    return;

  m_expressions.push_back(expression);
}

IExpression* CSequenceExpression::Clone()
{
  CSequenceExpression *clone = new CSequenceExpression(m_name, m_expressions);
  if (clone == NULL)
    return NULL;
  
  IExpression::clone(clone);
  return clone;
}

bool CSequenceExpression::parse(const std::string &value, size_t &processedCharacters)
{
  m_result.clear();

  std::string processedValue;
  for (std::vector<IExpressionPtr>::iterator itExpression = m_expressions.begin();
       itExpression != m_expressions.end();
       ++itExpression)
  {
    if (*itExpression == NULL)
      continue;
    
    size_t processed;
    IExpressionPtr clonedExp((*itExpression)->Clone());
    bool result = (*itExpression)->BeginParse() && clonedExp->Parse(value.substr(processedCharacters), processed);
    (*itExpression)->EndParse();
    if (!result)
    {
      m_result.clear();
      return false;
    }

    m_result.push_back(clonedExp);
    processedCharacters += processed;
    processedValue += clonedExp->GetValue();
  }

  m_value = processedValue;
  return true;
}

CAlternativeExpression::CAlternativeExpression(const std::string &name, const std::vector<IExpressionPtr> &expressions /* = std::vector<IExpressionPtr>() */)
  : IExpression(name),
    m_expressions(expressions)
{ }

void CAlternativeExpression::Add(const IExpressionPtr &expression)
{
  if (expression == NULL)
    return;

  m_expressions.push_back(expression);
}

IExpression* CAlternativeExpression::Clone()
{
  CAlternativeExpression *clone = new CAlternativeExpression(m_name, m_expressions);
  if (clone == NULL)
    return NULL;
  
  IExpression::clone(clone);
  return clone;
}

bool CAlternativeExpression::parse(const std::string &value, size_t &processedCharacters)
{
  m_result.reset();

  IExpressionPtr bestMatchingExpression;
  size_t bestProcessedCharacters = 0;
  std::string bestValue;

  for (std::vector<IExpressionPtr>::iterator itExpression = m_expressions.begin();
       itExpression != m_expressions.end();
       ++itExpression)
  {
    if (*itExpression == NULL)
      continue;
    
    processedCharacters = 0;
    IExpressionPtr clonedExp((*itExpression)->Clone());
    bool result = (*itExpression)->BeginParse() && clonedExp->Parse(value.substr(processedCharacters), processedCharacters);
    (*itExpression)->EndParse();
    if (result &&
       (bestMatchingExpression == NULL || bestProcessedCharacters < processedCharacters))
    {
      bestProcessedCharacters = processedCharacters;
      bestValue = clonedExp->GetValue();
      bestMatchingExpression = clonedExp;
    }
  }

  processedCharacters = bestProcessedCharacters;
  m_value = bestValue;
  m_result = bestMatchingExpression;
  return m_result != NULL;
}

bool CExpressionParser::Define(const IExpressionPtr &expression)
{
  if (expression == NULL ||
      m_expression != NULL)
    return false;

  m_expression = expression;
  return true;
}

bool CExpressionParser::Parse(const std::string &value)
{
  m_result.reset();

  if (m_expression == NULL)
    return false;

  size_t processed;
  IExpressionPtr clonedExp(m_expression->Clone());
  bool result = m_expression->BeginParse() && clonedExp->Parse(value, processed);
  m_expression->EndParse();

  if (!result || processed != value.size())
    return false;

  m_result = clonedExp;
  return true;
}
