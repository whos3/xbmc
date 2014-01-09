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

#include "gtest/gtest.h"

#include "network/upnp/UPnPSearchCriteria.h"
#include "utils/ExpressionParser.h"

TEST(TestExpressionParser, CharacterExpression)
{
  size_t processed;
  CCharacterExpression expSingle("test", 'a');
  CCharacterExpression expOptional("test", 'a', 0, 1);
  CCharacterExpression expRange("test", 'a', 1, 2);
  CCharacterExpression expUnlimited("test", 'a', 0, -1);

  // test of single character
  EXPECT_TRUE(expSingle.Parse("a", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expSingle.Parse("abc", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_FALSE(expSingle.Parse("b", processed));

  // test of single optional character
  EXPECT_TRUE(expOptional.Parse("a", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expOptional.Parse("", processed));
  EXPECT_EQ(0, processed);
  
  EXPECT_TRUE(expOptional.Parse("abc", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expOptional.Parse("b", processed));
  EXPECT_EQ(0, processed);

  // test of 1 or 2 identical characters
  EXPECT_TRUE(expRange.Parse("a", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expRange.Parse("aa", processed));
  EXPECT_EQ(2, processed);
  
  EXPECT_TRUE(expRange.Parse("aaa", processed));
  EXPECT_EQ(2, processed);
  
  EXPECT_TRUE(expRange.Parse("abc", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_FALSE(expRange.Parse("", processed));
  EXPECT_FALSE(expRange.Parse("b", processed));
  EXPECT_FALSE(expRange.Parse("ba", processed));

  // test of unlimited optional characters
  EXPECT_TRUE(expUnlimited.Parse("a", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("", processed));
  EXPECT_EQ(0, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("abc", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("aaaaaaaaaabc", processed));
  EXPECT_EQ(10, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("b", processed));
  EXPECT_EQ(0, processed);
}

TEST(TestExpressionParser, StringExpression)
{
  size_t processed;
  CStringExpression expSingle("test", "abc");
  CStringExpression expOptional("test", "abc", 0, 1);
  CStringExpression expRange("test", "abc", 1, 4);
  CStringExpression expUnlimited("test", "abc", 0, -1);

  // test of single character
  EXPECT_TRUE(expSingle.Parse("a", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expSingle.Parse("cab", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_FALSE(expSingle.Parse("d", processed));

  // test of single optional character
  EXPECT_TRUE(expOptional.Parse("a", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expOptional.Parse("", processed));
  EXPECT_EQ(0, processed);
  
  EXPECT_TRUE(expOptional.Parse("cab", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_FALSE(expSingle.Parse("d", processed));

  // test of 1 to 4 characters
  EXPECT_TRUE(expRange.Parse("b", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expRange.Parse("ba", processed));
  EXPECT_EQ(2, processed);
  
  EXPECT_TRUE(expRange.Parse("cbab", processed));
  EXPECT_EQ(4, processed);
  
  EXPECT_TRUE(expRange.Parse("bbbbb", processed));
  EXPECT_EQ(4, processed);
  
  EXPECT_TRUE(expRange.Parse("cabd", processed));
  EXPECT_EQ(3, processed);
  
  EXPECT_FALSE(expRange.Parse("", processed));
  EXPECT_FALSE(expRange.Parse("d", processed));
  EXPECT_FALSE(expRange.Parse("da", processed));

  // test of unlimited optional characters
  EXPECT_TRUE(expUnlimited.Parse("a", processed));
  EXPECT_EQ(1, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("", processed));
  EXPECT_EQ(0, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("cab", processed));
  EXPECT_EQ(3, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("abcabcabcabcd", processed));
  EXPECT_EQ(12, processed);
  
  EXPECT_TRUE(expUnlimited.Parse("d", processed));
  EXPECT_EQ(0, processed);
}

TEST(TestExpressionParser, FixedStringExpression)
{
  size_t processed;
  CFixedStringExpression exp("test", "hello");
  CFixedStringExpression expInsensitive("test", "hello", false);

  // tests with case sensitive
  EXPECT_TRUE(exp.Parse("hello", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_TRUE(exp.Parse("hello world", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_FALSE(exp.Parse("hell", processed));
  EXPECT_FALSE(exp.Parse("world hello", processed));

  // tests with case insensitive
  EXPECT_TRUE(expInsensitive.Parse("HeLlO", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_TRUE(expInsensitive.Parse("HeLlO wOrLd", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_FALSE(expInsensitive.Parse("hell", processed));
  EXPECT_FALSE(expInsensitive.Parse("wOrLd HeLlO", processed));
}

TEST(TestExpressionParser, SequenceExpression)
{
  size_t processed;
  CSequenceExpression expSingle("test");
  expSingle.Add(IExpressionPtr(new CFixedStringExpression("hello", "hello")));

  CSequenceExpression expMulti("test");
  expMulti.Add(IExpressionPtr(new CFixedStringExpression("hello", "hello")));
  expMulti.Add(IExpressionPtr(new CStringExpression("world", " world", 1, 6)));

  // tests with single sequence
  EXPECT_TRUE(expSingle.Parse("hello", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_TRUE(expSingle.Parse("hello world", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_FALSE(expSingle.Parse("hell", processed));
  EXPECT_FALSE(expSingle.Parse("world hello", processed));

  // tests with multi sequence
  EXPECT_TRUE(expMulti.Parse("hello ", processed));
  EXPECT_EQ(6, processed);
  
  EXPECT_TRUE(expMulti.Parse("hello world", processed));
  EXPECT_EQ(11, processed);
  
  EXPECT_TRUE(expMulti.Parse("hello woldo", processed));
  EXPECT_EQ(11, processed);

  EXPECT_TRUE(expMulti.Parse("hello worldX", processed));
  EXPECT_EQ(11, processed);
  
  EXPECT_FALSE(expMulti.Parse("hell", processed));
  EXPECT_FALSE(expMulti.Parse("world hello", processed));
  EXPECT_FALSE(expMulti.Parse("helloX", processed));
}

TEST(TestExpressionParser, AlternativeExpression)
{
  size_t processed;
  CAlternativeExpression expSingle("test");
  expSingle.Add(IExpressionPtr(new CFixedStringExpression("hello", "hello")));

  CAlternativeExpression expMulti("test");
  expMulti.Add(IExpressionPtr(new CFixedStringExpression("hello", "hello")));
  expMulti.Add(IExpressionPtr(new CStringExpression("world", " world", 1, 6)));

  // tests with single sequence
  EXPECT_TRUE(expSingle.Parse("hello", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_TRUE(expSingle.Parse("hello world", processed));
  EXPECT_EQ(5, processed);
  
  EXPECT_FALSE(expSingle.Parse("hell", processed));
  EXPECT_FALSE(expSingle.Parse("world hello", processed));

  // tests with multi sequence
  EXPECT_TRUE(expMulti.Parse("hello", processed));
  EXPECT_EQ(5, processed);
  EXPECT_STREQ("hello", expMulti.GetValue().c_str());
  
  EXPECT_TRUE(expMulti.Parse("world", processed));
  EXPECT_EQ(5, processed);
  EXPECT_STREQ("world", expMulti.GetValue().c_str());
  
  EXPECT_TRUE(expMulti.Parse("woldo", processed));
  EXPECT_EQ(5, processed);
  EXPECT_STREQ("woldo", expMulti.GetValue().c_str());
  
  EXPECT_FALSE(expMulti.Parse("hell", processed));
}

TEST(TestExpressionParser, ExpressionParser)
{
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
  EXPECT_TRUE(expParser.Define(expASearchCriteria));
  
  EXPECT_TRUE(expParser.Parse("upnp:class = \"object.item.videoItem.movie\""));
  EXPECT_TRUE(expParser.Parse("upnp:class = \"object.item.videoItem.movie\" and @id = \"videodb://movies/titles/1\""));
  EXPECT_TRUE(expParser.Parse("upnp:class = \"object.item.videoItem.movie\" and (@id = \"videodb://movies/titles/1\" or @id = \"videodb://movies/titles/2\")"));
  EXPECT_FALSE(expParser.Parse("\"object.item.videoItem.movie\""));
  EXPECT_FALSE(expParser.Parse("upnp::1234 != \"object.item.videoItem.movie\""));
  EXPECT_FALSE(expParser.Parse(" = \"object.item.videoItem.movie\""));
  EXPECT_FALSE(expParser.Parse("upnp:class = "));
  EXPECT_FALSE(expParser.Parse("upnp:class \"object.item.videoItem.movie\""));
  EXPECT_FALSE(expParser.Parse("upnp:class = \"object.item.videoItem.movie"));
}

TEST(TestExpressionParser, UPnPSearchCriteria)
{
  CUPnPSearchCriteria criteriaAll;
  EXPECT_TRUE(criteriaAll.Deserialize("*"));

  CUPnPSearchCriteria criteriaSingle;
  EXPECT_TRUE(criteriaSingle.Deserialize("upnp:class = \"object.item.videoItem.movie\""));

  CUPnPSearchCriteria criteriaAnd;
  EXPECT_TRUE(criteriaAnd.Deserialize("upnp:class = \"object.item.videoItem.movie\" and @id = \"videodb://movies/titles/1\""));

  CUPnPSearchCriteria criteriaOr;
  EXPECT_TRUE(criteriaOr.Deserialize("upnp:class = \"object.item.videoItem.movie\" or upnp:class = \"object.item.videoItem.musicVideoClip\""));

  CUPnPSearchCriteria criteriaMixed;
  EXPECT_TRUE(criteriaMixed.Deserialize("upnp:class = \"object.item.videoItem.movie\" and (@id = \"videodb://movies/titles/1\" or @id = \"videodb://movies/titles/2\")"));

  CUPnPSearchCriteria criteriaComplex;
  EXPECT_TRUE(criteriaComplex.Deserialize("(upnp:class = \"object.item.videoItem.movie\" and @id = \"videodb://movies/titles/1\") or (upnp:class = \"object.item.videoItem.musicVideoClip\" and @id = \"videodb://musicvideos/titles/1\") or (upnp:class = \"object.item.videoItem.videoBroadcast\" and @id = \"videodb://episodes/titles/1\")"));
}
