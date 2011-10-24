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

#include "ClientAuthManager.h"
#include "JSONUtils.h"
#include "settings/GUISettings.h"
#include "dialogs/GUIDialogPermissions.h"

using namespace std;
using namespace JSONRPC;

std::map<std::string, CClientAuthManager::CManagedClient> CClientAuthManager::m_clients;

CClientAuthManager::CManagedClient::CManagedClient()
{
  m_permissionFlags = PermissionNone;
  m_announcementFlags = 0;
  m_authenticated = false;
  m_identification = "";
  m_name = "";
  m_store = false;
}

CClientAuthManager::CManagedClient::CManagedClient(IClient *client, bool store /* = true */)
{
  if (client == NULL)
    throw std::exception();

  m_permissionFlags = client->GetPermissionFlags();
  m_announcementFlags = client->GetAnnouncementFlags();
  m_authenticated = client->IsAuthenticated();
  m_identification = client->GetIdentification();
  m_name = client->GetName();
  m_store = store;
}

CClientAuthManager::CManagedClient::CManagedClient(std::string identification, std::string name, int permissions, bool authenticated, int announcementFlags, bool store /* = true */)
{
  if (identification.empty() || name.empty())
    throw std::exception();

  m_permissionFlags = permissions;
  m_announcementFlags = announcementFlags;
  m_authenticated = authenticated;
  m_identification = identification;
  m_name = name;
  m_store = store;
}

bool CClientAuthManager::Add(IClient *client, bool store /* = true */)
{
  if (client == NULL)
    return false;

  std::map<std::string, CClientAuthManager::CManagedClient>::const_iterator existingClient = m_clients.find(client->GetIdentification());
  if (existingClient != m_clients.end())
    return false;

  try
  {
    CManagedClient wrapperClient(client, store);
    m_clients[client->GetIdentification()] = wrapperClient;
  }
  catch (...)
  {
    return false;
  }

  return true;
}

bool CClientAuthManager::Add(std::string identification, std::string name, int permissions, bool authenticated, int announcementFlags, bool store /* = true */)
{
  if (identification.empty() || name.empty())
    return false;

  std::map<std::string, CClientAuthManager::CManagedClient>::const_iterator existingClient = m_clients.find(identification);
  if (existingClient != m_clients.end())
    return false;

  try
  {
    CManagedClient wrapperClient(identification, name, permissions, authenticated, announcementFlags, store);
    m_clients[identification] = wrapperClient;
  }
  catch (...)
  {
    return false;
  }

  return true;
}

bool CClientAuthManager::Update(IClient *client)
{
  if (client == NULL)
    return false;

  std::map<std::string, CClientAuthManager::CManagedClient>::iterator existingClient = m_clients.find(client->GetIdentification());
  if (existingClient != m_clients.end())
    return false;

  existingClient->second.SetPermissionFlags(client->GetPermissionFlags());
  return true;
}

bool CClientAuthManager::Update(std::string identification, int permissions)
{
  std::map<std::string, CClientAuthManager::CManagedClient>::iterator existingClient = m_clients.find(identification);
  if (existingClient != m_clients.end())
    return false;

  existingClient->second.SetPermissionFlags(permissions);
  return true;
}

bool CClientAuthManager::Remove(std::string identification)
{
  std::map<std::string, CClientAuthManager::CManagedClient>::const_iterator existingClient = m_clients.find(identification);
  if (existingClient != m_clients.end())
    return false;

  m_clients.erase(identification);
  return true;
}

CLIENT_AUTH_STATUS CClientAuthManager::Check(std::string identification, std::string name, int permissions, bool &authenticated)
{
  authenticated = false;
  std::map<std::string, CClientAuthManager::CManagedClient>::iterator foundClient = m_clients.find(identification);
  if (foundClient != m_clients.end())
  {
    if (foundClient->second.GetName() == name)
    {
      authenticated = foundClient->second.IsAuthenticated();
      if ((foundClient->second.GetPermissionFlags() | (permissions | PermissionReadData)) == foundClient->second.GetPermissionFlags())
      {
        return KnownClient;
      }
      else
        return PartiallyKnownClient;
    }
  }

  return UnknownClient;
}

bool CClientAuthManager::Authenticate(IClient *client, int permissions)
{
  if (client == NULL)
    return false;

  if (permissions <= PermissionReadData)
    return true;

  int originalPermissions = client->GetPermissionFlags();
  client->SetPermissionFlags(permissions);

  bool isAuthenticated = false;
  CLIENT_AUTH_STATUS clientStatus = Check(client->GetIdentification(), client->GetName(), client->GetPermissionFlags(), isAuthenticated);

  CGUIDialogPermissions::PermissionsResult userChoice;

  if (!g_guiSettings.GetBool("services.clientauthentication") ||
      (clientStatus == KnownClient && isAuthenticated) ||
      (clientStatus != KnownClient &&
      (userChoice = CGUIDialogPermissions::ShowAndGetInput(client, clientStatus == PartiallyKnownClient)) != CGUIDialogPermissions::PermissionsRejected))
  {
    if (!isAuthenticated)
      client->SetAuthenticated();

    if (clientStatus == UnknownClient)
      Add(client, g_guiSettings.GetBool("services.rememberclientauthentication") && userChoice == CGUIDialogPermissions::PermissionsGrantedAlways);
    else if (clientStatus == PartiallyKnownClient)
      Update(client);

    return true;
  }
  
  // TODO: Add "always deny" clients to CClientAuthManager
  client->SetPermissionFlags(originalPermissions);
  return false;
}

void CClientAuthManager::GetClients(std::vector<IClient*> &clients)
{
  std::map<std::string, CClientAuthManager::CManagedClient>::const_iterator iter;
  std::map<std::string, CClientAuthManager::CManagedClient>::const_iterator iterEnd = m_clients.end();

  for (iter = m_clients.begin(); iter != iterEnd; iter++)
  {
    clients.push_back((IClient *)&(iter->second));
  }
}
