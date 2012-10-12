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

#include "IUpdatable.h"
#include "Variant.h"
#include "XBDateTime.h"

bool IUpdatable::Update(const std::string &propertyName, const CVariant &propertyValue)
{
  return SetHandledPropertyValue(propertyName, propertyValue);
}

bool IUpdatable::Update(const std::map<std::string, CVariant> &properties)
{
  bool ret = true;
  for (std::map<std::string, CVariant>::const_iterator it = properties.begin(); it != properties.end(); it++)
    ret &= Update(it->first, it->second);

  return ret;
}

bool IUpdatable::Update(Field propertyField, const CVariant &propertyValue)
{
  return SetHandledPropertyValue(propertyField, propertyValue);
}

bool IUpdatable::Update(const std::map<Field, CVariant> &properties)
{
  bool ret = true;
  for (std::map<Field, CVariant>::const_iterator it = properties.begin(); it != properties.end(); it++)
    ret &= Update(it->first, it->second);

  return ret;
}
