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

#include <algorithm>

#include "ohUPnPTransfer.h"
#include "FileItem.h"
#include "URL.h"
#include "filesystem/UPnPDirectory.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/controlpoints/ohUPnPContentDirectoryControlPoint.h"
#include "utils/log.h"
#include "utils/URIUtils.h"

// TODO: this shouldn't be necessary here
#include "music/tags/MusicInfoTag.h"
#include "video/VideoInfoTag.h"

static bool MediaServerSupportsImport(const COhUPnPControlPointDevice& device)
{
  auto& contentDirectory = COhUPnP::GetInstance().GetContentDirectoryClient();
  return contentDirectory.SupportsCreateObject(device.GetUuid()) &&
         contentDirectory.SupportsImportResource(device.GetUuid());
}

bool COhUPnPTransfer::ImportItemFromMediaServer(const CFileItem& item)
{
  // get the resource-centric version of the item to be imported
  CFileItem extendedItem;
  if (!XFILE::CUPnPDirectory::GetResource(CURL(item.GetPath()), extendedItem))
  {
    CLog::Log(LOGWARNING, "COhUPnPTransfer: ImportItemFromMediaServer(%s) couldn't retrieve details for item \"%s\" to be imported",
      item.GetPath().c_str(), item.GetLabel().c_str());
    return false;
  }

  // make sure that the item is accessible through HTTP GET
  std::string sourceURI = extendedItem.GetPath();
  if (!URIUtils::IsHTTP(sourceURI))
  {
    CLog::Log(LOGWARNING, "COhUPnPTransfer: ImportItemFromMediaServer(%s) cannot import item \"%s\" because it's not accessible over HTTP (%s)",
      item.GetPath().c_str(), item.GetLabel().c_str(), sourceURI.c_str());
    return false;
  }

  // try to transfer the item
  uint32_t transferId;
  if (!COhUPnP::GetInstance().GetTransferManager().Import(COhUPnP::GetInstance().GetContentDirectoryClient().GetLocalDevice(), sourceURI, item, nullptr, transferId))
  {
    CLog::Log(LOGDEBUG, "COhUPnPTransfer: ImportItemFromMediaServer(%s) failed to create transfer for item \"%s\" (%s)",
      item.GetPath().c_str(), item.GetLabel().c_str(), sourceURI.c_str());
    return false;
  }

  CLog::Log(LOGINFO, "COhUPnPTransfer: successfully initiated importing item %s",
    item.GetPath().c_str());
  return true;
}

bool COhUPnPTransfer::SendItemToMediaServer(const COhUPnPDevice& mediaServer, const std::string& containerId, const CFileItem& item)
{
  if (!mediaServer.IsValid() || containerId.empty() || !COhUPnP::GetInstance().IsContentDirectoryClientRunning())
    return false;

  auto& upnp = COhUPnP::GetInstance();
  const auto& contentDirectoryControlPoint = upnp.GetContentDirectoryClient();
  const auto& uuid = mediaServer.GetUuid();

  // TODO: this shouldn't be necessary here
  std::string itemPath = item.GetPath();
  if (item.HasVideoInfoTag())
    itemPath = item.GetVideoInfoTag()->GetPath();
  else if (item.HasMusicInfoTag())
    itemPath = item.GetMusicInfoTag()->GetURL();

  // try to create a matching object on the MediaServer
  std::string objectId, importUri;
  if (!contentDirectoryControlPoint.CreateObject(uuid, containerId, item, objectId, importUri))
  {
    CLog::Log(LOGWARNING, "COhUPnPTransfer: SendItemToMediaServer(%s, %s, %s) couldn't create a matching object on the MediaServer",
      uuid.c_str(), containerId.c_str(), itemPath.c_str());
    return false;
  }

  // put together the URI through which the MediaServer can import/download the item
  const std::string sourceUri = COhUPnPResourceManager::GetFullResourceUri(upnp.GetResourceUriPrefix(), itemPath);

  // try to tell the MediaServer to import the item through the source URI
  uint32_t transferId;
  if (!contentDirectoryControlPoint.ImportResource(uuid, sourceUri, importUri, transferId))
  {
    CLog::Log(LOGWARNING, "COhUPnPTransfer: SendItemToMediaServer(%s, %s, %s) couldn't tell the MediaServer to import the item",
      uuid.c_str(), containerId.c_str(), itemPath.c_str());
    return false;
  }

  // add the transfer to the transfer manager
  if (!upnp.GetTransferManager().Export(transferId, mediaServer, sourceUri, item, nullptr))
  {
    CLog::Log(LOGWARNING, "COhUPnPTransfer: SendItemToMediaServer(%s, %s, %s) couldn't add the export to the MediaServer to the transfer manager",
      uuid.c_str(), containerId.c_str(), itemPath.c_str());
    return false;
  }

  CLog::Log(LOGINFO, "COhUPnPTransfer: successfully initiated sending item %s to MediaServer \"%s\" (%s)",
    itemPath.c_str(), mediaServer.GetFriendlyName().c_str(), uuid.c_str());
  return true;
}

bool COhUPnPTransfer::HasMediaServersSupportingImport()
{
  const auto& devices = COhUPnP::GetInstance().GetContentDirectoryClient().GetDevices();

  return std::any_of(devices.cbegin(), devices.cend(), MediaServerSupportsImport);
}

std::vector<COhUPnPControlPointDevice> COhUPnPTransfer::GetMediaServersSupportingImport()
{
  const auto& devices = COhUPnP::GetInstance().GetContentDirectoryClient().GetDevices();
  std::vector<COhUPnPControlPointDevice> result(devices.size());

  auto it = std::copy_if(devices.cbegin(), devices.cend(), result.begin(), MediaServerSupportsImport);
  result.resize(std::distance(result.begin(), it));

  return result;
}
