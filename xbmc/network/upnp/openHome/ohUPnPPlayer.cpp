/*
 *      Copyright (c) 2006 elupus (Joakim Plate)
 *      Copyright (C) 2006-2013 Team XBMC
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

#include "ohUPnPPlayer.h"
#include "Application.h"
#include "FileItem.h"
#include "GUIInfoManager.h"
#include "dialogs/GUIDialogBusy.h"
#include "guilib/GUIWindowManager.h"
#include "input/Key.h"
#include "messaging/ApplicationMessenger.h"
#include "messaging/helpers/DialogHelper.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/didllite/objects/FileItemUtils.h"
#include "network/upnp/openHome/didllite/objects/item/UPnPItem.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPResource.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "utils/Variant.h"

static const uint32_t AsyncTimeoutMs = 10000;

COhUPnPPlayer::COhUPnPPlayer(IPlayerCallback& callback, const std::string& uuid)
  : IPlayer(callback)
  , m_connectionManagerController(nullptr)
  , m_playbackController(nullptr)
  , m_renderingController(nullptr)
  , m_instance(0)
  , m_started(false)
  , m_stopremote(false)
  , m_lastInfoUpdate(0)
{
  m_connectionManagerController = COhUPnP::GetInstance().CreateConnectionManagerController(uuid, this);
  if (m_connectionManagerController == nullptr)
    CLog::Log(LOGWARNING, "COhUPnPPlayer: failed to get connection manager controller for device %s", uuid.c_str());

  m_playbackController = COhUPnP::GetInstance().CreatePlaybackController(uuid, this);
  if (m_playbackController == nullptr)
    CLog::Log(LOGERROR, "COhUPnPPlayer: failed to get playback controller for device %s", uuid.c_str());

  m_renderingController = COhUPnP::GetInstance().CreateRenderingController(uuid, this);
  if (m_renderingController == nullptr)
    CLog::Log(LOGWARNING, "COhUPnPPlayer: failed to get rendering controller for device %s", uuid.c_str());
}

COhUPnPPlayer::~COhUPnPPlayer()
{
  CloseFile();

  if (m_connectionManagerController != nullptr)
    COhUPnP::GetInstance().DestroyConnectionManagerController(m_connectionManagerController);
  if (m_renderingController != nullptr)
    COhUPnP::GetInstance().DestroyRenderingController(m_renderingController);
  if (m_playbackController != nullptr)
    COhUPnP::GetInstance().DestroyPlaybackController(m_playbackController);
}

bool COhUPnPPlayer::PlayFile(const CFileItem& file, const CPlayerOptions& options, CGUIDialogBusy*& dialog, XbmcThreads::EndTime timeout)
{
  if (m_playbackController == nullptr)
    return false;

  // get the transport info to evaluate the TransportState to be able to
  // determine whether we first need to call Stop()
  timeout.Set(timeout.GetInitialTimeoutValue());
  if (!m_playbackController->GetTransportInfoAsync(m_instance) ||
      !WaitOnEvent(m_getTransportInfoEvent, timeout, dialog))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): unable to get current state of renderer", file.GetPath().c_str());
    return false;
  }

  bool stopped = true;
  {
    CSingleLock lock(m_criticalTransportInfo);
    stopped = m_transportInfo.transportState == OhUPnPAVTransportTransportState::NoMediaPresent ||
              m_transportInfo.transportState == OhUPnPAVTransportTransportState::Stopped;
  }

  if (!stopped)
  {
    timeout.Set(timeout.GetInitialTimeoutValue());
    if (!m_playbackController->StopAsync(m_instance) ||
        !WaitOnEvent(m_stopEvent, timeout, dialog))
    {
      CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to stop current playback on renderer", file.GetPath().c_str());
      return false;
    }
  }

  std::string bestResource;
  std::string metadata;
  if (!FileItemToDidlLite(file, bestResource, metadata))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to serialize item", file.GetPath().c_str());
    return false;
  }

  // set the URI/item to be played
  timeout.Set(timeout.GetInitialTimeoutValue());
  if (!m_playbackController->SetAVTransportURIAsync(bestResource, metadata, m_instance) ||
      !WaitOnEvent(m_setAVTransportURIEvent, timeout, dialog))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to tell the renderer which URI to play", file.GetPath().c_str());
    return false;
  }

  // start playback
  timeout.Set(timeout.GetInitialTimeoutValue());
  if (!m_playbackController->PlayAsync("1", m_instance) ||
      !WaitOnEvent(m_playEvent, timeout, dialog))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to start playback on renderer", file.GetPath().c_str());
    return false;
  }

  // wait for PLAYING state
  timeout.Set(timeout.GetInitialTimeoutValue());
  do
  {
    if (!m_playbackController->GetTransportInfoAsync(m_instance))
    {
      CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to wait for renderer to start playback", file.GetPath().c_str());
      return false;
    }

    {
      CSingleLock lock(m_criticalTransportInfo);
      if (m_transportInfo.transportState == OhUPnPAVTransportTransportState::Playing ||
          m_transportInfo.transportState == OhUPnPAVTransportTransportState::PausedPlayback)
        break;

      if (m_transportInfo.transportState == OhUPnPAVTransportTransportState::Stopped &&
          m_transportInfo.transportStatus != OhUPnPAVTransportTransportStatus::OK)
      {
        CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to start playback on renderer", file.GetPath().c_str());
        return false;
      }
    }

    if (!WaitOnEvent(m_getTransportInfoEvent, timeout, dialog))
    {
      CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to wait for renderer to start playback", file.GetPath().c_str());
      return false;
    }
  }
  while(!timeout.IsTimePast());

  if (options.starttime > 0)
  {
    // many upnp units won't load file properly until after play (including xbmc)
    if (!m_playbackController->SeekAsync(OhUPnPAVTransportSeekUnit::RelativeTime, COhUtils::GetDurationFromSeconds(static_cast<uint32_t>(options.starttime)), m_instance))
    {
      CLog::Log(LOGERROR, "COhUPnPPlayer::PlayFile(%s): failed to seek to start offset at %f", file.GetPath().c_str(), options.starttime);
      return false;
    }
  }

  return true;
}

bool COhUPnPPlayer::OpenFile(const CFileItem& file, const CPlayerOptions& options)
{
  if (m_playbackController == nullptr)
    return false;

  CGUIDialogBusy* dialog = NULL;

  // if no path we want to attach to a already playing player
  if (file.GetPath().empty())
  {
    if (!m_playbackController->GetTransportInfoAsync(m_instance) ||
        !WaitOnEvent(m_getTransportInfoEvent, AsyncTimeoutMs, dialog))
    {
      CLog::Log(LOGERROR, "COhUPnPPlayer::OpenFile: unable to find any existing players");
      if (dialog != nullptr)
        dialog->Close();

      return false;
    }

    // make sure the attached player is actually playing
    {
      CSingleLock lock(m_criticalTransportInfo);
      if (m_transportInfo.transportState != OhUPnPAVTransportTransportState::Playing &&
          m_transportInfo.transportState != OhUPnPAVTransportTransportState::PausedPlayback)
      {
        CLog::Log(LOGERROR, "COhUPnPPlayer::OpenFile: unable to open file %s", file.GetPath().c_str());
        if (dialog != nullptr)
          dialog->Close();

        return false;
      }
    }
  }
  else
  {
    if (!PlayFile(file, options, dialog, AsyncTimeoutMs))
    {
      CLog::Log(LOGERROR, "COhUPnPPlayer::OpenFile: unable to open file %s", file.GetPath().c_str());
      if (dialog != nullptr)
        dialog->Close();

      return false;
    }
  }

  m_stopremote = true;
  m_started = true;
  m_callback.OnPlayBackStarted();

  bool success = true;
  // get the current state of playback
  if (!m_playbackController->GetPositionInfoAsync(m_instance) ||
      !m_playbackController->GetMediaInfoAsync(m_instance))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::OpenFile: unable to open file %s", file.GetPath().c_str());
    success = false;
  }

  if (dialog != nullptr)
    dialog->Close();

  return success;
}

bool COhUPnPPlayer::QueueNextFile(const CFileItem& file)
{
  if (m_playbackController == nullptr)
    return false;

  std::string bestResource;
  std::string metadata;
  if (!FileItemToDidlLite(file, bestResource, metadata))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::QueueNextFile(%s): failed to serialize item", file.GetPath().c_str());
    return false;
  }

  if (!m_playbackController->SetNextAVTransportURIAsync(bestResource, metadata, m_instance) ||
      !WaitOnEvent(m_setNextAVTransportURIEvent, AsyncTimeoutMs))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::QueueNextFile: unable to queue file %s", file.GetPath().c_str());
    return false;
  }

  return true;
}

bool COhUPnPPlayer::CloseFile(bool reopen)
{
  if (m_playbackController == nullptr)
    return false;

  if (m_stopremote)
  {
    if (!m_playbackController->StopSync(m_instance))
    {
      CLog::Log(LOGERROR, "COhUPnPPlayer::CloseFile: unable to stop playback");
      return false;
    }
  }

  if (m_started)
  {
    m_started = false;
    m_callback.OnPlayBackStopped();
  }

  return true;
}

void COhUPnPPlayer::Pause()
{
  if (m_playbackController == nullptr)
    return;

  bool success = false;
  if (IsPaused())
    success = m_playbackController->PlayAsync("1", m_instance);
  else
    success = m_playbackController->PauseAsync(m_instance);

  if (!success)
    CLog::Log(LOGERROR, "COhUPnPPlayer::Pause: unable to pause/unpause playback");
}

void COhUPnPPlayer::SeekTime(__int64 ms)
{
  if (m_playbackController == nullptr)
    return;

  if (!m_playbackController->SeekAsync(COhUPnPAVTransportSeekUnit(OhUPnPAVTransportSeekUnit::RelativeTime), COhUtils::GetDurationFromSeconds(ms / 1000), m_instance))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::SeekTime: unable to seek playback");
    return;
  }

  g_infoManager.SetDisplayAfterSeek();
}

float COhUPnPPlayer::GetPercentage()
{
  int64_t total = GetTotalTime();
  if (total > 0)
    return 100.0f * GetTime() / total;

  return 0.0f;
}

void COhUPnPPlayer::SeekPercentage(float percent)
{
  int64_t total = GetTotalTime();
  if (total > 0)
    SeekTime(static_cast<int64_t>(total * percent / 100));
}

void COhUPnPPlayer::DoAudioWork()
{
  if (m_playbackController == nullptr)
    return;

  UpdateTransportAndPositionInfo();

  if (!m_started)
    return;

  uint32_t numberOfTracks;
  CDateTimeSpan duration;
  std::string currentUri, currentUriMetadata, nextUri, nextUriMetadata;
  COhUPnPAVTransportStorageMedium playbackMedium;
  COhUPnPAVTransportStorageMedium recordMedium;
  COhUPnPAVTransportWriteStatus writeStatus;
  if (!m_playbackController->GetMediaInfoSync(numberOfTracks, duration, currentUri, currentUriMetadata, nextUri, nextUriMetadata, playbackMedium, recordMedium, writeStatus))
    return;

  if (m_currentUri != currentUri ||
      m_currentUriMetadata != currentUriMetadata)
  {
    m_currentUri = currentUri;
    m_currentUriMetadata = currentUriMetadata;

    CFileItem currentItem;
    FileItemUtils::DeserializeFileItem(currentUriMetadata, COhUPnP::GetInstance().GetFileItemElementFactory(),
      m_playbackController->GetDevice(), m_playbackController->GetProfile(), currentItem);

    g_application.CurrentFileItem() = currentItem;
    KODI::MESSAGING::CApplicationMessenger::GetInstance().PostMsg(TMSG_UPDATE_CURRENT_ITEM, 0, -1, static_cast<void*>(new CFileItem(currentItem)));
  }

  // check if the renderer is still playing
  {
    CSingleLock lock(m_criticalTransportInfo);
    if (m_transportInfo.transportState == OhUPnPAVTransportTransportState::Stopped)
    {
      m_started = false;
      m_callback.OnPlayBackEnded();
    }
  }
}

bool COhUPnPPlayer::IsPlaying() const
{
  if (m_playbackController == nullptr)
    return false;

  COhUPnPAVTransportTransportState transportState;
  COhUPnPAVTransportTransportStatus transportStatus;
  std::string speed;
  if (!m_playbackController->GetTransportInfoSync(transportState, transportStatus, speed, m_instance))
  {
    CLog::Log(LOGWARNING, "COhUPnPPlayer::IsPlaying: unable to determine playback state");
    return false;
  }

  return transportState != OhUPnPAVTransportTransportState::Stopped;
}

bool COhUPnPPlayer::IsPaused() const
{
  if (m_playbackController == nullptr)
    return false;

  COhUPnPAVTransportTransportState transportState;
  COhUPnPAVTransportTransportStatus transportStatus;
  std::string speed;
  if (!m_playbackController->GetTransportInfoSync(transportState, transportStatus, speed, m_instance))
  {
    CLog::Log(LOGWARNING, "COhUPnPPlayer::IsPlaying: unable to determine playback state");
    return false;
  }

  return transportState == OhUPnPAVTransportTransportState::PausedPlayback;
}

void COhUPnPPlayer::SetVolume(float volume)
{
  if (m_renderingController == nullptr)
    return;

  if (!m_renderingController->SetVolumeAsync(static_cast<uint32_t>(volume * 100), OhUPnPRenderingControlChannel::Master, m_instance))
    CLog::Log(LOGERROR, "COhUPnPPlayer::SetVolume: unable to set volume");
}

int64_t COhUPnPPlayer::GetTime()
{
  if (m_playbackController == nullptr)
    return 0LL;

  CSingleLock lock(m_criticalPositionInfo);
  if (!m_getPositionInfoEvent.success)
    return 0LL;

  return m_positionInfo.relativeTime.GetSecondsTotal() * 1000LL;
}

int64_t COhUPnPPlayer::GetTotalTime()
{
  if (m_playbackController == nullptr)
    return 0LL;

  CSingleLock lock(m_criticalPositionInfo);
  if (!m_getPositionInfoEvent.success)
    return 0LL;

  return m_positionInfo.duration.GetSecondsTotal() * 1000LL;
};

std::string COhUPnPPlayer::GetPlayingTitle()
{
  if (m_playbackController == nullptr || !m_getMediaInfoEvent.success)
    return "";

  return m_mediaInfo.currentItem.GetLabel();
};

bool COhUPnPPlayer::OnAction(const CAction &action)
{
  switch (action.GetID())
  {
    case ACTION_STOP:
      if (IsPlaying())
      {
        //stop on remote system
        m_stopremote = KODI::MESSAGING::HELPERS::ShowYesNoDialogText(CVariant{ 37022 }, CVariant{ 37023 }) == KODI::MESSAGING::HELPERS::DialogResponse::YES;
        
        return false; /* let normal code handle the action */
      }

    default:
      break;
  }

  return false;
}

