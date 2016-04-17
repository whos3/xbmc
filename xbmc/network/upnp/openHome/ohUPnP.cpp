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
#include "dialogs/GUIDialogJobManager.h"
#include "guilib/GUIWindowManager.h"
#include "network/upnp/UPnPSettings.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/controlpoints/ohUPnPAVTransportControlPoint.h"
#include "network/upnp/openHome/controlpoints/ohUPnPConnectionManagerControlPoint.h"
#include "network/upnp/openHome/controlpoints/ohUPnPContentDirectoryControlPoint.h"
#include "network/upnp/openHome/controlpoints/ohUPnPRenderingControlControlPoint.h"
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
#include "network/upnp/openHome/didllite/objects/item/audio/UPnPAudioBookItem.h"
#include "network/upnp/openHome/didllite/objects/item/audio/UPnPAudioBroadcastItem.h"
#include "network/upnp/openHome/didllite/objects/item/audio/UPnPAudioItem.h"
#include "network/upnp/openHome/didllite/objects/item/audio/UPnPMusicTrackItem.h"
#include "network/upnp/openHome/didllite/objects/item/image/UPnPImageItem.h"
#include "network/upnp/openHome/didllite/objects/item/image/UPnPPhotoImageItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPMovieItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPMusicVideoClipItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPVideoBroadcastItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPVideoItem.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfilesManager.h"
#include "network/upnp/openHome/rootdevices/mediarenderer/ohUPnPMediaRendererDevice.h"
#include "network/upnp/openHome/rootdevices/mediaserver/ohUPnPMediaServerDevice.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "profiles/ProfilesManager.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "settings/lib/Setting.h"
#include "threads/SingleLock.h"
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
    m_controlPointStackStarted(false),
    m_ipAddress(0),
    m_subnetAddress(0),
    m_ohEnvironment(nullptr),
    m_contentDirectoryClient(nullptr),
    m_connectionManagerController(nullptr),
    m_renderingController(nullptr),
    m_playbackController(nullptr),
    m_mediaServerDevice(nullptr),
    m_mediaRendererDevice(nullptr)
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
  CSingleLock lock(m_criticalInitialised);
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
  m_fileItemElementFactory.RegisterElement(new CUPnPAudioItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPAudioBookItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPAudioBroadcastItem());
  m_fileItemElementFactory.RegisterElement(new CUPnPMusicTrackItem());
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

  // determine the renderer and server UUID
  std::string rendererUuid = CUPnPSettings::GetInstance().GetRendererUUID();
  std::string serverUuid = CUPnPSettings::GetInstance().GetServerUUID();

  bool saveSettings = false;
  // create a renderer UUID if it doesn't exist yet
  if (rendererUuid.empty())
  {
    rendererUuid = StringUtils::CreateUUID();
    CUPnPSettings::GetInstance().SetRendererUUID(serverUuid);
    saveSettings = true;
  }

  // create a server UUID if it doesn't exist yet
  if (serverUuid.empty())
  {
    serverUuid = StringUtils::CreateUUID();
    CUPnPSettings::GetInstance().SetServerUUID(serverUuid);
    saveSettings = true;
  }

  if (saveSettings)
    CUPnPSettings::GetInstance().Save(GetUPnPSettingsFile());

  {
    CSingleLock lock(m_criticalMediaRendererDevice);
    m_mediaRendererDevice = std::make_unique<COhUPnPMediaRendererDevice>(rendererUuid, m_fileItemElementFactory, m_transferManager, m_resourceManager);
  }
  {
    CSingleLock lock(m_criticalMediaServerDevice);
    m_mediaServerDevice = std::make_unique<COhUPnPMediaServerDevice>(serverUuid, m_fileItemElementFactory, m_transferManager, m_resourceManager);
  }

  m_initialised = true;
  return true;
}

void COhUPnP::Uninitialize()
{
  CSingleLock lock(m_criticalInitialised);
  if (!m_initialised)
    return;

  StopContentDirectoryClient();
  StopRenderingController();
  StopPlaybackController();
  StopConnectionManagerController();

  StopMediaRenderer();
  StopMediaServer();
  StopResourceManager(true);

  {
    CSingleLock lock(m_criticalMediaRendererDevice);
    m_mediaRendererDevice.reset();
  }
  {
    CSingleLock lock(m_criticalMediaServerDevice);
    m_mediaServerDevice.reset();
  }

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
  CSingleLock lock(m_criticalInitialised);
  if (!m_initialised)
    return false;

  if (IsMediaServerRunning())
    return true;

  StartResourceManager();

  m_mediaServerDevice->SetSupportImporting(CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_UPNPTRANSFER) &&
                                           CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_UPNPTRANSFERIMPORT));

  StartMediaServerDevice(true);

  return true;
}

void COhUPnP::StopMediaServer()
{
  if (!IsMediaServerRunning())
    return;

  // stop all import transfers
  m_transferManager.StopImportTransfers();

  StopMediaServerDevice();

  StopResourceManager();
}

