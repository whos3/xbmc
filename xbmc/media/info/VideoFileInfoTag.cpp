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

#include "VideoFileInfoTag.h"
#include "utils/Variant.h"

CVideoFileInfoTag::CVideoFileInfoTag(const CVideoFileInfoTag& videoFileInfoTag)
  : CFileInfoTag(videoFileInfoTag)
{ }

CVideoFileInfoTag::CVideoFileInfoTag(CVideoFileInfoTag&& videoFileInfoTag)
  : CFileInfoTag(std::move(videoFileInfoTag))
{ }

void CVideoFileInfoTag::Serialize(CVariant& value) const
{
  // TODO

  CFileInfoTag::Serialize(value);
}

void CVideoFileInfoTag::ToSortable(SortItem& sortable, Field field) const
{
  switch (field)
  {
  // TODO

  default:
    break;
  }

  CFileInfoTag::ToSortable(sortable, field);
}
