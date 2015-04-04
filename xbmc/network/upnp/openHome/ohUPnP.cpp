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

#include <OpenHome/Private/Debug.h>
#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>

#include "ohUPnP.h"
#include "network/upnp/UPnPSettings.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/controlpoints/ohUPnPContentDirectoryControlPoint.h"
#include "network/upnp/openHome/didllite/objects/container/UPnPBookmarkFolderContainer.h"
#include "network/upnp/openHome/didllite/objects/container/UPnPContainer.h"
#include "network/upnp/openHome/didllite/objects/container/UPnPPlaylistContainer.h"
#include "network/upnp/openHome/didllite/objects/container/album/UPnPAlbumContainer.h"
#include "network/upnp/openHome/didllite/objects/container/album/UPnPMusicAlbumContainer.h"
#include "network/upnp/openHome/didllite/objects/container/album/UPnPPhotoAlbumContainer.h"
#include "network/upnp/openHome/didllite/objects/container/album/UPnPVideoAlbumContainer.h"
#include "network/upnp/openHome/didllite/objects/container/genre/UPnPGenreContainer.h"
#include "network/upnp/openHome/didllite/objects/container/genre/UPnPMovieGenreContainer.h"
#include "network/upnp/openHome/didllite/objects/container/genre/UPnPMusicGenreContainer.h"
#include "network/upnp/openHome/didllite/objects/container/person/UPnPMusicArtistPersonContainer.h"
#include "network/upnp/openHome/didllite/objects/container/person/UPnPPersonContainer.h"
#include "network/upnp/openHome/didllite/objects/container/storage/UPnPStorageFolderContainer.h"
#include "network/upnp/openHome/didllite/objects/container/storage/UPnPStorageSystemContainer.h"
#include "network/upnp/openHome/didllite/objects/container/storage/UPnPStorageVolumeContainer.h"
#include "network/upnp/openHome/didllite/objects/item/UPnPItem.h"
#include "network/upnp/openHome/didllite/objects/item/image/UPnPImageItem.h"
#include "network/upnp/openHome/didllite/objects/item/image/UPnPPhotoImageItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPMovieItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPMusicVideoClipItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPVideoBroadcastItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPVideoItem.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfilesManager.h"
#include "network/upnp/openHome/rootdevices/ohUPnPMediaServerDevice.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "profiles/ProfilesManager.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "settings/lib/Setting.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"
#include "utils/URIUtils.h"

static std::string GetUPnPSettingsFile()
{ 
  return URIUtils::AddFileToFolder(CProfilesManager::GetInstance().GetUserDataFolder(), "upnpserver.xml");
}

static bool GetIpAndSubnetAddress(TIpAddress& ipAddress, TIpAddress& subnetAddress)
{
  // get a list of available subnets
  std::vector<OpenHome::NetworkAdapter*> *subnets = OpenHome::Net::UpnpLibrary::CreateSubnetList();
  if (subnets->empty())
  {
    OpenHome::Net::UpnpLibrary::DestroySubnetList(subnets);
    return false;
  }

  // try to find a matching subnet
  subnetAddress = CSettings::GetInstance().GetInt(CSettings::SETTING_SERVICES_UPNPINTERFACE);
  auto& subnet = std::find_if(subnets->cbegin(), subnets->cend(),
    [subnetAddress](const OpenHome::NetworkAdapter* subnet)
  {
    return subnet->Subnet() == subnetAddress;
  });

  // if the setting didn't match any interface, take the first one
  if (subnet == subnets->cend())
    subnet = subnets->cbegin();

  ipAddress = (*subnet)->Address();
  subnetAddress = (*subnet)->Subnet();
  OpenHome::Net::UpnpLibrary::DestroySubnetList(subnets);

  return true;
}

COhUPnP::COhUPnP()
  : m_initialised(false),
    m_ipAddress(0),
    m_subnetAddress(0),
    m_ohEnvironment(nullptr),
    m_contentDirectoryClient(nullptr),
    m_mediaServer(nullptr)
{ }

COhUPnP::~COhUPnP()
{
  Uninitialize();
}

COhUPnP& COhUPnP::GetInstance()
{
  static COhUPnP s_instance;
  return s_instance;
}

