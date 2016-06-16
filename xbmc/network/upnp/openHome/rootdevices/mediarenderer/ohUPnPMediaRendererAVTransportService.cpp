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

#include <limits>

#include "ohUPnPMediaRendererAVTransportService.h"
#include "Application.h"
#include "FileItem.h"
#include "GUIInfoManager.h" // TODO
#include "GUIUserMessages.h"
#include "guilib/GUIWindowManager.h" // TODO
#include "input/Key.h"
#include "interfaces/AnnouncementManager.h"
#include "messaging/ApplicationMessenger.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfile.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfilesManager.h"
#include "network/upnp/openHome/didllite/objects/FileItemUtils.h"
#include "network/upnp/openHome/rootdevices/ohUPnPClientDevice.h"
#include "network/upnp/openHome/utils/ohUPnPAVTransportLastChange.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "pictures/GUIWindowSlideShow.h"
#include "pictures/PictureInfoTag.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "utils/propertytree/xml/XmlPropertyTreeSerializer.h"

static bool IsSlideshowActive()
{
  return g_windowManager.GetActiveWindow() == WINDOW_SLIDESHOW;
}

COhUPnPMediaRendererAVTransportService::COhUPnPMediaRendererAVTransportService(COhUPnPRootDevice& device,
  const CFileItemElementFactory& fileItemElementFactory)
  : IOhUPnPService(UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_AVTRANSPORT, 2)
  , m_device(device)
  , m_avTransport(nullptr)
  , m_elementFactory(fileItemElementFactory)
{ }

COhUPnPMediaRendererAVTransportService::~COhUPnPMediaRendererAVTransportService()
{
  Stop();
}

void COhUPnPMediaRendererAVTransportService::Start()
{
  if (m_avTransport != nullptr)
    return;

  // create a ContentDirectory service
  m_avTransport.reset(new AVTransport(*this, *m_device.GetDevice()));
}

void COhUPnPMediaRendererAVTransportService::Stop()
{
  if (!IsRunning())
    return;

  // then destroy the ContentDirectory service
  m_avTransport.reset();
}

bool COhUPnPMediaRendererAVTransportService::IsRunning() const
{
  return m_avTransport != nullptr;
}

void COhUPnPMediaRendererAVTransportService::UpdateState()
{
  if (m_avTransport != nullptr)
    m_avTransport->UpdateState();
}

COhUPnPMediaRendererAVTransportService::AVTransport::AVTransport(COhUPnPMediaRendererAVTransportService& service, OpenHome::Net::DvDeviceStd& device)
  : OpenHome::Net::DvProviderUpnpOrgAVTransport2Cpp(device)
  , m_service(service)
{
  // default values
  m_variables.SetTransportState(OhUPnPAVTransportTransportState::Stopped);
  m_variables.SetTransportStatus(OhUPnPAVTransportTransportStatus::OK);
  m_variables.SetTransportPlaySpeed(1.f);
  m_variables.SetNumberOfTracks(0);
  m_variables.SetCurrentTrack(0);
  m_variables.SetCurrentMediaDuration(0);
  m_variables.SetCurrentTrackDuration(0);
  m_variables.SetRelativeTimePosition(0);
  m_variables.SetAbsoluteTimePosition(0);
  m_variables.SetRelativeCounterPosition(std::numeric_limits<int32_t>::max());
  m_variables.SetAbsoluteCounterPosition(std::numeric_limits<uint32_t>::max());
  m_variables.Fix();

  // enable properties
  EnablePropertyLastChange();
  SetPropertyLastChange("");

  // enable required actions
  EnableActionSetAVTransportURI();
  EnableActionGetMediaInfo();
  EnableActionGetTransportInfo();
  EnableActionGetPositionInfo();
  EnableActionGetDeviceCapabilities();
  EnableActionGetTransportSettings();
  EnableActionStop();
  EnableActionPlay();
  EnableActionSeek();
  EnableActionNext();
  EnableActionPrevious();

  // enable optional actions
  EnableActionSetNextAVTransportURI();
  EnableActionPause();
  EnableActionSetPlayMode();
  EnableActionGetCurrentTransportActions();
  EnableActionGetStateVariables();

  ANNOUNCEMENT::CAnnouncementManager::GetInstance().AddAnnouncer(this);
}

