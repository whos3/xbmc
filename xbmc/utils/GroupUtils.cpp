/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include <map>
#include <set>

#include "GroupUtils.h"
#include "FileItem.h"
#include "utils/StringUtils.h"
#include "utils/Variant.h"
#include "video/VideoDbUrl.h"
#include "video/VideoInfoTag.h"
#include "utils/URIUtils.h"
#include "filesystem/MultiPathDirectory.h"

using namespace std;

typedef map<int, set<CFileItemPtr> > GroupMap;

bool GroupUtils::Group(GroupBy groupBy, const std::string &baseDir, const CFileItemList &items, CFileItemList &groupedItems, GroupAttribute groupAttributes /* = GroupAttributeNone */)
{
  if (groupBy == GroupByNone)
    return false;

  // nothing to do if there are no items to group
  if (items.Size() <= 0)
    return true;

  GroupMap setMap;
  GroupMap itemMap;
  for (int index = 0; index < items.Size(); index++)
  {
    bool add = true;
    const CFileItemPtr item = items.Get(index);

    // group by sets
    if ((groupBy & GroupBySet) &&
        item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_iSetId > 0)
    {
      add = false;
      setMap[item->GetVideoInfoTag()->m_iSetId].insert(item);
    }
    if (add && (groupBy & GroupByItem) &&
        item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_iDbId > 0)
    {
      add = false;
      itemMap[item->GetVideoInfoTag()->m_iDbId].insert(item);
    }

    if (add)
      groupedItems.Add(item);
  }

  if ((groupBy & GroupBySet) && !setMap.empty())
  {
    CVideoDbUrl itemsUrl;
    if (!itemsUrl.FromString(baseDir))
      return false;

    for (GroupMap::const_iterator set = setMap.begin(); set != setMap.end(); ++set)
    {
      // only one item in the set, so just re-add it
      if (set->second.size() == 1 && (groupAttributes & GroupAttributeIgnoreSingleItems))
      {
        groupedItems.Add(*set->second.begin());
        continue;
      }

      CFileItemPtr pItem(new CFileItem((*set->second.begin())->GetVideoInfoTag()->m_strSet));
      pItem->SetProperty(PROPERTY_GROUPED, true);
      pItem->GetVideoInfoTag()->m_iDbId = set->first;
      pItem->GetVideoInfoTag()->m_type = MediaTypeVideoCollection;

      std::string basePath = StringUtils::Format("videodb://movies/sets/%i/", set->first);
      CVideoDbUrl videoUrl;
      if (!videoUrl.FromString(basePath))
        pItem->SetPath(basePath);
      else
      {
        videoUrl.AddOptions(itemsUrl.GetOptionsString());
        pItem->SetPath(videoUrl.ToString());
      }
      pItem->m_bIsFolder = true;

      CVideoInfoTag* setInfo = pItem->GetVideoInfoTag();
      setInfo->m_strPath = pItem->GetPath();
      setInfo->m_strTitle = pItem->GetLabel();

      int ratings = 0;
      int iWatched = 0; // have all the movies been played at least once?
      std::set<std::string> pathSet;
      for (std::set<CFileItemPtr>::const_iterator movie = set->second.begin(); movie != set->second.end(); ++movie)
      {
        CVideoInfoTag* movieInfo = (*movie)->GetVideoInfoTag();
        // handle rating
        if (movieInfo->m_fRating > 0.0f)
        {
          ratings++;
          setInfo->m_fRating += movieInfo->m_fRating;
        }
        
        // handle year
        if (movieInfo->m_iYear > setInfo->m_iYear)
          setInfo->m_iYear = movieInfo->m_iYear;
        
        // handle lastplayed
        if (movieInfo->m_lastPlayed.IsValid() && movieInfo->m_lastPlayed > setInfo->m_lastPlayed)
          setInfo->m_lastPlayed = movieInfo->m_lastPlayed;
        
        // handle dateadded
        if (movieInfo->m_dateAdded.IsValid() && movieInfo->m_dateAdded > setInfo->m_dateAdded)
          setInfo->m_dateAdded = movieInfo->m_dateAdded;
        
        // handle playcount/watched
        setInfo->m_playCount += movieInfo->m_playCount;
        if (movieInfo->m_playCount > 0)
          iWatched++;

        //accumulate the path for a multipath construction
        CFileItem video(movieInfo->m_basePath, false);
        if (video.IsVideo())
          pathSet.insert(URIUtils::GetParentPath(movieInfo->m_basePath));
        else
          pathSet.insert(movieInfo->m_basePath);
      }
      setInfo->m_basePath = XFILE::CMultiPathDirectory::ConstructMultiPath(pathSet);

      if (ratings > 1)
        pItem->GetVideoInfoTag()->m_fRating /= ratings;
        
      setInfo->m_playCount = iWatched >= (int)set->second.size() ? (setInfo->m_playCount / set->second.size()) : 0;
      pItem->SetProperty("total", (int)set->second.size());
      pItem->SetProperty("watched", iWatched);
      pItem->SetProperty("unwatched", (int)set->second.size() - iWatched);
      pItem->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, setInfo->m_playCount > 0);

      groupedItems.Add(pItem);
    }
  }

  if ((groupBy & GroupByItem) && !itemMap.empty())
  {
    std::set<CStreamDetail::StreamType> streamTypes;
    streamTypes.insert(CStreamDetail::VIDEO);
    streamTypes.insert(CStreamDetail::AUDIO);
    streamTypes.insert(CStreamDetail::SUBTITLE);

    for (GroupMap::const_iterator itItem = itemMap.begin(); itItem != itemMap.end(); ++itItem)
    {
      // only one item in the set, so just re-add it
      if (itItem->second.size() == 1)
      {
        groupedItems.Add(*itItem->second.begin());
        continue;
      }

      CFileItemPtr pItem(*itItem->second.begin());
      CVideoInfoTag* info = pItem->GetVideoInfoTag();
      pItem->SetProperty(PROPERTY_GROUPED, true);
      pItem->m_bIsFolder = true;

      int iWatched = 0; // have all the items been played at least once?

      // go through all items that are the same (start with the second because the first is already used as a base
      for (std::set<CFileItemPtr>::const_iterator item = ++itItem->second.begin(); item != itItem->second.end(); ++item)
      {
        CVideoInfoTag* itemInfo = (*item)->GetVideoInfoTag();

        // handle lastplayed
        if (itemInfo->m_lastPlayed.IsValid() && itemInfo->m_lastPlayed > info->m_lastPlayed)
          info->m_lastPlayed = itemInfo->m_lastPlayed;

        // handle dateadded
        if (itemInfo->m_dateAdded.IsValid() && itemInfo->m_dateAdded > info->m_dateAdded)
          info->m_dateAdded = itemInfo->m_dateAdded;

        // handle playcount/watched
        info->m_playCount += itemInfo->m_playCount;
        if (itemInfo->m_playCount > 0)
          iWatched++;

        // put together all available "best" streams
        if (itemInfo->HasStreamDetails())
        {
          for (std::set<CStreamDetail::StreamType>::const_iterator streamType = streamTypes.begin(); streamType != streamTypes.end(); ++streamType)
          {
            if (itemInfo->m_streamDetails.GetStreamCount(*streamType) > 0)
            {
              CStreamDetail* newStream = NULL;
              const CStreamDetail* stream = itemInfo->m_streamDetails.GetNthStream(*streamType, 0);
              switch (*streamType)
              {
              case CStreamDetail::VIDEO:
                newStream = new CStreamDetailVideo(*static_cast<const CStreamDetailVideo*>(stream));
                break;

              case CStreamDetail::AUDIO:
                newStream = new CStreamDetailAudio(*static_cast<const CStreamDetailAudio*>(stream));
                break;

              case CStreamDetail::SUBTITLE:
                newStream = new CStreamDetailSubtitle(*static_cast<const CStreamDetailSubtitle*>(stream));
                break;
              }

              if (newStream != NULL)
                info->m_streamDetails.AddStream(newStream);
            }
          }
        }
      }

      // determine the best streams
      info->m_streamDetails.DetermineBestStreams();

      pItem->SetProperty("total", (int)itItem->second.size());
      pItem->SetProperty("watched", iWatched);
      pItem->SetProperty("unwatched", (int)itItem->second.size() - iWatched);
      pItem->SetOverlayImage(CGUIListItem::ICON_OVERLAY_UNWATCHED, info->m_playCount > 0);

      groupedItems.Add(pItem);
    }
  }

  return true;
}

bool GroupUtils::GroupAndSort(GroupBy groupBy, const std::string &baseDir, const CFileItemList &items, const SortDescription &sortDescription, CFileItemList &groupedItems, GroupAttribute groupAttributes /* = GroupAttributeNone */)
{
  if (!Group(groupBy, baseDir, items, groupedItems, groupAttributes))
    return false;

  groupedItems.Sort(sortDescription);
  return true;
}
