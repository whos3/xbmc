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

#include "SeasonImportHandler.h"
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
 * Checks whether two seasons are the same by comparing them by title and year
 */
static bool IsSameSeason(CVideoInfoTag& left, CVideoInfoTag& right)
{
  return left.m_strShowTitle == right.m_strShowTitle
      && left.m_iYear        == right.m_iYear
      && left.m_iSeason      == right.m_iSeason;
}

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

std::set<MediaType> CSeasonImportHandler::GetDependencies() const
{ 
  std::set<MediaType> types;
  types.insert(MediaTypeTvShow);
  return types;
}

bool CSeasonImportHandler::HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;
  
  task->SetProgressTitle("Importing seasons from " + import.GetSource().GetFriendlyName()); // TODO: localization
  task->SetProgressText("");

  CFileItemList storedItems;
  videodb.GetSeasonsByWhere("videodb://tvshows/titles/", GetFilter(import), storedItems, true);
  
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
    bool found = false;
    for (int j = 0; j < newItems.Size() ; j++)
    {
      if (checkCancelled && task->ShouldCancel(progress, items.Size()))
        return false;

      CFileItemPtr& newItem = newItems[j];
      task->SetProgressText("Checking " + newItem->GetVideoInfoTag()->m_strShowTitle + ": " + newItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

      if (IsSameSeason(*oldItem->GetVideoInfoTag(), *newItem->GetVideoInfoTag()))
      {
        // get rid of items we already have from the new items list
        newItems.Remove(j);
        total--;
        found = true;

        thumbLoader.LoadItem(oldItem.get());
        
        // check if we need to update (db writing is expensive)
        if (!Compare(oldItem.get(), newItem.get()))
        {
          task->SetProgressText("Updating " + newItem->GetVideoInfoTag()->m_strShowTitle + ": " + newItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

          PrepareItem(newItem.get(), oldItem.get());
          videodb.SetDetailsForSeason(*(newItem->GetVideoInfoTag()), newItem->GetArt(), newItem->GetVideoInfoTag()->m_iIdShow, newItem->GetVideoInfoTag()->m_iDbId);
        }
        break;
      }
    }

    // delete items that are not in newItems
    if (!found)
    {
      task->SetProgressText("Removing " + oldItem->GetVideoInfoTag()->m_strShowTitle + ": " + oldItem->GetVideoInfoTag()->m_strTitle); // TODO: localization
      videodb.DeleteSeason(oldItem->GetVideoInfoTag()->m_iDbId);
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

    task->SetProgressText("Adding " + pItem->GetVideoInfoTag()->m_strShowTitle + ": " + pItem->GetVideoInfoTag()->m_strTitle); // TODO: localization
    
    pItem->GetVideoInfoTag()->m_iIdShow = FindTvShow(tvshowsMap, pItem);
    videodb.SetDetailsForSeason(*(pItem->GetVideoInfoTag()), pItem->GetArt(), pItem->GetVideoInfoTag()->m_iIdShow);

    task->SetProgress(progress++, total);
  }

  return true;
}