COhUPnPMediaRendererAVTransportService::AVTransport::~AVTransport()
{
  ANNOUNCEMENT::CAnnouncementManager::GetInstance().RemoveAnnouncer(this);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::UpdateState()
{
  COhUPnPAVTransportLastChangeLock lock(m_variables);

  // don't update state while transitioning
  if (m_variables.GetTransportState() == OhUPnPAVTransportTransportState::Transitioning)
    return;

  m_variables.SetTransportStatus(OhUPnPAVTransportTransportStatus::OK);

  if (g_application.m_pPlayer->IsPlaying() || g_application.m_pPlayer->IsPausedPlayback())
  {
    m_variables.SetNumberOfTracks(1); // TODO: playlists
    m_variables.SetCurrentTrack(1); // TODO: playlists

    uint32_t position = static_cast<uint32_t>(g_infoManager.GetPlayTime() / 1000);
    uint32_t duration = static_cast<uint32_t>(g_infoManager.GetPlayDuration());
    m_variables.SetRelativeTimePosition(position);
    m_variables.SetAbsoluteTimePosition(position);
    m_variables.SetCurrentTrackDuration(duration);
    m_variables.SetCurrentMediaDuration(duration);
  }
  else if (IsSlideshowActive())
  {
    m_variables.SetTransportState(OhUPnPAVTransportTransportState::Playing);
    const std::string path = g_infoManager.GetPictureLabel(SLIDE_FILE_PATH);
    m_variables.SetAVTransportURI(path);
    m_variables.SetCurrentTrackURI(path);
    m_variables.SetTransportPlaySpeed(1);

    CGUIWindowSlideShow *slideshow = (CGUIWindowSlideShow *)g_windowManager.GetWindow(WINDOW_SLIDESHOW);
    if (slideshow == nullptr)
    {
      m_variables.SetNumberOfTracks(slideshow->NumSlides());
      m_variables.SetCurrentTrack(slideshow->CurrentSlide());
    }

    m_variables.SetCurrentTrackMetadata("");
    m_variables.SetAVTransportURIMetadata("");
  }
  else
  {
    m_variables.SetTransportState(OhUPnPAVTransportTransportState::Stopped);
    m_variables.SetTransportPlaySpeed(1);
    m_variables.SetNumberOfTracks(0);
    m_variables.SetCurrentTrack(0);
    m_variables.SetRelativeTimePosition(0);
    m_variables.SetAbsoluteTimePosition(0);
    m_variables.SetCurrentTrackDuration(0);
    m_variables.SetCurrentMediaDuration(0);
    m_variables.SetNextAVTransportURI("");
    m_variables.SetNextAVTransportURIMetadata("");
  }

  UpdateLastChange();
}

void COhUPnPMediaRendererAVTransportService::AVTransport::SetAVTransportURI(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& currentURI,
  const std::string& currentURIMetaData)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- SetAVTransportURI(%u, %s, %s)", instanceID, currentURI.c_str(), currentURIMetaData.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: SetAVTransportURI() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  if (currentURI.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: unable to set empty URI");
    invocation.ReportError(UPNP_ERROR_INVALID_ARGUMENT_VALUE, "Argument Value Invalid");
    return;
  }

  // if the player isn't already playing wait for a call to Play()
  if (!g_application.m_pPlayer->IsPlaying() && !IsSlideshowActive())
  {
    COhUPnPAVTransportLastChangeLock lock(m_variables);
    m_variables.SetTransportState(OhUPnPAVTransportTransportState::Stopped);
    m_variables.SetTransportStatus(OhUPnPAVTransportTransportStatus::OK);
    m_variables.SetTransportPlaySpeed(1);
    m_variables.SetAVTransportURI(currentURI);
    m_variables.SetAVTransportURIMetadata(currentURIMetaData);
    m_variables.SetNextAVTransportURI("");
    m_variables.SetNextAVTransportURIMetadata("");

    UpdateLastChange();
  }
  else
    Play(currentURI, currentURIMetaData, clientDevice);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::SetNextAVTransportURI(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& nextURI,
  const std::string& nextURIMetaData)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- SetNextAVTransportURI(%u, %s, %s)", instanceID, nextURI.c_str(), nextURIMetaData.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: SetNextAVTransportURI() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  // for slideshows we can't handle a next item
  if (IsSlideshowActive())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: unable to handle next URI \"%s\" for active slideshow", nextURI.c_str());
    invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Action Failed");
    return;
  }

  CFileItem item;
  if (!GetFileItemFromMetadata(nextURI, nextURIMetaData, clientDevice, item))
    CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: failed to deserialize metadata for URI \"%s\": %s", nextURI.c_str(), nextURIMetaData.c_str());

  if (!g_application.m_pPlayer->IsPlaying())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: unable to handle next URI \"%s\" because there's nothing playing", nextURI.c_str());
    invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Action Failed");
    return;
  }

  // determine the playlist
  int playlist = PLAYLIST_MUSIC;
  if (item.IsVideo())
    playlist = PLAYLIST_VIDEO;

  {
    CSingleLock lock(g_graphicsContext);
    g_playlistPlayer.ClearPlaylist(playlist);
    g_playlistPlayer.Add(playlist, CFileItemPtr(new CFileItem(item)));

    g_playlistPlayer.SetCurrentSong(-1);
    g_playlistPlayer.SetCurrentPlaylist(playlist);
  }

  CGUIMessage msg(GUI_MSG_PLAYLIST_CHANGED, 0, 0);
  g_windowManager.SendThreadMessage(msg);

  m_variables.SetNextAVTransportURI(nextURI);
  m_variables.SetNextAVTransportURIMetadata(nextURIMetaData);

  UpdateLastChange();
}

