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

#include "utils/StdString.h"
#include "JSONRPC.h"

namespace JSONRPC
{
  class CSystemOperations
  {
  public:
    static JSON_STATUS GetProperties(const CStdString &method, ITransportLayer *transport, IInterfaceClient *client, const CVariant &parameterObject, CVariant &result);

    static JSON_STATUS Shutdown(const CStdString &method, ITransportLayer *transport, IInterfaceClient *client, const CVariant &parameterObject, CVariant &result);
    static JSON_STATUS Suspend(const CStdString &method, ITransportLayer *transport, IInterfaceClient *client, const CVariant &parameterObject, CVariant &result);
    static JSON_STATUS Hibernate(const CStdString &method, ITransportLayer *transport, IInterfaceClient *client, const CVariant &parameterObject, CVariant &result);
    static JSON_STATUS Reboot(const CStdString &method, ITransportLayer *transport, IInterfaceClient *client, const CVariant &parameterObject, CVariant &result);
  private:
    static JSON_STATUS GetPropertyValue(int permissions, const CStdString &property, CVariant &result);
  };
}