bool COhUPnP::IsMediaServerRunning() const
{
  return m_mediaServerDevice != nullptr && m_mediaServerDevice->IsRunning();
}

bool COhUPnP::StartContentDirectoryClient()
{
  CSingleLock lockInitialised(m_criticalInitialised);
  if (!m_initialised)
    return false;

  CSingleLock lock(m_criticalContentDirectoryClient);
  if (IsContentDirectoryClientRunning())
    return true;

  StartResourceManager();

  StartControlPointStack();

  CLog::Log(LOGINFO, "COhUPnP: starting ContentDirectory control point...");
  m_contentDirectoryClient = new COhUPnPContentDirectoryControlPoint(m_fileItemElementFactory, m_resourceManager);

  return true;
}

void COhUPnP::StopContentDirectoryClient()
{
  CSingleLock lock(m_criticalContentDirectoryClient);
  if (!IsContentDirectoryClientRunning())
    return;

  // stop all import transfers
  m_transferManager.StopExportTransfers();

  CLog::Log(LOGINFO, "COhUPnP: stopping ContentDirectory control point...");
  delete m_contentDirectoryClient;
  m_contentDirectoryClient = nullptr;

  StopResourceManager();
}

bool COhUPnP::IsContentDirectoryClientRunning() const
{
  return m_contentDirectoryClient != nullptr;
}

bool COhUPnP::StartConnectionManagerController()
{
  CSingleLock lockInitialised(m_criticalInitialised);
  if (!m_initialised)
    return false;

  CSingleLock lock(m_criticalConnectionManagerController);
  if (IsConnectionManagerControllerRunning())
    return true;

  StartControlPointStack();

  CLog::Log(LOGINFO, "COhUPnP: starting ConnectionManager control point...");
  m_connectionManagerController = new COhUPnPConnectionManagerControlPointManager();

  return true;
}

void COhUPnP::StopConnectionManagerController()
{
  CSingleLock lock(m_criticalConnectionManagerController);
  if (!IsConnectionManagerControllerRunning())
    return;

  CLog::Log(LOGINFO, "COhUPnP: stopping ConnectionManager control point...");
  delete m_connectionManagerController;
  m_connectionManagerController = nullptr;
}

bool COhUPnP::IsConnectionManagerControllerRunning() const
{
  return m_connectionManagerController != nullptr;
}

std::shared_ptr<COhUPnPConnectionManagerControlPoint> COhUPnP::CreateConnectionManagerController(const std::string& uuid, IOhUPnPConnectionManagerControlPointAsync* callback) const
{
  CSingleLock lock(m_criticalConnectionManagerController);
  if (!IsConnectionManagerControllerRunning())
    return nullptr;

  return m_connectionManagerController->CreateControlPoint(uuid, callback);
}

void COhUPnP::DestroyConnectionManagerController(std::shared_ptr<COhUPnPConnectionManagerControlPoint> connectionManagerController) const
{
  CSingleLock lock(m_criticalConnectionManagerController);
  if (!IsConnectionManagerControllerRunning())
    return;

  m_connectionManagerController->DestroyControlPoint(connectionManagerController);
}

bool COhUPnP::StartRenderingController()
{
  CSingleLock lockInitialised(m_criticalInitialised);
  if (!m_initialised)
    return false;

  CSingleLock lock(m_criticalRenderingController);
  if (IsRenderingControllerRunning())
    return true;

  StartControlPointStack();

  CLog::Log(LOGINFO, "COhUPnP: starting RenderingControl control point...");
  m_renderingController = new COhUPnPRenderingControlControlPointManager(UPNP_DEVICE_TYPE_MEDIARENDERER);

  return true;
}

void COhUPnP::StopRenderingController()
{
  CSingleLock lock(m_criticalRenderingController);
  if (!IsRenderingControllerRunning())
    return;

  CLog::Log(LOGINFO, "COhUPnP: stopping RenderingControl control point...");
  delete m_renderingController;
  m_renderingController = nullptr;
}

bool COhUPnP::IsRenderingControllerRunning() const
{
  return m_renderingController != nullptr;
}

std::shared_ptr<COhUPnPRenderingControlControlPoint> COhUPnP::CreateRenderingController(const std::string& uuid, IOhUPnPRenderingControlControlPointAsync* callback) const
{
  CSingleLock lock(m_criticalRenderingController);
  if (!IsRenderingControllerRunning())
    return nullptr;

  return m_renderingController->CreateControlPoint(uuid, callback);
}

void COhUPnP::DestroyRenderingController(std::shared_ptr<COhUPnPRenderingControlControlPoint> renderingController) const
{
  CSingleLock lock(m_criticalRenderingController);
  if (!IsRenderingControllerRunning())
    return;

  m_renderingController->DestroyControlPoint(renderingController);
}

bool COhUPnP::StartPlaybackController()
{
  CSingleLock lockInitialised(m_criticalInitialised);
  if (!m_initialised)
    return false;

  CSingleLock lock(m_criticalPlaybackController);
  if (IsPlaybackControllerRunning())
    return true;

  StartControlPointStack();

  CLog::Log(LOGINFO, "COhUPnP: starting MediaRenderer AVTransport control point...");
  m_playbackController = new COhUPnPAVTransportControlPointManager(UPNP_DEVICE_TYPE_MEDIARENDERER);

  return true;
}

