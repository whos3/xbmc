/*
 *      Copyright (C) 2013 Team XBMC
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

#include "UPnPImporter.h"
#include "URL.h"
#include "Platinum.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "network/upnp/UPnP.h"
#include "network/upnp/UPnPInternal.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoInfoTag.h"

using namespace UPNP;

#define UPNP_ROOT_CONTAINER_ID "0"

typedef struct {
  MediaType mediaType;
  const char* objectIdentification;
  const char* label; // TODO: localization
} SupportedMediaType;

static SupportedMediaType SupportedMediaTypes[] = {
  { MediaTypeMovie,       "object.item.videoItem.movie",                            "movies" },
  { MediaTypeTvShow,      "object.container.album.videoAlbum.videoBroadcastShow",   "tvshows" },
  { MediaTypeSeason,      "object.container.album.videoAlbum.videoBroadcastSeason", "seasons" },
  { MediaTypeEpisode,     "object.item.videoItem.videoBroadcast",                   "episodes" },
  { MediaTypeMusicVideo,  "object.item.videoItem.musicVideoClip",                   "musicvideos" },
  //{ MediaTypeArtist,      "object.container.person.musicArtist",                    "artists" },
  //{ MediaTypeAlbum,       "object.container.album.musicAlbum",                      "albums" },
  //{ MediaTypeSong,        "object.item.audioItem",                                  "songs" }
};

#define SupportedMediaTypesSize sizeof(SupportedMediaTypes) / sizeof(SupportedMediaType)

static bool FindServer(const std::string deviceUUID, PLT_DeviceDataReference &device)
{
  if (deviceUUID.empty())
    return false;

  return CUPnP::GetInstance()->m_MediaBrowser->FindServer(deviceUUID.c_str(), device) == NPT_SUCCESS;
}

static CFileItemPtr ConstructItem(PLT_DeviceDataReference& device, PLT_MediaObject* object)
{
  CFileItemPtr pItem = BuildObject(object);
  CStdString id;
  if (object->m_ReferenceID.IsEmpty())
    id = (const char*)object->m_ObjectID;
  else
    id = (const char*)object->m_ReferenceID;

  CURL::Encode(id);
  URIUtils::AddSlashAtEnd(id);
  pItem->SetPath("upnp://" + CStdString(device->GetUUID()) + "/" + id);
  pItem->GetVideoInfoTag()->m_strPath = pItem->GetVideoInfoTag()->m_strFileNameAndPath = pItem->GetPath();
  
  return pItem;
}

static void ConstructList(PLT_DeviceDataReference& device, PLT_MediaObjectListReference& list, std::vector<CFileItemPtr>& items)
{
  if (list.IsNull())
    return;

  for (PLT_MediaObjectList::Iterator entry = list->GetFirstItem() ; entry ; ++entry)
  {
    CFileItemPtr item(ConstructItem(device, *entry));
    if (item != NULL)
      items.push_back(item);
  }
}

static bool Search(PLT_DeviceDataReference& device, const std::string& search_criteria, std::vector<CFileItemPtr>& items)
{
  PLT_SyncMediaBrowser *mediaBrowser = CUPnP::GetInstance()->m_MediaBrowser;
  if (mediaBrowser == NULL)
    return false;

  PLT_MediaObjectListReference list;
  if (mediaBrowser->SearchSync(device, UPNP_ROOT_CONTAINER_ID, search_criteria.c_str(), list) == NPT_SUCCESS)
  {
    ConstructList(device, list, items);
    return true;
  }

  return false;
}

static bool Search(CMediaImportRetrievalTask* task, PLT_DeviceDataReference& device, const MediaType& mediaType, std::vector<CFileItemPtr>& items)
{
  SupportedMediaType* supportedMediaType = NULL;
  for (size_t i = 0; i < SupportedMediaTypesSize; i++)
  {
    if (mediaType == SupportedMediaTypes[i].mediaType)
    {
      supportedMediaType = &SupportedMediaTypes[i];
      break;
    }
  }

  if (supportedMediaType == NULL)
    return false;

  task->GetProgressBarHandle()->SetText(StringUtils::Format("Fetching %s", supportedMediaType->label)); // TODO: localization
  if (!Search(device, StringUtils::Format("upnp:class = \"%s\"", supportedMediaType->objectIdentification), items))
    return false;

  if (mediaType == MediaTypeTvShow || mediaType == MediaTypeSeason)
  {
    std::vector<CFileItemPtr> showItems;
    for (std::vector<CFileItemPtr>::iterator it = items.begin() ; it != items.end() ; ++it)
    { // discard video albums that are NOT tv shows
      if ((*it)->HasVideoInfoTag() && (*it)->GetVideoInfoTag()->m_type == mediaType)
        showItems.push_back(*it);
    }

    items.clear();
    items.assign(showItems.begin(), showItems.end());
  }

  return true;
}

CUPnPImporter::CUPnPImporter()
  : IMediaImporter(CMediaImport(""))
{ }

CUPnPImporter::CUPnPImporter(const CMediaImport &import)
  : IMediaImporter(import)
{
  CURL url(import.GetPath());
  if (url.GetProtocol() == "upnp")
    m_deviceUUID = url.GetHostName();
}

IMediaImporter* CUPnPImporter::Create(const CMediaImport &import) const
{
  if (!CanImport(import.GetPath()))
    return NULL;

  return new CUPnPImporter(import);
}

bool CUPnPImporter::CanImport(const std::string& path) const
{
  PLT_DeviceDataReference device;
  if (!validatePath(path, device))
    return false;

  if (!CSettings::Get().GetBool("services.upnpimportcompatibleonly"))
    return true;

  PLT_SyncMediaBrowser *mediaBrowser = CUPnP::GetInstance()->m_MediaBrowser;
  if (mediaBrowser == NULL)
    return false;

  NPT_String searchCapabilities;
  if (NPT_FAILED(mediaBrowser->GetSearchCapabilitiesSync(device, searchCapabilities)))
    return false;

  return !searchCapabilities.IsEmpty() && searchCapabilities.Find("upnp:class") >= 0;
}

bool CUPnPImporter::Import(CMediaImportRetrievalTask* task) const
{
  PLT_DeviceDataReference device;
  if (!FindServer(m_deviceUUID, device))
  {
    CLog::Log(LOGINFO, "CUPnPImporter: unable to import media items from unavailable source %s", m_deviceUUID.c_str());
    return false;
  }

  std::set<MediaType> handledMediaTypes;
  const std::set<MediaType> &importedMediaTypes = GetImport().GetImportedMediaTypes();
  for (size_t i = 0; i < SupportedMediaTypesSize; i++)
  {
    if (importedMediaTypes.find(SupportedMediaTypes[i].mediaType) != importedMediaTypes.end())
      handledMediaTypes.insert(SupportedMediaTypes[i].mediaType);
  }

  size_t current = 0;
  size_t total = handledMediaTypes.size();

  bool success = false;
  for (std::set<MediaType>::const_iterator itMediaType = handledMediaTypes.begin(); itMediaType != handledMediaTypes.end(); ++itMediaType)
  {
    if (task->ShouldCancel(current, total))
      return false;

    current++;

    std::vector<CFileItemPtr> items;
    if (!Search(task, device, *itMediaType, items))
      continue;

    // remove any items that are not part of the requested path
    for (std::vector<CFileItemPtr>::const_iterator itItem = items.begin(); itItem != items.end(); )
    {
      if (!URIUtils::IsInPath((*itItem)->GetPath(), GetPath()))
        itItem = items.erase(itItem);
      else
        ++itItem;
    }

    success = true;
    task->SetItems(*itMediaType, items);
  }

  return success;
}

bool CUPnPImporter::validatePath(const std::string& path, PLT_DeviceDataReference &device)
{
  CURL url(path);
  if (path.empty() || url.GetProtocol() != "upnp")
    return false;

  std::string deviceUUID = url.GetHostName();
  return FindServer(deviceUUID, device);
}