void COhUPnPMediaRendererAVTransportService::AVTransport::GetMediaInfo(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, uint32_t& nrTracks, std::string& mediaDuration,
  std::string& currentURI, std::string& currentURIMetaData, std::string& nextURI, std::string& nextURIMetaData, std::string& playMedium, std::string& recordMedium, std::string& writeStatus)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetMediaInfo(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: GetMediaInfo() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPAVTransportLastChangeLock lock(m_variables);
  nrTracks = m_variables.GetNumberOfTracks();
  mediaDuration = m_variables.GetCurrentMediaDuration();
  currentURI = m_variables.GetAVTransportURI();
  currentURIMetaData = m_variables.GetAVTransportURIMetaData();
  nextURI = m_variables.GetNextAVTransportURI();
  nextURIMetaData = m_variables.GetNextAVTransportURIMetaData();
  playMedium = m_variables.GetPlaybackStorageMedium().GetAsString();
  recordMedium = m_variables.GetRecordStorageMedium().GetAsString();
  writeStatus = m_variables.GetRecordMediumWriteStatus().GetAsString();

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> GetMediaInfo(%u): tracks = %u; duration = %s; current URI = %s; current metadata = %s; next URI = %s; next metadata = %s; play medium = %s; record medium = %s; write status = %s",
      instanceID, nrTracks, mediaDuration.c_str(), currentURI.c_str(), currentURIMetaData.c_str(), nextURI.c_str(), nextURIMetaData.c_str(), playMedium.c_str(), recordMedium.c_str(), writeStatus.c_str());
  }
}

