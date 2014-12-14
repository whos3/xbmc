/*
 *      Copyright (C) 2014 Team XBMC
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

#include "MusicImportRepository.h"
#include "FileItem.h"
#include "music/tags/MusicInfoTag.h"

CMusicImportRepository::CMusicImportRepository()
{ }

CMusicImportRepository::~CMusicImportRepository()
{ }

bool CMusicImportRepository::GetMediaType(const CFileItem &item, MediaType &mediaType) const
{
  if (!item.HasMusicInfoTag())
    return false;

  mediaType = item.GetMusicInfoTag()->GetType();
  return !mediaType.empty();
}

std::set<MediaType> CMusicImportRepository::getSupportedMediaTypes() const
{
  std::set<MediaType> supportedMediaTypes;
  supportedMediaTypes.insert(MediaTypeArtist);
  supportedMediaTypes.insert(MediaTypeAlbum);
  supportedMediaTypes.insert(MediaTypeSong);
  return supportedMediaTypes;
}

std::vector<CMediaImportSource> CMusicImportRepository::getSources()
{
  return m_db.GetSources();
}

int CMusicImportRepository::addSource(const CMediaImportSource &source)
{
  return m_db.AddSource(source);
}

bool CMusicImportRepository::updateSource(const CMediaImportSource &source)
{
  return m_db.SetDetailsForSource(source);
}

void CMusicImportRepository::removeSource(const CMediaImportSource &source)
{
  m_db.RemoveSource(source.GetIdentifier());
}

std::vector<CMediaImport> CMusicImportRepository::getImports()
{
  return m_db.GetImports();
}

int CMusicImportRepository::addImport(const CMediaImport &import)
{
  return m_db.AddImport(import);
}

bool CMusicImportRepository::updateImport(const CMediaImport &import)
{
  return m_db.SetDetailsForImport(import);
}

void CMusicImportRepository::removeImport(const CMediaImport &import)
{
  m_db.RemoveImport(import);
}

void CMusicImportRepository::updateLastSync(const CMediaImport &import, const CDateTime &lastSync)
{
  m_db.UpdateImportLastSynced(import, lastSync);
}
