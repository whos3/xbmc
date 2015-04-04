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
