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

#include "AlbumImportHandler.h"
#include "FileItem.h"
#include "utils/StringUtils.h"

/*!
 * Checks whether two albums are the same by comparing them by title and album artist
 */
static bool IsSameAlbum(const MUSIC_INFO::CMusicInfoTag& left, const MUSIC_INFO::CMusicInfoTag& right)
{
  return left.GetAlbum()      == right.GetAlbum()
    && left.GetAlbumArtist()  == right.GetAlbumArtist();
}

std::set<MediaType> CAlbumImportHandler::GetDependencies() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeArtist);
  return types;
}

std::set<MediaType> CAlbumImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeSong);
  return types;
}

std::vector<MediaType> CAlbumImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeArtist);
  types.push_back(MediaTypeAlbum);
  types.push_back(MediaTypeSong);
  return types;
}

std::string CAlbumImportHandler::GetItemLabel(const CFileItem* item) const
{
  if (item != NULL && item->HasMusicInfoTag() && !item->GetMusicInfoTag()->GetAlbum().empty())
  {
    return StringUtils::Format(g_localizeStrings.Get(39015).c_str(),
      item->GetMusicInfoTag()->GetAlbumArtist().at(0).c_str(), // TODO
      item->GetMusicInfoTag()->GetAlbum().c_str());
  }

  return CMusicImportHandler::GetItemLabel(item);
}

bool CAlbumImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL)
    return false;

  PrepareItem(import, item);

  CAlbum album(*item);
  if (!m_db.AddAlbum(album))
    return false;

  item->SetFromAlbum(album);
  return SetImportForItem(item, import);
}

bool CAlbumImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL || !item->HasMusicInfoTag())
    return false;

  CAlbum album(*item);
  return m_db.UpdateAlbum(album);
}

bool CAlbumImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == NULL || !item->HasMusicInfoTag())
    return false;

  // TODO

  return false;
}

bool CAlbumImportHandler::CleanupImportedItems(const CMediaImport &import)
{
  // TODO

  return true;
}

bool CAlbumImportHandler::GetLocalItems(CMusicDatabase &musicdb, const CMediaImport &import, CFileItemList& items)
{
  return musicdb.GetAlbumsByWhere("musicdb://albums/", GetFilter(import), items);
}

CFileItemPtr CAlbumImportHandler::FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems)
{
  for (int i = 0; i < localItems.Size(); ++i)
  {
    CFileItemPtr localItem = localItems.Get(i);
    if (IsSameAlbum(*item->GetMusicInfoTag(), *localItem->GetMusicInfoTag()))
      return localItem;
  }

  return CFileItemPtr();
}

void CAlbumImportHandler::RemoveImportedItem(CMusicDatabase &musicdb, const CMediaImport &import, const CFileItem* item)
{
  // TODO
}