bool COhUPnP::Initialize()
{
  if (m_initialised)
    return true;

  // load upnpserver.xml
  if (!CUPnPSettings::GetInstance().Load(GetUPnPSettingsFile()))
  {
    CLog::Log(LOGINFO, "COhUPnP: no or invalid settings available => creating new ones");
    CUPnPSettings::GetInstance().Clear();
    if (!CUPnPSettings::GetInstance().Save(GetUPnPSettingsFile()))
    {
      CLog::Log(LOGERROR, "COhUPnP: failed to save new settings");
      return false;
    }
  }

  if (!COhUPnPDeviceProfilesManager::GetInstance().Initialize())
  {
    CLog::Log(LOGERROR, "COhUPnP: initializing UPnP failed");
    return false;
  }

  // setup DIDL-Lite element factory
  m_fileItemElementFactory.RegisterElement(new CUPnPContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPAlbumContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPMusicAlbumContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPPhotoAlbumContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPVideoAlbumContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPPersonContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPMusicArtistPersonContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPGenreContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPMovieGenreContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPMusicGenreContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPPlaylistContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPStorageFolderContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPStorageSystemContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPStorageVolumeContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPBookmarkFolderContainer());
  m_fileItemElementFactory.RegisterElement(new CUPnPItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPImageItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPPhotoImageItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPVideoItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPMovieItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPMusicVideoClipItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPVideoBroadcastItem());

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    OpenHome::Debug::SetLevel(OpenHome::Debug::kAll);

  // put together the configuration for the ohNet UpnpLibrary
  OpenHome::Net::InitialisationParams* initParams = OpenHome::Net::InitialisationParams::Create();
  initParams->SetLogOutput(OpenHome::MakeFunctorMsg(*this, &COhUPnP::ohNetLogOutput));
  initParams->SetFatalErrorHandler(OpenHome::MakeFunctorMsg(*this, &COhUPnP::ohNetFatalErrorHandler));
  initParams->SetUserAgent(CSysInfo::GetUserAgent().c_str());

  // initialise the ohNet UpnpLibrary
  m_ohEnvironment.reset(OpenHome::Net::UpnpLibrary::Initialise(initParams));
  if (m_ohEnvironment == nullptr)
  {
    CLog::Log(LOGERROR, "COhUPnP: initializing UPnP failed");
    return false;
  }

  if (!GetIpAndSubnetAddress(m_ipAddress, m_subnetAddress))
  {
    CLog::Log(LOGERROR, "COhUPnP: initializing UPnP failed because no network interfaces are available");
    return false;
  }

  m_initialised = true;
  return true;
}

void COhUPnP::Uninitialize()
{
  if (!m_initialised)
    return;

  StopRenderer();
  StopContentDirectoryClient();
  StopMediaServer();
  StopResourceManager(true);

  m_mediaServer.reset();

  m_ipAddress = 0;
  m_subnetAddress = 0;

  m_ohEnvironment.reset();

  m_initialised = false;
}

std::string COhUPnP::GetResourceUriPrefix() const
{
  if (!m_resourceManager.IsRunning())
    return "";

  return COhUPnPResourceManager::GetResourceUriPrefix(m_ipAddress, m_resourceManager.GetPort());
}

const COhUPnPDeviceProfilesManager& COhUPnP::GetDeviceProfilesManager() const
{
  return COhUPnPDeviceProfilesManager::GetInstance();
}

bool COhUPnP::HasUPnPTransfers(const std::string &condition, const std::string &value, const CSetting *setting, void *data)
{
  return GetInstance().GetTransferManager().HasUnfinishedTransfers();
}

bool COhUPnP::StartMediaServer()
{
  if (!m_initialised)
    return false;

  if (IsMediaServerRunning())
    return true;

  StartResourceManager();

  if (m_mediaServer == nullptr)
  {
    CLog::Log(LOGINFO, "COhUPnP: starting device stack...");
    OpenHome::Net::UpnpLibrary::StartDv();

    std::string uuid = CUPnPSettings::GetInstance().GetServerUUID();

    // create a UUID if it doesn't exist yet
    if (uuid.empty())
    {
      uuid = StringUtils::CreateUUID();
      CUPnPSettings::GetInstance().SetServerUUID(uuid);
      CUPnPSettings::GetInstance().Save(GetUPnPSettingsFile());
    }

    m_mediaServer = std::make_unique<COhUPnPMediaServerDevice>(uuid, m_fileItemElementFactory, m_transferManager, m_resourceManager);
  }

  CLog::Log(LOGINFO, "COhUPnP: starting MediaServer...");
  m_mediaServer->Start(CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_UPNPTRANSFER) &&
                       CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_UPNPTRANSFERIMPORT));

  return true;
}

void COhUPnP::StopMediaServer()
{
  if (!IsMediaServerRunning())
    return;

  // stop all import transfers
  m_transferManager.StopImportTransfers();

  CLog::Log(LOGINFO, "COhUPnP: stopping media server...");
  m_mediaServer->Stop();

  StopResourceManager();
}

bool COhUPnP::IsMediaServerRunning() const
{
  if (m_mediaServer == nullptr)
    return false;

  return m_mediaServer->IsRunning();
}

