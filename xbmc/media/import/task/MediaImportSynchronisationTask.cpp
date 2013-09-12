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

#include "MediaImportSynchronisationTask.h"
#include "media/import/IMediaImportHandler.h"
#include "utils/log.h"

CMediaImportSynchronisationTask::CMediaImportSynchronisationTask(const CMediaImport &import, IMediaImportHandler *importHandler, const CFileItemList &items)
  : IMediaImportTask(import),
    m_importHandler(importHandler),
    m_items(items)
{ }

CMediaImportSynchronisationTask::~CMediaImportSynchronisationTask()
{
  delete m_importHandler;
}

bool CMediaImportSynchronisationTask::DoWork()
{
  if (m_importHandler == NULL)
    return false;

  GetProgressBarHandle("Synchronising media items from " + m_import.GetSource().GetFriendlyName()); // TODO: Localization

  if (ShouldCancel(0, (unsigned int)m_items.Size()))
    return false;

  CLog::Log(LOGINFO, "CMediaImportSynchronisationTask: handling %d imported %s items from %s",
    m_items.Size(), m_importHandler->GetMediaType().c_str(), m_import.GetSource().GetIdentifier().c_str());
  // handle the imported items of a specific media type
  m_importHandler->HandleImportedItems(m_import, m_items, this);

  // TODO: now make sure the items are enabled
  m_importHandler->SetImportedItemsEnabled(m_import.GetSource().GetIdentifier(), true);

  return true;
}
