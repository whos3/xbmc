/*
 *      Copyright (C) 2016 Team XBMC
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

#include "GUIDialogJobManager.h"
#include "FileItem.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "input/Key.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"

static const int ControlNumberOfItems = 2;

static const int ControlDetailedList = 6;

static const int ControlButtonStopAll = 10;
static const int ControlButtonRemoveEnded = 11;

static bool CompareJobItems(const CFileItemPtr &lhs, const CFileItemPtr &rhs)
{
  if (lhs == nullptr || rhs == nullptr)
    return lhs != nullptr;

  std::string lhsStatusStr = lhs->GetProperty("ManageableJob.Status").asString();
  std::string rhsStatusStr = rhs->GetProperty("ManageableJob.Status").asString();

  if (lhsStatusStr.empty() || rhsStatusStr.empty())
    return !lhsStatusStr.empty();

  ManageableJobStatus lhsStatus = ManageableJobStatusFromString(lhsStatusStr);
  ManageableJobStatus rhsStatus = ManageableJobStatusFromString(rhsStatusStr);

  return lhsStatus < rhsStatus;
}

CGUIDialogJobManager::CGUIDialogJobManager()
  : CGUIDialogBoxBase(WINDOW_DIALOG_JOB_MANAGER, "DialogJobManager.xml")
  , m_showEndedJobs(false)
  , m_manageableJobQueue(nullptr)
{
  m_loadType = KEEP_IN_MEMORY;
}

bool CGUIDialogJobManager::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_CLICKED:
    {
      int controlId = message.GetSenderId();
      if (m_viewControl.HasControl(controlId))
      {
        int itemIndex = m_viewControl.GetSelectedItem();
        int action = message.GetParam1();
        if (action == ACTION_SELECT_ITEM || action == ACTION_MOUSE_LEFT_CLICK ||
            action == ACTION_CONTEXT_MENU || action == ACTION_MOUSE_RIGHT_CLICK)
        {
          OnContextMenu(itemIndex);
          return true;
        }
      }

      if (controlId == ControlButtonStopAll)
      {
        m_manageableJobQueue->CancelAllJobs();
        return true;
      }
      if (controlId == ControlButtonRemoveEnded)
      {
        m_manageableJobQueue->ClearEndedJobs();
        return true;
      }

      break;
    }

    case GUI_MSG_SETFOCUS:
    {
      if (m_viewControl.HasControl(message.GetControlId()) && m_viewControl.GetCurrentControl() != message.GetControlId())
      {
        m_viewControl.SetFocused();
        return true;
      }

      break;
    }

    default:
      break;
  }

  return CGUIDialogBoxBase::OnMessage(message);
}

bool CGUIDialogJobManager::SetManageableJobQueue(CManageableJobQueue* jobQueue)
{
  // don't allow resetting the job queue or changing it while the dialog is active
  if (jobQueue == nullptr || IsActive())
    return false;

  m_manageableJobQueue = jobQueue;
  return true;
}

CGUIControl *CGUIDialogJobManager::GetFirstFocusableControl(int id)
{
  if (m_viewControl.HasControl(id))
    id = m_viewControl.GetCurrentControl();

  return CGUIDialogBoxBase::GetFirstFocusableControl(id);
}

void CGUIDialogJobManager::OnWindowLoaded()
{
  CGUIDialogBoxBase::OnWindowLoaded();

  m_viewControl.Reset();
  m_viewControl.SetParentWindow(GetID());
  m_viewControl.AddView(GetControl(ControlDetailedList));
}

void CGUIDialogJobManager::OnJobAdded(const CManageableJob* job)
{
  if (job == nullptr)
    return;

  CSingleLock lock(m_criticalJobItems);
  CFileItemPtr item = std::make_shared<CFileItem>();
  if (!job->UpdateFileItem(item.get()))
    return;

  m_jobItems.Add(item);

  UpdateItems();
  UpdateControls();
}

void CGUIDialogJobManager::OnJobProgress(const CManageableJob* job)
{
  if (job == nullptr)
    return;

  CSingleLock lock(m_criticalJobItems);
  CFileItemPtr item = m_jobItems.Get(job->GetIdentifier());
  if (item != nullptr && item->GetPath() == job->GetIdentifier())
    job->UpdateFileItem(item.get());
}

void CGUIDialogJobManager::OnJobEnded(const CManageableJob* job)
{
  if (job == nullptr)
    return;

  CSingleLock lock(m_criticalJobItems);
  CFileItemPtr item = m_jobItems.Get(job->GetIdentifier());
  if (item != nullptr && item->GetPath() == job->GetIdentifier() && job->UpdateFileItem(item.get()))
  {
    UpdateItems();
    UpdateControls();
  }
}

void CGUIDialogJobManager::OnJobRemoved(const std::string& jobIdentifier)
{
  if (jobIdentifier.empty())
    return;

  CSingleLock lock(m_criticalJobItems);
  CFileItemPtr item = m_jobItems.Get(jobIdentifier);
  if (item != nullptr)
  {
    m_jobItems.Remove(item.get());

    UpdateItems();
    UpdateControls();
  }
}

void CGUIDialogJobManager::OnInitWindow()
{
  if (m_manageableJobQueue == nullptr)
    return;

  // register for updates to the job queue
  m_manageableJobQueue->SetObserver(this);

  {
    CSingleLock lock(m_criticalJobItems);
    // load the list of jobs
    LoadItems();

    // update the list of jobs
    UpdateItems();

    // set the view to be used
    m_viewControl.SetCurrentView(ControlDetailedList);

    // update all controls
    UpdateControls();
  }

  CGUIDialogBoxBase::OnInitWindow();
}

void CGUIDialogJobManager::OnDeinitWindow(int nextWindowID)
{
  CGUIDialogBoxBase::OnDeinitWindow(nextWindowID);

  // clear the view control
  m_viewControl.Clear();

  // reset the internal state
  m_showEndedJobs = false;
  m_manageableJobQueue = nullptr;
  m_jobItems.Clear();
}

void CGUIDialogJobManager::OnWindowUnload()
{
  CGUIDialogBoxBase::OnWindowUnload();

  m_viewControl.Reset();
}

void CGUIDialogJobManager::LoadItems()
{
  std::vector<const CManageableJob* const> jobs;

  if (m_showEndedJobs)
    jobs = m_manageableJobQueue->GetAllJobs();
  else
    jobs = m_manageableJobQueue->GetActiveJobs();

  m_jobItems.Clear();
  for (const auto& job : jobs)
  {
    CFileItemPtr item = std::make_shared<CFileItem>();
    if (!job->UpdateFileItem(item.get()))
      continue;

    m_jobItems.Add(item);
  }
}

void CGUIDialogJobManager::UpdateItems()
{
  std::string jobIdentifier;
  int item = m_viewControl.GetSelectedItem();
  if (item >= 0 && item < m_jobItems.Size())
    jobIdentifier = m_jobItems.Get(item)->GetPath();

  m_viewControl.Clear();

  // sort the items
  m_jobItems.Sort(CompareJobItems);

  // assign the items to the view control
  m_viewControl.SetItems(m_jobItems);

  if (!jobIdentifier.empty())
    m_viewControl.SetSelectedItem(jobIdentifier);
}

void CGUIDialogJobManager::UpdateControls()
{
  if (m_manageableJobQueue == nullptr)
    return;

  // set the number of items being shown
  SET_CONTROL_LABEL(ControlNumberOfItems, StringUtils::Format("%i %s", // TODO: localization
    m_jobItems.Size(), g_localizeStrings.Get(127).c_str()));

  // enable/disable the buttons
  CONTROL_ENABLE_ON_CONDITION(ControlButtonStopAll, m_manageableJobQueue->GetActiveJobCount() > 0);

  if (m_showEndedJobs)
  {
    SET_CONTROL_VISIBLE(ControlButtonRemoveEnded);
    CONTROL_ENABLE_ON_CONDITION(ControlButtonRemoveEnded, m_manageableJobQueue->GetEndedJobCount() > 0);
  }
  else
    SET_CONTROL_HIDDEN(ControlButtonRemoveEnded);
}

void CGUIDialogJobManager::OnContextMenu(int itemIndex)
{
  static const int ContextButtonStop = 0;

  CSingleLock lock(m_criticalJobItems);
  if (itemIndex < 0 || itemIndex >= m_jobItems.Size())
    return;

  // check if the item is a valid job
  CFileItemPtr item = m_jobItems.Get(itemIndex);
  const std::string& jobIdentifier = item->GetPath();
  if (jobIdentifier.empty() || !item->HasProperty(CManageableJob::FileItemPropertyStatus))
    return;

  // retrieve the status
  ManageableJobStatus jobStatus = ManageableJobStatusFromString(item->GetProperty(CManageableJob::FileItemPropertyStatus).asString());
  bool stop = jobStatus == ManageableJobStatus::Queued || jobStatus == ManageableJobStatus::InProgress;

  CContextButtons buttons;
  buttons.Add(ContextButtonStop, stop ? 15017 : 15015);

  // mark the item
  item->Select(true);

  // get the user's choice
  int choice = CGUIDialogContextMenu::ShowAndGetChoice(buttons);

  // deselect our item
  item->Select(false);

  if (choice < 0)
    return;

  // handle chosen action
  if (choice != ContextButtonStop)
    return;

  if (stop)
    m_manageableJobQueue->CancelJob(jobIdentifier);
  else
    m_manageableJobQueue->RemoveEndedJob(jobIdentifier);
}
