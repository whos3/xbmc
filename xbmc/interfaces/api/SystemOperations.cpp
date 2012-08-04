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

#include <algorithm>

#include "SystemOperations.h"
#include "ApplicationMessenger.h"
#include "interfaces/Builtins.h"
#include "utils/Variant.h"
#include "powermanagement/PowerManager.h"

using namespace API;

APIStatus CSystemOperations::GetProperties(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVariant properties = CVariant(CVariant::VariantTypeObject);
  for (unsigned int index = 0; index < parameterObject["properties"].size(); index++)
  {
    CStdString propertyName = parameterObject["properties"][index].asString();
    CVariant property;
    APIStatus ret;
    if ((ret = GetPropertyValue(client->GetPermissionFlags(), propertyName, property)) != APIStatusOK)
      return ret;

    properties[propertyName] = property;
  }

  result = properties;

  return APIStatusOK;
}

APIStatus CSystemOperations::EjectOpticalDrive(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  return CBuiltins::Execute("EjectTray") == 0 ? APIStatusOK : APIStatusFailedToExecute;
}

APIStatus CSystemOperations::Shutdown(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  if (g_powerManager.CanPowerdown())
  {
    CApplicationMessenger::Get().Powerdown();
    return APIStatusOK;
  }
  else
    return APIStatusFailedToExecute;
}

APIStatus CSystemOperations::Suspend(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  if (g_powerManager.CanSuspend())
  {
    CApplicationMessenger::Get().Suspend();
    return APIStatusOK;
  }
  else
    return APIStatusFailedToExecute;
}

APIStatus CSystemOperations::Hibernate(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  if (g_powerManager.CanHibernate())
  {
    CApplicationMessenger::Get().Hibernate();
    return APIStatusOK;
  }
  else
    return APIStatusFailedToExecute;
}

APIStatus CSystemOperations::Reboot(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  if (g_powerManager.CanReboot())
  {
    CApplicationMessenger::Get().Restart();
    return APIStatusOK;
  }
  else
    return APIStatusFailedToExecute;
}

APIStatus CSystemOperations::GetPropertyValue(int permissions, const std::string &property, CVariant &result)
{
  std::string prop;
  std::transform(property.begin(), property.end(), prop.begin(), ::tolower);

  if (prop.compare("canshutdown") == 0)
    result = g_powerManager.CanPowerdown() && (permissions & APIPermissionControlPower);
  else if (prop.compare("cansuspend") == 0)
    result = g_powerManager.CanSuspend() && (permissions & APIPermissionControlPower);
  else if (prop.compare("canhibernate") == 0)
    result = g_powerManager.CanHibernate() && (permissions & APIPermissionControlPower);
  else if (prop.compare("canreboot") == 0)
    result = g_powerManager.CanReboot() && (permissions & APIPermissionControlPower);
  else
    return APIStatusInvalidParameters;

  return APIStatusOK;
}
