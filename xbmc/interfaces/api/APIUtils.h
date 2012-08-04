#pragma once
/*
 *      Copyright (C) 2012 Team XBMC
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
 *  the Free Software Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <string>

#include "IClient.h"
#include "ITransportLayer.h"
#include "utils/Variant.h"

namespace API
{
  /*!
   \brief Status codes of an API call
   */
  typedef enum {
    APIStatusOK,
    APIStatusMethodNotFound,
    APIStatusInvalidParameters,
    APIStatusInternalError,
    APIStatusBadPermission,
    APIStatusFailedToExecute
  } APIStatus;

  /*!
   \brief Definition of an API method
   */
  typedef APIStatus (*APIMethod)(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant& parameterObject, CVariant &result);

  /*!
   \brief Permission categories for API methods

   An API method will only be called if the caller 
   has the correct permissions to exectue the method.
   The method call needs to be perfectly threadsafe.
   */
  typedef enum
  {
    APIPermissionReadData        =   0x1,
    APIPermissionControlPlayback =   0x2,
    APIPermissionControlNotify   =   0x4,
    APIPermissionControlPower    =   0x8,
    APIPermissionUpdateData      =  0x10,
    APIPermissionRemoveData      =  0x20,
    APIPermissionNavigate        =  0x40,
    APIPermissionWriteFile       =  0x80,
    APIPermissionControlSystem   = 0x100,
    APIPermissionControlGUI      = 0x200
  } APIPermission;

  const int APIPermissionAll = (APIPermissionReadData | APIPermissionControlPlayback | APIPermissionControlNotify |
                                APIPermissionControlPower | APIPermissionUpdateData | APIPermissionRemoveData |
                                APIPermissionNavigate | APIPermissionWriteFile | APIPermissionControlSystem |
                                APIPermissionControlGUI);

  const int APIPermissionNotification = (APIPermissionControlPlayback | APIPermissionControlNotify | APIPermissionControlPower |
                                         APIPermissionUpdateData | APIPermissionRemoveData | APIPermissionNavigate |
                                         APIPermissionWriteFile | APIPermissionControlSystem | APIPermissionControlGUI);

  class APIUtils
  {
  public:
    /*!
     \brief Returns a string representation for the given permission

     \param permission Specific permission
     \return String representation of the given permission
     */
    static inline const char* PermissionToString(APIPermission permission)
    {
      switch (permission)
      {
      case APIPermissionReadData:
        return "ReadData";
      case APIPermissionControlPlayback:
        return "ControlPlayback";
      case APIPermissionControlNotify:
        return "ControlNotify";
      case APIPermissionControlPower:
        return "ControlPower";
      case APIPermissionUpdateData:
        return "UpdateData";
      case APIPermissionRemoveData:
        return "RemoveData";
      case APIPermissionNavigate:
        return "Navigate";
      case APIPermissionWriteFile:
        return "WriteFile";
      case APIPermissionControlSystem:
        return "ControlSystem";
      case APIPermissionControlGUI:
        return "ControlGUI";
      default:
        return "Unknown";
      }
    }

    /*!
     \brief Returns a permission value for the given string representation

     \param permission String representation of the permission
     \return APIPermission value of the given string representation
     */
    static inline APIPermission StringToPermission(const std::string &permission)
    {
      if (permission.compare("ControlPlayback") == 0)
        return APIPermissionControlPlayback;
      if (permission.compare("ControlNotify") == 0)
        return APIPermissionControlNotify;
      if (permission.compare("ControlPower") == 0)
        return APIPermissionControlPower;
      if (permission.compare("UpdateData") == 0)
        return APIPermissionUpdateData;
      if (permission.compare("RemoveData") == 0)
        return APIPermissionRemoveData;
      if (permission.compare("Navigate") == 0)
        return APIPermissionNavigate;
      if (permission.compare("WriteFile") == 0)
        return APIPermissionWriteFile;
      if (permission.compare("ControlSystem") == 0)
        return APIPermissionControlSystem;
      if (permission.compare("ControlGUI") == 0)
        return APIPermissionControlGUI;

      return APIPermissionReadData;
    }

    static void MillisecondsToTimeObject(int time, CVariant &result)
    {
      int ms = time % 1000;
      result["milliseconds"] = ms;
      time = (time - ms) / 1000;

      int s = time % 60;
      result["seconds"] = s;
      time = (time - s) / 60;

      int m = time % 60;
      result["minutes"] = m;
      time = (time -m) / 60;

      result["hours"] = time;
    }

  protected:
    static inline bool ParameterNotNull(const CVariant &parameterObject, std::string key) { return parameterObject.isMember(key) && !parameterObject[key].isNull(); }

    /*!
     \brief Copies the values from the variantStringArray to the stringArray. stringArray is cleared.

     \param variantStringArray CVariant object representing a string array
     \param stringArray String array where the values are copied into (cleared)
     */
    static void CopyStringArray(const CVariant &variantStringArray, std::vector<std::string> &stringArray)
    {
      if (!variantStringArray.isArray())
        return;

      stringArray.clear();
      for (CVariant::const_iterator_array it = variantStringArray.begin_array(); it != variantStringArray.end_array(); it++)
        stringArray.push_back(it->asString());
    }
  };
}
