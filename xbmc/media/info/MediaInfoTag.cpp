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

#include "MediaInfoTag.h"
#include "utils/Variant.h"

CMediaInfoTag::CMediaInfoTag(const CMediaInfoTag& mediaInfoTag)
  : m_dbId(mediaInfoTag.m_dbId)
  , m_title(mediaInfoTag.m_title)
{ }

CMediaInfoTag::CMediaInfoTag(CMediaInfoTag&& mediaInfoTag)
  : m_dbId(std::move(mediaInfoTag.m_dbId))
  , m_title(std::move(mediaInfoTag.m_title))
{ }

void CMediaInfoTag::Serialize(CVariant& value) const
{
  value["title"] = m_title;
  value["dbid"] = m_dbId;
}

void CMediaInfoTag::ToSortable(SortItem& sortable, Field field) const
{
  switch (field)
  {
    case FieldTitle:
    {
      // make sure not to overwrite an existing title with an empty one
      std::string title = m_title;
      if (!title.empty() || sortable.find(FieldTitle) == sortable.end())
        sortable[FieldTitle] = title;
      break;
    }

    case FieldId:
      sortable[FieldId] = m_dbId;
      break;

    default:
      break;
  }
}
