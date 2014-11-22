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

#include "SongImportHandler.h"
#include "FileItem.h"
#include "media/import/IMediaImportTask.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"

std::set<MediaType> CSongImportHandler::GetDependencies() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeArtist);
  types.insert(MediaTypeAlbum);
  return types;
}

std::vector<MediaType> CSongImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeArtist);
  types.push_back(MediaTypeAlbum);
  types.push_back(MediaTypeSong);
  return types;
}

std::string CSongImportHandler::GetItemLabel(const CFileItem* item) const
{
  if (item != NULL && item->HasMusicInfoTag() && !item->GetMusicInfoTag()->GetTitle().empty())
  {
    return StringUtils::Format(g_localizeStrings.Get(39015).c_str(),
      item->GetMusicInfoTag()->GetAlbum().c_str(),
      item->GetMusicInfoTag()->GetTitle().c_str());
  }

  return CMusicImportHandler::GetItemLabel(item);
}

bool CSongImportHandler::StartSynchronisation(const CMediaImport &import)
{
  if (!CMusicImportHandler::StartSynchronisation(import))
    return false;

  // create a map of artists and albums imported from the same source
  CFileItemList albums;
  if (!m_db.GetAlbumsByWhere("musicdb://albums/", CDatabase::Filter(), albums))
    return false;

  m_albums.clear();

  AlbumMap::iterator albumIter;
  for (int albumIndex = 0; albumIndex < albums.Size(); albumIndex++)
  {
    CFileItemPtr album = albums.Get(albumIndex);
    if (!album->HasMusicInfoTag() || album->GetMusicInfoTag()->GetTitle().empty())
      continue;

    albumIter = m_albums.find(album->GetMusicInfoTag()->GetTitle());
    if (albumIter == m_albums.end())
    {
      AlbumSet albumsSet; albumsSet.insert(album);
      m_albums.insert(make_pair(album->GetMusicInfoTag()->GetTitle(), albumsSet));
    }
    else
      albumIter->second.insert(album);
  }

  return true;
}

bool CSongImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL)
    return false;

  MUSIC_INFO::CMusicInfoTag *song = item->GetMusicInfoTag();
  PrepareItem(import, item);

  // try to find an existing album that the song belongs to
  int albumId = FindAlbumId(item);

  // if the album doesn't exist, create a very basic version of it with the info we got from the song
  if (albumId <= 0)
  {
    CAlbum album;
    album.art = item->GetArt();
    album.artist = song->GetArtist();
    album.bCompilation = false;
    album.enabled = true;
    album.genre = song->GetGenre();
    album.iRating = song->GetRating();
    album.iYear = song->GetYear();
    album.strAlbum = song->GetAlbum();
    album.strMusicBrainzAlbumID = song->GetMusicBrainzAlbumID();

    // add the basic album to the database
    if (!m_db.AddAlbum(album))
      return false;

    albumId = album.idAlbum;

    // turn the album into a CFileItem
    CFileItemPtr albumItem(new CFileItem(album.strAlbum));
    albumItem->SetFromAlbum(album);

    // copy any artwork from the song
    albumItem->SetArt(item->GetArt());

    // set the source and import paths
    albumItem->SetSource(item->GetSource());
    albumItem->SetImportPath(item->GetImportPath());

    // set the import on the album
    SetImportForItem(albumItem.get(), import);

    // add the album to the album map
    AlbumMap::iterator albumIter = m_albums.find(song->GetAlbum());
    if (albumIter == m_albums.end())
    {
      AlbumSet albumSet; albumSet.insert(albumItem);
      m_albums.insert(make_pair(song->GetAlbum(), albumSet));
    }
    else
      albumIter->second.insert(albumItem);
  }

  song->SetDatabaseId(m_db.AddSong(albumId, song->GetTitle(),
    song->GetMusicBrainzTrackID(),
    item->GetPath(), song->GetComment(),
    item->GetUserMusicThumb(true), StringUtils::Join(song->GetArtist(), g_advancedSettings.m_musicItemSeparator), // TODO: artist string
    song->GetGenre(), song->GetTrackNumber(),
    song->GetDuration(), song->GetYear(),
    song->GetPlayCount(), item->m_lStartOffset,
    item->m_lEndOffset, song->GetLastPlayed(),
    song->GetRating(), 0), MediaTypeSong);
  if (song->GetDatabaseId() <= 0)
    return false;

  SetDetailsForFile(item);
  return SetImportForItem(item, import);
}

bool CSongImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL || !item->HasMusicInfoTag())
    return false;

  CSong song(*item);
  m_db.UpdateSong(item->GetMusicInfoTag()->GetDatabaseId(), song);

  if (import.GetSettings().UpdatePlaybackMetadataFromSource())
    SetDetailsForFile(item);

  return true;
}

bool CSongImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == NULL || !item->HasMusicInfoTag() || item->GetMusicInfoTag()->GetDatabaseId() <= 0)
    return false;

  // TODO

  return false;
}

bool CSongImportHandler::GetLocalItems(CMusicDatabase &musicdb, const CMediaImport &import, CFileItemList& items)
{
  return musicdb.GetSongsByWhere("musicdb://songs/", GetFilter(import), items);
}

CFileItemPtr CSongImportHandler::FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems)
{
  for (int i = 0; i < localItems.Size(); ++i)
  {
    CFileItemPtr localItem = localItems.Get(i);
    if (!localItem->HasMusicInfoTag())
      continue;

    if (localItem->GetMusicInfoTag()->GetURL() == item->GetMusicInfoTag()->GetURL())
      return localItem;
  }

  return CFileItemPtr();
}

void CSongImportHandler::RemoveImportedItem(CMusicDatabase &musicdb, const CMediaImport &import, const CFileItem* item)
{
  // TODO
}

int CSongImportHandler::FindAlbumId(const CFileItem* songItem)
{
  if (songItem == NULL)
    return -1;

  // no comparison possible without a title
  if (songItem->GetMusicInfoTag()->GetAlbum().empty())
    return -1;

  // check if there is an album with a matching title
  AlbumMap::const_iterator albumIter = m_albums.find(songItem->GetMusicInfoTag()->GetAlbum());
  if (albumIter == m_albums.end() ||
    albumIter->second.size() <= 0)
    return -1;

  // if there is only one matching album, we can go with that one
  if (albumIter->second.size() == 1)
    return albumIter->second.begin()->get()->GetMusicInfoTag()->GetDatabaseId();

  // use the artist of the song and album to find the right album
  for (AlbumSet::const_iterator it = albumIter->second.begin(); it != albumIter->second.end(); ++it)
  {
    if (songItem->GetMusicInfoTag()->GetAlbumArtist() == (*it)->GetMusicInfoTag()->GetAlbumArtist()) // TODO: does this work???
      return (*it)->GetMusicInfoTag()->GetDatabaseId();
  }

  return -1;
}
