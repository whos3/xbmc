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

class CXmlPropertyTreeDeserializer
{
public:
  CXmlPropertyTreeDeserializer() = default;
  ~CXmlPropertyTreeDeserializer() = default;

  bool RegisterElement(const IPropertyTreeElement* element);

  bool Deserialize(const std::string& data, CXmlPropertyTree& tree) const;

protected:
  using IPropertyTreeElementPtr = std::shared_ptr<const IPropertyTreeElement>;
  using IPropertyTreeElementPair = std::pair<std::string, IPropertyTreeElementPtr>;
  using IPropertyTreeElementPairs = std::vector<IPropertyTreeElementPair>;

  static bool DeserializeElement(const TiXmlNode* node, IPropertyTreeElement* element);
  static bool DeserializeProperty(const TiXmlNode* node, CPropertyTreeElementProperty& prop);
  static bool DeserializeAttribute(const TiXmlNode* node, CPropertyTreeElementProperty& prop);

  static IPropertyTreeElementPairs::const_iterator FindElementByName(const IPropertyTreeElementPairs& elements, const std::string& elementName);

  IPropertyTreeElementPairs m_elements;
};