void COhUPnPMediaRendererAVTransportService::AVTransport::GetTransportInfo(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& currentTransportState,
  std::string& currentTransportStatus, std::string& currentSpeed)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportInfo(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: GetTransportInfo() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPAVTransportLastChangeLock lock(m_variables);
  currentTransportState = m_variables.GetTransportState().GetAsString();
  currentTransportStatus = m_variables.GetTransportStatus().GetAsString();
  currentSpeed = StringUtils::Format("%.1f", m_variables.GetTransportPlaySpeed());

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> GetTransportInfo(%u): current state = %s; current status = %s; current speed = %s",
      instanceID, currentTransportState.c_str(), currentTransportStatus.c_str(), currentSpeed.c_str());
  }
}

void COhUPnPMediaRendererAVTransportService::AVTransport::GetPositionInfo(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, uint32_t& track, std::string& trackDuration,
  std::string& trackMetaData, std::string& trackURI, std::string& relTime, std::string& absTime, int32_t& relCount, int32_t& absCount)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetPositionInfo(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: GetPositionInfo() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPAVTransportLastChangeLock lock(m_variables);
  track = m_variables.GetCurrentTrack();
  trackDuration = m_variables.GetCurrentTrackDuration();
  trackMetaData = m_variables.GetCurrentTrackMetaData();
  trackURI = m_variables.GetCurrentTrackURI();
  relTime = m_variables.GetRelativeTimePosition();
  absTime = m_variables.GetAbsoluteTimePosition();
  relCount = std::numeric_limits<int32_t>::max(); // not supported
  absCount = std::numeric_limits<int32_t>::max(); // not supported

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> GetPositionInfo(%u): track = %u; metadata = %s; URI = %s; relative time = %s; absolute time = %s; relative count = %d; absolute count = %d",
      instanceID, track, trackDuration.c_str(), trackMetaData.c_str(), trackURI.c_str(), relTime.c_str(), absTime.c_str(), relCount, absCount);
  }
}

void COhUPnPMediaRendererAVTransportService::AVTransport::GetDeviceCapabilities(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& playMedia, std::string& recMedia,
  std::string& recQualityModes)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetDeviceCapabilities(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: GetDeviceCapabilities() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPAVTransportLastChangeLock lock(m_variables);
  std::vector<std::string> possiblePlaybackStorageMedia;
  for (const auto& playbackMedium : m_variables.GetPossiblePlaybackStorageMedia())
    possiblePlaybackStorageMedia.push_back(playbackMedium.GetAsString());
  playMedia = COhUtils::ToCSV(possiblePlaybackStorageMedia);

  std::vector<std::string> possibleRecordStorageMedia;
  for (const auto& recordMedium : m_variables.GetPossibleRecordStorageMedia())
    possibleRecordStorageMedia.push_back(recordMedium.GetAsString());
  recMedia = COhUtils::ToCSV(possibleRecordStorageMedia);

  std::vector<std::string> possibleRecordQualityModes;
  for (const auto& recordQualityMode : m_variables.GetPossibleRecordQualityModes())
    possibleRecordQualityModes.push_back(recordQualityMode.GetAsString());
  recQualityModes = COhUtils::ToCSV(possibleRecordQualityModes);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetDeviceCapabilities(%u): play media = %s; record media = %s; record quality modes = %s", instanceID, playMedia.c_str(), recMedia.c_str(), recQualityModes.c_str());
}

