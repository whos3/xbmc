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

#include "MovieImportHandler.h"
#include "FileItem.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

bool CMovieImportHandler::HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;

  task->SetProgressTitle("Importing movies from " + import.GetSource().GetFriendlyName()); // TODO: localization
  task->SetProgressText("");

  CFileItemList storedItems;
  videodb.GetMoviesByWhere("videodb://movies/titles/", GetFilter(import), storedItems, SortDescription(), true);

  CVideoThumbLoader thumbLoader;
  thumbLoader.OnLoaderStart();
  
  int progress = 0;
  int total = storedItems.Size() + items.Size();
  CFileItemList newItems; newItems.Copy(items);
  for (int i = 0; i < storedItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr& oldItem = storedItems[i];
    CFileItemPtr pItem = newItems.Get(oldItem->GetVideoInfoTag()->m_strFileNameAndPath);

    task->SetProgressText("Checking " + pItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

    // delete items that are not in newItems
    if (pItem == NULL)
    {
      task->SetProgressText("Removing " + oldItem->GetVideoInfoTag()->m_strTitle); // TODO: localization
      videodb.DeleteMovie(oldItem->GetVideoInfoTag()->m_iDbId);
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

        videodb.SetDetailsForMovie(pItem->GetPath(), *(pItem->GetVideoInfoTag()), pItem->GetArt(), pItem->GetVideoInfoTag()->m_iDbId);
        SetDetailsForFile(pItem.get(), true, videodb);
      }
    }

    task->SetProgress(progress++, total);
  }

  thumbLoader.OnLoaderFinish();

  // add any (remaining) new items
  for (int i = 0; i < newItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr& pItem = newItems[i];
    PrepareItem(import, pItem.get(), videodb);

    task->SetProgressText("Adding " + pItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

    videodb.SetDetailsForMovie(pItem->GetPath(), *(pItem->GetVideoInfoTag()), pItem->GetArt());
    SetDetailsForFile(pItem.get(), false, videodb);

    task->SetProgress(progress++, total);
  }

  return true;
}
