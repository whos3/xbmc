/*
 *      Copyright (C) 2016 Team XBMC
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

#include "UPnPBookmarkFolderContainer.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"

CUPnPBookmarkFolderContainer::CUPnPBookmarkFolderContainer()
  : CUPnPBookmarkFolderContainer("object.container.bookmarkFolder")
{ }

CUPnPBookmarkFolderContainer::CUPnPBookmarkFolderContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPContainer(classType, className)
{
  initializeProperties();
}

CUPnPBookmarkFolderContainer::CUPnPBookmarkFolderContainer(const CUPnPBookmarkFolderContainer& bookmarkFolderContainer)
  : CUPnPContainer(bookmarkFolderContainer)
{
  initializeProperties();
  copyPropertyValidity(&bookmarkFolderContainer);
}

CUPnPBookmarkFolderContainer::~CUPnPBookmarkFolderContainer()
{ }

void CUPnPBookmarkFolderContainer::initializeProperties()
{
  // define all attributes
  getProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageUsed").SetRequired();
}
