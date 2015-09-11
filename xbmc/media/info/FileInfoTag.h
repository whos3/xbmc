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

#include <cstdint>
#include <string>

#include "XBDateTime.h"
#include "media/MediaType.h"
#include "media/info/MediaInfoTag.h"

class CFileInfoTag : public CMediaInfoTag
{
public:
  virtual ~CFileInfoTag() = default;

  // specializations of CMediaInfoTag
  virtual void Serialize(CVariant& value) const = 0;
  virtual void ToSortable(SortItem& sortable, Field field) const = 0;

  virtual inline const std::string& GetPath() const { return m_path; }
  virtual inline void SetPath(const std::string& path) { m_path = path; }

  virtual inline const CDateTime& GetDateAdded() const { return m_dateAdded; }
  virtual inline void SetDateAdded(const CDateTime& dateAdded) { m_dateAdded = dateAdded; }

  virtual inline std::uint32_t GetPlaycount() const { return m_playcount; }
  virtual inline void SetPlaycount(std::uint32_t playcount) { m_playcount = playcount; }

  virtual inline const CDateTime& GetLastPlayed() const { return m_lastPlayed; }
  virtual inline void SetLastPlayed(const CDateTime& lastPlayed) { m_lastPlayed = lastPlayed; }

protected:
  CFileInfoTag(const MediaType& mediaType)
    : CMediaInfoTag(),
      m_playcount(0)
  {
    // don't allow containers
    assert(!MediaTypes::IsContainer(mediaType));
  }
  CFileInfoTag(const CFileInfoTag&);
  CFileInfoTag(CFileInfoTag&&);

private:
  std::string m_path;
  CDateTime m_dateAdded;
  std::uint32_t m_playcount;
  CDateTime m_lastPlayed;
};
