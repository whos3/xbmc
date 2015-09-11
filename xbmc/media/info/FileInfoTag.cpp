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

#include "FileInfoTag.h"
#include "utils/Variant.h"

CFileInfoTag::CFileInfoTag(const CFileInfoTag& fileInfoTag)
  : CMediaInfoTag(fileInfoTag)
  , m_path(fileInfoTag.m_path)
  , m_dateAdded(fileInfoTag.m_dateAdded)
  , m_playcount(fileInfoTag.m_playcount)
  , m_lastPlayed(fileInfoTag.m_lastPlayed)
{ }

CFileInfoTag::CFileInfoTag(CFileInfoTag&& fileInfoTag)
  : CMediaInfoTag(std::move(fileInfoTag))
  , m_path(std::move(fileInfoTag.m_path))
  , m_dateAdded(std::move(fileInfoTag.m_dateAdded))
  , m_playcount(std::move(fileInfoTag.m_playcount))
  , m_lastPlayed(std::move(fileInfoTag.m_lastPlayed))
{ }

void CFileInfoTag::Serialize(CVariant& value) const
{
  value["path"] = m_path;
  value["dateadded"] = m_dateAdded.IsValid() ? m_dateAdded.GetAsDBDateTime() : "";
  value["playcount"] = m_playcount;
  value["lastplayed"] = m_lastPlayed.IsValid() ? m_lastPlayed.GetAsDBDateTime() : "";

  CMediaInfoTag::Serialize(value);
}

void CFileInfoTag::ToSortable(SortItem& sortable, Field field) const
{
  switch (field)
  {
  case FieldDateAdded:
    sortable[FieldDateAdded] = m_dateAdded.IsValid() ? m_dateAdded.GetAsDBDateTime() : "";
    break;

  case FieldPlaycount:
    sortable[FieldPlaycount] = m_playcount;
    break;

  case FieldLastPlayed:
    sortable[FieldLastPlayed] = m_lastPlayed.IsValid() ? m_lastPlayed.GetAsDBDateTime() : "";
    break;

  default:
    break;
  }

  CMediaInfoTag::ToSortable(sortable, field);
}