void COhUPnPPlayer::SetAVTransportURIResult(bool success)
{
  m_setAVTransportURIEvent.success = success;
  m_setAVTransportURIEvent.evt.Set();
}

void COhUPnPPlayer::SetNextAVTransportURIResult(bool success)
{
  m_setNextAVTransportURIEvent.success = success;
  m_setNextAVTransportURIEvent.evt.Set();
}

void COhUPnPPlayer::GetMediaInfoResult(bool success, uint32_t numberOfTracks, const CDateTimeSpan& duration, const std::string& currentUri, const std::string& currentUriMetadata,
  const std::string& nextUri, const std::string& nextUriMetadata, const COhUPnPAVTransportStorageMedium& playbackMedium,
  const COhUPnPAVTransportStorageMedium& recordMedium, const COhUPnPAVTransportWriteStatus& writeStatus)
{
  m_getMediaInfoEvent.success = success;

  {
    CSingleLock lock(m_criticalMediaInfo);
    m_mediaInfo.numberOfTracks = numberOfTracks;
    m_mediaInfo.duration = duration;
    m_mediaInfo.currentUri = currentUri;
    m_mediaInfo.currentUriMetadata = currentUriMetadata;
    m_mediaInfo.nextUri = nextUri;
    m_mediaInfo.nextUriMetadata = nextUriMetadata;
    m_mediaInfo.playbackMedium = playbackMedium;
    m_mediaInfo.recordMedium = recordMedium;
    m_mediaInfo.writeStatus = writeStatus;

    FileItemUtils::DeserializeFileItem(currentUriMetadata, COhUPnP::GetInstance().GetFileItemElementFactory(),
      m_playbackController->GetDevice(), m_playbackController->GetProfile(), m_mediaInfo.currentItem);
  }

  m_getMediaInfoEvent.evt.Set();
}

