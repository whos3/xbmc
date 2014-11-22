/*
 *      Copyright (C) 2014 Team XBMC
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

#include "MusicImportHandler.h"
#include "FileItem.h"
#include "music/MusicDatabase.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

std::string CMusicImportHandler::GetItemLabel(const CFileItem* item) const
{
  if (item == NULL)
    return "";

  return item->GetLabel();
}

bool CMusicImportHandler::GetLocalItems(const CMediaImport &import, CFileItemList& items)
{
  if (!m_db.Open())
    return false;

  bool result = GetLocalItems(m_db, import, items);

  m_db.Close();
  return result;
}

bool CMusicImportHandler::StartChangeset(const CMediaImport &import)
{
  // start the background loader if necessary
  if (import.GetSettings().UpdateImportedMediaItems())
    m_thumbLoader.OnLoaderStart();

  return true;
}

bool CMusicImportHandler::FinishChangeset(const CMediaImport &import)
{
  // stop the background loader if necessary
  if (import.GetSettings().UpdateImportedMediaItems())
    m_thumbLoader.OnLoaderFinish();

  return true;
}

MediaImportChangesetType CMusicImportHandler::DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemList& localItems)
{
  if (item == NULL || !item->HasMusicInfoTag())
    return MediaImportChangesetTypeNone;

  CFileItemPtr localItem = FindMatchingLocalItem(item, localItems);
  if (localItem == NULL)
    return MediaImportChangesetTypeAdded;

  // remove the matching item from the local list so that the imported item is not considered non-existant
  localItems.Remove(localItem.get());

  const CMediaImportSettings& settings = import.GetSettings();

  // nothing to do if we don't need to update imported media items
  if (!settings.UpdateImportedMediaItems())
    return MediaImportChangesetTypeNone;

  // retrieve all details for the previously imported item
  if (!m_thumbLoader.LoadItem(localItem.get()))
    CLog::Log(LOGWARNING, "Failed to retrieve details for local item %s during media importing", localItem->GetMusicInfoTag()->GetURL().c_str());

  // compare the previously imported item with the newly imported item
  if (Compare(localItem.get(), item))
    return MediaImportChangesetTypeNone;

  // the newly imported item has changed from the previously imported one so get some information from the local item as preparation
  PrepareExistingItem(item, localItem.get());

  return MediaImportChangesetTypeChanged;
}

bool CMusicImportHandler::StartSynchronisation(const CMediaImport &import)
{
  if (!m_db.Open())
    return false;

  m_db.BeginTransaction();
  return true;
}

bool CMusicImportHandler::FinishSynchronisation(const CMediaImport &import)
{
  if (!m_db.IsOpen())
    return false;

  // now make sure the items are enabled
  SetImportedItemsEnabled(import, true);

  m_db.CommitTransaction();
  m_db.Close();
  return true;
}

bool CMusicImportHandler::RemoveImportedItems(const CMediaImport &import)
{
  if (!m_db.IsOpen())
    return false;

  // get all imported items
  CFileItemList items;
  if (!GetLocalItems(m_db, import, items))
  {
    m_db.Close();
    return false;
  }

  m_db.BeginTransaction();

  for (int index = 0; index < items.Size(); ++index)
    RemoveImportedItem(m_db, import, items.Get(index).get());

  m_db.CommitTransaction();

  m_db.Close();
  return true;
}

void CMusicImportHandler::SetImportedItemsEnabled(const CMediaImport &import, bool enable)
{
  CMusicDatabase musicdb;
  if (!musicdb.Open())
    return;

  musicdb.SetImportItemsEnabled(enable, import);
}

void CMusicImportHandler::PrepareItem(const CMediaImport &import, CFileItem* pItem)
{
  if (pItem == NULL || !pItem->HasMusicInfoTag() ||
      import.GetPath().empty() || import.GetSource().GetIdentifier().empty())
    return;

  const std::string &sourceID = import.GetSource().GetIdentifier();
  m_db.AddPath(sourceID);
  int idPath = m_db.AddPath(import.GetPath());

  // set the proper source
  pItem->SetSource(sourceID);
  pItem->SetImportPath(import.GetPath());
}

void CMusicImportHandler::PrepareExistingItem(CFileItem *updatedItem, const CFileItem *originalItem)
{
  if (updatedItem == NULL || originalItem == NULL ||
      !updatedItem->HasMusicInfoTag() || !originalItem->HasMusicInfoTag())
    return;

  updatedItem->GetMusicInfoTag()->SetDatabaseId(originalItem->GetMusicInfoTag()->GetDatabaseId(), originalItem->GetMusicInfoTag()->GetType());
  updatedItem->GetMusicInfoTag()->SetAlbumId(originalItem->GetMusicInfoTag()->GetAlbumId());

  updatedItem->SetSource(originalItem->GetSource());
  updatedItem->SetImportPath(originalItem->GetImportPath());
}

void CMusicImportHandler::SetDetailsForFile(const CFileItem *pItem)
{
  m_db.SetPlayCount(*pItem, pItem->GetMusicInfoTag()->GetPlayCount(), pItem->GetMusicInfoTag()->GetLastPlayed());
}

bool CMusicImportHandler::SetImportForItem(const CFileItem *pItem, const CMediaImport &import)
{
  return m_db.SetImportForItem(pItem->GetMusicInfoTag()->GetDatabaseId(), import);
}

CDatabase::Filter CMusicImportHandler::GetFilter(const CMediaImport &import, bool enabledItems /* = false */)
{
  std::string strWhere;
  if (!import.GetPath().empty())
    strWhere += StringUtils::Format("importPath = '%s'", import.GetPath().c_str());

  return CDatabase::Filter(strWhere);
}

bool CMusicImportHandler::Compare(const CFileItem *originalItem, const CFileItem *newItem)
{
  if (originalItem == NULL || !originalItem->HasMusicInfoTag() ||
      newItem == NULL || !newItem->HasMusicInfoTag())
    return false;

  /* TODO
  if (originalItem->GetArt() != newItem->GetArt())
    return false;
  */

  /* TODO
  if (originalItem->GetVideoInfoTag()->Equals(*newItem->GetVideoInfoTag(), true))
    return true;

  std::set<Field> differences;
  if (!originalItem->GetVideoInfoTag()->GetDifferences(*newItem->GetVideoInfoTag(), differences, true))
    return true;

  return differences.empty();
  */

  return true;
}
