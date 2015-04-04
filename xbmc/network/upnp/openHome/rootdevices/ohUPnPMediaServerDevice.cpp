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

#include "ohUPnPMediaServerDevice.h"
#include "URL.h"
#include "filesystem/Directory.h"
#include "guilib/WindowIDs.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/didllite/DidlLiteDocument.h"
#include "network/upnp/openHome/didllite/objects/FileItemElementFactory.h"
#include "network/upnp/openHome/didllite/objects/FileItemUtils.h"
#include "network/upnp/openHome/didllite/objects/item/UPnPItem.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPResource.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/profile/ohUPnPDeviceProfilesManager.h"
#include "network/upnp/openHome/rootdevices/ohUPnPClientDevice.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "utils/FileUtils.h"
#include "utils/log.h"
#include "utils/PerformanceMeasurement.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"
#include "utils/URIUtils.h"
#include "view/GUIViewState.h"

// TODO: this should be used somewhere else
#include "filesystem/MusicDatabaseDirectory.h"
#include "filesystem/VideoDatabaseDirectory.h"
#include "filesystem/MusicDatabaseDirectory/DirectoryNode.h"
#include "filesystem/MusicDatabaseDirectory/QueryParams.h"
#include "filesystem/VideoDatabaseDirectory/DirectoryNode.h"
#include "filesystem/VideoDatabaseDirectory/QueryParams.h"
#include "music/MusicDatabase.h"
#include "music/tags/MusicInfoTag.h"
#include "video/VideoDatabase.h"

static const uint32_t DeviceDisableTimeoutMs = 5000;

static const std::string UPnPPathRoot = "virtualpath://upnproot/";

static const std::string UPnPCsvSeparator = ",";

static const std::map<std::string, SortDescription> SortCapabilities = {
  { "dc:title",                 SortDescription(SortByTitle, SortAttributeIgnoreArticle) },
  { "res@duration",             SortDescription(SortByTime) },
  { "res@size",                 SortDescription(SortBySize) },
  { "res@bitrate",              SortDescription(SortByBitrate) },
  { "res@nrAudioChannels",      SortDescription(SortByAudioChannels) },
  { "upnp:playcount",           SortDescription(SortByPlaycount) },
  { "upnp:playcount",           SortDescription(SortByPlaycount) },
  { "upnp:artist",              SortDescription(SortByArtist, SortAttributeIgnoreArticle) },
  { "dc:publisher",             SortDescription(SortByStudio, SortAttributeIgnoreArticle) },
  { "upnp:genre",               SortDescription(SortByGenre) },
  { "upnp:album",               SortDescription(SortByAlbum) },
  { "dc:date",                  SortDescription(SortByDate) },
  { "upnp:playcount",           SortDescription(SortByPlaycount) },
  { "upnp:lastPlaybackTime",    SortDescription(SortByLastPlayed) },
  { "upnp:seriesTitle",         SortDescription(SortByTvShowTitle) },
  { "upnp:episodeCount",        SortDescription(SortByNumberOfEpisodes) },
  { "upnp:episodeNumber",       SortDescription(SortByEpisodeNumber) },
  { "upnp:episodeSeason",       SortDescription(SortBySeason) },
  { "upnp:rating",              SortDescription(SortByMPAA) },
  { "upnp:originalTrackNumber", SortDescription(SortByTrackNumber) }
};

static bool ParseSortCriteria(const std::string& sortCriteria, SortDescription& sort)
{
  sort.sortBy = SortByNone;
  sort.sortOrder = SortOrderAscending;
  if (sortCriteria.empty())
    return true;

  const auto sortCapabilities = StringUtils::Split(sortCriteria, UPnPCsvSeparator);
  if (sortCriteria.empty())
    return true;

  const auto& desiredSortCapability = sortCapabilities.front();
  if (StringUtils::StartsWith(desiredSortCapability, "+"))
    sort.sortOrder = SortOrderAscending;
  else if (StringUtils::StartsWith(desiredSortCapability, "-"))
    sort.sortOrder = SortOrderDescending;

  const auto& sortCapability = SortCapabilities.find(desiredSortCapability);
  if (sortCapability == SortCapabilities.end())
    return false;

  sort.sortBy = sortCapability->second.sortBy;
  sort.sortAttributes = sortCapability->second.sortAttributes;

  return true;
}

static bool GetDefaultSort(const std::string& path, const CFileItemList& items, SortDescription& sort)
{
  int windowID = WINDOW_INVALID;
  if (StringUtils::StartsWith(path, "videodb://") || StringUtils::StartsWith(path, "library://video/"))
    windowID = WINDOW_VIDEO_NAV;
  else if (StringUtils::StartsWith(path, "musicdb://") || StringUtils::StartsWith(path, "library://music/"))
    windowID = WINDOW_MUSIC_NAV;
  else
    return false;

  std::unique_ptr<CGUIViewState> viewState(CGUIViewState::GetViewState(windowID, items));
  if (viewState == nullptr)
    return false;

  sort = viewState->GetSortMethod();

  return true;
}