void COhUPnPMediaRendererAVTransportService::AVTransport::GetTransportSettings(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& playMode, std::string& recQualityMode)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransportSettings(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: GetTransportSettings() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPAVTransportLastChangeLock lock(m_variables);
  playMode = m_variables.GetCurrentPlayMode().GetAsString();
  recQualityMode = m_variables.GetCurrentRecordQualityMode().GetAsString();

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetTransportSettings(%u): play mode = %s; record quality mode = %s", instanceID, playMode.c_str(), recQualityMode.c_str());
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Stop(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- Stop(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: Stop() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  if (IsSlideshowActive())
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_GUI_ACTION, WINDOW_SLIDESHOW, -1, static_cast<void*>(new CAction(ACTION_NEXT_PICTURE)));
  else
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_MEDIA_STOP);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Play(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& speed)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- Play(%u, %s)", instanceID, speed.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: Play() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  if (IsSlideshowActive())
    return;

  if (g_application.m_pPlayer->IsPausedPlayback())
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_MEDIA_PAUSE);
  else if (!g_application.m_pPlayer->IsPlaying())
    Play(m_variables.GetAVTransportURI(), m_variables.GetAVTransportURIMetaData(), clientDevice);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Pause(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- Pause(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: Pause() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  if (IsSlideshowActive())
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_GUI_ACTION, WINDOW_SLIDESHOW, -1, static_cast<void*>(new CAction(ACTION_NEXT_PICTURE)));
  else if (!g_application.m_pPlayer->IsPausedPlayback())
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_MEDIA_PAUSE);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Seek(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& unit, const std::string& target)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- Seek(%u, %s, %s)", instanceID, unit.c_str(), target.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: Seek() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  // can't seek if nothing is playing
  if (!g_application.m_pPlayer->IsPlaying())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: can't seek because nothing is playing");
    invocation.ReportError(UPNP_ERROR_AVT_NO_CONTENTS, "No contents");
    return;
  }

  COhUPnPAVTransportSeekUnit seekUnit(unit);
  // for now we only support seeking by relative time
  // TODO: support more seek units
  if (seekUnit != OhUPnPAVTransportSeekUnit::RelativeTime)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: seek unit \"%s\" is not supported", unit.c_str());
    invocation.ReportError(UPNP_ERROR_AVT_UNSUPPORTED_SEEK_MODE, "Seek mode not supported");
    return;
  }

  g_application.SeekTime(static_cast<double>(COhUtils::GetDurationInSeconds(target)));
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Next(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- Next(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: Next() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  if (IsSlideshowActive())
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_GUI_ACTION, WINDOW_SLIDESHOW, -1, static_cast<void*>(new CAction(ACTION_NEXT_PICTURE)));
  else
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_PLAYLISTPLAYER_NEXT);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Previous(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- Previous(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: Previous() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  if (IsSlideshowActive())
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_GUI_ACTION, WINDOW_SLIDESHOW, -1, static_cast<void*>(new CAction(ACTION_PREV_PICTURE)));
  else
    KODI::MESSAGING::CApplicationMessenger::GetInstance().SendMsg(TMSG_PLAYLISTPLAYER_PREV);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::SetPlayMode(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& newPlayMode)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- SetPlayMode(%u, %s)", instanceID, newPlayMode.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: SetPlayMode() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  m_variables.SetCurrentPlayMode(COhUPnPAVTransportPlayMode(newPlayMode));

  UpdateLastChange();
}

void COhUPnPMediaRendererAVTransportService::AVTransport::GetCurrentTransportActions(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, std::string& actions)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetCurrentTransportActions(%u)", instanceID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: GetCurrentTransportActions() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_AVT_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPAVTransportLastChangeLock lock(m_variables);
  std::vector<std::string> currentTransportActions;
  for (const auto& transportAction : m_variables.GetCurrentTransportActions())
    currentTransportActions.push_back(transportAction.GetAsString());
  actions = COhUtils::ToCSV(currentTransportActions);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetCurrentTransportActions(%u): actions = %s", instanceID, actions.c_str());
}

