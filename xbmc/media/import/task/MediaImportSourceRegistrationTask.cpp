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

#include "MediaImportSourceRegistrationTask.h"
#include "ApplicationMessenger.h"
#include "FileItem.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "dialogs/GUIDialogYesNo.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/MediaImportManager.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

CMediaImportSourceRegistrationTask::CMediaImportSourceRegistrationTask(const CMediaImportSource &source)
  : IMediaImportTask(CMediaImport(source.GetIdentifier(), "", source)),
    m_source(source),
    m_importDecision(false)
{ }

bool CMediaImportSourceRegistrationTask::DoWork()
{
  if (CMediaImportManager::Get().GetImporter(m_import.GetPath()) == NULL)
  {
    CLog::Log(LOGINFO, "CMediaImportSourceRegistrationTask: unable to find an importer for source %s", m_source.GetIdentifier().c_str());
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, g_localizeStrings.Get(39000).c_str(), StringUtils::Format(g_localizeStrings.Get(39001).c_str(), m_source.GetFriendlyName().c_str()));
    return false;
  }

  CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (pDialog == NULL)
    return false;

  pDialog->SetHeading(39000);
  pDialog->SetText(StringUtils::Format(g_localizeStrings.Get(39002).c_str(), m_source.GetFriendlyName().c_str()));
  pDialog->SetChoice(0, 39003);
  pDialog->SetChoice(1, 39004);

  if (ShouldCancel(0, 2))
    return false;

  // send message and wait for user input
  ThreadMessage tMsg = { TMSG_DIALOG_DOMODAL, WINDOW_DIALOG_YES_NO, (unsigned int)g_windowManager.GetActiveWindow() };
  CApplicationMessenger::Get().SendMessage(tMsg, true);

  m_importDecision = pDialog->IsConfirmed();
  return true;
}
