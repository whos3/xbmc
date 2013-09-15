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

#include "VideoImportHandler.h"
#include "FileItem.h"
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"

void CVideoImportHandler::SetImportedItemsEnabled(const std::string &source, bool enable)
{
  CVideoDatabase videodb;
  if (!videodb.Open())
    return;

  videodb.SetItemsEnabled(enable, source, GetMediaType());
}

void CVideoImportHandler::HandleImportedItems(const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  if (task != NULL && task->ShouldCancel(0, items.Size()))
    return;

  CVideoDatabase videodb;
  if (!videodb.Open())
    return;

  videodb.BeginTransaction();

  if (!PrepareImport(import, videodb) ||
      !HandleImportedItems(videodb, import, items, task))
  {
    videodb.RollbackTransaction();
    return;
  }

  videodb.CommitTransaction();
  videodb.Close();
}

bool CVideoImportHandler::PrepareImport(const CMediaImport &import, CVideoDatabase &videodb)
{
  if (import.GetPath().empty())
    return false;

  videodb.UpdateImportLastSynced(import.GetPath());

  return true;
}

void CVideoImportHandler::PrepareItem(const CMediaImport &import, CFileItem* pItem, CVideoDatabase &videodb)
{
  if (pItem == NULL || !pItem->HasVideoInfoTag() ||
      import.GetPath().empty() || import.GetSource().GetIdentifier().empty())
    return;

  videodb.AddPath(import.GetSource().GetIdentifier());
  int idPath = videodb.AddPath(import.GetPath());

  // set the proper source
  pItem->GetVideoInfoTag()->m_strSource = import.GetSource().GetIdentifier();

  // set the proper base and parent path
  pItem->GetVideoInfoTag()->m_basePath = import.GetSource().GetIdentifier(); // TODO
  pItem->GetVideoInfoTag()->m_parentPathID = idPath;
}

void CVideoImportHandler::PrepareItem(CFileItem *updatedItem, const CFileItem *originalItem)
{
  if (updatedItem == NULL || originalItem == NULL ||
      !updatedItem->HasVideoInfoTag() || !originalItem->HasVideoInfoTag())
    return;

  updatedItem->GetVideoInfoTag()->m_iDbId = originalItem->GetVideoInfoTag()->m_iDbId;
  updatedItem->GetVideoInfoTag()->m_iFileId = originalItem->GetVideoInfoTag()->m_iFileId;
  updatedItem->GetVideoInfoTag()->m_iIdShow = originalItem->GetVideoInfoTag()->m_iIdShow;
  updatedItem->GetVideoInfoTag()->m_iIdSeason = originalItem->GetVideoInfoTag()->m_iIdSeason;

  updatedItem->GetVideoInfoTag()->m_strSource = originalItem->GetVideoInfoTag()->m_strSource;
  updatedItem->GetVideoInfoTag()->m_basePath = originalItem->GetVideoInfoTag()->m_basePath;
  updatedItem->GetVideoInfoTag()->m_parentPathID = originalItem->GetVideoInfoTag()->m_parentPathID;

  updatedItem->GetVideoInfoTag()->m_resumePoint = originalItem->GetVideoInfoTag()->m_resumePoint;
}

void CVideoImportHandler::SetDetailsForFile(const CFileItem *pItem, bool reset, CVideoDatabase &videodb)
{
  // clean resume bookmark
  if (reset)
    videodb.DeleteResumeBookMark(pItem->GetPath());

  if (pItem->GetVideoInfoTag()->m_resumePoint.IsPartWay())
    videodb.AddBookMarkToFile(pItem->GetPath(), pItem->GetVideoInfoTag()->m_resumePoint, CBookmark::RESUME);

  if (pItem->GetVideoInfoTag()->m_playCount)
    videodb.SetPlayCount(*pItem, pItem->GetVideoInfoTag()->m_playCount, pItem->GetVideoInfoTag()->m_lastPlayed);
}

CDatabase::Filter CVideoImportHandler::GetFilter(const CMediaImport &import, bool enabledItems /* = false */)
{
  std::string strWhere;
  if (!import.GetSource().GetIdentifier().empty())
    strWhere += StringUtils::Format("strSource = '%s'", import.GetSource().GetIdentifier().c_str());

  return CDatabase::Filter(strWhere);
}

bool CVideoImportHandler::Compare(const CFileItem *originalItem, const CFileItem *newItem)
{
  if (originalItem == NULL || !originalItem->HasVideoInfoTag() ||
      newItem == NULL || !newItem->HasVideoInfoTag())
    return false;

  if (originalItem->GetArt() != newItem->GetArt())
    return false;

  if (originalItem->GetVideoInfoTag()->Equals(*newItem->GetVideoInfoTag(), true))
    return true;

  std::set<Field> differences;
  if (!originalItem->GetVideoInfoTag()->GetDifferences(*newItem->GetVideoInfoTag(), differences, true))
    return true;

  // special handling for actors without artwork
  std::set<Field>::const_iterator it = differences.find(FieldActor);
  if (it != differences.end())
  {
    const std::vector<SActorInfo> &originalCast = originalItem->GetVideoInfoTag()->m_cast;
    const std::vector<SActorInfo> &newCast = newItem->GetVideoInfoTag()->m_cast;
    if (originalCast.size() == newCast.size())
    {
      bool equal = true;
      for(size_t index = 0; index < originalCast.size(); ++index)
      {
        const SActorInfo &originalActor = originalCast.at(index);
        const SActorInfo &newActor = newCast.at(index);

        if (originalActor.strName != newActor.strName ||
            originalActor.strRole != newActor.strRole ||
            (!newActor.thumb.empty() && originalActor.thumb != newActor.thumb))
        {
          equal = false;
          break;
        }
      }

      if (equal)
        differences.erase(it);
    }
  }

  return differences.empty();
}
