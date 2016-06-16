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

class CPropertyTreeElementProperty;
class CXmlPropertyTree;
class IPropertyTreeElement;
class TiXmlNode;

class CXmlPropertyTreeSerializer
{
public:
  CXmlPropertyTreeSerializer() = default;
  ~CXmlPropertyTreeSerializer() = default;

  bool Serialize(const CXmlPropertyTree& tree, std::string& result) const;

protected:
  static bool Serialize(const IPropertyTreeElement* element, TiXmlNode* node);
  static bool Serialize(const CPropertyTreeElementProperty& prop, TiXmlNode* node);
  static bool SerializeAttribute(const CPropertyTreeElementProperty& prop, TiXmlNode* node);
};