void COhUPnPMediaRendererAVTransportService::AVTransport::GetStateVariables(OpenHome::Net::IDvInvocationStd& invocation, uint32_t instanceID, const std::string& stateVariableList,
  std::string& stateVariableValuePairs)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetStateVariables(%u, %s)", instanceID, stateVariableList.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: GetStateVariables() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (instanceID != 0)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: invalid instance ID %u referenced", instanceID);
    invocation.ReportError(UPNP_ERROR_RC_INVALID_INSTANCE_ID, "Invalid InstanceID");
    return;
  }

  COhUPnPAVTransportLastChangeLock lock(m_variables);

  const auto& stateVariables = COhUtils::SplitCSV(stateVariableList);
  std::vector<std::string> stateVariableValues;
  for (const auto& stateVariable : stateVariables)
  {
    std::string value;
    if (stateVariable == "TransportState")
      value = m_variables.GetTransportState().GetAsString();
    else if (stateVariable == "TransportStatus")
      value = m_variables.GetTransportStatus().GetAsString();
    else if (stateVariable == "CurrentMediaCategory")
      value = m_variables.GetCurrentMediaCategory().GetAsString();
    else if (stateVariable == "PlaybackStorageMedium")
      value = m_variables.GetPlaybackStorageMedium().GetAsString();
    else if (stateVariable == "RecordStorageMedium")
      value = m_variables.GetRecordStorageMedium().GetAsString();
    else if (stateVariable == "PossiblePlaybackStorageMedia")
    {
      std::vector<std::string> possiblePlaybackStorageMedia;
      for (const auto& playbackMedium : m_variables.GetPossiblePlaybackStorageMedia())
        possiblePlaybackStorageMedia.push_back(playbackMedium.GetAsString());
      value = COhUtils::ToCSV(possiblePlaybackStorageMedia);
    }
    else if (stateVariable == "PossibleRecordStorageMedia")
    {
      std::vector<std::string> possibleRecordStorageMedia;
      for (const auto& recordMedium : m_variables.GetPossibleRecordStorageMedia())
        possibleRecordStorageMedia.push_back(recordMedium.GetAsString());
      value = COhUtils::ToCSV(possibleRecordStorageMedia);
    }
    else if (stateVariable == "CurrentPlayMode")
      value = m_variables.GetCurrentPlayMode().GetAsString();
    else if (stateVariable == "TransportPlaySpeed")
      value = StringUtils::Format("%.1f", m_variables.GetTransportPlaySpeed());
    else if (stateVariable == "RecordMediumWriteStatus")
      value = m_variables.GetRecordMediumWriteStatus().GetAsString();
    else if (stateVariable == "PossibleRecordQualityModes")
    {
      std::vector<std::string> possibleRecordQualityModes;
      for (const auto& recordQualityMode : m_variables.GetPossibleRecordQualityModes())
        possibleRecordQualityModes.push_back(recordQualityMode.GetAsString());
      value = COhUtils::ToCSV(possibleRecordQualityModes);
    }
    else if (stateVariable == "CurrentRecordQualityMode")
      value = m_variables.GetCurrentRecordQualityMode().GetAsString();
    else if (stateVariable == "NumberOfTracks")
      value = StringUtils::Format("%u", m_variables.GetNumberOfTracks());
    else if (stateVariable == "CurrentTrack")
      value = StringUtils::Format("%u", m_variables.GetCurrentTrack());
    else if (stateVariable == "CurrentTrackDuration")
      value = m_variables.GetCurrentTrackDuration();
    else if (stateVariable == "CurrentMediaDuration")
      value = m_variables.GetCurrentMediaDuration();
    else if (stateVariable == "CurrentTrackURI")
      value = m_variables.GetCurrentTrackURI();
    else if (stateVariable == "CurrentTrackMetaData")
    {
      // don't get the value from the state variable as that value does not respect any device specific profiles 
      if (g_application.m_pPlayer->IsPlaying())
      {
        if (!GetMetadataFromFileItem(g_application.CurrentFileItem(), clientDevice, value))
          CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: failed to serialize CurrentTrackMetaData for item \"%s\"", g_application.CurrentFileItem().GetLabel().c_str());
      }
    }
    else if (stateVariable == "AVTransportURI")
      value = m_variables.GetAVTransportURI();
    else if (stateVariable == "AVTransportURIMetaData")
    {
      // don't get the value from the state variable as that value does not respect any device specific profiles
      if (g_application.m_pPlayer->IsPlaying())
      {
        if (!GetMetadataFromFileItem(g_application.CurrentFileItem(), clientDevice, value))
          CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: failed to serialize AVTransportURIMetaData for item \"%s\"", g_application.CurrentFileItem().GetLabel().c_str());
      }
    }
    else if (stateVariable == "NextAVTransportURI")
      value = m_variables.GetNextAVTransportURI();
    else if (stateVariable == "NextAVTransportURIMetaData")
      value = m_variables.GetNextAVTransportURIMetaData();
    else if (stateVariable == "RelativeTimePosition")
      value = m_variables.GetRelativeTimePosition();
    else if (stateVariable == "AbsoluteTimePosition")
      value = m_variables.GetAbsoluteTimePosition();
    else if (stateVariable == "RelativeCounterPosition")
      value = StringUtils::Format("%d", m_variables.GetRelativeCounterPosition()); // not supported
    else if (stateVariable == "AbsoluteCounterPosition")
      value = StringUtils::Format("%u", m_variables.GetAbsoluteCounterPosition()); // not supported
    else if (stateVariable == "CurrentTransportActions")
    {
      std::vector<std::string> currentTransportActions;
      for (const auto& transportAction : m_variables.GetCurrentTransportActions())
        currentTransportActions.push_back(transportAction.GetAsString());
      value = COhUtils::ToCSV(currentTransportActions);
    }
    else
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaRendererAVTransportService: unknown state variable \"%s\" requested", stateVariable.c_str());
      continue;
    }

    stateVariableValues.push_back(StringUtils::Format("%s=%s", stateVariable.c_str(), value.c_str()));
  }

  stateVariableValuePairs = COhUtils::ToCSV(stateVariableValues);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetStateVariables(%u, %s): values = %s", instanceID, stateVariableList.c_str(), stateVariableValuePairs.c_str());
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const CVariant &data)
{
  // ignore non-xbmc announcements
  if (strcmp(sender, "xbmc") != 0)
    return;

  COhUPnPAVTransportLastChangeLock lock(m_variables);

  if (flag == ANNOUNCEMENT::Player)
  {
    if (strcmp(message, "OnPlay") == 0)
    {
      const std::string& path = g_application.CurrentFile();
      m_variables.SetAVTransportURI(path);
      m_variables.SetCurrentTrackURI(path);

      // beware that the generated metadata does not respect any device specific profiles
      std::string metadata;
      if (!GetMetadataFromFileItem(g_application.CurrentFileItem(), COhUPnPClientDevice(""), metadata))
        CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: failed to serialize metadata for item \"%s\"", g_application.CurrentFileItem().GetLabel().c_str());
      else
      {
        m_variables.SetAVTransportURIMetadata(metadata);
        m_variables.SetCurrentTrackMetadata(metadata);
      }

      m_variables.SetTransportPlaySpeed(static_cast<float>(data["player"]["speed"].asInteger()));
      m_variables.SetTransportState(OhUPnPAVTransportTransportState::Playing);

      // this could be a transition to next track, so clear next
      m_variables.SetNextAVTransportURI("");
      m_variables.SetNextAVTransportURIMetadata("");
    }
    else if (strcmp(message, "OnPause") == 0)
    {
      // TransportPlaySpeed must never be 0 and defaults to 1
      m_variables.SetTransportPlaySpeed(1);
      m_variables.SetTransportState(OhUPnPAVTransportTransportState::PausedPlayback);
    }
    else if (strcmp(message, "OnSpeedChanged") == 0)
      m_variables.SetTransportPlaySpeed(static_cast<float>(data["player"]["speed"].asInteger()));
  }

  UpdateLastChange();
}

