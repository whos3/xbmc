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

#include <string>

#include "network/upnp/openHome/profile/ohUPnPDeviceProfile.h"

class COhUPnPDevice;

class COhUPnPDeviceProfilesManager
{
public:
  COhUPnPDeviceProfilesManager(const COhUPnPDeviceProfilesManager&) = delete;
  void operator=(const COhUPnPDeviceProfilesManager&) = delete;

  static COhUPnPDeviceProfilesManager& GetInstance();

  bool Initialize();

  bool FindProfile(const COhUPnPDevice& device, COhUPnPDeviceProfile& profile) const;

private:
  COhUPnPDeviceProfilesManager();

  bool GetDefaultProfile(COhUPnPDeviceProfile& defaultProfile) const;

  bool m_initialized;
  std::vector<COhUPnPDeviceProfile> m_profiles;
};
