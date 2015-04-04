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

#include "UPnPStorageSystemContainer.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"

CUPnPStorageSystemContainer::CUPnPStorageSystemContainer()
  : CUPnPStorageSystemContainer("object.container.storageSystem")
{ }

CUPnPStorageSystemContainer::CUPnPStorageSystemContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPContainer(classType, className)
{
  initializeProperties();
}

CUPnPStorageSystemContainer::CUPnPStorageSystemContainer(const CUPnPStorageSystemContainer& storageSystemContainer)
  : CUPnPContainer(storageSystemContainer)
{
  initializeProperties();
  copyPropertyValidity(&storageSystemContainer);
}

CUPnPStorageSystemContainer::~CUPnPStorageSystemContainer()
{ }

void CUPnPStorageSystemContainer::initializeProperties()
{
  // define all attributes
  getProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageTotal").SetRequired();
  getProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageUsed").SetRequired();
  getProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageFree").SetRequired();
  getProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageMaxPartition").SetRequired();
}
