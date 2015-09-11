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

#include "media/info/FileInfoTag.h"

class CVideoFileInfoTag : public CFileInfoTag
{
public:
  virtual ~CVideoFileInfoTag() = default;

  // specializations of CFileInfoTag
  virtual void Serialize(CVariant& value) const = 0;
  virtual void ToSortable(SortItem& sortable, Field field) const = 0;

  // TODO

protected:
  CVideoFileInfoTag(const MediaType& mediaType)
    : CFileInfoTag(mediaType)
  { }
  CVideoFileInfoTag(const CVideoFileInfoTag&);
  CVideoFileInfoTag(CVideoFileInfoTag&&);

private:
  // TODO
};
