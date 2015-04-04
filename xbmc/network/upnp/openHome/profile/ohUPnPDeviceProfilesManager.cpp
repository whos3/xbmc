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

#include <algorithm>

#include "ohUPnPDeviceProfilesManager.h"
#include "FileItem.h"
#include "filesystem/Directory.h"
#include "network/upnp/openHome/ohUPnPDevice.h"
#include "utils/log.h"

COhUPnPDeviceProfilesManager::COhUPnPDeviceProfilesManager()
  : m_initialized(false)
{ }

COhUPnPDeviceProfilesManager& COhUPnPDeviceProfilesManager::GetInstance()
{
  static COhUPnPDeviceProfilesManager s_instance;
  return s_instance;
}

bool COhUPnPDeviceProfilesManager::Initialize()
{
  static const std::string ProfileDirectory = "special://xbmc/system/upnpprofiles/";

  if (m_initialized)
    return true;

  CFileItemList profiles;
  if (!XFILE::CDirectory::GetDirectory(ProfileDirectory, profiles, "*.xml") && profiles.IsEmpty())
  {
    CLog::Log(LOGERROR, "COhUPnPDeviceProfilesManager: failed to load profile definitions from %s", ProfileDirectory.c_str());
    return false;
  }

  for (int i = 0; i < profiles.Size(); i++)
  {
    COhUPnPDeviceProfile profile;
    if (!profile.Load(profiles.Get(i)->GetPath()))
    {
      CLog::Log(LOGWARNING, "COhUPnPDeviceProfilesManager: failed to load profile from %s", profiles.Get(i)->GetPath().c_str());
      continue;
    }

    CLog::Log(LOGDEBUG, "COhUPnPDeviceProfilesManager: loaded profile %s from %s", profile.GetName().c_str(), profiles.Get(i)->GetPath().c_str());
    m_profiles.push_back(profile);
  }

  // make sure that the default profile has been loaded
  COhUPnPDeviceProfile defaultProfile;
  if (!GetDefaultProfile(defaultProfile))
  {
    CLog::Log(LOGERROR, "COhUPnPDeviceProfilesManager: no default profile loaded");
    m_profiles.clear();
    return false;
  }

  return true;
}

bool COhUPnPDeviceProfilesManager::FindProfile(const COhUPnPDevice& device, COhUPnPDeviceProfile& profile) const
{
  if (device.IsValid())
  {
    for (const auto& prof : m_profiles)
    {
      if (prof.Matches(device))
      {
        profile = prof;
        return true;
      }
    }
  }

  // fall back to the default profile
  GetDefaultProfile(profile);
  return false;
}

bool COhUPnPDeviceProfilesManager::GetDefaultProfile(COhUPnPDeviceProfile& defaultProfile) const
{
  static const std::string DefaultProfileName = "Default";

  const auto& it = std::find_if(m_profiles.cbegin(), m_profiles.cend(),
    [](const COhUPnPDeviceProfile& profile)
    {
      return profile.GetName() == DefaultProfileName;
    });
  if (it == m_profiles.cend())
    return false;

  defaultProfile = *it;
  return true;
}
