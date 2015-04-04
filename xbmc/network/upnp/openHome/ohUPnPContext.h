#pragma once
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
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/didllite/DidlLitePropertyList.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfile.h"

class CThumbLoader;

struct OhUPnPControlPointContext
{
  const COhUPnPDevice device;
  const COhUPnPDeviceProfile profile;
};

struct OhUPnPRootDeviceContext
{
  const COhUPnPDevice& device;
  const COhUPnPDeviceProfile& profile;
  const std::string resourceUriPrefix;
  const CDidlLitePropertyList filters;

  mutable std::map<std::string, std::shared_ptr<CThumbLoader>> thumbLoaders;

  OhUPnPRootDeviceContext(const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile)
    : OhUPnPRootDeviceContext(device, profile, "")
  { }

  OhUPnPRootDeviceContext(const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix)
    : OhUPnPRootDeviceContext(device, profile, resourceUriPrefix, CDidlLitePropertyList())
  { }

  OhUPnPRootDeviceContext(const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const CDidlLitePropertyList& filters)
    : OhUPnPRootDeviceContext(device, profile, "", filters)
  { }

  OhUPnPRootDeviceContext(const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix, const CDidlLitePropertyList& filters)
    : device(device),
      profile(profile),
      resourceUriPrefix(resourceUriPrefix),
      filters(filters)
  { }
};
