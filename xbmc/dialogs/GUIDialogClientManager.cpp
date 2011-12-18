/*
 *      Copyright (C) 2011 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "GUIDialogClientManager.h"
#include "interfaces/ClientAuthManager.h"
#include "dialogs/GUIDialogPermissions.h"
#include "guilib/GUIWindowManager.h"
#include "FileItem.h"
#include "guilib/Key.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/Variant.h"

#define BUTTON_CLOSE     10
#define CONTROL_LIST     20

using namespace std;

CGUIDialogClientManager::CGUIDialogClientManager() :
    CGUIDialog(WINDOW_DIALOG_CLIENT_MANAGER, "DialogClientManager.xml"),
    m_iSelected(0),
    m_clientItems(new CFileItemList)
{
}

CGUIDialogClientManager::~CGUIDialogClientManager()
{
  delete m_clientItems;
}

bool CGUIDialogClientManager::OnAction(const CAction &action)
{
  int iActionId = action.GetID();
  if (iActionId == ACTION_PREVIOUS_MENU || iActionId == ACTION_PARENT_DIR)
  {
    Close();
    return true;
  }
  else if (GetFocusedControlID() == CONTROL_LIST &&
      (iActionId == ACTION_MOVE_DOWN || iActionId == ACTION_MOVE_UP ||
       iActionId == ACTION_PAGE_DOWN || iActionId == ACTION_PAGE_UP))
  {
    CGUIDialog::OnAction(action);
    int iSelected = m_viewControl.GetSelectedItem();
    if (iSelected != m_iSelected)
      m_iSelected = iSelected;
    return true;
  }

  return CGUIDialog::OnAction(action);
}

bool CGUIDialogClientManager::OnMessageInit(CGUIMessage &message)
{
  CGUIWindow::OnMessage(message);
  m_iSelected = 0;
  Update();

  return true;
}

bool CGUIDialogClientManager::OnClickList(CGUIMessage &message)
{
  IInterfaceClient* client = GetCurrentClient();
  if (CGUIDialogPermissions::ShowAndGetInputOnRemove(client))
  {
    CClientAuthManager::Remove(client->GetIdentification());
    Update();
  }

  return true;
}

bool CGUIDialogClientManager::OnClickButtonClose(CGUIMessage &message)
{
  Close();
  return true;
}

bool CGUIDialogClientManager::OnMessageClick(CGUIMessage &message)
{
  switch(message.GetSenderId())
  {
  case CONTROL_LIST:
    return OnClickList(message);
  case BUTTON_CLOSE:
    return OnClickButtonClose(message);
  default:
    return false;
  }
}

bool CGUIDialogClientManager::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_WINDOW_DEINIT:
      Clear();
      break;
    case GUI_MSG_ITEM_SELECT:
      return true;
    case GUI_MSG_WINDOW_INIT:
      {
        OnMessageInit(message);
        break;
      }
    case GUI_MSG_CLICKED:
      return OnMessageClick(message);
  }

  return CGUIDialog::OnMessage(message);
}

void CGUIDialogClientManager::OnWindowLoaded()
{
  CGUIDialog::OnWindowLoaded();

  m_viewControl.Reset();
  m_viewControl.SetParentWindow(GetID());
  const CGUIControl *list = GetControl(CONTROL_LIST);
  m_viewControl.AddView(list);
}

void CGUIDialogClientManager::OnWindowUnload()
{
  CGUIDialog::OnWindowUnload();
  m_viewControl.Reset();
}

CFileItemPtr CGUIDialogClientManager::GetCurrentListItem() const
{
  return m_clientItems->Get(m_iSelected);
}

IInterfaceClient* CGUIDialogClientManager::GetCurrentClient() const
{
  return m_clients.at(m_iSelected);
}

void CGUIDialogClientManager::Update()
{
  CSingleLock lock(g_graphicsContext);

  m_viewControl.SetCurrentView(CONTROL_LIST);
  Clear();

  CClientAuthManager::GetClients(m_clients, false);
  for (unsigned int index = 0; index < m_clients.size(); index++)
  {
    CFileItemPtr clientItem(new CFileItem(m_clients[index]->GetName()));
    clientItem->SetProperty("Identification", m_clients[index]->GetIdentification());
    clientItem->SetProperty("IsAuthenticated", m_clients[index]->IsAuthenticated());

    m_clientItems->Add(clientItem);
  }

  m_viewControl.SetItems(*m_clientItems);
  m_viewControl.SetSelectedItem(m_iSelected);

  CGUIControl *list = (CGUIControl *) GetControl(CONTROL_LIST);
  if (list)
    list->SetInvalid();
}

void CGUIDialogClientManager::Clear()
{
  m_viewControl.Clear();
  m_clientItems->Clear();
  m_clients.clear();
}
