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

#include "PropertyTree.h"

CPropertyTree::CPropertyTree(const std::string& name)
  : IPropertyTreeElement(name)
{ }

CPropertyTree::~CPropertyTree()
{
  clear();
}

void CPropertyTree::AddElement(IPropertyTreeElement* element)
{
  if (element == nullptr)
    return;

  m_elements.push_back(element);
}

std::vector<const IPropertyTreeElement*> CPropertyTree::GetElements() const
{
  std::vector<const IPropertyTreeElement*> elements;
  for (const auto& element : m_elements)
    elements.push_back(element);

  return elements;
}

void CPropertyTree::clear()
{
  for (const auto& element : m_elements)
    delete element;

  m_elements.clear();
}
