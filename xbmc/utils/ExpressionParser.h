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

#include <set>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

class IExpression
{
public:
  IExpression(const std::string &name);
  virtual ~IExpression() { }

  /*! \brief Clone the expression
   *
   * Cloning is required to be able to access the result of the parsing process
   * for expressions which are used recursively.
   *
   * \return Cloned expression instance
   */
  virtual IExpression* Clone() = 0;

  /*! \brief Parse the given value
   *
   * \param value Value to be parsed
   * \param processedCharacters Amount of processed characters in the given value
   * \return True if parsing was successful, false otherwise
   */
  bool Parse(const std::string &value, size_t &processedCharacters);

  /*! \brief Begin parsing
   *
   * This call is required for recursive call guards in combination with cloned
   * expression instances. It is only required for un-cloned expressions.
   *
   * \return True if parsing can be done, false otherwise
   */
  bool BeginParse();
  /*! \brief End parsing
   *
   * This call is required for recursive call guards in combination with cloned
   * expression instances. It is only required for un-cloned expressions.
   */
  void EndParse();

  const std::string& GetName() const { return m_name; }
  const std::string& GetValue() const { return m_value; }

protected:
  virtual bool parse(const std::string &value, size_t &processedCharacters) = 0;

  void clone(IExpression *clonedExpression);

  std::string m_name;
  std::string m_value;
  size_t m_recursiveCalls;
};

typedef boost::shared_ptr<IExpression> IExpressionPtr;

class CCharacterExpression : public IExpression
{
public:
  CCharacterExpression(const std::string &name, char character, int minimum = 1, int maximum = 1);
  virtual ~CCharacterExpression() { }

  // implementation of IExpression
  virtual IExpression* Clone();

protected:
  // implementation of IExpression
  virtual bool parse(const std::string &value, size_t &processedCharacters);

  char m_character;
  int m_minimum;
  int m_maximum;
};

class CStringExpression : public IExpression
{
public:
  CStringExpression(const std::string &name, const std::string &characters, int minimum = 1, int maximum = 1);
  virtual ~CStringExpression() { }

  // implementation of IExpression
  virtual IExpression* Clone();

protected:
  // implementation of IExpression
  virtual bool parse(const std::string &value, size_t &processedCharacters);

  std::string m_characters;
  int m_minimum;
  int m_maximum;
};

class CFixedStringExpression : public IExpression
{
public:
  CFixedStringExpression(const std::string &name, const std::string &expression, bool caseSensitive = true);
  virtual ~CFixedStringExpression() { }

  // implementation of IExpression
  virtual IExpression* Clone();

protected:
  // implementation of IExpression
  virtual bool parse(const std::string &value, size_t &processedCharacters);

  std::string m_expression;
  bool m_caseSensitive;
};

class CRegexpExpression : public IExpression
{
public:
  CRegexpExpression(const std::string &name, const std::string &regexp, bool caseSensitive = true);
  virtual ~CRegexpExpression() { }

  // implementation of IExpression
  virtual IExpression* Clone();

protected:
  // implementation of IExpression
  virtual bool parse(const std::string &value, size_t &processedCharacters);

  std::string m_regexp;
  bool m_caseSensitive;
};

class CQuantificationExpression : public IExpression
{
public:
  CQuantificationExpression(const std::string &name, const IExpressionPtr &expression, int minimum = 1, int maximum = 1);
  virtual ~CQuantificationExpression() { }

  const IExpressionPtr& GetResult() { return m_result; }

  // implementation of IExpression
  virtual IExpression* Clone();

protected:
  // implementation of IExpression
  virtual bool parse(const std::string &value, size_t &processedCharacters);

  IExpressionPtr m_expression;
  int m_minimum;
  int m_maximum;
  IExpressionPtr m_result;
};

class CSequenceExpression : public IExpression
{
public:
  CSequenceExpression(const std::string &name, const std::vector<IExpressionPtr> &expressions = std::vector<IExpressionPtr>());
  virtual ~CSequenceExpression() { }

  void Add(const IExpressionPtr &expression);

  const std::vector<IExpressionPtr>& GetExpressions() const { return m_expressions; }

  const  std::vector<IExpressionPtr>& GetResult() { return m_result; }

  // implementation of IExpression
  virtual IExpression* Clone();

protected:
  // implementation of IExpression
  virtual bool parse(const std::string &value, size_t &processedCharacters);

  std::vector<IExpressionPtr> m_expressions;
  std::vector<IExpressionPtr> m_result;
};

class CAlternativeExpression : public IExpression
{
public:
  CAlternativeExpression(const std::string &name, const std::vector<IExpressionPtr> &expressions = std::vector<IExpressionPtr>());
  virtual ~CAlternativeExpression() { }

  void Add(const IExpressionPtr &expression);

  const IExpressionPtr& GetResult() const { return m_result; }

  // implementation of IExpression
  virtual IExpression* Clone();

protected:
  // implementation of IExpression
  virtual bool parse(const std::string &value, size_t &processedCharacters);

  std::vector<IExpressionPtr> m_expressions;
  IExpressionPtr m_result;
};

class CExpressionParser
{
public:
  CExpressionParser() { }
  ~CExpressionParser() { }

  bool Define(const IExpressionPtr &expression);
  bool Parse(const std::string &value);

  const IExpressionPtr& GetResult() const { return m_result; }

private:
  IExpressionPtr m_expression;
  IExpressionPtr m_result;
};
