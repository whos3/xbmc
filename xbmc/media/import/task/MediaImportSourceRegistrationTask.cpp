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

#include "MediaImportSourceRegistrationTask.h"
#include "ApplicationMessenger.h"
#include "FileItem.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "dialogs/GUIDialogSelect.h"
#include "dialogs/GUIDialogYesNo.h"
#include "guilib/GUIWindowManager.h"
#include "media/import/MediaImportManager.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "utils/log.h"

CMediaImportSourceRegistrationTask::CMediaImportSourceRegistrationTask(const CMediaImport &import)
  : IMediaImportTask(import),
    m_source(import.GetSource())
{ }

bool CMediaImportSourceRegistrationTask::DoWork()
{
  if (CMediaImportManager::Get().GetImporter(m_import.GetPath()) == NULL)
  {
    CLog::Log(LOGINFO, "CMediaImportSourceRegistrationTask: unable to find an importer for source %s", m_source.GetIdentifier().c_str());
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "New media source detected", StringUtils::Format("No importer found for %s", m_source.GetFriendlyName().c_str())); // TODO: localization
    return false;
  }

  std::set<MediaType> mediaTypesToImport;
  CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (pDialog == NULL)
    return false;

  pDialog->SetHeading("New media source detected"); // TODO: localization
  pDialog->SetLine(1, "Would you like to import media items from"); // TODO: localization
  pDialog->SetLine(2, m_source.GetFriendlyName());
  pDialog->SetChoice(0, "Ignore"); // TODO: localization
  pDialog->SetChoice(1, "Import"); // TODO: localization

  if (ShouldCancel(0, 2))
    return false;

  //send message and wait for user input
  ThreadMessage tMsg = { TMSG_DIALOG_DOMODAL, WINDOW_DIALOG_YES_NO, (unsigned int)g_windowManager.GetActiveWindow() };
  CApplicationMessenger::Get().SendMessage(tMsg, true);

  if (pDialog->IsConfirmed())
  {
    CGUIDialogSelect* pDialogSelect = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
    if (pDialogSelect != NULL)
    {
      pDialogSelect->SetHeading("Available media types"); // TODO: localization
      pDialogSelect->SetMultiSelection(true);
      CFileItemList items;
      for (std::set<MediaType>::const_iterator it = m_source.GetAvailableMediaTypes().begin(); it != m_source.GetAvailableMediaTypes().end(); ++it)
      {
        CFileItemPtr pItem(new CFileItem(*it)); // TODO: localization
        pItem->Select(true);
        items.Add(pItem);
      }
      pDialogSelect->Add(items);
      pDialogSelect->DoModal();

      if (pDialogSelect->IsConfirmed() && pDialogSelect->GetSelectedItems().Size() > 0)
      {
        const CFileItemList &selectedItems = pDialogSelect->GetSelectedItems();
        for (int index = 0; index < selectedItems.Size(); ++index)
          mediaTypesToImport.insert(selectedItems.Get(index)->GetLabel());
      }

      // TODO: ask what path to import
      m_imports.push_back(CMediaImport(m_source.GetIdentifier(), m_source, mediaTypesToImport));
    }
  }

  return true;
}
