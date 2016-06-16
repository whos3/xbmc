#pragma once
/*
 *      Copyright (C) 2016 Team XBMC
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
#include <vector>

class CPropertyTreePath
{
public:
  static const std::string Separator;
  static const std::string AttributePrefix;

public:
  explicit CPropertyTreePath(const std::string& path);
  ~CPropertyTreePath() = default;

  operator std::string() const { return ToString(); }

  inline bool IsEmpty() const { return m_elements.empty(); }

  bool IsElement() const;
  inline bool IsAttribute() const { return !IsElement(); }

  std::string GetCurrentElement() const;

  CPropertyTreePath Next() const;

  std::string ToString() const;

private:
  struct PathElement
  {
    std::string name;
    bool isAttribute;
  };
  using PathElements = std::vector<PathElement>;

  explicit CPropertyTreePath(const PathElements& elements);

  PathElements m_elements;
};
