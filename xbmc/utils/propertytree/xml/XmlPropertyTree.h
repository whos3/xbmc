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

#include "utils/propertytree/PropertyTree.h"

class CXmlPropertyTree : public CPropertyTree
{
public:
  CXmlPropertyTree(const std::string& name);
  virtual ~CXmlPropertyTree() = default;

  // implementations of IPropertyTreeElement
  virtual IPropertyTreeElement* Create() const override { return nullptr; }
  virtual IPropertyTreeElement* Clone() const override { return nullptr; }

  void AddNamespace(const std::string& uri);
  void AddNamespace(const std::string& name, const std::string& uri);

  const std::map<std::string, std::string>& GetNamespaces() const { return m_namespaces; }
  bool HasNamespace(const std::string& name) const;
  std::string GetNamespaceUri(const std::string& name) const;

protected:
  std::map<std::string, std::string> m_namespaces;
};