void COhUPnPMediaRendererAVTransportService::AVTransport::Play(const std::string& uri, const std::string& metadata, const COhUPnPClientDevice& device)
{
  {
    COhUPnPAVTransportLastChangeLock lock(m_variables);
    m_variables.SetTransportState(OhUPnPAVTransportTransportState::Transitioning);
    m_variables.SetTransportStatus(OhUPnPAVTransportTransportStatus::OK);

    UpdateLastChange();
  }

  CFileItem item;
  if (!GetFileItemFromMetadata(uri, metadata, device, item))
    CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: failed to deserialize metadata for URI \"%s\": %s", uri.c_str(), metadata.c_str());

  if (item.IsPicture())
    KODI::MESSAGING::CApplicationMessenger::GetInstance().PostMsg(TMSG_PICTURE_SHOW, -1, -1, nullptr, item.GetPath());
  else
  {
    CFileItemList *list = new CFileItemList; //don't delete,
    list->Add(std::make_shared<CFileItem>(item));
    KODI::MESSAGING::CApplicationMessenger::GetInstance().PostMsg(TMSG_MEDIA_PLAY, -1, -1, static_cast<void*>(list));
  }

  // just return success because the play actions are asynchronous
  COhUPnPAVTransportLastChangeLock lock(m_variables);
  m_variables.SetTransportState(OhUPnPAVTransportTransportState::Playing);
  m_variables.SetTransportStatus(OhUPnPAVTransportTransportStatus::OK);
  m_variables.SetAVTransportURI(uri);
  m_variables.SetAVTransportURIMetadata(metadata);
  m_variables.SetCurrentTrackMetadata(metadata);

  m_variables.SetNextAVTransportURI("");
  m_variables.SetNextAVTransportURIMetadata("");

  UpdateLastChange();
}

