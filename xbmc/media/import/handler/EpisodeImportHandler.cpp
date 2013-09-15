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

#include "EpisodeImportHandler.h"
#include "FileItem.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

typedef std::set<CFileItemPtr> TvShowsSet;
typedef std::map<std::string, TvShowsSet> TvShowsMap;

/*!
 * Tries to find the tvshow from the given map to which the given episode belongs
 */
static int FindTvShow(const TvShowsMap &tvshowsMap, CFileItemPtr episodeItem)
{
  if (episodeItem == NULL)
    return -1;

  // no comparison possible without a title
  if (episodeItem->GetVideoInfoTag()->m_strShowTitle.empty())
    return -1;

  // check if there is a tvshow with a matching title
  TvShowsMap::const_iterator tvshowsIter = tvshowsMap.find(episodeItem->GetVideoInfoTag()->m_strShowTitle);
  if (tvshowsIter == tvshowsMap.end() ||
      tvshowsIter->second.size() <= 0)
    return -1;

  // if there is only one matching tvshow, we can go with that one
  if (tvshowsIter->second.size() == 1)
    return tvshowsIter->second.begin()->get()->GetVideoInfoTag()->m_iDbId;

  // use the path of the episode and tvshow to find the right tvshow
  for (TvShowsSet::const_iterator it = tvshowsIter->second.begin(); it != tvshowsIter->second.end(); ++it)
  {
    if (URIUtils::IsInPath(episodeItem->GetVideoInfoTag()->GetPath(), (*it)->GetVideoInfoTag()->GetPath()))
      return (*it)->GetVideoInfoTag()->m_iDbId;
  }

  return -1;
}

std::set<MediaType> CEpisodeImportHandler::GetDependencies() const
{ 
  std::set<MediaType> types;
  types.insert(MediaTypeTvShow);
  types.insert(MediaTypeSeason);
  return types;
}

bool CEpisodeImportHandler::HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;
  
  task->SetProgressTitle("Importing episodes from " + import.GetSource().GetFriendlyName()); // TODO: localization
  task->SetProgressText("");

  CFileItemList storedItems;
  videodb.GetEpisodesByWhere("videodb://tvshows/titles/", GetFilter(import), storedItems, true, SortDescription(), true);
  
  int total = storedItems.Size() + items.Size();
  if (checkCancelled && task->ShouldCancel(0, total))
    return false;

  CVideoThumbLoader thumbLoader;
  thumbLoader.OnLoaderStart();

  int progress = 0;
  CFileItemList newItems; newItems.Copy(items);
  for (int i = 0; i < storedItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr& oldItem = storedItems[i];
    CFileItemPtr pItem = newItems.Get(oldItem->GetVideoInfoTag()->GetPath());

    task->SetProgressText("Checking " + pItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

    // delete items that are not in newItems
    if (pItem == NULL)
    {
      task->SetProgressText("Removing " + oldItem->GetVideoInfoTag()->m_strTitle); // TODO: localization
      videodb.DeleteEpisode(oldItem->GetVideoInfoTag()->m_iDbId);
    }
    // item is in both lists
    else
    {
      // get rid of items we already have from the new items list
      newItems.Remove(pItem.get());
      total--;

      thumbLoader.LoadItem(oldItem.get());

      // check if we need to update (db writing is expensive)
      if (!Compare(oldItem.get(), pItem.get()))
      {
        task->SetProgressText("Updating " + pItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

        PrepareItem(pItem.get(), oldItem.get());

        videodb.SetDetailsForEpisode(pItem->GetPath(), *(pItem->GetVideoInfoTag()), pItem->GetArt(), pItem->GetVideoInfoTag()->m_iIdShow, pItem->GetVideoInfoTag()->m_iDbId);
        SetDetailsForFile(pItem.get(), true, videodb);
      }
    }

    task->SetProgress(progress++, total);
  }

  thumbLoader.OnLoaderFinish();

  if (newItems.Size() <= 0)
    return true;

  // create a map of tvshows imported from the same source
  CFileItemList tvshows;
  videodb.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), tvshows);

  TvShowsMap tvshowsMap;
  TvShowsMap::iterator tvshowsIter;
  for (int tvshowsIndex = 0; tvshowsIndex < tvshows.Size(); tvshowsIndex++)
  {
    CFileItemPtr& tvshow = tvshows[tvshowsIndex];

    if (!tvshow->HasVideoInfoTag() || tvshow->GetVideoInfoTag()->m_strTitle.empty())
      continue;

    tvshowsIter = tvshowsMap.find(tvshow->GetVideoInfoTag()->m_strTitle);
    if (tvshowsIter == tvshowsMap.end())
    {
      TvShowsSet tvshowsSet; tvshowsSet.insert(tvshow);
      tvshowsMap.insert(make_pair(tvshow->GetVideoInfoTag()->m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshow);
  }

  // add any (remaining) new items
  for (int i = 0; i < newItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr& pItem = newItems[i];
    PrepareItem(import, pItem.get(), videodb);

    task->SetProgressText("Adding " + pItem->GetVideoInfoTag()->m_strTitle); // TODO: localization
    
    int tvshowid = FindTvShow(tvshowsMap, pItem);
    videodb.SetDetailsForEpisode(pItem->GetPath(), *(pItem->GetVideoInfoTag()), pItem->GetArt(), tvshowid);
    SetDetailsForFile(pItem.get(), false, videodb);

    task->SetProgress(progress++, total);
  }

  return true;
}
