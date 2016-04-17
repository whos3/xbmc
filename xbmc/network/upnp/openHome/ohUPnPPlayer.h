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

#include <string>

#include "FileItem.h"
#include "cores/IPlayer.h"
#include "network/upnp/openHome/controlpoints/ohUPnPAVTransportControlPoint.h"
#include "network/upnp/openHome/controlpoints/ohUPnPConnectionManagerControlPoint.h"
#include "network/upnp/openHome/controlpoints/ohUPnPRenderingControlControlPoint.h"
#include "threads/CriticalSection.h"
#include "threads/Event.h"

class CGUIDialogBusy;

namespace XbmcThreads {
  class EndTime;
}

class COhUPnPPlayer
  : public IPlayer
  , public IOhUPnPAVTransportControlPointAsync
  , public IOhUPnPConnectionManagerControlPointAsync
  , public IOhUPnPRenderingControlControlPointAsync
{
public:
  COhUPnPPlayer(IPlayerCallback& callback, const std::string& uuid);
  virtual ~COhUPnPPlayer();

  // implementation of IPlayer
  virtual bool OpenFile(const CFileItem& file, const CPlayerOptions& options) override;
  virtual bool QueueNextFile(const CFileItem &file) override;
  virtual bool CloseFile(bool reopen = false) override;
  virtual bool IsPlaying() const override;
  virtual void Pause() override;
  virtual bool IsPaused() const override;
  virtual bool HasVideo() const override { return false; }
  virtual bool HasAudio() const override { return false; }
  virtual void Seek(bool bPlus, bool bLargeStep, bool bChapterOverride) override { }
  virtual void SeekPercentage(float fPercent = 0) override;
  virtual float GetPercentage() override;
  virtual void SetVolume(float volume) override;
  virtual void GetAudioInfo(std::string& strAudioInfo) override { }
  virtual void GetVideoInfo(std::string& strVideoInfo) override { }
  virtual void GetGeneralInfo(std::string& strVideoInfo) override { }
  virtual bool CanRecord() override { return false; }
  virtual bool IsRecording() override { return false; }
  virtual bool Record(bool bOnOff) override { return false; }
  virtual int  GetChapterCount() override { return 0; }
  virtual int  GetChapter() override { return -1; }
  virtual void GetChapterName(std::string& strChapterName, int chapterIdx = -1) override { return; }
  virtual int  SeekChapter(int iChapter) override { return -1; }
  virtual void SeekTime(__int64 iTime = 0) override;
  virtual int64_t GetTime() override;
  virtual int64_t GetTotalTime() override;
  virtual void ToFFRW(int iSpeed = 0) override { }
  virtual bool SkipNext() override { return false; }
  virtual bool IsCaching() const override { return false; };
  virtual int GetCacheLevel() const override { return -1; };
  virtual void DoAudioWork() override;
  virtual bool OnAction(const CAction &action) override;
  virtual std::string GetPlayingTitle() override;

  // implementation of IOhUPnPAVTransportControlPointAsync
  virtual void SetAVTransportURIResult(bool success) override;
  virtual void SetNextAVTransportURIResult(bool success) override;
  virtual void GetMediaInfoResult(bool success, uint32_t numberOfTracks, const CDateTimeSpan& duration, const std::string& currentUri, const std::string& currentUriMetadata,
    const std::string& nextUri, const std::string& nextUriMetadata, const COhUPnPAVTransportStorageMedium& playbackMedium,
    const COhUPnPAVTransportStorageMedium& recordMedium, const COhUPnPAVTransportWriteStatus& writeStatus) override;
  virtual void GetTransportInfoResult(bool success, const COhUPnPAVTransportTransportState& transportState,
    const COhUPnPAVTransportTransportStatus& transportStatus, const std::string& speed) override;
  virtual void GetPositionInfoResult(bool success, uint32_t track, const CDateTimeSpan& duration, const std::string& metadata, const std::string& uri,
    const CDateTimeSpan& relativeTime, const CDateTimeSpan& absoluteTime, int32_t relativeCount, int32_t absoluteCount) override;
  virtual void StopResult(bool success) override;
  virtual void PlayResult(bool success) override;
  virtual void PauseResult(bool success) override;
  virtual void SeekResult(bool success) override;
  virtual void NextResult(bool success) override;
  virtual void PreviousResult(bool success) override;
  // TODO: virtual void SetStaticPlaylistResult(bool success, TODO) override;
  // TODO: virtual void GetPlaylistInfoResult(bool success, TODO) override;

  // implementation of IOhUPnPRenderingControlControlPointAsync
  virtual void SetVolumeResult(bool success) override;

  bool PlayFile(const CFileItem& file, const CPlayerOptions& options, CGUIDialogBusy*& dialog, XbmcThreads::EndTime timeout);

private:
  bool FileItemToDidlLite(const CFileItem& item, std::string& resourcePath, std::string& didlLite) const;
  void UpdateTransportAndPositionInfo();

  std::shared_ptr<COhUPnPConnectionManagerControlPoint> m_connectionManagerController;
  std::shared_ptr<COhUPnPAVTransportControlPoint> m_playbackController;
  std::shared_ptr<COhUPnPRenderingControlControlPoint> m_renderingController;

  uint32_t m_instance;
  std::string m_currentUri;
  std::string m_currentUriMetadata;

  bool m_started;
  bool m_stopremote;

  uint32_t m_lastInfoUpdate;

  struct AsyncEvent
  {
    CEvent evt;
    bool success;

    AsyncEvent()
      : evt()
      , success(false)
    { }
  };

  static bool WaitOnEvent(AsyncEvent& evt, const XbmcThreads::EndTime& timeout);
  static bool WaitOnEvent(AsyncEvent& evt, const XbmcThreads::EndTime& timeout, CGUIDialogBusy*& dialog);

  AsyncEvent m_setAVTransportURIEvent;
  AsyncEvent m_setNextAVTransportURIEvent;
  AsyncEvent m_getMediaInfoEvent;
  AsyncEvent m_getTransportInfoEvent;
  AsyncEvent m_getPositionInfoEvent;
  AsyncEvent m_stopEvent;
  AsyncEvent m_playEvent;
  AsyncEvent m_pauseEvent;
  AsyncEvent m_seekEvent;
  AsyncEvent m_nextEvent;
  AsyncEvent m_previousEvent;

  AsyncEvent m_setVolumeEvent;

  CCriticalSection m_criticalMediaInfo;
  struct
  {
    uint32_t numberOfTracks;
    CDateTimeSpan duration;
    std::string currentUri;
    std::string currentUriMetadata;
    std::string nextUri;
    std::string nextUriMetadata;
    COhUPnPAVTransportStorageMedium playbackMedium;
    COhUPnPAVTransportStorageMedium recordMedium;
    COhUPnPAVTransportWriteStatus writeStatus;

    CFileItem currentItem;
  } m_mediaInfo;

  CCriticalSection m_criticalTransportInfo;
  struct
  {
    COhUPnPAVTransportTransportState transportState;
    COhUPnPAVTransportTransportStatus transportStatus;
    std::string speed;
  } m_transportInfo;

  CCriticalSection m_criticalPositionInfo;
  struct
  {
    uint32_t track;
    CDateTimeSpan duration;
    std::string metadata;
    std::string uri;
    CDateTimeSpan relativeTime;
    CDateTimeSpan absoluteTime;
    int32_t relativeCount;
    int32_t absoluteCount;
  } m_positionInfo;
};