static bool LoadFileItem(OpenHome::Net::IDvInvocationStd& invocation, CFileItemPtr item)
{
  if (item == nullptr)
    return false;

  const std::string& path = item->GetPath();

  /* TODO
  // attempt to determine the parent of this item
  std::string parent;
  if (URIUtils::IsVideoDb((const char*)id) || URIUtils::IsMusicDb((const char*)id) || StringUtils::StartsWithNoCase((const char*)id, "library://video/")) {
    if (!URIUtils::GetParentPath((const char*)id, parent)) {
      parent = "0";
    }
  }
  else {
    // non-library objects - playlists / sources
    //
    // we could instead store the parents in a hash during every browse
    // or could handle this in URIUtils::GetParentPath() possibly,
    // however this is quicker to implement and subsequently purge when a
    // better solution presents itself
    std::string child_id((const char*)id);
    if      (StringUtils::StartsWithNoCase(child_id, "special://musicplaylists/"))          parent = "musicdb://";
    else if (StringUtils::StartsWithNoCase(child_id, "special://videoplaylists/"))          parent = "library://video/";
    else if (StringUtils::StartsWithNoCase(child_id, "sources://video/"))                   parent = "library://video/";
    else if (StringUtils::StartsWithNoCase(child_id, "special://profile/playlists/music/")) parent = "special://musicplaylists/";
    else if (StringUtils::StartsWithNoCase(child_id, "special://profile/playlists/video/")) parent = "special://videoplaylists/";
    else parent = "sources://video/"; // this can only match video sources
  }

  if (item->IsVideoDb())
      thumb_loader = NPT_Reference<CThumbLoader>(new CVideoThumbLoader());
  else if (item->IsMusicDb())
    thumb_loader = NPT_Reference<CThumbLoader>(new CMusicThumbLoader());

  if (!thumb_loader.IsNull())
    thumb_loader->OnLoaderStart();
  */

  // TODO: this needs to be done somewhere else
  if (StringUtils::StartsWith(path, "videodb://") ||
      StringUtils::StartsWith(path, "library://video/"))
  {
    if (path == "videodb://" || path == "library://video/")
    {
      item->SetLabel("Video Library"); // TODO: localization?
      item->SetLabelPreformated(true);
    }
    else
    {
      if (!item->HasVideoInfoTag())
      {
        XFILE::VIDEODATABASEDIRECTORY::CQueryParams params;
        XFILE::VIDEODATABASEDIRECTORY::CDirectoryNode::GetDatabaseInfo(path, params);

        CVideoDatabase db;
        if (!db.Open())
        {
          CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: failed to open video database to get details for \"%s\"", path.c_str());
          invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Failed to access database");
          return false;
        }

        if (params.GetMovieId() >= 0)
          db.GetMovieInfo(path, *item->GetVideoInfoTag(), params.GetMovieId());
        else if (params.GetMVideoId() >= 0)
          db.GetMusicVideoInfo(path, *item->GetVideoInfoTag(), params.GetMVideoId());
        else if (params.GetEpisodeId() >= 0)
          db.GetEpisodeInfo(path, *item->GetVideoInfoTag(), params.GetEpisodeId());
        else if (params.GetTvShowId() >= 0)
        {
          if (params.GetSeason() >= 0)
            db.GetSeasonInfo(db.GetSeasonId(params.GetTvShowId(), params.GetSeason()), *item->GetVideoInfoTag());
          else
            db.GetTvShowInfo(path, *item->GetVideoInfoTag(), params.GetTvShowId());
        }
      }

      if (item->GetLabel().empty()) {
        // try to grab it from the folder
        std::string label;
        if (XFILE::CVideoDatabaseDirectory::GetLabel(path, label))
        {
          item->SetLabel(label);
          item->SetLabelPreformated(true);
        }
      }
    }
  }
  else if (StringUtils::StartsWith(path, "musicdb://") ||
    StringUtils::StartsWith(path, "library://music/"))
  {
    if (path == "musicdb://" || path == "library://music/")
    {
      item->SetLabel("Music Library"); // TODO: localization?
      item->SetLabelPreformated(true);
    }
    else
    {
      if (!item->HasMusicInfoTag())
      {
        XFILE::MUSICDATABASEDIRECTORY::CQueryParams params;
        XFILE::MUSICDATABASEDIRECTORY::CDirectoryNode::GetDatabaseInfo(path, params);

        CMusicDatabase db;
        if (!db.Open())
        {
          CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: failed to open video database to get details for \"%s\"", path.c_str());
          invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Failed to access database");
          return false;
        }

        if (params.GetSongId() >= 0)
        {
          CSong song;
          if (db.GetSong(params.GetSongId(), song))
            item->GetMusicInfoTag()->SetSong(song);
        }
        else if (params.GetAlbumId() >= 0)
        {
          CAlbum album;
          if (db.GetAlbum(params.GetAlbumId(), album, false))
            item->GetMusicInfoTag()->SetAlbum(album);
        }
        else if (params.GetArtistId() >= 0)
        {
          CArtist artist;
          if (db.GetArtist(params.GetArtistId(), artist, false))
            item->GetMusicInfoTag()->SetArtist(artist);
        }
      }

      if (item->GetLabel().empty())
      {
        // if no label try to grab it from node type
        std::string label;
        if (XFILE::CMusicDatabaseDirectory::GetLabel(path, label)) {
          item->SetLabel(label);
          item->SetLabelPreformated(true);
        }
      }
    }
  }
  else
  {
    // retrieve the parent directory and look for the item
    std::string parentPath = URIUtils::GetParentPath(path);
    if (parentPath.empty())
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: unknown object \"%s\" requested", path.c_str());
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_OBJECT, "Object doesn't exist");
      return false;
    }

    CFileItemList allItems;
    if (!XFILE::CDirectory::GetDirectory(parentPath, allItems) || !allItems.Contains(path))
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: unknown object \"%s\" requested", path.c_str());
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_OBJECT, "Object doesn't exist");
      return false;
    }

    item = allItems.Get(path);
  }

  return true;
}

