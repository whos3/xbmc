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

#include "media/MediaType.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"

/*!
 * \brief TODO
 */
class IMediaImportRepository
{
public:
  virtual ~IMediaImportRepository() { }

  virtual bool Initialize() = 0;

  virtual std::vector<CMediaImport> GetImports(const std::string &sourceIdentifier = "") const = 0;
  virtual bool GetImport(const std::string &path, CMediaImport &import) const = 0;

  virtual bool AddImport(const CMediaImport &import) = 0;
  virtual bool UpdateImport(const CMediaImport &import) = 0;
  virtual bool RemoveImport(const std::string &path) = 0;
  
  virtual bool UpdateLastSync(const std::string &path) = 0;
  virtual bool SetMediaTypesForImport(const std::string &path, const std::set<MediaType> &mediaTypes) = 0;

  virtual std::vector<CMediaImportSource> GetSources(const MediaType &mediaType = MediaTypeNone) const = 0;
  virtual bool GetSource(const std::string &identifier, CMediaImportSource &source) const = 0;

  virtual bool AddSource(const CMediaImportSource &source) = 0;
  virtual bool UpdateSource(const CMediaImportSource &source) = 0;
  virtual bool RemoveSource(const std::string &identifier) = 0;
};
