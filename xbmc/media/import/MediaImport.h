#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include <set>
#include <string>

#include "XBDateTime.h"
#include "media/MediaType.h"
#include "media/import/MediaImportSource.h"

/*!
 * \brief TODO
 */
class CMediaImport
{
public:
  CMediaImport(const std::string &importPath, const CMediaImportSource &source,
               const std::set<MediaType>& importedMediaTypes = std::set<MediaType>(),
               const CDateTime &lastSynced = CDateTime::GetCurrentDateTime())
    : m_importPath(importPath),
      m_source(source),
      m_importedMediaTypes(importedMediaTypes),
      m_lastSynced(lastSynced)
  { }
  CMediaImport(const std::string &importPath, const std::string &sourceIdentifier = "",
               const std::set<MediaType>& importedMediaTypes = std::set<MediaType>(),
               const CDateTime &lastSynced = CDateTime::GetCurrentDateTime())
    : m_importPath(importPath),
      m_source(sourceIdentifier, "", importedMediaTypes, lastSynced),
      m_importedMediaTypes(importedMediaTypes),
      m_lastSynced(lastSynced)
  { }

  bool operator==(const CMediaImport &other) const
  {
    if (m_importPath.compare(other.m_importPath) != 0 ||
        m_source != other.m_source ||
        m_importedMediaTypes != other.m_importedMediaTypes ||
        m_lastSynced != other.m_lastSynced)
      return false;

    return true;
  }
  bool operator!=(const CMediaImport &other) const { return !(*this == other); }

  const std::string& GetPath() const { return m_importPath; }

  const CMediaImportSource& GetSource() const { return m_source; }
  void SetSource(const CMediaImportSource &source)
  {
    if (source.GetIdentifier().empty())
      return;

    m_source = source;
  }

  const std::set<MediaType>& GetImportedMediaTypes() const { return m_importedMediaTypes; }
  void SetImportedMediaTypes(const std::set<MediaType> &mediaTypes) { m_importedMediaTypes = mediaTypes; }

  const CDateTime& GetLastSynced() const { return m_lastSynced; }
  void SetLastSynced(const CDateTime &lastSynced) { m_lastSynced = lastSynced; }

private:
  std::string m_importPath;
  CMediaImportSource m_source;
  std::set<MediaType> m_importedMediaTypes;
  CDateTime m_lastSynced;
};
