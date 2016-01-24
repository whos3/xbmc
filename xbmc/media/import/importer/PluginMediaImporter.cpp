/*
 *      Copyright (C) 2015 Team XBMC
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

#include "PluginMediaImporter.h"

#include "FileItem.h"
#include "URL.h"
#include "addons/AddonManager.h"
#include "filesystem/Directory.h"
#include "media/import/MediaImport.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "music/tags/MusicInfoTag.h"
#include "utils/URIUtils.h"
#include "video/VideoInfoTag.h"

CPluginMediaImporter::CPluginMediaImporter()
  : IMediaImporter(CMediaImport("", { MediaTypeNone }))
{ }

CPluginMediaImporter::CPluginMediaImporter(const CMediaImport &import)
  : IMediaImporter(import)
{ }

CPluginMediaImporter::~CPluginMediaImporter()
{ }

bool CPluginMediaImporter::CanImport(const std::string& path) const
{
  // extract the addon/plugin identifier
  std::string addonId;
  if (!getAddonId(path, addonId))
    return false;

  // can't import from non-existing plugins
  ADDON::AddonPtr plugin;
  if (!ADDON::CAddonMgr::GetInstance().GetAddon(addonId, plugin, ADDON::ADDON_PLUGIN) || plugin == nullptr)
    return false;

  return true;
}

bool CPluginMediaImporter::GetSourceIdentifier(const std::string& path, std::string& sourceIdentifier) const
{
  // can't import non-plugin:// paths
  if (path.empty() || !URIUtils::IsPlugin(path))
    return false;

  std::string addonId;
  if (!getAddonId(path, addonId))
    return false;

  CURL url;
  url.SetProtocol("plugin");
  url.SetHostName(addonId);
  sourceIdentifier = url.Get();

  return true;
}

IMediaImporter* CPluginMediaImporter::Create(const CMediaImport &import) const
{
  if (!CanImport(import.GetPath()))
    return NULL;

  return new CPluginMediaImporter(import);
}

bool CPluginMediaImporter::Import(CMediaImportRetrievalTask* task) const
{
  if (task == NULL || !CanImport(GetPath()))
    return false;

  CFileItemList items;
  if (!XFILE::CDirectory::GetDirectory(GetPath(), items, "", XFILE::DIR_FLAG_BYPASS_CACHE, false))
    return false;

  const auto& mediaTypes = GetImport().GetMediaTypes();
  for (const auto& importedMediaType : mediaTypes)
  {
    std::vector<CFileItemPtr> itemList;
    for (int i = 0; i < items.Size(); )
    {
      CFileItemPtr item = items.Get(i);

      // get the media type of the item
      MediaType mediaType;
      if (item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_type.empty())
        mediaType = item->GetVideoInfoTag()->m_type;
      else if (item->HasMusicInfoTag() && !item->GetMusicInfoTag()->GetType().empty())
        mediaType = item->GetMusicInfoTag()->GetType();
      else if (!GetImport().IsRecursive() && mediaTypes.size() == 1)
      {
        // for non-recursive imports with a single media type, force it
        mediaType = mediaTypes.front();

        // fill in some obvious values into the info tags
        if (mediaType == MediaTypeVideo || mediaType == MediaTypeVideoCollection ||
            mediaType == MediaTypeMovie || mediaType == MediaTypeMusicVideo ||
            mediaType == MediaTypeTvShow || mediaType == MediaTypeSeason ||
            mediaType == MediaTypeEpisode)
        {
          item->GetVideoInfoTag()->m_type = mediaType;
          item->GetVideoInfoTag()->m_strTitle = item->GetLabel();
          item->GetVideoInfoTag()->m_strFileNameAndPath = item->GetPath();
        }
        else if (mediaType == MediaTypeMusic || mediaType == MediaTypeArtist ||
                 mediaType == MediaTypeAlbum || mediaType == MediaTypeSong)
        {
          item->GetMusicInfoTag()->SetDatabaseId(-1, mediaType);
          item->GetMusicInfoTag()->SetTitle(item->GetLabel());
          item->GetMusicInfoTag()->SetURL(item->GetPath());
        }
      }

      // ingore items that don't have a media type or none we are interested in
      if (mediaType.empty() || std::find(mediaTypes.begin(), mediaTypes.end(), mediaType) == mediaTypes.end())
      {
        items.Remove(i);
        continue;
      }

      // check if the media type matches the one we are interested in right now
      if (mediaType == importedMediaType)
      {
        itemList.push_back(item);
        items.Remove(i);
        continue;
      }

      ++i;
    }

    // if we do selective importing, remove any item that hasn't already been imported before
    // but only if we have already imported the items once
    if (!GetImport().IsRecursive() && GetImport().GetLastSynced().IsValid())
    {
      const auto& localItems = task->GetLocalItems(importedMediaType);

      for (auto& item = itemList.begin(); item != itemList.end();)
      {
        const auto& itLocalItem = std::find_if(localItems.begin(), localItems.end(),
          [&item](CFileItemPtr localItem) { return localItem->IsSamePath((*item).get()); });

        if (itLocalItem == localItems.end())
        {
          itemList.erase(item);
          continue;
        }

        ++item;
      }
    }

    task->AddItems(itemList, importedMediaType, MediaImportChangesetTypeNone);
  }

  return true;
}

bool CPluginMediaImporter::getAddonId(const std::string& path, std::string& addonId)
{
  // can't import non-plugin:// paths
  if (path.empty() || !URIUtils::IsPlugin(path))
    return false;

  CURL url(path);
  addonId = url.GetHostName();

  return true;
}
