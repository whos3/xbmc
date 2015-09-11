/*
 *      Copyright (C) 2015 Team XBMC
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

#include "ContainerInfoTag.h"

CContainerInfoTag::CContainerInfoTag(const CContainerInfoTag& containerInfoTag)
  : CMediaInfoTag(containerInfoTag)
{ }

CContainerInfoTag::CContainerInfoTag(CContainerInfoTag&& containerInfoTag)
  : CMediaInfoTag(std::move(containerInfoTag))
{ }

/* TODO
void CContainerInfoTag::Serialize(CVariant& value) const
{
  // TODO

  CMediaInfoTag::Serialize(value);
}

void CContainerInfoTag::ToSortable(SortItem& sortable, Field field) const
{
  switch (field)
  {
  // TODO

  default:
    break;
  }

  CMediaInfoTag::ToSortable(sortable, field);
}
*/