COhUPnPMediaServerDevice::COhUPnPMediaServerDevice(const std::string& uuid,
  const CFileItemElementFactory& fileItemElementFactory,
  COhUPnPTransferManager& transferManager,
  COhUPnPResourceManager& resourceManager)
  : IOhUPnPService(UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_MEDIASERVER, 3),
    m_transferManager(transferManager),
    m_resourceManager(resourceManager),
    m_uuid(uuid),
    m_device(nullptr),
    m_contentDirectory(nullptr),
    m_elementFactory(fileItemElementFactory)
{ }

COhUPnPMediaServerDevice::~COhUPnPMediaServerDevice()
{
  Stop();
}

void COhUPnPMediaServerDevice::Start(bool supportImporting)
{
  if (m_contentDirectory != nullptr)
    return;

  // setup device
  m_device = std::make_unique<COhUPnPRootDevice>(m_uuid, m_resourceManager);
  m_device->SetDomain(m_domain);
  m_device->SetDeviceType(m_serviceName);
  m_device->SetDeviceTypeVersion(m_serviceVersion);
  m_device->SetFriendlyName(CSysInfo::GetDeviceName());
  m_device->SetModelName(CSysInfo::GetAppName());
  m_device->SetModelNumber(CSysInfo::GetVersion());
  m_device->SetModelDescription(StringUtils::Format("%s - Media Server", CSysInfo::GetAppName().c_str()));
  m_device->SetModelUrl("http://kodi.tv/");
  m_device->SetManufacturer("XBMC Foundation");
  m_device->SetManufacturerUrl("http://kodi.tv/");
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_WEBSERVER))
    m_device->SetPresentationUrl(StringUtils::Format("http://%s:%d", COhUtils::TIpAddressToString(COhUPnP::GetInstance().GetIpAddress()).c_str(), CSettings::GetInstance().GetInt(CSettings::SETTING_SERVICES_WEBSERVERPORT)));
  m_device->SetIcons({
    CUPnPIcon(m_resourceManager.AddSmallResource(*m_device, "special://xbmc/media/icon256x256.png", "icon256x256.png"), "image/png", 256, 256, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*m_device, "special://xbmc/media/icon120x120.png", "icon120x120.png"), "image/png", 120, 120, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*m_device, "special://xbmc/media/icon48x48.png", "icon48x48.png"), "image/png", 48, 48, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*m_device, "special://xbmc/media/icon32x32.png", "icon32x32.png"), "image/png", 32, 32, 8),
    CUPnPIcon(m_resourceManager.AddSmallResource(*m_device, "special://xbmc/media/icon16x16.png", "icon16x16.png"), "image/png", 16, 16, 8)
  });

  // create a ContentDirectory service
  m_contentDirectory.reset(new ContentDirectory(*this, m_device->GetDevice(), supportImporting));

  // enable the device
  m_device->Enable();
}

void COhUPnPMediaServerDevice::Stop()
{
  if (!IsRunning())
    return;

  // first disable the device
  m_device->Disable(OpenHome::MakeFunctor(*this, &COhUPnPMediaServerDevice::OnDeviceDisabled));

  // wait for the device to be disabled
  if (!m_deviceDisabledEvent.WaitMSec(DeviceDisableTimeoutMs))
    CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: device didn't stop in time");

  // then destroy the ContentDirectory service
  m_contentDirectory.reset();

  // and finally destroy the device
  m_device.reset();
}

