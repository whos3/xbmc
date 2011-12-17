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

#include "GUIDialogPermissions.h"
#include "FileItem.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/Key.h"
#include "threads/SingleLock.h"
#include "settings/GUISettings.h"
#include "utils/log.h"
#include "utils/StdString.h"

#define BUTTON_REJECT_ALWAYS  10
#define BUTTON_REJECT         11
#define BUTTON_GRANT          12
#define BUTTON_GRANT_ALWAYS   13
#define CONTROL_LIST          20

using namespace std;
using namespace JSONRPC;

map<InterfacePermission, CGUIDialogPermissions::InterfacePermissionInfo> CGUIDialogPermissions::m_permissionInfo = fillPermissionInfo();

map<InterfacePermission, CGUIDialogPermissions::InterfacePermissionInfo> CGUIDialogPermissions::fillPermissionInfo()
{
  map<InterfacePermission, InterfacePermissionInfo> map;
  InterfacePermissionInfo info;

  info.localisedLabel = 37020;
  info.level = 0;
  map[PermissionControlPlayback] = info;
  
  info.localisedLabel = 37021;
  info.level = 0;
  map[PermissionControlNotify] = info;
  
  info.localisedLabel = 37022;
  info.level = 1;
  map[PermissionControlPower] = info;
  
  info.localisedLabel = 37023;
  info.level = 1;
  map[PermissionUpdateData] = info;
  
  info.localisedLabel = 37024;
  info.level = 2;
  map[PermissionRemoveData] = info;
  
  info.localisedLabel = 37025;
  info.level = 0;
  map[PermissionNavigate] = info;
  
  info.localisedLabel = 37026;
  info.level = 2;
  map[PermissionWriteFile] = info;

  return map;
}

CGUIDialogPermissions::CGUIDialogPermissions() :
  CGUIDialogBoxBase(WINDOW_DIALOG_PERMISSIONS, "DialogPermissions.xml"),
  m_permissionUpdate(false),
  m_client(NULL),
  m_permissions(new CFileItemList),
  m_result(PermissionsRejected)
{ }

CGUIDialogPermissions::~CGUIDialogPermissions()
{
  delete m_permissions;
}

bool CGUIDialogPermissions::OnMessage(CGUIMessage& message)
{
  unsigned int iMessage = message.GetMessage();

  switch (iMessage)
  {
    case GUI_MSG_WINDOW_INIT:
      CGUIWindow::OnMessage(message);
      Update();
      break;

    case GUI_MSG_WINDOW_DEINIT:
      clear();
      break;

    case GUI_MSG_ITEM_SELECT:
      return true;

    case GUI_MSG_CLICKED:
      return OnMessageClick(message);

    default:
      break;
  }

  return CGUIDialogBoxBase::OnMessage(message);
}

bool CGUIDialogPermissions::OnAction(const CAction &action)
{
  int iActionId = action.GetID();
  if (iActionId == ACTION_PREVIOUS_MENU || iActionId == ACTION_PARENT_DIR)
  {
    Close();
    return true;
  }

  return CGUIDialogBoxBase::OnAction(action);
}

void CGUIDialogPermissions::OnWindowLoaded(void)
{
  CGUIDialogBoxBase::OnWindowLoaded();

  m_viewControl.Reset();
  m_viewControl.SetParentWindow(GetID());
  const CGUIControl *list = GetControl(CONTROL_LIST);
  m_viewControl.AddView(list);
}

void CGUIDialogPermissions::OnWindowUnload(void)
{
  CGUIDialogBoxBase::OnWindowUnload();
  m_viewControl.Reset();
}

