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

#include "ArtistImportHandler.h"
#include "FileItem.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"

std::set<MediaType> CArtistImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeSong);
  return types;
}

std::vector<MediaType> CArtistImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeArtist);
  types.push_back(MediaTypeAlbum);
  types.push_back(MediaTypeSong);
  return types;
}

bool CArtistImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL)
    return false;

  PrepareItem(import, item);

  std::string mbArtistId;
  if (!item->GetMusicInfoTag()->GetMusicBrainzArtistID().empty())
    mbArtistId = item->GetMusicInfoTag()->GetMusicBrainzArtistID().at(0);

  item->GetMusicInfoTag()->SetDatabaseId(m_db.AddArtist(item->GetLabel(), mbArtistId), MediaTypeArtist);
  if (item->GetMusicInfoTag()->GetDatabaseId() <= 0)
    return false;

  if (!UpdateArtist(*item))
    return false;

  return SetImportForItem(item, import);
}

bool CArtistImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL || !item->HasMusicInfoTag())
    return false;

  return UpdateArtist(*item);
}

bool CArtistImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == NULL || !item->HasMusicInfoTag())
    return false;

  // TODO

  return false;
}

bool CArtistImportHandler::CleanupImportedItems(const CMediaImport &import)
{
  // TODO

  return true;
}

bool CArtistImportHandler::GetLocalItems(CMusicDatabase &musicdb, const CMediaImport &import, CFileItemList& items)
{
  return musicdb.GetArtistsByWhere("musicdb://artists/", GetFilter(import), items);
}

CFileItemPtr CArtistImportHandler::FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems)
{
  for (int i = 0; i < localItems.Size(); ++i)
  {
    CFileItemPtr localItem = localItems.Get(i);
    if (item->GetLabel() == localItem->GetLabel())
      return localItem;
  }

  return CFileItemPtr();
}

void CArtistImportHandler::RemoveImportedItem(CMusicDatabase &musicdb, const CMediaImport &import, const CFileItem* item)
{
  // TODO
}

bool CArtistImportHandler::UpdateArtist(const CFileItem &artistItem)
{
  if (!artistItem.HasMusicInfoTag() || artistItem.GetMusicInfoTag()->GetDatabaseId() <= 0)
    return false;

  const MUSIC_INFO::CMusicInfoTag& artist = *artistItem.GetMusicInfoTag();

  std::string mbArtistId;
  if (!artist.GetMusicBrainzArtistID().empty())
    mbArtistId = artist.GetMusicBrainzArtistID().at(0);

  m_db.UpdateArtist(artist.GetDatabaseId(), artistItem.GetLabel(), mbArtistId,
    "", "", StringUtils::Join(artist.GetGenre(), g_advancedSettings.m_musicItemSeparator) /* TODO */,
    "", "", "", "", "", "", "", artistItem.GetArt("thumb"), artistItem.GetArt("fanart"));
  /* TODO
  musicdb.UpdateArtist(artistId, pItem->GetLabel(), mbArtistId,
    "born", "formed", "genres", "moods",
    "styles", "instruments", "biography", "died",
    "disbanded", "yearsactive", "image", "fanart");
  */
  m_db.SetArtForItem(artist.GetDatabaseId(), MediaTypeArtist, artistItem.GetArt());

  return true;
}