void COhUPnPPlayer::GetTransportInfoResult(bool success, const COhUPnPAVTransportTransportState& transportState,
  const COhUPnPAVTransportTransportStatus& transportStatus, const std::string& speed)
{
  m_getTransportInfoEvent.success = success;

  {
    CSingleLock lock(m_criticalTransportInfo);
    m_transportInfo.transportState = transportState;
    m_transportInfo.transportStatus = transportStatus;
  }

  m_getTransportInfoEvent.evt.Set();
}

void COhUPnPPlayer::GetPositionInfoResult(bool success, uint32_t track, const CDateTimeSpan& duration, const std::string& metadata, const std::string& uri,
  const CDateTimeSpan& relativeTime, const CDateTimeSpan& absoluteTime, int32_t relativeCount, int32_t absoluteCount)
{
  m_getPositionInfoEvent.success = success;

  {
    CSingleLock lock(m_criticalPositionInfo);
    m_positionInfo.track = track;
    m_positionInfo.duration = duration;
    m_positionInfo.metadata = metadata;
    m_positionInfo.uri = uri;
    m_positionInfo.relativeTime = relativeTime;
    m_positionInfo.absoluteTime = absoluteTime;
    m_positionInfo.relativeCount = relativeCount;
    m_positionInfo.absoluteCount = absoluteCount;
  }

  m_lastInfoUpdate = CTimeUtils::GetFrameTime() + 500;

  m_getPositionInfoEvent.evt.Set();
}

