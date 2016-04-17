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

#include <OpenHome/Net/Cpp/DvUpnpOrgConnectionManager2.h>

#include "network/upnp/openHome/ohUPnPService.h"
#include "network/upnp/openHome/rootdevices/ohUPnPRootDevice.h"

class COhUPnPRootDevice;

class COhUPnPMediaServerConnectionManagerService : public IOhUPnPService
{
public:
  COhUPnPMediaServerConnectionManagerService(COhUPnPRootDevice& device);
  virtual ~COhUPnPMediaServerConnectionManagerService();

  void Start();
  void Stop();
  bool IsRunning() const;

private:
  class ConnectionManager : public OpenHome::Net::DvProviderUpnpOrgConnectionManager2Cpp
  {
  public:
    ConnectionManager(COhUPnPMediaServerConnectionManagerService& service, OpenHome::Net::DvDeviceStd& device);
    virtual ~ConnectionManager();

  protected:
    void GetProtocolInfo(OpenHome::Net::IDvInvocationStd& invocation, std::string& source, std::string& sink) override;
    void PrepareForConnection(OpenHome::Net::IDvInvocationStd& invocation, const std::string& remoteProtocolInfo,
      const std::string& peerConnectionManager, int32_t peerConnectionID, const std::string& direction,
      int32_t& connectionID, int32_t& avTransportID, int32_t& rcsID) override;
    void ConnectionComplete(OpenHome::Net::IDvInvocationStd& invocation, int32_t connectionID) override;
    void GetCurrentConnectionIDs(OpenHome::Net::IDvInvocationStd& invocation, std::string& connectionIDs) override;
    void GetCurrentConnectionInfo(OpenHome::Net::IDvInvocationStd& invocation, int32_t connectionID, int32_t& rcsID,
      int32_t& avTransportID, std::string& protocolInfo, std::string& peerConnectionManager,
      int32_t& peerConnectionID, std::string& direction, std::string& status) override;

  private:
    COhUPnPMediaServerConnectionManagerService& m_service;
  };

  friend class ConnectionManager;

  COhUPnPRootDevice& m_device;
  std::unique_ptr<ConnectionManager> m_connectionManager;
};
