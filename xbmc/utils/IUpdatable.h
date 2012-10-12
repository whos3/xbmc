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

#include "IPropertyHandler.h"

class IUpdatable : virtual protected IPropertyHandler
{
public:
  virtual ~IUpdatable() { }
  
  virtual bool Update(const std::string &propertyName, const CVariant &propertyValue);
  virtual bool Update(const std::map<std::string, CVariant> &properties);
  virtual bool Update(Field propertyField, const CVariant &propertyValue);
  virtual bool Update(const std::map<Field, CVariant> &properties);
};