bool COhUPnPMediaServerDevice::IsRunning() const
{
  return m_contentDirectory != nullptr;
}

void COhUPnPMediaServerDevice::OnDeviceDisabled()
{
  m_deviceDisabledEvent.Set();
}

COhUPnPMediaServerDevice::ContentDirectory::ContentDirectory(COhUPnPMediaServerDevice& mediaServer, OpenHome::Net::DvDeviceStd& device, bool supportImporting)
  : OpenHome::Net::DvProviderUpnpOrgContentDirectory3Cpp(device),
    m_mediaServer(mediaServer),
    m_createdObjectID(0)
{
  if (supportImporting)
  {
    // enable the properties we support
    EnablePropertyTransferIDs();
    SetPropertyTransferIDs("");
  }

  // enable the actions we support
  EnableActionGetSearchCapabilities();
  EnableActionGetSortCapabilities();
  // TODO: EnableActionGetFeatureList();
  // TODO (causes a crash on startup): EnableActionGetSystemUpdateID();
  EnableActionGetServiceResetToken();
  EnableActionBrowse();
  // TODO: EnableActionSearch();
  if (supportImporting)
    EnableActionCreateObject();
  // TODO: EnableActionUpdateObject();
  if (supportImporting)
    EnableActionImportResource();
  // TODO: EnableActionExportResource();
  if (supportImporting)
  {
    EnableActionStopTransferResource();
    EnableActionGetTransferProgress();
  }
}

COhUPnPMediaServerDevice::ContentDirectory::~ContentDirectory()
{ }

void COhUPnPMediaServerDevice::ContentDirectory::GetSearchCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& searchCaps)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetSearchCapabilities()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: GetSearchCapabilities() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  searchCaps = "upnp:class";

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetSearchCapabilities(): searchCaps = %s", searchCaps.c_str());
}

void COhUPnPMediaServerDevice::ContentDirectory::GetSortCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& sortCaps)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetSortCapabilities()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: GetSortCapabilities() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  std::vector<std::string> sortProperties;
  for (const auto& sortCapability : SortCapabilities)
    sortProperties.push_back(sortCapability.first);

  sortCaps = StringUtils::Join(sortProperties, UPnPCsvSeparator);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetSortCapabilities(): sortCaps = %s", sortCaps.c_str());
}

void COhUPnPMediaServerDevice::ContentDirectory::GetSortExtensionCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& sortExtensionCaps)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: GetSortExtensionCapabilities is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::GetFeatureList(OpenHome::Net::IDvInvocationStd& invocation, std::string& featureList)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: GetFeatureList is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::GetSystemUpdateID(OpenHome::Net::IDvInvocationStd& invocation, uint32_t& id)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetSystemUpdateID()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: GetSystemUpdateID() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  // TODO
  id = 0;

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetSystemUpdateID(): system update ID = %u", id);
}

void COhUPnPMediaServerDevice::ContentDirectory::GetServiceResetToken(OpenHome::Net::IDvInvocationStd& invocation, std::string& resetToken)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetServiceResetToken()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice: GetServiceResetToken() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  // TODO
  resetToken = "0";

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetServiceResetToken(): reset token = %s", resetToken.c_str());
}