void COhUPnPPlayer::StopResult(bool success)
{
  m_stopEvent.success = success;
  m_stopEvent.evt.Set();
}

void COhUPnPPlayer::PlayResult(bool success)
{
  m_playEvent.success = success;
  m_playEvent.evt.Set();
}

void COhUPnPPlayer::PauseResult(bool success)
{
  m_pauseEvent.success = success;
  m_pauseEvent.evt.Set();
}

void COhUPnPPlayer::SeekResult(bool success)
{
  m_seekEvent.success = success;
  m_seekEvent.evt.Set();
}

void COhUPnPPlayer::NextResult(bool success)
{
  m_nextEvent.success = success;
  m_nextEvent.evt.Set();
}

void COhUPnPPlayer::PreviousResult(bool success)
{
  m_previousEvent.success = success;
  m_previousEvent.evt.Set();
}

void COhUPnPPlayer::SetVolumeResult(bool success)
{
  m_setVolumeEvent.success = success;
  m_setVolumeEvent.evt.Set();
}

bool COhUPnPPlayer::FileItemToDidlLite(const CFileItem& item, std::string& resourcePath, std::string& didlLite) const
{
  CUPnPObject* upnpObj = nullptr;
  if (!FileItemUtils::ConvertFileItem(item, COhUPnP::GetInstance().GetFileItemElementFactory(), m_playbackController->GetDevice(), m_playbackController->GetProfile(), upnpObj))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::FileItemToDidlLite(%s): failed to serialize item", item.GetPath().c_str());
    return false;
  }

  CUPnPItem* upnpItem = dynamic_cast<CUPnPItem*>(upnpObj);
  if (upnpItem == nullptr)
  {
    CLog::Log(LOGWARNING, "COhUPnPPlayer::FileItemToDidlLite(%s): item \"%s\" (%s) of type %s is not a UPnP item",
      item.GetPath().c_str(), item.GetLabel().c_str(), item.GetPath().c_str(), upnpObj->GetType().c_str());
    return false;
  }

  resourcePath = item.GetPath();
  const auto& resources = upnpItem->GetResources();
  if (!resources.empty())
  {
    // default to the first resource
    resourcePath = resources.front()->GetUri();

    // try to find the best matching resource using the renderer's sink protocol information
    if (m_connectionManagerController != nullptr)
    {
      std::string sourceInfo, sinkInfo;
      if (!m_connectionManagerController->GetProtocolInfoSync(sourceInfo, sinkInfo))
        CLog::Log(LOGWARNING, "COhUPnPPlayer::FileItemToDidlLite(%s): failed to get protocol info", item.GetPath().c_str());
      else
        resourcePath = COhAVTransportUtils::FindBestResource(resources, sinkInfo);
    }
  }

  if (!FileItemUtils::SerializeObject(upnpItem, COhUPnP::GetInstance().GetFileItemElementFactory(), m_playbackController->GetDevice(), m_playbackController->GetProfile(), didlLite))
  {
    CLog::Log(LOGERROR, "COhUPnPPlayer::FileItemToDidlLite(%s): failed to serialize item", item.GetPath().c_str());
    return false;
  }

  return true;
}

