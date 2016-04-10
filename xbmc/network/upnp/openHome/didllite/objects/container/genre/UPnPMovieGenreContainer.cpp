/*
 *      Copyright (C) 2015 Team XBMC
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

#include "UPnPMovieGenreContainer.h"
#include "FileItem.h"
#include "filesystem/VideoDatabaseDirectory.h"
#include "video/VideoInfoTag.h"

CUPnPMovieGenreContainer::CUPnPMovieGenreContainer()
  : CUPnPMovieGenreContainer("object.container.genre.movieGenre")
{ }

CUPnPMovieGenreContainer::CUPnPMovieGenreContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPGenreContainer(classType, className)
{ }

CUPnPMovieGenreContainer::CUPnPMovieGenreContainer(const CUPnPMovieGenreContainer& movieGenreContainer)
  : CUPnPGenreContainer(movieGenreContainer)
{ }

CUPnPMovieGenreContainer::~CUPnPMovieGenreContainer()
{ }

bool CUPnPMovieGenreContainer::CanHandleFileItem(const CFileItem& item) const
{
  if (!CUPnPGenreContainer::CanHandleFileItem(item))
    return false;

  if (item.HasVideoInfoTag())
    return item.GetVideoInfoTag()->m_type == "genre";

  if (item.IsVideoDb())
  {
    XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = XFILE::CVideoDatabaseDirectory::GetDirectoryType(item.GetPath());
    return node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_GENRE;
  }

  return false;
}

bool CUPnPMovieGenreContainer::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPGenreContainer::ToFileItem(item, context))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();

  const auto& longDescription = GetLongDescription();
  const auto& description = GetDescription();
  videoInfo.m_strPlot = !longDescription.empty() ? longDescription : description;
  videoInfo.m_strTagLine = description;

  return true;
}

bool CUPnPMovieGenreContainer::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (!CUPnPGenreContainer::FromFileItem(item, context))
    return false;

  if (!item.HasVideoInfoTag())
    return true;

  const CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();

  if (!videoInfo.m_strPlot.empty())
    SetLongDescription(videoInfo.m_strPlot);

  return true;
}
