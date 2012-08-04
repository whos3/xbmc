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

#include "XBMCOperations.h"
#include "ApplicationMessenger.h"
#include "Util.h"
#include "utils/Variant.h"
#include "powermanagement/PowerManager.h"

using namespace API;

APIStatus CXBMCOperations::GetInfoLabels(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::vector<CStdString> info;

  for (unsigned int i = 0; i < parameterObject["labels"].size(); i++)
  {
   CStdString field = parameterObject["labels"][i].asString();
    field = field.ToLower();

    info.push_back(parameterObject["labels"][i].asString());
  }

  if (info.size() > 0)
  {
    std::vector<CStdString> infoLabels = CApplicationMessenger::Get().GetInfoLabels(info);
    for (unsigned int i = 0; i < info.size(); i++)
    {
      if (i >= infoLabels.size())
        break;
      result[info[i].c_str()] = infoLabels[i];
    }
  }

  return APIStatusOK;
}

APIStatus CXBMCOperations::GetInfoBooleans(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  std::vector<CStdString> info;

  bool canControlPower = (client->GetPermissionFlags() & APIPermissionControlPower) > 0;

  for (unsigned int i = 0; i < parameterObject["booleans"].size(); i++)
  {
    CStdString field = parameterObject["booleans"][i].asString();
    field = field.ToLower();

    // Need to override power management of whats in infomanager since jsonrpc
    // have a security layer aswell.
    if (field.Equals("system.canshutdown"))
      result[parameterObject["booleans"][i].asString()] = (g_powerManager.CanPowerdown() && canControlPower);
    else if (field.Equals("system.canpowerdown"))
      result[parameterObject["booleans"][i].asString()] = (g_powerManager.CanPowerdown() && canControlPower);
    else if (field.Equals("system.cansuspend"))
      result[parameterObject["booleans"][i].asString()] = (g_powerManager.CanSuspend() && canControlPower);
    else if (field.Equals("system.canhibernate"))
      result[parameterObject["booleans"][i].asString()] = (g_powerManager.CanHibernate() && canControlPower);
    else if (field.Equals("system.canreboot"))
      result[parameterObject["booleans"][i].asString()] = (g_powerManager.CanReboot() && canControlPower);
    else
      info.push_back(parameterObject["booleans"][i].asString());
  }

  if (info.size() > 0)
  {
    std::vector<bool> infoLabels = CApplicationMessenger::Get().GetInfoBooleans(info);
    for (unsigned int i = 0; i < info.size(); i++)
    {
      if (i >= infoLabels.size())
        break;
      result[info[i].c_str()] = CVariant(infoLabels[i]);
    }
  }

  return APIStatusOK;
}
