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

#include <vector>
#include <map>
#include "IInterfaceClient.h"

enum CLIENT_AUTH_STATUS
{
  UnknownClient,
  PartiallyKnownClient,
  KnownClient
};

class CClientAuthManager
{
public:
  static CLIENT_AUTH_STATUS Check(std::string identification, std::string name, InterfacePermission permissions, bool &authenticated);
  static bool Authenticate(IInterfaceClient *client, InterfacePermission permissions);
  static int GetPermissions(std::string identification);
  static bool Add(IInterfaceClient *client, bool store = true);
  static bool Add(std::string identification, std::string name, InterfacePermission permissions, bool authenticated, int announcementFlags, bool store = true);
  static bool Update(IInterfaceClient *client);
  static bool Update(std::string identification, InterfacePermission permissions);
  static bool Remove(std::string identification);
  static void Clear();
  static unsigned int Size() { return m_clients.size(); }
  static void GetClients(std::vector<IInterfaceClient*> &clients);

private:
  class CManagedClient : public IInterfaceClient
  {
  public:
    CManagedClient();
    CManagedClient(IInterfaceClient *client, bool store = true);
    CManagedClient(std::string identification, std::string name, InterfacePermission permissions, bool authenticated, int announcementFlags, bool store = true);
    virtual InterfacePermission GetPermissionFlags() { return m_permissionFlags; }
    virtual bool SetPermissionFlags(InterfacePermission flags) { return false; }
    virtual int  GetAnnouncementFlags() { return m_announcementFlags; }
    virtual bool SetAnnouncementFlags(int flags) { return false; }

    virtual bool SetAuthenticated() { return false; }
    virtual bool IsAuthenticated() { return m_authenticated; };
    virtual bool SetIdentification(std::string identification) { return false; }
    virtual std::string GetIdentification() { return m_identification; }
    virtual bool SetName(std::string name) { return false; }
    virtual std::string GetName() { return m_name; }

    bool m_store;

  private:
    InterfacePermission m_permissionFlags;
    int m_announcementFlags;
    bool m_authenticated;
    std::string m_identification;
    std::string m_name;
  };

  static std::map<std::string, CManagedClient> m_clients;
};