void COhUPnPMediaServerDevice::ContentDirectory::Browse(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& browseFlag,
  const std::string& filter, uint32_t startingIndex, uint32_t requestedCount, const std::string& sortCriteria,
  std::string& result, uint32_t& numberReturned, uint32_t& totalMatches, uint32_t& aUpdateID)
{
  static const std::string BrowseFlagDirectChildren = "BrowseDirectChildren";
  static const std::string BrowseFlagMetadata = "BrowseMetadata";

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] <-- Browse(%s, %s, %s, %u, %u, %s)",
      objectID.c_str(), browseFlag.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria.c_str());
  }

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse(%s, %s, %s, %u, %u, %s) from %s (version %u; user-agent: %s)",
    objectID.c_str(), browseFlag.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria.c_str(),
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  result.clear();
  numberReturned = 0;
  totalMatches = 0;
  aUpdateID = 0; // TODO

  // get a (matching) profile
  COhUPnPDeviceProfile profile;
  if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(clientDevice, profile))
    CLog::Log(LOGINFO, "COhUPnPMediaServerDevice::Browse: client doesn't match any profiles");
  else
    CLog::Log(LOGINFO, "COhUPnPMediaServerDevice::Browse: client matches profile %s", profile.GetName().c_str());

  // parse the sort criteria
  SortDescription sort;
  if (!ParseSortCriteria(sortCriteria, sort))
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: invalid sort criteria \"%s\"", sortCriteria.c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_SORT_CRITERIA, "Unsupported or invalid sort criteria");
    return;
  }

  // map special objectID's
  std::string path = objectID;
  if (profile.GetMappedPath(objectID, path))
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: mapped requested object ID \"%s\" to path \"%s\"", objectID.c_str(), path.c_str());

  if (path.empty() || path == "0")
    path = UPnPPathRoot;

  // make sure access to the path/item is allowed
  if (!CFileUtils::RemoteAccessAllowed(path))
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: access to \"%s\" denied", path.c_str());
    invocation.ReportError(UPNP_ERROR_CD_SOURCE_RESOURCE_ACCESS_DENIED, "Access denied");
    return;
  }

  CFileItemList items;
  std::string parent;
  uint32_t total = 0;

  if (browseFlag == BrowseFlagDirectChildren)
  {
    // try to determine the proper parentID
    parent = objectID;
    if (parent.empty())
      parent = "-1";
    else if (parent == UPnPPathRoot)
      parent = "0";

    items.SetPath(path);

    if (!items.Load())
    {
      CPerformanceMeasurement<> getDirectoryPerformance;

      if (StringUtils::StartsWith(path, UPnPPathRoot))
      {
        CFileItemPtr item;

        // music library
        item.reset(new CFileItem("library://music/", true));
        item->SetLabel("Music Library"); // TODO: localization?
        item->SetLabelPreformated(true);
        items.Add(item);

        // video library
        item.reset(new CFileItem("library://video/", true));
        item->SetLabel("Video Library"); // TODO: localization?
        item->SetLabelPreformated(true);
        items.Add(item);

        items.Sort(SortByLabel, SortOrderAscending);
      }
      else
      {
        // this is the only way to hide unplayable items in the 'files'
        // view as we cannot tell what context (eg music vs video) the
        // request came from
        std::string supported = g_advancedSettings.m_pictureExtensions + "|"
          + g_advancedSettings.m_videoExtensions + "|"
          + g_advancedSettings.GetMusicExtensions() + "|"
          + g_advancedSettings.m_discStubExtensions;
        if (!XFILE::CDirectory::GetDirectory(path, items, supported))
        {
          CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: unknown container \"%s\" requested", path.c_str());
          invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_CONTAINER, "Container doesn't exist");
          return;
        }

        // if there's no sorting defined try to get the default sorting
        if (sort.sortBy == SortByNone)
          GetDefaultSort(path, items, sort);

        // sort the items
        items.Sort(sort);
      }

      getDirectoryPerformance.Stop();

      // cache anything that takes more than a second to retrieve
      if (items.CacheToDiscAlways() ||
         (items.CacheToDiscIfSlow() && getDirectoryPerformance.GetDurationInSeconds() > 1.0))
        items.Save();

      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: [Performance] get directory: %fs", getDirectoryPerformance.GetDurationInSeconds());
    }

    // sanitize the input parameters
    total = static_cast<uint32_t>(items.Size());
    uint32_t start = std::min(startingIndex, total);
    uint32_t count = std::min(requestedCount, total - start);

    {
      CPerformanceMeasurement<> limitResultsPerformance;

      // remove all items that are before the starting and after the ending index
      uint32_t end = count + start;
      for (uint32_t index = 0; index < total; ++index)
      {
        // remove the first item until we reach the starting index
        if (index < start)
          items.Remove(0);
        // remove all items after the end
        else if (index >= end)
          items.Remove(count);
      }

      limitResultsPerformance.Stop();
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: [Performance] limit results: %fs", limitResultsPerformance.GetDurationInSeconds());
    }
  }
  else if (browseFlag == BrowseFlagMetadata)
  {
    CFileItemPtr item;
    if (path == UPnPPathRoot)
    {
      item.reset(new CFileItem(path, true));
      item->SetLabel(m_mediaServer.m_device->GetFriendlyName());
      item->SetLabelPreformated(true);

      parent = "-1";
    }
    else
    {
      item.reset(new CFileItem(path, XFILE::CDirectory::Exists(path)));

      if (!LoadFileItem(invocation, item))
        return;
    }

    if (item == nullptr)
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: unknown object \"%s\" requested", path.c_str());
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_OBJECT, "Object doesn't exist");
      return;
    }

    total = 1;
    items.Add(item);
  }
  else
  {
    CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice::Browse: called with unknown BrowseFlag \"%s\"", browseFlag.c_str());
    invocation.ReportError(UPNP_ERROR_INVALID_ARGS, "Unknown BrowseFlag");
    return;
  }

  // prepare the DIDL-Lite document
  CDidlLiteDocument doc(m_mediaServer.m_elementFactory);
  doc.AddNamespace(UPNP_DIDL_LITE_NAMESPACE_URI);
  doc.AddNamespace(UPNP_DIDL_DC_NAMESPACE, UPNP_DIDL_DC_NAMESPACE_URI);
  doc.AddNamespace(UPNP_DIDL_UPNP_NAMESPACE, UPNP_DIDL_UPNP_NAMESPACE_URI);

  // determine the proper IP address and port for resource access
  std::string ipAddress = COhUtils::TIpAddressToString(COhUPnP::GetInstance().GetIpAddress());
  CURL resourceUrl;
  if (invocation.ResourceUriPrefix() != nullptr)
  {
    resourceUrl.Parse(invocation.ResourceUriPrefix());
    if (!resourceUrl.GetHostName().empty())
      ipAddress = resourceUrl.GetHostName();
  }

  // set the currently valid resource URI prefix
  const std::string resourceUriPrefix = COhUPnPResourceManager::GetResourceUriPrefix(ipAddress, m_mediaServer.m_resourceManager.GetPort());

  // get a context
  OhUPnPRootDeviceContext context = { clientDevice, profile, resourceUriPrefix, CDidlLitePropertyList(filter) };

  {
    CPerformanceMeasurement<> fileItemsConversionPerformance;

    // turn the items into a DIDL-Lite document
    if (!FileItemUtils::FileItemListToDocument(items, doc, m_mediaServer.m_elementFactory, context, parent))
    {
      CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice::Browse: failed to convert CFileItemList into DIDL-Lite document for \"%s\"", path.c_str());
      invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Failed to convert items");
      return;
    }

    fileItemsConversionPerformance.Stop();
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: [Performance] CFileItemList to CDidlLiteDocument: %fs", fileItemsConversionPerformance.GetDurationInSeconds());
  }

  {
    CPerformanceMeasurement<> serializationPerformance;

    // serialize the output/result
    if (!doc.Serialize(result, context))
    {
      result.clear();

      CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice::Browse: failed to serialize DIDL-Lite document for \"%s\"", path.c_str());
      invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Failed to serialize DIDL-Lite elements");
      return;
    }

    serializationPerformance.Stop();
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::Browse: [Performance] CDidlLiteDocument serialization: %fs", serializationPerformance.GetDurationInSeconds());
  }

  // set the output parameters
  totalMatches = total;
  startingIndex = std::min(startingIndex, totalMatches);
  numberReturned = std::min(requestedCount, totalMatches - startingIndex);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> Browse(%s, %s, %s, %u, %u, %s): count = %u; total = %u",
      objectID.c_str(), browseFlag.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria.c_str(),
      numberReturned, totalMatches);
    CLog::Log(LOGDEBUG, "[ohNet] --> Browse(%s, %s, %s, %u, %u, %s): result =\n%s",
      objectID.c_str(), browseFlag.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria.c_str(), result.c_str());
  }
}

