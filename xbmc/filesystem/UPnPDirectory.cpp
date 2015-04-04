/*
 * UPnP Support for XBMC
 *      Copyright (c) 2006 c0diq (Sylvain Rebaud)
 *      Portions Copyright (c) by the authors of libPlatinum
 *      http://www.plutinosoft.com/blog/category/platinum/
 *
 *      Copyright (C) 2010-2013 Team XBMC
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
#include <Platinum/Source/Platinum/Platinum.h>
#include <Platinum/Source/Devices/MediaServer/PltSyncMediaBrowser.h>

#include "UPnPDirectory.h"
#include "URL.h"
#include "media/MediaType.h"
#include "network/upnp/UPnP.h"
#include "network/upnp/UPnPInternal.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/controlpoints/ohUPnPContentDirectoryControlPoint.h"
#include "FileItem.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"

using namespace MUSIC_INFO;
using namespace XFILE;
using namespace UPNP;

namespace XFILE
{

static bool FindDeviceWait(const std::string &uuid, COhUPnPControlPointDevice& device, int timeoutSeconds = 5)
{
  if (timeoutSeconds <= 0)
    timeoutSeconds = 5;

  bool clientStarted = COhUPnP::GetInstance().IsContentDirectoryClientRunning();
  if (!clientStarted && !COhUPnP::GetInstance().StartContentDirectoryClient())
    return false;

  const COhUPnPContentDirectoryControlPoint& client = COhUPnP::GetInstance().GetContentDirectoryClient();

  CDateTime endTime = CDateTime::GetCurrentDateTime() + CDateTimeSpan(0, 0, 0, timeoutSeconds);
  while (endTime > CDateTime::GetCurrentDateTime())
  {
    if (client.GetDevice(uuid, device))
      break;

    // fail right away if device not found and upnp client was already running
    if (clientStarted)
      return false;

    // sleep for 500 millisecond
    Sleep(500);
  }

  return device.IsValid();
}

/*----------------------------------------------------------------------
|   CUPnPDirectory::GetFriendlyName
+---------------------------------------------------------------------*/
const char*
CUPnPDirectory::GetFriendlyName(const CURL& url)
{
  std::string strUrl = url.Get();
  if (!URIUtils::IsUPnP(strUrl))
    return nullptr;

  if (StringUtils::EqualsNoCase(strUrl, "upnp://"))
    return "UPnP Media Servers (Auto-Discover)"; // TODO: localizable
    
  std::string path = strUrl;
  URIUtils::AddSlashAtEnd(path);
  CURL upnpUrl(path);
  if (upnpUrl.GetHostName().empty())
    return nullptr;

  COhUPnPControlPointDevice device;
  if (!FindDeviceWait(upnpUrl.GetHostName(), device) ||
      !device.IsValid())
    return nullptr;

  return device.GetFriendlyName().c_str();
}

/*----------------------------------------------------------------------
|   CUPnPDirectory::GetResource
+---------------------------------------------------------------------*/
bool CUPnPDirectory::GetResource(const CURL& path, CFileItem &item)
{
  if(!path.IsProtocol("upnp"))
    return false;

  std::string uuid = path.GetHostName();
  std::string objectId = path.GetFileName();
  URIUtils::RemoveSlashAtEnd(objectId);
  objectId = CURL::Decode(objectId);

  COhUPnPControlPointDevice device;
  if (!FindDeviceWait(uuid, device) ||
      !device.IsValid())
  {
    CLog::Log(LOGERROR, "CUPnPDirectory::GetResource: unable to find uuid %s", uuid.c_str());
    return false;
  }

  if (!COhUPnP::GetInstance().GetContentDirectoryClient().BrowseMetadataSync(uuid, objectId, item))
  {
    CLog::Log(LOGERROR, "CUPnPDirectory::GetResource: unable to get metadata for object %s", objectId.c_str());
    return false;
  }

  return true;
}


