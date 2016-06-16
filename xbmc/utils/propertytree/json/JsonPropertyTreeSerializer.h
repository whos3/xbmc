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

#include <map>
#include <string>

#include "IPropertyTreeElement.h"

class CPropertyTree : IPropertyTreeElement
{
public:
  virtual ~CPropertyTree();

  // implementations of IPropertyTreeElement
  virtual IPropertyTreeElement* Create() const override { return nullptr; }
  virtual IPropertyTreeElement* Clone() const override { return nullptr; }
  virtual std::string GetIdentifier() const { return "PropertyTree"; }
  virtual std::set<std::string> Extends() const { return { }; }

  void AddElement(IPropertyTreeElement* element);

  std::vector<const IPropertyTreeElement*> GetElements() const;

protected:
  CPropertyTree(const std::string& name);

  void clear();

  std::vector<IPropertyTreeElement*> m_elements;
};
