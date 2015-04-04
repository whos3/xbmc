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
#include <string>

#include <openHome/Net/Cpp/DvDevice.h>

#include "network/WebServer.h"
#include "network/httprequesthandler/HTTPVfsHandler.h"

class COhUPnPDevice;

class COhUPnPResourceManager : public OpenHome::Net::IResourceManagerStd
{
public:
  COhUPnPResourceManager();
  virtual ~COhUPnPResourceManager();

  uint16_t Start(uint16_t port);
  void Stop();
  bool IsRunning() const;

  uint16_t GetPort() const { return m_port; }

  std::string AddSmallResource(const COhUPnPDevice& device, const std::string& resourcePath, const std::string& urlPath);
  std::string AddSmallResource(const COhUPnPDevice& device, const std::string& resourcePath);

  static std::string GetResourceUriPrefix(TIpAddress ipAddress, uint16_t port);
  static std::string GetResourceUriPrefix(const std::string& ipAddress, uint16_t port);
  static std::string GetFullResourceUri(const std::string& resourceUriPrefix, const std::string& resourcePath);

private:
  // implementation of IResourceManagerStd
  virtual void WriteResource(const std::string& uriTail, TIpAddress iface, std::vector<char*>& languageList, OpenHome::Net::IResourceWriter& resourceWriter) override;

  std::map<std::string, std::string> m_smallResources;

  uint16_t m_port;

  CWebServer m_webServer;
  CHTTPVfsHandler m_httpVfsHandler;
};