/*----------------------------------------------------------------------
|   CUPnPDirectory::GetDirectory
+---------------------------------------------------------------------*/
bool
CUPnPDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
  if (!COhUPnP::GetInstance().IsContentDirectoryClientRunning())
    return false;

  const COhUPnPContentDirectoryControlPoint &client = COhUPnP::GetInstance().GetContentDirectoryClient();

  // list all available ContentDirectory services
  if (url.GetHostName().empty())
  {
    // get the list of devices
    std::vector<COhUPnPControlPointDevice> services = client.GetDevices();
    for (const auto& service : services)
    {
      CFileItemPtr item(new CFileItem("upnp://" + service.GetUuid() + "/", true));
      item->SetLabel(service.GetFriendlyName());
      item->SetArt("thumb", service.GetIconUrl());

      items.Add(item);
    }

    return true;
  }

  std::string uuid = url.GetHostName();
  std::string objectId = url.GetFileName();
  URIUtils::RemoveSlashAtEnd(objectId);

  if (!objectId.empty())
    objectId = CURL::Decode(objectId);

  COhUPnPControlPointDevice device;
  if (!FindDeviceWait(uuid, device) ||
      !device.IsValid())
    return false;

  // if the object identifier is empty, use "0" for the root listing
  if (objectId.empty())
    objectId = "0";

  /* TODO
  // just a guess as to what types of files we want
  bool video = true;
  bool audio = true;
  bool image = true;
  StringUtils::TrimLeft(m_strFileMask, "/");
  if (!m_strFileMask.empty()) {
    video = m_strFileMask.find(".wmv") != std::string::npos;
    audio = m_strFileMask.find(".wma") != std::string::npos;
    image = m_strFileMask.find(".jpg") != std::string::npos;
  }

  // special case for Windows Media Connect and WMP11 when looking for root
  // We can target which root subfolder we want based on directory mask
  if (object_id == "0" &&
     (device->GetFriendlyName().Find("Windows Media Connect", 0, true) >= 0 || device->m_ModelName == "Windows Media Player Sharing")) {
    // look for a specific type to differentiate which folder we want
    if (audio && !video && !image) {
      // music
      object_id = "1";
    } else if (!audio && video && !image) {
      // video
      object_id = "2";
    } else if (!audio && !video && image) {
      // pictures
      object_id = "3";
    }
  }
  */

  if (!client.BrowseSync(uuid, objectId, items))
    return false;

  // an empty list is ok
  if (items.IsEmpty())
    return true;

  /* TODO
  PLT_MediaObjectList::Iterator entry = list->GetFirstItem();
  while (entry) {
    // disregard items with wrong class/type
    if( (!video && (*entry)->m_ObjectClass.type.CompareN("object.item.videoitem", 21,true) == 0)
     || (!audio && (*entry)->m_ObjectClass.type.CompareN("object.item.audioitem", 21,true) == 0)
     || (!image && (*entry)->m_ObjectClass.type.CompareN("object.item.imageitem", 21,true) == 0) )
    {
      ++entry;
      continue;
    }

    // never show empty containers in media views
    if((*entry)->IsContainer()) {
      if((audio || video || image) && ((PLT_MediaContainer*)(*entry))->m_ChildrenCount == 0) {
        ++entry;
        continue;
      }
    }
  }
  */

  for (int i = 0; i < items.Size(); ++i)
  {
    CFileItemPtr item = items[i];

    // put together the path of the item
    std::string id = CURL::Encode(item->GetPath());
    URIUtils::AddSlashAtEnd(id);
    item->SetPath("upnp://" + uuid + "/" + id);
  }

  std::string content = items.GetContent();
  if (content.empty())
  {
    content = "files";

    // TODO: what is this doing here?
    items.AddSortMethod(SortByNone, 571, LABEL_MASKS("%L", "%I", "%L", ""));
    items.AddSortMethod(SortByLabel, SortAttributeIgnoreFolders, 551, LABEL_MASKS("%L", "%I", "%L", ""));
    items.AddSortMethod(SortBySize, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));
    items.AddSortMethod(SortByDate, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));
  }
  else
    content = CMediaTypes::ToPlural(content);
  items.SetContent(content);

  return true;
}

}