void COhUPnPPlayer::UpdateTransportAndPositionInfo()
{
  if (m_lastInfoUpdate == 0 || m_lastInfoUpdate > CTimeUtils::GetFrameTime())
    return;

  m_lastInfoUpdate = 0;

  m_playbackController->GetTransportInfoAsync(m_instance);
  m_playbackController->GetPositionInfoAsync(m_instance);
}

bool COhUPnPPlayer::WaitOnEvent(AsyncEvent& evt, const XbmcThreads::EndTime& timeout)
{
  if (!evt.evt.WaitMSec(timeout.GetInitialTimeoutValue()))
    return false;

  return evt.success;
}

bool COhUPnPPlayer::WaitOnEvent(AsyncEvent& evt, const XbmcThreads::EndTime& timeout, CGUIDialogBusy*& dialog)
{
  if (evt.evt.WaitMSec(0))
    return evt.success;

  if (dialog == nullptr) {
    dialog = static_cast<CGUIDialogBusy*>(g_windowManager.GetWindow(WINDOW_DIALOG_BUSY));
    dialog->Open();
  }

  g_windowManager.ProcessRenderLoop(false);

  do {
    if (evt.evt.WaitMSec(100))
      return evt.success;

    g_windowManager.ProcessRenderLoop(false);

    if (dialog->IsCanceled())
      return false;

  } while (!timeout.IsTimePast());

  return false;
}