void COhUPnP::StopPlaybackController()
{
  CSingleLock lock(m_criticalPlaybackController);
  if (!IsPlaybackControllerRunning())
    return;

  CLog::Log(LOGINFO, "COhUPnP: stopping MediaRenderer AVTransport control point...");
  delete m_playbackController;
  m_playbackController = nullptr;
}

bool COhUPnP::IsPlaybackControllerRunning() const
{
  return m_playbackController != nullptr;
}

std::shared_ptr<COhUPnPAVTransportControlPoint> COhUPnP::CreatePlaybackController(const std::string& uuid, IOhUPnPAVTransportControlPointAsync* callback) const
{
  CSingleLock lock(m_criticalPlaybackController);
  if (!IsPlaybackControllerRunning())
    return nullptr;

  return m_playbackController->CreateControlPoint(uuid, callback);
}

void COhUPnP::DestroyPlaybackController(std::shared_ptr<COhUPnPAVTransportControlPoint> playbackController) const
{
  CSingleLock lock(m_criticalPlaybackController);
  if (!IsPlaybackControllerRunning())
    return;

  m_playbackController->DestroyControlPoint(playbackController);
}

bool COhUPnP::StartMediaRenderer()
{
  CSingleLock lock(m_criticalInitialised);
  if (!m_initialised)
    return false;

  if (IsMediaRendererRunning())
    return true;

  StartMediaRendererDevice(true);

  return true;
}

void COhUPnP::StopMediaRenderer()
{
  if (!IsMediaRendererRunning())
    return;

  StopMediaRendererDevice();
}

bool COhUPnP::IsMediaRendererRunning() const
{
  CSingleLock lock(m_criticalMediaRendererDevice);
  return m_mediaRendererDevice != nullptr && m_mediaRendererDevice->IsRunning();
}

void COhUPnP::OnSettingChanged(const CSetting* setting)
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
    if (IsMediaServerRunning())
    {
      // we need to restart the MediaServer
      CLog::Log(LOGINFO, "COhUPnP: restarting MediaServer to consider changed transfer settings...");
      StopMediaServer();
      StartMediaServer();
    }
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

void COhUPnP::OnSettingAction(const CSetting* setting)
{
  if (setting == nullptr)
    return;

  const std::string& settingId = setting->GetId();
  if (settingId == CSettings::SETTING_SERVICES_UPNPTRANSFERMANAGE)
  {
    auto* dialog = static_cast<CGUIDialogJobManager*>(g_windowManager.GetWindow(WINDOW_DIALOG_JOB_MANAGER));
    dialog->SetHeading(CVariant{ "UPnP Transfers" }); // TODO: localization
    dialog->ShowEndedJobs(true);
    dialog->SetManageableJobQueue(&m_transferManager);
    dialog->Open();
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

void COhUPnP::StartControlPointStack()
{
  if (m_controlPointStackStarted)
    return;

  CLog::Log(LOGINFO, "COhUPnP: starting control point stack on subnet %s...", COhUtils::TIpAddressToString(m_subnetAddress).c_str());
  OpenHome::Net::UpnpLibrary::StartCp(m_subnetAddress);

  m_controlPointStackStarted = true;
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

void COhUPnP::StartMediaServerDevice(bool forceRestart /* = false */)
{
  CSingleLock lock(m_criticalMediaServerDevice);
  if (m_mediaServerDevice == nullptr)
    return;

  if (!forceRestart && m_mediaServerDevice->IsRunning())
    return;

  if (forceRestart && m_mediaServerDevice->IsRunning())
    StopMediaServerDevice();

  m_mediaServerDevice->Start(m_ipAddress);
}

void COhUPnP::StopMediaServerDevice()
{
  CSingleLock lock(m_criticalMediaServerDevice);
  if (m_mediaServerDevice == nullptr)
    return;

  m_mediaServerDevice->Stop();
}

void COhUPnP::StartMediaRendererDevice(bool forceRestart /* = false */)
{
  CSingleLock lock(m_criticalMediaRendererDevice);
  if (m_mediaRendererDevice == nullptr)
    return;

  if (!forceRestart && m_mediaRendererDevice->IsRunning())
    return;

  if (forceRestart && m_mediaRendererDevice->IsRunning())
    StopMediaRendererDevice();

  m_mediaRendererDevice->Start(m_ipAddress);
}

void COhUPnP::StopMediaRendererDevice()
{
  CSingleLock lock(m_criticalMediaRendererDevice);
  if (m_mediaRendererDevice == nullptr)
    return;

  m_mediaRendererDevice->Stop();
}

void COhUPnP::ohNetLogOutput(const char* msg)
{
  CLog::Log(LOGDEBUG, "[ohNet] %s", msg);
}

void COhUPnP::ohNetFatalErrorHandler(const char* msg)
{
  CLog::Log(LOGFATAL, "[ohNet] %s", msg);
}
