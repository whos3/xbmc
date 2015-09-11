#pragma once
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

#include <string>
#include <vector>

#include "media/MediaType.h"
#include "media/info/VideoFileInfoTag.h"

class CMovieInfoTag : public CVideoFileInfoTag
{
public:
  CMovieInfoTag()
    : CVideoFileInfoTag(Type)
  { }
  CMovieInfoTag(const CMovieInfoTag&);
  CMovieInfoTag(CMovieInfoTag&&);
  virtual ~CMovieInfoTag() = default;

  // specializations of CFileInfoTag
  virtual void Serialize(CVariant& value) const;
  virtual void ToSortable(SortItem& sortable, Field field) const;

  static const MediaType Type;

  // TODO

private:
  std::string m_originalTitle;
  std::string m_sortTitle;
  std::string m_plot;
  std::string m_plotOutline;
  std::string m_tagline;
  std::string m_votes;
  float m_rating;
  std::vector<std::string> m_writingCredits;
  int m_year;
  int m_duration;
  std::string m_mpaaRating;
  int m_top250;
  std::vector<std::string> m_genre;
  std::vector<std::string> m_director;
  std::vector<std::string> m_studio;
  std::vector<std::string> m_country;
  std::string m_trailer;

  int m_basePathId;
  std::string m_basePath;

  int m_fileId;

  int m_setId;
  std::string m_set;

  // TODO: std::string m_imdbNumber;
  // TODO: CScraperUrl m_strPictureURL;
  // TODO: CFanart m_fanart;
};

using CMovieInfoTagPtr = std::shared_ptr<CMovieInfoTag>;
