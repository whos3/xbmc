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

#include <set>
#include <string>
#include <vector>

#include "FilterRule.h"
#include "FilterRuleCombination.h"
#include "utils/DatabaseUtils.h"

class CDatabase;
class CFileItemList;

class CFilter
{
public:
  CFilter();
  virtual ~CFilter() { }
  
  virtual void Reset();
  virtual bool IsEmpty() const { return m_ruleCombination.GetRules().empty() && m_ruleCombination.GetCombinations().empty(); }
  
  virtual bool Load(const CVariant &obj);
  virtual bool Save(CVariant &obj) const;
  
  MediaType GetType() const { return m_type; }
  virtual void SetType(MediaType type) { m_type = type; }

  CFilterRuleCombination& GetRuleCombination() { return m_ruleCombination; }
  const CFilterRuleCombination& GetRuleCombination() const { return m_ruleCombination; }

  void SetMatchAllRules(bool matchAll) { m_ruleCombination.SetType(matchAll ? CFilterRuleCombination::CombinationAnd : CFilterRuleCombination::CombinationOr); }
  bool GetMatchAllRules() const { return m_ruleCombination.GetType() == CFilterRuleCombination::CombinationAnd; }

  virtual bool Filter(const CDatabase &db, std::string &query) const;
  virtual bool Filter(CFileItemList& items) const;

  static void GetAvailableFields(std::vector<std::string> &fieldList);
  static const std::string& TranslateField(Field field);
  static Field TranslateField(const std::string &field);
  static std::string GetLocalizedField(Field field);
  
  static void GetAvailableOperators(std::vector<std::string> &operatorList);
  static const std::string& TranslateOperator(const CFilterOperator &op);
  static const CFilterOperator& TranslateOperator(const std::string &op);
  static std::string GetLocalizedOperator(const CFilterOperator &op);

protected:
  virtual bool filter(const CDatabase &db, std::set<std::string> &referencedPlaylists, std::string &query) const;
  virtual bool filter(std::set<std::string> &referencedPlaylists, CFileItemList& items) const;
  
  CFilterRuleCombination m_ruleCombination;
  MediaType m_type;
};
