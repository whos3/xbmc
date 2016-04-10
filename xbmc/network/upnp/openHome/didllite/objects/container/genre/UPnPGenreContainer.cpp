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

#include "UPnPGenreContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"

CUPnPGenreContainer::CUPnPGenreContainer()
  : CUPnPGenreContainer("object.container.genre")
{ }

CUPnPGenreContainer::CUPnPGenreContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPContainer(classType, className)
{ }

CUPnPGenreContainer::CUPnPGenreContainer(const CUPnPGenreContainer& genreContainer)
  : CUPnPContainer(genreContainer)
{
  copyPropertyValidity(&genreContainer);
}

CUPnPGenreContainer::~CUPnPGenreContainer()
{ }

bool CUPnPGenreContainer::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPContainer::ToFileItem(item, context))
    return false;

  const auto& genres = GetGenres();
  if (genres.size() == 1)
    item.SetLabel(genres.front()->GetGenre());

  return true;
}

bool CUPnPGenreContainer::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (!CUPnPContainer::FromFileItem(item, context))
    return false;

  SetGenres({ item.GetLabel() });

  return true;
}
