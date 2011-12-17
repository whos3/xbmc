#pragma once
/*
 *      Copyright (C) 2005-2011 Team XBMC
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
 
 #include <string>

enum InterfacePermission
{
  PermissionNone            =   0x0,
  PermissionReadData        =   0x1,
  PermissionControlPlayback =   0x2,
  PermissionControlNotify   =   0x4,
  PermissionControlPower    =   0x8,
  PermissionUpdateData      =  0x10,
  PermissionRemoveData      =  0x20,
  PermissionNavigate        =  0x40,
  PermissionWriteFile       =  0x80,
  PermissionAuthentication  = 0x100
};

static const InterfacePermission INTERFACEPERMISSION_ALL = (InterfacePermission)(PermissionReadData | PermissionControlPlayback | PermissionControlNotify |
                                             PermissionControlPower | PermissionUpdateData | PermissionRemoveData |
                                             PermissionNavigate | PermissionWriteFile | PermissionAuthentication);

static const InterfacePermission INTERFACEPERMISSION_UNAUTHENTICATED = (InterfacePermission)(PermissionReadData | PermissionAuthentication);

/*!
  \brief Permissions appliceable when authentication is turned of
  */
static const InterfacePermission INTERFACEPERMISSION_NOAUTHENTICATION = (InterfacePermission)(INTERFACEPERMISSION_ALL & ~PermissionAuthentication);

class IInterfaceClient
{
public:
  virtual ~IInterfaceClient() { };
  virtual InterfacePermission GetPermissionFlags() = 0;
  virtual bool SetPermissionFlags(InterfacePermission flags) = 0;
  virtual int GetAnnouncementFlags() = 0;
  virtual bool SetAnnouncementFlags(int flags) = 0;

  virtual bool SetAuthenticated() = 0;
  virtual bool IsAuthenticated() = 0;
  virtual bool SetIdentification(std::string identification) = 0;
  virtual std::string GetIdentification() = 0;
  virtual bool SetName(std::string name) = 0;
  virtual std::string GetName() = 0;
};
