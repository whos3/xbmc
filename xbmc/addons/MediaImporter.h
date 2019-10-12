/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "addons/Addon.h"
#include "threads/CriticalSection.h"

#include <unordered_map>
#include <string>

namespace ADDON
{

struct AddonEvent;

class CMediaImporter : public CAddon
{
public:
  explicit CMediaImporter(const AddonInfoPtr& addonInfo);

  std::string Protocol() const;

  bool HasDiscoveryService() const { return !m_discoveryService.empty(); }
  std::string DiscoveryServicePath() const;

  bool HasObserverService() const { return !m_observerService.empty(); }
  std::string ObserverServicePath() const;

  bool CanLookupProvider() const { return m_canLookupProvider; }

  std::string ProviderSettingsPath() const;
  std::string ImportSettingsPath() const;

private:
  std::string m_protocol;
  std::string m_discoveryService;
  std::string m_observerService;
  bool m_canLookupProvider;
  std::string m_providerSettingsPath;
  std::string m_importSettingsPath;
};

class CMediaImportAddonManager
{
public:
  explicit CMediaImportAddonManager(CAddonMgr& addonMgr);
  ~CMediaImportAddonManager();

  /*!
   * Start all services.
   */
  void Start();

  /*!
   * Start discovery service by add-on id.
   */
  bool StartDiscovery(const AddonPtr& addon);
  bool StartDiscovery(const std::string& addonId);

  /*!
   * Start observer service by add-on id.
   */
  bool StartObserver(const AddonPtr& addon);
  bool StartObserver(const std::string& addonId);

  /*!
   * Stop all services.
   */
  void Stop();

  /*!
   * Stop discovery service by add-on id.
   */
  bool StopDiscovery(const AddonPtr& addon);
  bool StopDiscovery(const std::string& addonId);

  /*!
   * Stop observer service by add-on id.
   */
  bool StopObserver(const AddonPtr& addon);
  bool StopObserver(const std::string& addonId);

private:
  /*! add-on id -> script id */
  typedef struct ServiceHandlers
  {
    int discovery = -1;
    int observer = -1;
  } ServiceHandlers;
  using ServicesMap = std::unordered_map<std::string, ServiceHandlers>;

  AddonPtr GetAddon(const std::string& addonId) const;

  void OnEvent(const AddonEvent& event);

  void Register(const std::string& addonId);
  void Register(const AddonPtr& addon);
  void Unregister(const std::string& addonId);
  void Unregister(const AddonPtr& addon);

  bool Stop(const AddonPtr& addon);
  bool Stop(const std::string& addonId);
  bool Stop(const AddonPtr& addon, int& handle);
  void Cleanup(ServicesMap::iterator service);

  CAddonMgr& m_addonMgr;
  CCriticalSection m_criticalSection;
  bool m_started = false;
  ServicesMap m_services;
};

} /*namespace ADDON*/
