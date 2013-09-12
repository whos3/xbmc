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

#include "MediaImportRetrievalTask.h"
#include "media/import/MediaImportManager.h"
#include "utils/log.h"

CMediaImportRetrievalTask::CMediaImportRetrievalTask(const CMediaImport &import)
  : IMediaImportTask(import),
    m_importer(NULL)
{ }

CMediaImportRetrievalTask::~CMediaImportRetrievalTask()
{
  delete m_importer;
}

bool CMediaImportRetrievalTask::DoWork()
{
  GetProgressBarHandle("Retrieving media items from " + m_import.GetSource().GetFriendlyName()); // TODO: Localization

  if (m_importer == NULL)
  {
    // look for an importer than can handle the given path
    const IMediaImporter *importer = CMediaImportManager::Get().GetImporter(m_import.GetPath());
    if (importer == NULL)
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: no importer capable of handling %s (%s) found", m_import.GetSource().GetFriendlyName().c_str(), m_import.GetPath().c_str());
      return false;
    }

    // try to create a new instance of the matching importer for this import
    m_importer = importer->Create(m_import);
    if (m_importer == NULL)
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: failed to create importer %s to handle %s", importer->GetIdentification(), m_import.GetPath().c_str());
      return false;
    }
  }

  return m_importer->Import(this);
}

void CMediaImportRetrievalTask::AddItem(const MediaType &mediaType, const CFileItemPtr& item)
{
  m_importedMedia[mediaType].push_back(item);
}

void CMediaImportRetrievalTask::SetItems(const MediaType &mediaType, const std::vector<CFileItemPtr>& items)
{
  m_importedMedia[mediaType] = items;
}

bool CMediaImportRetrievalTask::GetImportedMedia(const MediaType &mediaType, CFileItemList& list) const
{
  list.SetFastLookup(true);
  if (m_importer->GetImportedMediaTypes().find(mediaType) == m_importer->GetImportedMediaTypes().end())
    return false;

  std::map<MediaType, std::vector<CFileItemPtr> >::const_iterator it = m_importedMedia.find(mediaType);
  if (it != m_importedMedia.end())
  {
    for (std::vector<CFileItemPtr>::const_iterator it2 = it->second.begin() ; it2 != it->second.end() ; it2++)
      list.Add(*it2);
  }

  return true;
}
