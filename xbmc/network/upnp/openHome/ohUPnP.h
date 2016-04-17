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

#include <memory>
#include <string>
#include <vector>

#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/didllite/DidlLiteElementFactory.h"
#include "network/upnp/openHome/didllite/objects/FileItemElementFactory.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferManager.h"
#include "network/upnp/openHome/utils/ohEnvironment.h"
#include "settings/lib/ISettingCallback.h"
#include "threads/CriticalSection.h"

class COhUPnPContentDirectoryControlPoint;
class COhUPnPDeviceProfilesManager;
class COhUPnPMediaRendererDevice;
class COhUPnPAVTransportControlPoint;
class COhUPnPAVTransportControlPointManager;
class COhUPnPConnectionManagerControlPoint;
class COhUPnPConnectionManagerControlPointManager;
class COhUPnPMediaServerDevice;
class COhUPnPRenderingControlControlPoint;
class COhUPnPRenderingControlControlPointManager;
class CSetting;
class IOhUPnPAVTransportControlPointAsync;
class IOhUPnPConnectionManagerControlPointAsync;
class IOhUPnPRenderingControlControlPointAsync;

class COhUPnP : public ISettingCallback
{
public:
  virtual ~COhUPnP();

  static COhUPnP& GetInstance();

  bool Initialize();
  void Uninitialize();

  uint32_t GetIpAddress() const { return m_ipAddress; }
  uint32_t GetSubnetAddress() const { return m_subnetAddress; }
  std::string GetResourceUriPrefix() const;

  const COhUPnPDeviceProfilesManager& GetDeviceProfilesManager() const;

  const CDidlLiteElementFactory& GetDidlLiteElementFactory() const { return m_fileItemElementFactory; }
  const CFileItemElementFactory& GetFileItemElementFactory() const { return m_fileItemElementFactory; }

  COhUPnPTransferManager& GetTransferManager() { return m_transferManager; }
  static bool HasUPnPTransfers(const std::string &condition, const std::string &value, const CSetting *setting, void *data);

  bool StartMediaServer();
  void StopMediaServer();
  bool IsMediaServerRunning() const;

  bool StartContentDirectoryClient();
  void StopContentDirectoryClient();
  bool IsContentDirectoryClientRunning() const;
  COhUPnPContentDirectoryControlPoint& GetContentDirectoryClient() { return *m_contentDirectoryClient; }

  bool StartConnectionManagerController();
  void StopConnectionManagerController();
  bool IsConnectionManagerControllerRunning() const;
  std::shared_ptr<COhUPnPConnectionManagerControlPoint> CreateConnectionManagerController(const std::string& uuid, IOhUPnPConnectionManagerControlPointAsync* callback) const;
  void DestroyConnectionManagerController(std::shared_ptr<COhUPnPConnectionManagerControlPoint> connectionManagerController) const;

  bool StartRenderingController();
  void StopRenderingController();
  bool IsRenderingControllerRunning() const;
  std::shared_ptr<COhUPnPRenderingControlControlPoint> CreateRenderingController(const std::string& uuid, IOhUPnPRenderingControlControlPointAsync* callback) const;
  void DestroyRenderingController(std::shared_ptr<COhUPnPRenderingControlControlPoint> renderingController) const;

  bool StartPlaybackController();
  void StopPlaybackController();
  bool IsPlaybackControllerRunning() const;
  std::shared_ptr<COhUPnPAVTransportControlPoint> CreatePlaybackController(const std::string& uuid, IOhUPnPAVTransportControlPointAsync* callback) const;
  void DestroyPlaybackController(std::shared_ptr<COhUPnPAVTransportControlPoint> playbackController) const;

  bool StartMediaRenderer();
  void StopMediaRenderer();
  bool IsMediaRendererRunning() const;

  // implementation of ISettingCallback
  void OnSettingChanged(const CSetting* setting) override;
  void OnSettingAction(const CSetting* setting) override;

  // TODO

  static void SettingOptionsUPnPInterfacesFiller(const CSetting *setting, std::vector< std::pair<std::string, int> > &list, int &current, void *data);
private:
  COhUPnP();
  COhUPnP(const COhUPnP&);
  COhUPnP const& operator=(COhUPnP const&);

  void StartControlPointStack();

  void StartResourceManager();
  void StopResourceManager(bool force = false);

  void StartMediaServerDevice(bool forceRestart = false);
  void StopMediaServerDevice();

  void StartMediaRendererDevice(bool forceRestart = false);
  void StopMediaRendererDevice();

  void ohNetLogOutput(const char* msg);
  void ohNetFatalErrorHandler(const char* msg);

  CCriticalSection m_criticalInitialised;
  bool m_initialised;
  bool m_controlPointStackStarted;
  uint32_t m_ipAddress;
  uint32_t m_subnetAddress;
  COhEnvironment m_ohEnvironment;

  CFileItemElementFactory m_fileItemElementFactory;
  COhUPnPResourceManager m_resourceManager;
  COhUPnPTransferManager m_transferManager;

  CCriticalSection m_criticalContentDirectoryClient;
  COhUPnPContentDirectoryControlPoint* m_contentDirectoryClient;

  CCriticalSection m_criticalConnectionManagerController;
  COhUPnPConnectionManagerControlPointManager* m_connectionManagerController;

  CCriticalSection m_criticalRenderingController;
  COhUPnPRenderingControlControlPointManager* m_renderingController;

  CCriticalSection m_criticalPlaybackController;
  COhUPnPAVTransportControlPointManager* m_playbackController;

  CCriticalSection m_criticalMediaServerDevice;
  std::unique_ptr<COhUPnPMediaServerDevice> m_mediaServerDevice;

  CCriticalSection m_criticalMediaRendererDevice;
  std::unique_ptr<COhUPnPMediaRendererDevice> m_mediaRendererDevice;
};
