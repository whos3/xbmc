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

#include <memory>
#include <string>
#include <vector>

class IDidlLiteElement;
class TiXmlNode;

class CDidlLiteElementFactory
{
public:
  CDidlLiteElementFactory();
  virtual ~CDidlLiteElementFactory();

  IDidlLiteElement* GetElement(const TiXmlNode* xmlNode) const;

  virtual bool RegisterElement(const IDidlLiteElement* element);

protected:
  using IDidlLiteElementPtr = std::shared_ptr<const IDidlLiteElement>;
  using IDidlLiteElementPair = std::pair<std::string, IDidlLiteElementPtr>;
  using IDidlLiteElementPairs = std::vector<IDidlLiteElementPair>;

  static IDidlLiteElementPairs::const_iterator FindElementByName(const IDidlLiteElementPairs& elements, const std::string& elementName);

  IDidlLiteElementPairs m_elements;
};