void COhUPnPMediaServerDevice::ContentDirectory::Search(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& searchCriteria,
  const std::string& filter, uint32_t startingIndex, uint32_t requestedCount, const std::string& sortCriteria,
  std::string& result, uint32_t& numberReturned, uint32_t& totalMatches, uint32_t& aUpdateID)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: Search is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::CreateObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& elements,
  std::string& objectID, std::string& result)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- CreateObject(%s, %s)", containerID.c_str(), elements.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject(%s) from %s (version %u; user-agent: %s)",
    containerID.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  objectID.clear();
  result.clear();

  if (containerID.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: unknown container ID");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_CONTAINER, "No such container");
    return;
  }

  if (elements.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: empty elements");
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // get a (matching) profile
  COhUPnPDeviceProfile profile;
  if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(clientDevice, profile))
    CLog::Log(LOGINFO, "COhUPnPMediaServerDevice::CreateObject: client doesn't match any profiles");
  else
    CLog::Log(LOGINFO, "COhUPnPMediaServerDevice::CreateObject: client matches profile %s", profile.GetName().c_str());

  // map special objectID's
  std::string path = containerID;
  if (profile.GetMappedPath(containerID, path))
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: mapped requested container ID \"%s\" to path \"%s\"", containerID.c_str(), path.c_str());

  if (path.empty() || path == "0")
    path = UPnPPathRoot;

  // make sure access to the path/item is allowed
  if (!CFileUtils::RemoteAccessAllowed(path))
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: access to \"%s\" denied", path.c_str());
    invocation.ReportError(UPNP_ERROR_CD_SOURCE_RESOURCE_ACCESS_DENIED, "Access denied");
    return;
  }

  OhUPnPControlPointContext deserializationContext = { clientDevice, profile };

  // parse and deserialize the elements as a DIDL-Lite document
  CDidlLiteDocument elementsDoc(m_mediaServer.m_elementFactory);
  if (!elementsDoc.Deserialize(elements, deserializationContext))
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid elements");
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  const auto& items = elementsDoc.GetElements();
  if (items.size() != 1)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid number of elements (%u)", static_cast<unsigned int>(items.size()));
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  const auto& item = items.front();
  const CUPnPItem* upnpItem = dynamic_cast<const CUPnPItem*>(item);
  if (upnpItem == nullptr)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: unknown item <%s>",
      DidlLiteUtils::GetElementName(item->GetElementNamespace(), item->GetElementName()).c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // the @id attribute must be empty
  if (!upnpItem->GetId().empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid <item> with non-empty @id \"%s\"", upnpItem->GetId().c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // the @parentID attribute must match the containerID parameter
  if (upnpItem->GetParentId() != containerID)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid <item> with mismatching @parentID \"%s\" <> \"%s\"",
      upnpItem->GetParentId().c_str(), containerID.c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // the @restricted attribute must be false
  if (upnpItem->IsRestricted())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid <item> due to being restricted");
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // we don't support pre-defined resources
  auto resources = upnpItem->GetResources();
  if (resources.size() > 1)
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid <item> due to too many pre-defined <res> properties (%u)",
      static_cast<unsigned int>(resources.size()));
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  for (const auto& resource : resources)
  {
    if (!resource->GetUri().empty())
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid <item> due to not supporting pre-defined <res> properties");
      invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
      return;
    }
  }

  // make sure that we support the item's media type
  const auto& mediaType = profile.GetClassMapping().GetMediaType(upnpItem);
  if (mediaType.empty() || CMediaTypes::IsContainer(mediaType))
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: invalid <item> due to unsupported upnp:class \"%s\"", upnpItem->GetClass().GetType().c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // create the object ID
  objectID = StringUtils::Format("upnp://%s/created/%llu", m_mediaServer.m_device->GetUuid().c_str(), m_createdObjectID++);

  // clone the created object/item and assign an object ID
  std::shared_ptr<CUPnPItem> createdItem(upnpItem->Clone());
  createdItem->SetId(objectID);
  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::CreateObject: creating %s item \"%s\" with object ID \"%s\"...",
    mediaType.c_str(), createdItem->GetTitle().c_str(), objectID.c_str());

  // add/adjust the <res> property by injecting the @importUri attribute
  resources = createdItem->GetResources();
  CUPnPResource* resource = nullptr;
  if (!resources.empty())
    resource = resources.front()->Clone();
  else
    resource = new CUPnPResource("");

  resource->SetImportUri(objectID);

  auto protocolInfo = resource->GetProtocolInfo();
  protocolInfo.SetProtocol("http-get"); // TODO: http?
  resource->SetProtocolInfo(protocolInfo);
  createdItem->SetResources({ resource });

  // get a context
  OhUPnPRootDeviceContext serializationContext = { clientDevice, profile };

  // prepare the DIDL-Lite document for serialization of the created object/item
  CDidlLiteDocument resultDoc(m_mediaServer.m_elementFactory);
  resultDoc.AddNamespace(UPNP_DIDL_LITE_NAMESPACE_URI);
  resultDoc.AddNamespace(UPNP_DIDL_DC_NAMESPACE, UPNP_DIDL_DC_NAMESPACE_URI);
  resultDoc.AddNamespace(UPNP_DIDL_UPNP_NAMESPACE, UPNP_DIDL_UPNP_NAMESPACE_URI);

  // add the created object/item
  resultDoc.AddElement(createdItem->Clone());

  // serialize the created object/item
  if (!resultDoc.Serialize(result, serializationContext))
  {
    result.clear();

    CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice::CreateObject: failed to serialize DIDL-Lite document for created object \"%s\"", objectID.c_str());
    invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Failed to serialize DIDL-Lite elements");
    return;
  }

  // remember the created object/item
  CSingleLock lock(m_criticalCreatedObjects);
  m_createdObjects.insert({ objectID, createdItem });

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> CreateObject(%s, %s): object ID = %s", containerID.c_str(), elements.c_str(), objectID.c_str());
    CLog::Log(LOGDEBUG, "[ohNet] --> CreateObject(%s, %s): result =\n%s", containerID.c_str(), elements.c_str(), result.c_str());
  }
}

