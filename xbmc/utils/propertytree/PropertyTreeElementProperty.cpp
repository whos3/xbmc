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

#include "PropertyTreeElementProperty.h"
#include "utils/propertytree/IPropertyTreeElement.h"
#include "utils/propertytree/PropertyTreePath.h"
#include "utils/StringUtils.h"

CPropertyTreeElementProperty::CPropertyTreeElementProperty(Type type, void* value)
  : CPropertyTreeElementProperty("", type, value)
{ }

CPropertyTreeElementProperty::CPropertyTreeElementProperty(const std::string& name, Type type, void* value)
  : m_value(value),
    m_type(type),
    m_isElement(true),
    m_required(false),
    m_multipleValues(false),
    m_valid(false),
    m_valueGenerator(nullptr),
    m_valueGenerated(false)
{
  assert(m_value != nullptr);

  CPropertyTreePath path(name);
  m_isElement = path.IsElement();
  m_name = path.GetCurrentElement();
}

CPropertyTreeElementProperty::~CPropertyTreeElementProperty()
{
  m_value = nullptr;
}

IPropertyTreeElement* const CPropertyTreeElementProperty::GenerateValue()
{
  if (!HasGenerator())
    return nullptr;

  return m_valueGenerator->Clone();
}
