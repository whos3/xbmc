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

#include "MediaImportHelper.h"

#include "URL.h"
#include "addons/AddonManager.h"
#include "dialogs/GUIDialogSelect.h"
#include "guilib/GUIWindowManager.h"
#include "media/import/MediaImportManager.h"
#include "music/tags/MusicInfoTag.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "video/VideoInfoTag.h"

bool CMediaImportHelper::ImportItem(const std::string& importPath, const CFileItemPtr& item)
{
  if (importPath.empty() || item == nullptr)
    return false;

  CMediaImportManager& manager = CMediaImportManager::GetInstance();
  // nothing to do if we can't import the item at all
  if (!manager.CanImport(importPath))
    return false;

  // if the item hasn't been imported yet, we need to add it
  if (!manager.IsImported(importPath))
  {
    std::string sourceIdentifier;
    if (!GetSourceIdentifier(importPath, sourceIdentifier))
      return false;

    // extract the source and import information from the item
    CMediaImportSource source(sourceIdentifier);
    CMediaImport import(importPath, GroupedMediaTypes(), source);
    if (!GetSourceAndImport(importPath, item, sourceIdentifier, source, import))
      return false;

    // show the select dialog with all the media types available for import
    CGUIDialogSelect* selectDialog = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
    if (selectDialog == nullptr)
      return false;

    selectDialog->Reset();
    selectDialog->SetHeading(39005);
    selectDialog->SetMultiSelection(false);

    std::vector<GroupedMediaTypes> groupedMediaTypes = manager.GetSupportedMediaTypesGrouped(manager.GetSupportedMediaTypes());
    CFileItemList mediaTypeItems;
    for (const auto& supportedMediaType : groupedMediaTypes)
    {
      CFileItemPtr mediaTypeItem(new CFileItem(MediaTypesToLabel(supportedMediaType)));
      mediaTypeItem->SetPath(CMediaTypes::Join(supportedMediaType)); // abuse the path for the media type's identification

      if (import.GetMediaTypes() == supportedMediaType)
        mediaTypeItem->Select(true);

      mediaTypeItems.Add(mediaTypeItem);
    }

    selectDialog->SetItems(mediaTypeItems);
    selectDialog->Open();

    if (!selectDialog->IsConfirmed() || selectDialog->GetSelectedFileItem() == nullptr)
      return false;

    import.SetMediaTypes(StringUtils::Split(selectDialog->GetSelectedFileItem()->GetPath(), ","));
    if (source.GetAvailableMediaTypes().empty())
      source.SetAvailableMediaTypes(MediaTypes(import.GetMediaTypes().begin(), import.GetMediaTypes().end()));

    // check if the source already exists and register it if not
    if (!manager.GetSource(sourceIdentifier, source))
      manager.RegisterSourceSync(sourceIdentifier, source.GetFriendlyName(), source.GetIconUrl(), source.GetAvailableMediaTypes());

    // register the import
    if (!manager.AddImport(sourceIdentifier, import.GetPath(), import.GetMediaTypes(), import.IsRecursive()))
      return false;
  }

  return SynchroniseItem(importPath, item);
}

bool CMediaImportHelper::SynchroniseItem(const std::string& importPath, const CFileItemPtr& item)
{
  if (importPath.empty() || item == nullptr)
    return false;

  CMediaImportManager& manager = CMediaImportManager::GetInstance();
  // we can only synchronise already imported items/paths
  if (!manager.IsImported(importPath))
    return false;

  std::string sourceIdentifier;
  if (!GetSourceIdentifier(importPath, sourceIdentifier))
    return false;

  // extract the source and import information from the item
  CMediaImportSource source(sourceIdentifier);
  CMediaImport import(importPath, GroupedMediaTypes(), source);
  if (!GetSourceAndImport(importPath, item, sourceIdentifier, source, import))
    return false;

  auto& imports = manager.GetImportsByPath(importPath, false);
  if (imports.size() != 1)
    return false;

  import = imports.front();
  return manager.Import(import.GetPath(), import.GetMediaTypes());
}

bool CMediaImportHelper::GetSourceIdentifier(const std::string& importPath, std::string& sourceIdentifier)
{
  const MediaImporterConstPtr importer = CMediaImportManager::GetInstance().GetImporter(importPath);
  if (importer == nullptr)
    return false;

  return importer->GetSourceIdentifier(importPath, sourceIdentifier);
}

bool CMediaImportHelper::GetSourceAndImport(const std::string& importPath, const CFileItemPtr& item, const std::string& sourceIdentifier, CMediaImportSource& source, CMediaImport& import)
{
  if (URIUtils::IsPlugin(importPath))
  {
    std::string addonId = CURL(sourceIdentifier).GetHostName();
    ADDON::AddonPtr plugin;
    if (!ADDON::CAddonMgr::GetInstance().GetAddon(addonId, plugin, ADDON::ADDON_PLUGIN) || plugin == nullptr)
      return false;

    source.SetFriendlyName(plugin->Name());
    source.SetIconUrl(plugin->Icon());

    MediaType mediaType;
    if (item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_type.empty())
      mediaType = item->GetVideoInfoTag()->m_type;
    else if (item->HasMusicInfoTag() && !item->GetMusicInfoTag()->GetType().empty())
      mediaType = item->GetMusicInfoTag()->GetType();

    if (!mediaType.empty())
    {
      import.SetMediaTypes(CMediaImportManager::GetInstance().GetGroupedMediaTypes(mediaType));
      source.SetAvailableMediaTypes(MediaTypes(import.GetMediaTypes().begin(), import.GetMediaTypes().end()));
    }
  }
  else
    return false;

  import.SetSource(source);

  // if the item is a folder import recursively otherwise import selectively
  import.SetRecursive(item->m_bIsFolder);
  return true;
}

bool CMediaImportHelper::RegisterSource(const std::string& importPath, const CFileItemPtr& item, const std::string& sourceIdentifier)
{
  CMediaImportSource source(sourceIdentifier);
  CMediaImport import(importPath, GroupedMediaTypes(), source);
  if (!GetSourceAndImport(importPath, item, sourceIdentifier, source, import))
    return false;

  CMediaImportManager::GetInstance().RegisterSource(sourceIdentifier, source.GetFriendlyName(), source.GetIconUrl(), source.GetAvailableMediaTypes());

  return true;
}
