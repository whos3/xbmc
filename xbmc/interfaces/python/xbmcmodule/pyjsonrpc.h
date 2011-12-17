#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
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
#include "system.h"
#include "settings/GUISettings.h"
#include "interfaces/IInterfaceClient.h"
#include "interfaces/json-rpc/ITransportLayer.h"
#include "interfaces/json-rpc/JSONRPC.h"

class CPythonTransport : public JSONRPC::ITransportLayer
{
public:
  virtual bool PrepareDownload(const char *path, CVariant &details, std::string &protocol) { return false; }
  virtual bool Download(const char *path, CVariant &result) { return false; }
  virtual int GetCapabilities() { return JSONRPC::Response; }

  class CPythonClient : public IInterfaceClient
  {
  public:
    virtual bool SetPermissionFlags(InterfacePermission flags) { m_permissionFlags = (InterfacePermission)(PermissionReadData | flags); return true; };
    virtual InterfacePermission GetPermissionFlags()
    {
      if (g_guiSettings.GetBool("services.clientauthentication"))
        return m_permissionFlags;
      else
        return INTERFACEPERMISSION_NOAUTHENTICATION;
    }
    virtual int  GetAnnouncementFlags() { return 0; }
    virtual bool SetAnnouncementFlags(int flags) { return true; }

    virtual bool SetAuthenticated() { return true; }
    virtual bool IsAuthenticated() { return true; }
    virtual bool SetIdentification(std::string identification) { m_identification = identification; return true; }
    virtual std::string GetIdentification() { return m_identification; }
    virtual bool SetName(std::string name) { m_name = name; return true; }
    virtual std::string GetName() { return m_name; }
  private:
    InterfacePermission m_permissionFlags;
    std::string m_identification;
    std::string m_name;
  };
};
