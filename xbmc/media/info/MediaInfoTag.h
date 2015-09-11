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

#include <memory>
#include <string>

#include "utils/ISerializable.h"
#include "utils/ISortable.h"

class CMediaInfoTag : public ISerializable, public ISortable
{
public:
  virtual ~CMediaInfoTag() = default;

  // implementation of ISerializable
  virtual void Serialize(CVariant& value) const = 0;

  // implementation of ISortable
  virtual void ToSortable(SortItem& sortable, Field field) const = 0;

  virtual inline int GetDatabaseId() const { return m_dbId; }
  virtual inline void SetDatabaseId(int dbId) { m_dbId = dbId; }

  virtual inline const std::string& GetTitle() const { return m_title; }
  virtual inline void SetTitle(const std::string& title) { m_title = title; }

protected:
  CMediaInfoTag() = default;
  CMediaInfoTag(const CMediaInfoTag&);
  CMediaInfoTag(CMediaInfoTag&&);

private:
  int m_dbId;
  std::string m_title;
};

using CMediaInfoTagPtr = std::shared_ptr<CMediaInfoTag>;
