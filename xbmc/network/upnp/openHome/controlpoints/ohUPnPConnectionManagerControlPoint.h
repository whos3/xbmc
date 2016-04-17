#pragma once
/*
 *      Copyright (C) 2016 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <OpenHome/Net/Cpp/CpUpnpOrgConnectionManager2.h>

#include "network/upnp/openHome/controlpoints/ohUPnPControlPoint.h"

class COhUPnPConnectionManagerControlPoint;

class IOhUPnPConnectionManagerControlPointAsync
{
public:
  virtual ~IOhUPnPConnectionManagerControlPointAsync() = default;

  virtual void GetProtocolInfoResult(bool success, std::string& sourceInfo, std::string& sinkInfo) { }

protected:
  IOhUPnPConnectionManagerControlPointAsync() = default;
};

class COhUPnPConnectionManagerControlPointManager
  : public IOhUPnPControlPointManager<OpenHome::Net::CpProxyUpnpOrgConnectionManager2Cpp, COhUPnPConnectionManagerControlPointManager, COhUPnPConnectionManagerControlPoint, IOhUPnPConnectionManagerControlPointAsync>
{
public:
  COhUPnPConnectionManagerControlPointManager(const std::string& deviceType = "");
  virtual ~COhUPnPConnectionManagerControlPointManager();

protected:
  friend class COhUPnPConnectionManagerControlPoint;

  bool GetProtocolInfoSync(const std::string& uuid, std::string& sourceInfo, std::string& sinkInfo) const;
  bool GetProtocolInfoAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid) const;
  bool GetProtocolInfoAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, std::string& sourceInfo, std::string& sinkInfo) const;

  // TODO
};

class COhUPnPConnectionManagerControlPoint : public IOhUPnPControlPointInstance<COhUPnPConnectionManagerControlPointManager, IOhUPnPConnectionManagerControlPointAsync>
{
public:
  virtual ~COhUPnPConnectionManagerControlPoint() = default;

  // implementation of IOhUPnPControlPointInstance
  const COhUPnPDeviceProfile& GetProfile() const override { return m_profile; }

  bool GetProtocolInfoSync(std::string& sourceInfo, std::string& sinkInfo) const;
  bool GetProtocolInfoAsync();

  // TODO

protected:
  friend class IOhUPnPControlPointManager<OpenHome::Net::CpProxyUpnpOrgConnectionManager2Cpp, COhUPnPConnectionManagerControlPointManager, COhUPnPConnectionManagerControlPoint, IOhUPnPConnectionManagerControlPointAsync>;

  COhUPnPConnectionManagerControlPoint(const COhUPnPConnectionManagerControlPointManager* manager,
    const std::string& uuid, IOhUPnPConnectionManagerControlPointAsync* callback);

  void GetProtocolInfoCallback(OpenHome::Net::IAsync& result);

  // TODO

private:
  COhUPnPDeviceProfile m_profile;
};
