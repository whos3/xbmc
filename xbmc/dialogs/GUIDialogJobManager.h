#pragma once
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

#include "FileItem.h"
#include "dialogs/GUIDialogBoxBase.h"
#include "threads/CriticalSection.h"
#include "utils/ManageableJob.h"
#include "utils/Variant.h"
#include "view/GUIViewControl.h"

class CGUIDialogJobManager : public CGUIDialogBoxBase, protected IManageableJobQueueObserver
{
public:
  CGUIDialogJobManager();
  virtual ~CGUIDialogJobManager() = default;

  // specialization of CGUIDialogBoxBase
  virtual bool OnMessage(CGUIMessage& message) override;

  void ShowEndedJobs(bool showEndedJobs) { m_showEndedJobs = showEndedJobs; }
  bool SetManageableJobQueue(CManageableJobQueue* jobQueue);

protected:
  // implementations of IManageableJobQueueObserver
  virtual void OnJobAdded(const CManageableJob* job) override;
  virtual void OnJobProgress(const CManageableJob* job) override;
  virtual void OnJobEnded(const CManageableJob* job) override;
  virtual void OnJobRemoved(const std::string& jobIdentifier) override;

  // specializations of CGUIDialogBoxBase
  virtual void OnInitWindow() override;
  virtual void OnDeinitWindow(int nextWindowID) override;

  // specialization of CGUIDialog
  virtual void OnWindowLoaded() override;

  // specialization of CGUIWindow
  virtual void OnWindowUnload() override;

  // specialization of CGUIControlGroup
  virtual CGUIControl *GetFirstFocusableControl(int id) override;

private:
  void LoadItems();
  void UpdateItems();
  void UpdateControls();

  void OnContextMenu(int itemIndex);

  bool m_showEndedJobs;

  CManageableJobQueue* m_manageableJobQueue;

  CFileItemList m_jobItems;
  CCriticalSection m_criticalJobItems;

  CGUIViewControl m_viewControl;
};
