#pragma once
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

#include <map>

#include "dialogs/GUIDialogBoxBase.h"
#include "GUIViewControl.h"
#include "interfaces/json-rpc/IClient.h"
#include "interfaces/json-rpc/JSONUtils.h"

class CGUIDialogPermissions : public CGUIDialogBoxBase
{
public:
  enum PermissionsResult
  {
    PermissionsRejected = 0,
    PermissionsGranted,
    PermissionsGrantedAlways
  };

public:
  CGUIDialogPermissions();
  virtual ~CGUIDialogPermissions();

  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction& action);
  
  virtual void OnWindowLoaded();
  virtual void OnWindowUnload();

  virtual bool HasListItems() const { return m_client != NULL; };
  virtual void Update();
  
  bool SetClient(JSONRPC::IClient *client);
  PermissionsResult GetChoice() const { return m_result; }
  static PermissionsResult ShowAndGetInput(JSONRPC::IClient *client, bool permissionUpdate = false);

protected:
  virtual bool OnMessageClick(CGUIMessage &message);

private:
  void clear();

  bool m_permissionUpdate;
  JSONRPC::IClient *m_client;
  CFileItemList *m_permissions;
  CGUIViewControl m_viewControl;
  PermissionsResult m_result;

  typedef struct OperationPermissionInfo
  {
    int localisedLabel;
    int level;
  } OperationPermissionInfo;

  static std::map<JSONRPC::OperationPermission, OperationPermissionInfo> m_permissionInfo;
  static std::map<JSONRPC::OperationPermission, OperationPermissionInfo> fillPermissionInfo();
};