void COhUPnPMediaServerDevice::ContentDirectory::DestroyObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: DestroyObject is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::UpdateObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& currentTagValue, const std::string& newTagValue)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: UpdateObject is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::MoveObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& newParentID, std::string& newObjectID)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: MoveObject is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::ImportResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& sourceURI, const std::string& destinationURI, uint32_t& transferID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- ImportResource(%s, %s)", sourceURI.c_str(), destinationURI.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::ImportResource(%s, %s) from %s (version %u; user-agent: %s)",
    sourceURI.c_str(), destinationURI.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  transferID = 0;

  if (sourceURI.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::ImportResource: unknown source URI");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_SOURCE_RESOURCE, "No such source resource");
    return;
  }

  if (destinationURI.empty() || m_createdObjects.find(destinationURI) == m_createdObjects.end())
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::ImportResource: unknown destination URI");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource");
    return;
  }

  // find the created object
  {
    CSingleLock lock(m_criticalCreatedObjects);
    const auto& createdObject = m_createdObjects.find(destinationURI);
    if (createdObject == m_createdObjects.end())
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::ImportResource: destination URI hasn't been created");
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource");
      return;
    }

    // get a (matching) profile
    COhUPnPDeviceProfile profile;
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(clientDevice, profile))
      CLog::Log(LOGINFO, "COhUPnPMediaServerDevice::CreateObject: client doesn't match any profiles");
    else
      CLog::Log(LOGINFO, "COhUPnPMediaServerDevice::CreateObject: client matches profile %s", profile.GetName().c_str());

    // create a transfer for the object/item to be imported
    CFileItem item;
    if (!createdObject->second->ToFileItem(item, { clientDevice, profile }))
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::ImportResource: failed to determine item to be imported");
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource"); // TODO
      return;
    }

    if (!m_mediaServer.m_transferManager.Import(clientDevice, sourceURI, item, this, transferID))
    {
      CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::ImportResource: failed to create transfer");
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource"); // TODO
      return;
    }
  }

  // append the new transfer ID to the list of active transfer IDs
  PropertiesLock();
  std::string transferIds;
  GetPropertyTransferIDs(transferIds);

  if (!transferIds.empty())
    transferIds += ",";
  transferIds += StringUtils::Format("%u", transferID);
  SetPropertyTransferIDs(transferIds);
  PropertiesUnlock();

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> ImportResource(%s, %s): transfer ID = %u", sourceURI.c_str(), destinationURI.c_str(), transferID);
}