bool COhUPnPMediaRendererAVTransportService::AVTransport::GetFileItemFromMetadata(const std::string& uri, const std::string& metadata, const COhUPnPClientDevice& device, CFileItem& item) const
{
  bool result = true;

  if (!metadata.empty())
  {
    // try to find a matching profile
    COhUPnPDeviceProfile profile;
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(device, profile))
      CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: client doesn't match any profiles");
    else
      CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: client matches profile %s", profile.GetName().c_str());

    // try to deserialize the metadata into an item
    if (!FileItemUtils::DeserializeFileItem(metadata, m_service.m_elementFactory, device, profile, item))
      result = false;
  }

  // make sure the path of the item is set to the given URI
  item.SetPath(uri);

  return result;
}

bool COhUPnPMediaRendererAVTransportService::AVTransport::GetMetadataFromFileItem(const CFileItem& item, const COhUPnPClientDevice& device, std::string& metadata) const
{
  // try to find a matching profile
  COhUPnPDeviceProfile profile;
  if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(device, profile))
    CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: client doesn't match any profiles");
  else
    CLog::Log(LOGINFO, "COhUPnPMediaRendererAVTransportService: client matches profile %s", profile.GetName().c_str());

  return FileItemUtils::SerializeFileItem(item, m_service.m_elementFactory, device, profile, metadata);
}

void COhUPnPMediaRendererAVTransportService::AVTransport::UpdateLastChange()
{
  // nothing to do if no state variable has changed
  if (!m_variables.HasChanged())
    return;

  // create a copy
  COhUPnPAVTransportLastChange* lastChange = new COhUPnPAVTransportLastChange(m_variables);
  COhUPnPAVTransportLastChangeDocument lastChangeDoc;
  lastChangeDoc.AddElement(lastChange);

  // serialize the changed variables into an Event XML document
  CXmlPropertyTreeSerializer lastChangeSerializer;
  std::string lastChangeXml;
  if (!lastChangeSerializer.Serialize(lastChangeDoc, lastChangeXml))
  {
    CLog::Log(LOGWARNING, "COhUPnPMediaRendererAVTransportService: failed to serialize LastChange event variable");
    return;
  }

  // finally set the LastChange property
  SetPropertyLastChange(lastChangeXml);

  // finally fix the new values of the state variables
  m_variables.Fix();
}
