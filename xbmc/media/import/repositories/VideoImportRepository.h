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

#include <map>

#include "media/import/IMediaImportRepository.h"
#include "threads/CriticalSection.h"
#include "video/VideoDatabase.h"

class CVideoImportRepository : public IMediaImportRepository
{
public:
  CVideoImportRepository()
    : m_loaded(false)
  { }
  virtual ~CVideoImportRepository();

  virtual bool Initialize();

  virtual std::vector<CMediaImport> GetImports(const std::string &sourceIdentifier = "") const;
  virtual bool GetImport(const std::string &path, CMediaImport &import) const;

  virtual bool AddImport(const CMediaImport &import);
  virtual bool UpdateImport(const CMediaImport &import);
  virtual bool RemoveImport(const std::string &path);
  
  virtual bool UpdateLastSync(const std::string &path);
  virtual bool SetMediaTypesForImport(const std::string &path, const std::set<MediaType> &mediaTypes);

  virtual std::vector<CMediaImportSource> GetSources(const MediaType &mediaType = MediaTypeNone) const;
  virtual bool GetSource(const std::string &identifier, CMediaImportSource &source) const;

  virtual bool AddSource(const CMediaImportSource &source);
  virtual bool UpdateSource(const CMediaImportSource &source);
  virtual bool RemoveSource(const std::string &identifier);

protected:
  CVideoDatabase m_db;
  bool m_loaded;

  typedef std::map<std::string, CMediaImport> MediaImportMap;
  CCriticalSection m_importsLock;
  MediaImportMap m_imports;

  typedef std::map<std::string, CMediaImportSource> MediaImportSourceMap;
  CCriticalSection m_sourcesLock;
  MediaImportSourceMap m_sources;
};