void COhUPnPMediaServerDevice::ContentDirectory::ExportResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& sourceURI, const std::string& destinationURI, uint32_t& transferID)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: ExportResource is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::DeleteResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& resourceURI)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: DeleteResource is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::StopTransferResource(OpenHome::Net::IDvInvocationStd& invocation, uint32_t transferID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- StopTransferResource(%u)", transferID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::StopTransferResource(%u) from %s (version %u; user-agent: %s)",
    transferID, COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (!m_mediaServer.m_transferManager.StopTransfer(transferID))
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::StopTransferResource: unable to stop unknown transfer");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_FILE_TRANSFER, "No such file transfer");
    return;
  }
}

void COhUPnPMediaServerDevice::ContentDirectory::GetTransferProgress(OpenHome::Net::IDvInvocationStd& invocation, uint32_t transferID, std::string& transferStatus, std::string& transferLength, std::string& transferTotal)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransferProgress(%u)", transferID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::GetTransferProgress(%u) from %s (version %u; user-agent: %s)",
    transferID, COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  ohUPnPTransferStatus status;
  uint64_t progress, total;
  if (!m_mediaServer.m_transferManager.GetTransferProgress(transferID, status, progress, total))
  {
    CLog::Log(LOGDEBUG, "COhUPnPMediaServerDevice::GetTransferProgress: unable to monitor unknown transfer");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_FILE_TRANSFER, "No such file transfer");
    return;
  }

  switch (status)
  {
  case ohUPnPTransferStatus::Completed:
    transferStatus = "COMPLETED";
    break;

  case ohUPnPTransferStatus::Stopped:
    transferStatus = "STOPPED";
    break;

  case ohUPnPTransferStatus::Error:
    transferStatus = "ERROR";
    break;

  case ohUPnPTransferStatus::InProgress:
  default:
    transferStatus = "IN_PROGRESS";
    break;
  }

  transferLength = StringUtils::Format("%" PRIu64, progress);
  transferTotal = StringUtils::Format("%" PRIu64, total);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> GetTransferProgress(%u): status = %s; length = %s; total = %s",
      transferID, transferStatus.c_str(), transferLength.c_str(), transferTotal.c_str());
  }
}

void COhUPnPMediaServerDevice::ContentDirectory::CreateReference(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& objectID, std::string& newID)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: CreateReference is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::FreeFormQuery(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, uint32_t CDSView, const std::string& queryRequest, std::string& qQueryResult, uint32_t& updateID)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: FreeFormQuery is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::GetFreeFormQueryCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& FFQCapabilities)
{
  CLog::Log(LOGWARNING, "COhUPnPMediaServerDevice: GetFreeFormQueryCapabilities is unsupported");
}

void COhUPnPMediaServerDevice::ContentDirectory::OnTransferCompleted(uint32_t transferId)
{
  // TODO
}

void COhUPnPMediaServerDevice::ContentDirectory::OnTransferFailed(uint32_t transferId)
{
  // TODO
}
