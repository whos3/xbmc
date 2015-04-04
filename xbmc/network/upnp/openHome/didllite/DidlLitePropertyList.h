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
#include <set>
#include <string>
#include <vector>

class CDidlLitePropertyList
{
public:
  CDidlLitePropertyList();
  explicit CDidlLitePropertyList(const std::string &properties);
  explicit CDidlLitePropertyList(const std::vector<std::string> &properties);
  explicit CDidlLitePropertyList(const std::set<std::string> &properties);
  ~CDidlLitePropertyList() = default;

  const std::set<std::string>& Get() const { return m_properties; }
  std::string ToString() const;

  bool Contains(const std::string& prop) const;
  bool Contains(const std::vector<std::string>& properties) const;
  bool Contains(const std::set<std::string>& properties) const;

private:
  void AddProperty(const std::string& prop);

  std::set<std::string> m_properties;
};
