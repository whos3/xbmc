#pragma once
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

#include <string>

#include "media/MediaType.h"
#include "media/info/MediaInfoTag.h"

class CContainerInfoTag : public CMediaInfoTag
{
public:
  virtual ~CContainerInfoTag() = default;

  /* TODO
  // specializations of CMediaInfoTag
  virtual void Serialize(CVariant& value) const = 0;
  virtual void ToSortable(SortItem& sortable, Field field) const = 0;
  */

  // TODO

protected:
  CContainerInfoTag(const MediaType& mediaType)
    : CMediaInfoTag()
  {
    // only allow containers
    assert(MediaTypes::IsContainer(mediaType));
  }
  CContainerInfoTag(const CContainerInfoTag&);
  CContainerInfoTag(CContainerInfoTag&&);

private:
  // TODO
};
