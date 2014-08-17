/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "DirectoryNodeTitleMovies.h"
#include "QueryParams.h"
#include "video/VideoDatabase.h"

using namespace XFILE::VIDEODATABASEDIRECTORY;

CDirectoryNodeTitleMovies::CDirectoryNodeTitleMovies(const std::string& strName, CDirectoryNode* pParent)
  : CDirectoryNode(NODE_TYPE_TITLE_MOVIES, strName, pParent)
{

}

NODE_TYPE CDirectoryNodeTitleMovies::GetChildType() const
{
  std::string name = GetName();
  if (name.empty())
    return NODE_TYPE_NONE;

  char *end = NULL;
  int idMovie = strtol(name.c_str(), &end, 0);
  if ((end != NULL && *end != '\0') ||
      idMovie <= 0)
    return NODE_TYPE_NONE;

  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return NODE_TYPE_NONE;

  std::set<int> files;
  bool result = videodatabase.GetFilesForItem(idMovie, MediaTypeMovie, files);
  videodatabase.Close();

  if (result && files.size() > 1)
    return NODE_TYPE_TITLE_MOVIES;

  return NODE_TYPE_NONE;
}

bool CDirectoryNodeTitleMovies::GetContent(CFileItemList& items) const
{
  CVideoDatabase videodatabase;
  if (!videodatabase.Open())
    return false;

  CQueryParams params;
  CollectQueryParams(params);

  bool bSuccess = false;
  if (params.GetMovieId() <= 0)
    bSuccess = videodatabase.GetMoviesNav(BuildPath(), items, params.GetGenreId(), params.GetYear(), params.GetActorId(), params.GetDirectorId(), params.GetStudioId(), params.GetCountryId(), params.GetSetId(), params.GetTagId());
  else
  {
    CVideoDbUrl videoUrl;
    if (videoUrl.FromString(BuildPath()))
    {
      videoUrl.AddOption("movieid", params.GetMovieId());
      bSuccess = videodatabase.GetMoviesByWhere(videoUrl.ToString(), CDatabase::Filter(), items);
    }
  }

  videodatabase.Close();

  return bSuccess;
}