bool COhUPnP::StartContentDirectoryClient()
{
  if (!m_initialised)
    return false;

  if (IsContentDirectoryClientRunning())
    return true;

  StartResourceManager();

  CLog::Log(LOGINFO, "COhUPnP: starting control point stack on subnet %s...", COhUtils::TIpAddressToString(m_subnetAddress).c_str());
  OpenHome::Net::UpnpLibrary::StartCp(m_subnetAddress);

  m_contentDirectoryClient = new COhUPnPContentDirectoryControlPoint(m_fileItemElementFactory, m_resourceManager);

  return true;
}

void COhUPnP::StopContentDirectoryClient()
{
  if (!IsContentDirectoryClientRunning())
    return;

  // stop all import transfers
  m_transferManager.StopExportTransfers();

  CLog::Log(LOGINFO, "COhUPnP: stopping control point stack...");
  delete m_contentDirectoryClient;
  m_contentDirectoryClient = nullptr;

  StopResourceManager();
}

bool COhUPnP::IsContentDirectoryClientRunning() const
{
  return m_contentDirectoryClient != nullptr;
}

bool COhUPnP::StartRenderer()
{
  // TODO

  return false;
}

void COhUPnP::StopRenderer()
{
  // TODO
}

bool COhUPnP::IsRendererRunning() const
{
  // TODO

  return false;
}

void COhUPnP::OnSettingChanged(const CSetting *setting)
{
  if (setting == nullptr)
    return;

  const std::string& settingId = setting->GetId();
  if (settingId == CSettings::SETTING_SERVICES_UPNPINTERFACE)
  {
    int subnet = static_cast<const CSettingInt*>(setting)->GetValue();
    if (subnet > 0)
    {
      if (IsContentDirectoryClientRunning())
      {
        CLog::Log(LOGINFO, "COhUPnP: changing subnet for control point stack to %s...", COhUtils::TIpAddressToString(subnet).c_str());
        OpenHome::Net::UpnpLibrary::SetCurrentSubnet(subnet);
      }
    }
  }
  else if (settingId == CSettings::SETTING_SERVICES_UPNPTRANSFER ||
           settingId == CSettings::SETTING_SERVICES_UPNPTRANSFERIMPORT)
  {
    // we need to restart the MediaServer
    CLog::Log(LOGINFO, "COhUPnP: restarting MediaServer to consider changed transfer settings...");
    StopMediaServer();
    StartMediaServer();
  }
  else if (settingId == CSettings::SETTING_DEBUG_EXTRALOGGING ||
           settingId == CSettings::SETTING_DEBUG_SETEXTRALOGLEVEL)
  {
    OpenHome::TUint logLevel = OpenHome::Debug::kError;
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      logLevel = OpenHome::Debug::kAll;

    OpenHome::Debug::SetLevel(logLevel);
  }
}

void COhUPnP::SettingOptionsUPnPInterfacesFiller(const CSetting *setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data)
{
  static_cast<void>(data);

  // make sure we are initialized
  if (!GetInstance().Initialize())
    return;

  std::vector<OpenHome::NetworkAdapter*> *subnets = OpenHome::Net::UpnpLibrary::CreateSubnetList();
  current = 0;
  for (const auto& subnet : *subnets)
  {
    list.push_back(std::make_pair(StringUtils::Format("%s (%s)", subnet->Name(), COhUtils::TIpAddressToString(subnet->Address()).c_str()), subnet->Subnet()));

    if (static_cast<const CSettingInt*>(setting)->GetValue() == subnet->Subnet())
      current = subnet->Subnet();
  }

  if (current <= 0 && !subnets->empty())
    current = subnets->at(0)->Subnet();

  OpenHome::Net::UpnpLibrary::DestroySubnetList(subnets);
}

void COhUPnP::StartResourceManager()
{
  if (m_resourceManager.IsRunning())
    return;

  uint16_t port = CUPnPSettings::GetInstance().GetServerPort();

  // start the resource manager and retrieve the real port being used
  uint16_t realPort = m_resourceManager.Start(port);

  // if the real port being used doesn't match the port from the configuration
  // replace it and save the configuration
  if (port != realPort)
  {
    CUPnPSettings::GetInstance().SetServerPort(realPort);
    CUPnPSettings::GetInstance().Save(GetUPnPSettingsFile());
  }
}

void COhUPnP::StopResourceManager(bool force /* = false */)
{
  // nothing to do if the resource manager isn't running or
  // if we aren't force stopping it and either the MediaServer or the ContentDirectory are still running
  if (!m_resourceManager.IsRunning() ||
     (!force && (IsMediaServerRunning() || !IsContentDirectoryClientRunning())))
    return;

  m_resourceManager.Stop();
}

void COhUPnP::ohNetLogOutput(const char* msg)
{
  CLog::Log(LOGDEBUG, "[ohNet] %s", msg);
}

void COhUPnP::ohNetFatalErrorHandler(const char* msg)
{
  CLog::Log(LOGFATAL, "[ohNet] %s", msg);
}