void CGUIDialogPermissions::Update()
{
  if (m_client == NULL)
    return;

  CSingleLock lock(g_graphicsContext);

  m_result = PermissionsRejected;
  bool remember = g_guiSettings.GetBool("services.rememberclientauthentication");
  CONTROL_ENABLE_ON_CONDITION(BUTTON_REJECT_ALWAYS, remember);
  CONTROL_ENABLE_ON_CONDITION(BUTTON_GRANT_ALWAYS, remember);
  m_viewControl.SetCurrentView(CONTROL_LIST);

  SetHeading(m_permissionUpdate ? 37001 : 37000);
  CStdString line;
  line.Format(GetLocalized(37002), m_client->GetName());
  SetLine(0, line);
  SetLine(1, 37003);
  SetChoice(BUTTON_REJECT_ALWAYS - 10, 37010);
  SetChoice(BUTTON_REJECT - 10, 37011);
  SetChoice(BUTTON_GRANT - 10, 37012);
  SetChoice(BUTTON_GRANT_ALWAYS - 10, 37013);

  int permissions = m_client->GetPermissionFlags();

  m_permissions->Clear();
  vector<InterfacePermissionInfo> permissionList;
  for (int permission = (int)PermissionReadData * 2; permission <= INTERFACEPERMISSION_ALL; permission *= 2)
  {
    if ((permissions & permission) > 0)
    {
      map<InterfacePermission, InterfacePermissionInfo>::const_iterator permissionInfo = m_permissionInfo.find((InterfacePermission)permission);
      if (permissionInfo == m_permissionInfo.end())
        continue;

      // Sort by level
      bool added = false;
      for (unsigned int index = 0; index < permissionList.size(); index++)
      {
        if (permissionInfo->second.level >= permissionList[index].level)
        {
          permissionList.insert(permissionList.begin() + index, permissionInfo->second);
          added = true;
          break;
        }
      }

      if (!added)
        permissionList.push_back(permissionInfo->second);
    }
  }

  for (unsigned int index = 0; index < permissionList.size(); index++)
  {
    CFileItemPtr permissionItem(new CFileItem(GetLocalized(permissionList[index].localisedLabel)));
    switch (permissionList[index].level)
    {
      case 1:
        permissionItem->SetIconImage("PermissionLevelMedium.png");
        break;
          
      case 2:
        permissionItem->SetIconImage("PermissionLevelHigh.png");
        break;

      case 0:
      default:
        permissionItem->SetIconImage("PermissionLevelLow.png");
        break;
    }

    m_permissions->Add(permissionItem);
  }
  
  m_viewControl.SetItems(*m_permissions);
  m_viewControl.SetSelectedItem(0);

  CGUIControl *list = (CGUIControl *) GetControl(CONTROL_LIST);
  if (list)
    list->SetInvalid();
}

bool CGUIDialogPermissions::SetClient(IInterfaceClient *client)
{
  if (client == NULL)
    return false;

  m_client = client;
  return true;
}

CGUIDialogPermissions::PermissionsResult CGUIDialogPermissions::ShowAndGetInput(IInterfaceClient *client, bool permissionUpdate /* = false */)
{
  CGUIDialogPermissions *dialog = (CGUIDialogPermissions *)g_windowManager.GetWindow(WINDOW_DIALOG_PERMISSIONS);
  if (!dialog)
    return CGUIDialogPermissions::PermissionsRejected;

  dialog->clear();
  if (!dialog->SetClient(client))
    return CGUIDialogPermissions::PermissionsRejected;

  dialog->m_permissionUpdate = permissionUpdate;
  dialog->DoModal();
  return dialog->m_result;
}

bool CGUIDialogPermissions::OnMessageClick(CGUIMessage &message)
{
  int iControl = message.GetSenderId();
  switch(iControl)
  {
  case CONTROL_LIST:
    return true;

  case BUTTON_REJECT_ALWAYS:
    m_result = PermissionsRejectedAlways;
    Close();
    return true;

  case BUTTON_REJECT:
    m_result = PermissionsRejected;
    Close();
    return true;

  case BUTTON_GRANT:
    m_result = PermissionsGranted;
    Close();
    return true;

  case BUTTON_GRANT_ALWAYS:
    m_result = PermissionsGrantedAlways;
    Close();
    return true;

  default:
    return false;
  }
}

void CGUIDialogPermissions::clear()
{
  m_client = NULL;
  m_viewControl.Clear();
  m_permissions->Clear();
}
