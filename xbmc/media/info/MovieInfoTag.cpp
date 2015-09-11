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

#include "MovieInfoTag.h"

const MediaType CMovieInfoTag::Type = MediaTypeMovie;

CMovieInfoTag::CMovieInfoTag(const CMovieInfoTag& movieInfoTag)
  : CVideoFileInfoTag(movieInfoTag)
  , m_originalTitle(movieInfoTag.m_originalTitle)
  , m_sortTitle(movieInfoTag.m_sortTitle)
  , m_plot(movieInfoTag.m_plot)
  , m_plotOutline(movieInfoTag.m_plotOutline)
  , m_tagline(movieInfoTag.m_tagline)
  , m_votes(movieInfoTag.m_votes)
  , m_rating(movieInfoTag.m_rating)
  , m_writingCredits(movieInfoTag.m_writingCredits)
  , m_year(movieInfoTag.m_year)
  , m_duration(movieInfoTag.m_duration)
  , m_mpaaRating(movieInfoTag.m_mpaaRating)
  , m_top250(movieInfoTag.m_top250)
  , m_genre(movieInfoTag.m_genre)
  , m_director(movieInfoTag.m_director)
  , m_studio(movieInfoTag.m_studio)
  , m_country(movieInfoTag.m_country)
  , m_trailer(movieInfoTag.m_trailer)
  , m_basePathId(movieInfoTag.m_basePathId)
  , m_basePath(movieInfoTag.m_basePath)
  , m_fileId(movieInfoTag.m_fileId)
  , m_setId(movieInfoTag.m_setId)
  , m_set(movieInfoTag.m_set)
{ }

CMovieInfoTag::CMovieInfoTag(CMovieInfoTag&& movieInfoTag)
  : CVideoFileInfoTag(std::move(movieInfoTag))
  , m_originalTitle(std::move(movieInfoTag.m_originalTitle))
  , m_sortTitle(std::move(movieInfoTag.m_sortTitle))
  , m_plot(std::move(movieInfoTag.m_plot))
  , m_plotOutline(std::move(movieInfoTag.m_plotOutline))
  , m_tagline(std::move(movieInfoTag.m_tagline))
  , m_votes(std::move(movieInfoTag.m_votes))
  , m_rating(std::move(movieInfoTag.m_rating))
  , m_writingCredits(std::move(movieInfoTag.m_writingCredits))
  , m_year(std::move(movieInfoTag.m_year))
  , m_duration(std::move(movieInfoTag.m_duration))
  , m_mpaaRating(std::move(movieInfoTag.m_mpaaRating))
  , m_top250(std::move(movieInfoTag.m_top250))
  , m_genre(std::move(movieInfoTag.m_genre))
  , m_director(std::move(movieInfoTag.m_director))
  , m_studio(std::move(movieInfoTag.m_studio))
  , m_country(std::move(movieInfoTag.m_country))
  , m_trailer(std::move(movieInfoTag.m_trailer))
  , m_basePathId(std::move(movieInfoTag.m_basePathId))
  , m_basePath(std::move(movieInfoTag.m_basePath))
  , m_fileId(std::move(movieInfoTag.m_fileId))
  , m_setId(std::move(movieInfoTag.m_setId))
  , m_set(std::move(movieInfoTag.m_set))
{ }

void CMovieInfoTag::Serialize(CVariant& value) const
{
  // TODO

  CVideoFileInfoTag::Serialize(value);
}

void CMovieInfoTag::ToSortable(SortItem& sortable, Field field) const
{
  switch (field)
  {
    // TODO

  default:
    break;
  }

  CVideoFileInfoTag::ToSortable(sortable, field);
}
