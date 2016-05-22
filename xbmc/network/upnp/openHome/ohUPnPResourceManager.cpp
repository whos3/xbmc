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

#include <chrono>
#include <random>

#include "ohUPnPResourceManager.h"
#include "URL.h"
#include "filesystem/File.h"
#include "network/httprequesthandler/HTTPRequestHandlerUtils.h"
#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfilesManager.h"
#include "network/upnp/openHome/rootdevices/ohUPnPClientDevice.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "utils/log.h"
#include "utils/md5.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

static const std::string ResourceUrlProtocol = "Upnp";
static const std::string ResourceUrlPrefix = "resource";

static std::string BuildResourceUrlPrefix(const COhUPnPDevice& device)
{
  std::string path = URIUtils::AddFileToFolder(device.GetUuid(), ResourceUrlProtocol);
  path = URIUtils::AddFileToFolder(path, ResourceUrlPrefix);

  return path;
}

static std::string BuildResourceUrl(const COhUPnPDevice& device, const std::string& urlPath)
{
  // a resource path looks like <device uuid>/Upnp/resource/<path>
  return URIUtils::AddFileToFolder(BuildResourceUrlPrefix(device), urlPath);
}

COhUPnPResourceManager::COhUPnPResourceManager()
  : m_port(0)
  , m_httpVfsHandler(std::make_shared<const CMimeTypeGetter>())
{
  m_webServer.RegisterRequestHandler(&m_httpVfsHandler);
}

COhUPnPResourceManager::~COhUPnPResourceManager()
{
  Stop();

  m_webServer.UnregisterRequestHandler(&m_httpVfsHandler);

  m_smallResources.clear();
}

uint16_t COhUPnPResourceManager::Start(uint16_t port)
{
  if (m_webServer.IsStarted())
    return port;

  // prepare the random number generator in case we need to generate a port
  uint32_t seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
  std::mt19937 random(seed);

  // setup and start the webserver instance
  while (true)
  {
    if (port == 0)
    {
      // generate a new random port between 1025 and 65535
      do
      {
        port = static_cast<uint16_t>(random());
      } while (port <= 1024);
    }

    // try to start the webserver
    CLog::Log(LOGDEBUG, "COhUPnPResourceManager: starting webserver on port %hu...", port);
    if (m_webServer.Start(port))
      break;

    CLog::Log(LOGDEBUG, "COhUPnPResourceManager: failed to start webserver on port %hu. Trying another port...", port);
    port = 0;
  }

  m_port = port;

  return m_port;
}

void COhUPnPResourceManager::Stop()
{
  // stop the webserver instance
  m_webServer.Stop();

  m_port = 0;
}

bool COhUPnPResourceManager::IsRunning() const
{
  return m_webServer.IsStarted();
}

std::string COhUPnPResourceManager::AddSmallResource(const COhUPnPDevice& device, const std::string& resourcePath, const std::string& urlPath)
{
  if (!device.IsValid() ||
      resourcePath.empty() || urlPath.empty() ||
      !XFILE::CFile::Exists(resourcePath))
    return "";

  const auto& smallResource = m_smallResources.find(urlPath);
  if (smallResource != m_smallResources.cend())
  {
    if (smallResource->second != resourcePath)
      return "";
  }
  else
    m_smallResources.insert(std::make_pair(urlPath, resourcePath));

  return BuildResourceUrl(device, urlPath);
}

std::string COhUPnPResourceManager::AddSmallResource(const COhUPnPDevice& device, const std::string& resourcePath)
{
  if (!device.IsValid() ||
      resourcePath.empty())
    return "";

  CURL resourceUrl(resourcePath);

  // determine the filename to provide context to md5'd urls
  std::string filename;
  if (resourceUrl.IsProtocol("image"))
    filename = URIUtils::GetFileName(resourceUrl.GetHostName());
  else
    filename = URIUtils::GetFileName(resourcePath);

  filename = CURL::Encode(filename);
  std::string md5 = XBMC::XBMC_MD5::GetMD5(resourcePath);
  md5 = URIUtils::AddFileToFolder(md5, filename);

  return AddSmallResource(device, resourcePath, md5);
}

std::string COhUPnPResourceManager::GetResourceUriPrefix(TIpAddress ipAddress, uint16_t port)
{
  return GetResourceUriPrefix(COhUtils::TIpAddressToString(ipAddress), port);
}

std::string COhUPnPResourceManager::GetResourceUriPrefix(const std::string& ipAddress, uint16_t port)
{
  return StringUtils::Format("http://%s:%hu/vfs/", ipAddress.c_str(), port);
}

std::string COhUPnPResourceManager::GetFullResourceUri(const std::string& resourceUriPrefix, const std::string& resourceUrl)
{
  if (resourceUriPrefix.empty() ||
      resourceUrl.empty())
    return "";

  return URIUtils::AddFileToFolder(resourceUriPrefix, CURL::Encode(resourceUrl));
}

void COhUPnPResourceManager::WriteResource(const std::string& uriTail, TIpAddress iface, std::vector<char*>& languageList, OpenHome::Net::IResourceWriter& resourceWriter)
{
  if (uriTail.empty())
  {
    CLog::Log(LOGWARNING, "COhUPnPResourceManager: invalid request from %s", COhUtils::TIpAddressToString(iface).c_str());
    return;
  }

  std::string path;
  const auto& resource = m_smallResources.find(uriTail);
  if (resource == m_smallResources.cend())
  {
    CLog::Log(LOGDEBUG, "COhUPnPResourceManager: request for unknown resource %s from %s", uriTail.c_str(), COhUtils::TIpAddressToString(iface).c_str());
    return;
  }

  // get the actual path
  path = resource->second;

  // open the file
  XFILE::CFile file;
  if (!file.Open(path))
  {
    CLog::Log(LOGDEBUG, "COhUPnPResourceManager: request for non-existing resource %s at %s from %s", uriTail.c_str(), path.c_str(), COhUtils::TIpAddressToString(iface).c_str());
    return;
  }

  // initialize the output
  resourceWriter.WriteResourceBegin(static_cast<uint32_t>(file.GetLength()), file.GetContentMimeType().c_str());

  // read the file and write its data to the output
  uint8_t buffer[1024] = { 0 };
  ssize_t bytesRead = 0;
  while ((bytesRead = file.Read(buffer, sizeof(buffer))) > 0)
    resourceWriter.WriteResource(buffer, bytesRead);

  // finalize the output
  resourceWriter.WriteResourceEnd();
}

std::string COhUPnPResourceManager::CMimeTypeGetter::GetMimeTypeFromExtension(const std::string& extension, const HTTPRequest &request) const
{
  std::string userAgent = HTTPRequestHandlerUtils::GetRequestHeaderValue(request.connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_USER_AGENT);
  std::multimap<std::string, std::string> headerValues;
  HTTPRequestHandlerUtils::GetRequestHeaderValues(request.connection, MHD_HEADER_KIND, headerValues);
  if (!userAgent.empty())
  {
    COhUPnPClientDevice device(userAgent);

    COhUPnPDeviceProfile profile;
    if (COhUPnPDeviceProfilesManager::GetInstance().FindProfile(device, profile))
      return profile.GetMimeType(extension);
  }

  return CDefaultMimeTypeGetter::GetMimeTypeFromExtension(extension, request);
}
