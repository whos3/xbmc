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

#include "ohUPnPContentDirectoryService.h"
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
          CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: failed to open video database to get details for \"%s\"", path.c_str());
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
          CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: failed to open video database to get details for \"%s\"", path.c_str());
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
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: unknown object \"%s\" requested", path.c_str());
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_OBJECT, "Object doesn't exist");
      return false;
    }

    CFileItemList allItems;
    if (!XFILE::CDirectory::GetDirectory(parentPath, allItems) || !allItems.Contains(path))
    {
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: unknown object \"%s\" requested", path.c_str());
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_OBJECT, "Object doesn't exist");
      return false;
    }

    item = allItems.Get(path);
  }

  return true;
}

COhUPnPContentDirectoryService::COhUPnPContentDirectoryService(COhUPnPRootDevice& device,
  const CFileItemElementFactory& fileItemElementFactory,
  COhUPnPTransferManager& transferManager,
  COhUPnPResourceManager& resourceManager)
  : IOhUPnPService(UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_CONTENTDIRECTORY, 3),
    m_device(device),
    m_contentDirectory(nullptr),
    m_transferManager(transferManager),
    m_resourceManager(resourceManager),
    m_elementFactory(fileItemElementFactory)
{ }

COhUPnPContentDirectoryService::~COhUPnPContentDirectoryService()
{
  Stop();
}

void COhUPnPContentDirectoryService::Start(bool supportImporting)
{
  if (m_contentDirectory != nullptr)
    return;

  // create a ContentDirectory service
  m_contentDirectory.reset(new ContentDirectory(*this, *m_device.GetDevice(), supportImporting));
}

void COhUPnPContentDirectoryService::Stop()
{
  if (!IsRunning())
    return;

  // then destroy the ContentDirectory service
  m_contentDirectory.reset();
}

bool COhUPnPContentDirectoryService::IsRunning() const
{
  return m_contentDirectory != nullptr;
}

COhUPnPContentDirectoryService::ContentDirectory::ContentDirectory(COhUPnPContentDirectoryService& service, OpenHome::Net::DvDeviceStd& device, bool supportImporting)
  : OpenHome::Net::DvProviderUpnpOrgContentDirectory3Cpp(device),
    m_service(service),
    m_createdObjectID(0)
{
  // enable the properties we support
  if (supportImporting)
  {
    EnablePropertyTransferIDs();
    SetPropertyTransferIDs("");
  }

  EnablePropertySystemUpdateID();
  SetPropertySystemUpdateID(0);

  // enable the actions we support
  EnableActionGetSearchCapabilities();
  EnableActionGetSortCapabilities();
  EnableActionGetFeatureList();
  EnableActionGetSystemUpdateID();
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

COhUPnPContentDirectoryService::ContentDirectory::~ContentDirectory()
{ }

void COhUPnPContentDirectoryService::ContentDirectory::GetSearchCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& searchCaps)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetSearchCapabilities()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: GetSearchCapabilities() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  searchCaps = ""; // TODO: "@id,@parentID,upnp:class";

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetSearchCapabilities(): searchCaps = %s", searchCaps.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::GetSortCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& sortCaps)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetSortCapabilities()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: GetSortCapabilities() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  std::vector<std::string> sortProperties;
  for (const auto& sortCapability : SortCapabilities)
    sortProperties.push_back(sortCapability.first);

  sortCaps = StringUtils::Join(sortProperties, UPnPCsvSeparator);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetSortCapabilities(): sortCaps = %s", sortCaps.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::GetSortExtensionCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& sortExtensionCaps)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetSortExtensionCapabilities()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: GetSortExtensionCapabilities() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: GetSortExtensionCapabilities is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetSortExtensionCapabilities(): sortExtensionCaps = %s", sortExtensionCaps.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::GetFeatureList(OpenHome::Net::IDvInvocationStd& invocation, std::string& featureList)
{
  static CXBMCTinyXML doc;
  static TiXmlPrinter printer;
  if (doc.RootElement() == nullptr)
  {
    doc.InsertEndChild(TiXmlElement("Features"));
    doc.Accept(&printer);
  }

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetFeatureList()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: GetFeatureList() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  featureList = printer.Str();

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetFeatureList(): featureList = %s", featureList.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::GetSystemUpdateID(OpenHome::Net::IDvInvocationStd& invocation, uint32_t& id)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetSystemUpdateID()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: GetSystemUpdateID() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  GetPropertySystemUpdateID(id);

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetSystemUpdateID(): system update ID = %u", id);
}

void COhUPnPContentDirectoryService::ContentDirectory::GetServiceResetToken(OpenHome::Net::IDvInvocationStd& invocation, std::string& resetToken)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetServiceResetToken()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: GetServiceResetToken() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  resetToken = "0";

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetServiceResetToken(): reset token = %s", resetToken.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::Browse(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& browseFlag,
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

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse(%s, %s, %s, %u, %u, %s) from %s (version %u; user-agent: %s)",
    objectID.c_str(), browseFlag.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria.c_str(),
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  result.clear();
  numberReturned = 0;
  totalMatches = 0;
  GetPropertySystemUpdateID(aUpdateID);

  // get a (matching) profile
  COhUPnPDeviceProfile profile;
  if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(clientDevice, profile))
    CLog::Log(LOGINFO, "COhUPnPContentDirectoryService::Browse: client doesn't match any profiles");
  else
    CLog::Log(LOGINFO, "COhUPnPContentDirectoryService::Browse: client matches profile %s", profile.GetName().c_str());

  // parse the sort criteria
  SortDescription sort;
  if (!ParseSortCriteria(sortCriteria, sort))
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: invalid sort criteria \"%s\"", sortCriteria.c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_SORT_CRITERIA, "Unsupported or invalid sort criteria");
    return;
  }

  // map special objectID's
  std::string path = objectID;
  if (profile.GetMappedPath(objectID, path))
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: mapped requested object ID \"%s\" to path \"%s\"", objectID.c_str(), path.c_str());

  if (path.empty() || path == "0")
    path = UPnPPathRoot;

  // make sure access to the path/item is allowed
  if (!CFileUtils::RemoteAccessAllowed(path))
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: access to \"%s\" denied", path.c_str());
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
          CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: unknown container \"%s\" requested", path.c_str());
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

      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: [Performance] get directory: %fs", getDirectoryPerformance.GetDurationInSeconds());
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
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: [Performance] limit results: %fs", limitResultsPerformance.GetDurationInSeconds());
    }
  }
  else if (browseFlag == BrowseFlagMetadata)
  {
    CFileItemPtr item;
    if (path == UPnPPathRoot)
    {
      item.reset(new CFileItem(path, true));
      item->SetLabel(m_service.m_device.GetFriendlyName());
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
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: unknown object \"%s\" requested", path.c_str());
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_OBJECT, "Object doesn't exist");
      return;
    }

    total = 1;
    items.Add(item);
  }
  else
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService::Browse: called with unknown BrowseFlag \"%s\"", browseFlag.c_str());
    invocation.ReportError(UPNP_ERROR_INVALID_ARGS, "Unknown BrowseFlag");
    return;
  }

  // prepare the DIDL-Lite document
  CDidlLiteDocument doc(m_service.m_elementFactory);
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
  const std::string resourceUriPrefix = COhUPnPResourceManager::GetResourceUriPrefix(ipAddress, m_service.m_resourceManager.GetPort());

  // get a context
  OhUPnPRootDeviceContext context = { clientDevice, profile, resourceUriPrefix, CDidlLitePropertyList(filter) };

  {
    CPerformanceMeasurement<> fileItemsConversionPerformance;

    // turn the items into a DIDL-Lite document
    if (!FileItemUtils::FileItemListToDocument(items, doc, m_service.m_elementFactory, context, parent))
    {
      CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService::Browse: failed to convert CFileItemList into DIDL-Lite document for \"%s\"", path.c_str());
      invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Failed to convert items");
      return;
    }

    fileItemsConversionPerformance.Stop();
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: [Performance] CFileItemList to CDidlLiteDocument: %fs", fileItemsConversionPerformance.GetDurationInSeconds());
  }

  {
    CPerformanceMeasurement<> serializationPerformance;

    // serialize the output/result
    if (!doc.Serialize(result, context))
    {
      result.clear();

      CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService::Browse: failed to serialize DIDL-Lite document for \"%s\"", path.c_str());
      invocation.ReportError(UPNP_ERROR_ACTION_FAILED, "Failed to serialize DIDL-Lite elements");
      return;
    }

    serializationPerformance.Stop();
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::Browse: [Performance] CDidlLiteDocument serialization: %fs", serializationPerformance.GetDurationInSeconds());
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

void COhUPnPContentDirectoryService::ContentDirectory::Search(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& searchCriteria,
  const std::string& filter, uint32_t startingIndex, uint32_t requestedCount, const std::string& sortCriteria,
  std::string& result, uint32_t& numberReturned, uint32_t& totalMatches, uint32_t& updateID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- Search(%s, %s, %s, %u, %u, %s)",
    containerID.c_str(), searchCriteria.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: Search(%s, %s, %s, %u, %u, %s) from %s (version %u; user-agent: %s)",
    containerID.c_str(), searchCriteria.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria,
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: Search is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> Search(%s, %s, %s, %u, %u, %s): count = %u; total = %u; update ID = %u",
      containerID.c_str(), searchCriteria.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria,
      numberReturned, totalMatches, updateID);
    CLog::Log(LOGDEBUG, "[ohNet] --> Search(%s, %s, %s, %u, %u, %s): result =\n%s",
      containerID.c_str(), searchCriteria.c_str(), filter.c_str(), startingIndex, requestedCount, sortCriteria, result.c_str());
  }
}

void COhUPnPContentDirectoryService::ContentDirectory::CreateObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& elements,
  std::string& objectID, std::string& result)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- CreateObject(%s, %s)", containerID.c_str(), elements.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject(%s) from %s (version %u; user-agent: %s)",
    containerID.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  objectID.clear();
  result.clear();

  if (containerID.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: unknown container ID");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_CONTAINER, "No such container");
    return;
  }

  if (elements.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: empty elements");
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // get a (matching) profile
  COhUPnPDeviceProfile profile;
  if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(clientDevice, profile))
    CLog::Log(LOGINFO, "COhUPnPContentDirectoryService::CreateObject: client doesn't match any profiles");
  else
    CLog::Log(LOGINFO, "COhUPnPContentDirectoryService::CreateObject: client matches profile %s", profile.GetName().c_str());

  // map special objectID's
  std::string path = containerID;
  if (profile.GetMappedPath(containerID, path))
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: mapped requested container ID \"%s\" to path \"%s\"", containerID.c_str(), path.c_str());

  if (path.empty() || path == "0")
    path = UPnPPathRoot;

  // make sure access to the path/item is allowed
  if (!CFileUtils::RemoteAccessAllowed(path))
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: access to \"%s\" denied", path.c_str());
    invocation.ReportError(UPNP_ERROR_CD_SOURCE_RESOURCE_ACCESS_DENIED, "Access denied");
    return;
  }

  OhUPnPControlPointContext deserializationContext = { clientDevice, profile };

  // parse and deserialize the elements as a DIDL-Lite document
  CDidlLiteDocument elementsDoc(m_service.m_elementFactory);
  if (!elementsDoc.Deserialize(elements, deserializationContext))
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid elements");
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  const auto& items = elementsDoc.GetElements();
  if (items.size() != 1)
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid number of elements (%u)", static_cast<unsigned int>(items.size()));
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  const auto& item = items.front();
  const CUPnPItem* upnpItem = dynamic_cast<const CUPnPItem*>(item);
  if (upnpItem == nullptr)
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: unknown item <%s>",
      DidlLiteUtils::GetElementName(item->GetElementNamespace(), item->GetElementName()).c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // the @id attribute must be empty
  if (!upnpItem->GetId().empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid <item> with non-empty @id \"%s\"", upnpItem->GetId().c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // the @parentID attribute must match the containerID parameter
  if (upnpItem->GetParentId() != containerID)
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid <item> with mismatching @parentID \"%s\" <> \"%s\"",
      upnpItem->GetParentId().c_str(), containerID.c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // the @restricted attribute must be false
  if (upnpItem->IsRestricted())
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid <item> due to being restricted");
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // we don't support pre-defined resources
  auto resources = upnpItem->GetResources();
  if (resources.size() > 1)
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid <item> due to too many pre-defined <res> properties (%u)",
      static_cast<unsigned int>(resources.size()));
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  for (const auto& resource : resources)
  {
    if (!resource->GetUri().empty())
    {
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid <item> due to not supporting pre-defined <res> properties");
      invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
      return;
    }
  }

  // make sure that we support the item's media type
  const auto& mediaType = profile.GetClassMapping().GetMediaType(upnpItem);
  if (mediaType.empty() || CMediaTypes::IsContainer(mediaType))
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: invalid <item> due to unsupported upnp:class \"%s\"", upnpItem->GetClass().GetType().c_str());
    invocation.ReportError(UPNP_ERROR_CD_BAD_METADATA, "Bad metadata");
    return;
  }

  // create the object ID
  objectID = StringUtils::Format("upnp://%s/created/%llu", m_service.m_device.GetUuid().c_str(), m_createdObjectID++);

  // clone the created object/item and assign an object ID
  std::shared_ptr<CUPnPItem> createdItem(upnpItem->Clone());
  createdItem->SetId(objectID);
  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::CreateObject: creating %s item \"%s\" with object ID \"%s\"...",
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
  CDidlLiteDocument resultDoc(m_service.m_elementFactory);
  resultDoc.AddNamespace(UPNP_DIDL_LITE_NAMESPACE_URI);
  resultDoc.AddNamespace(UPNP_DIDL_DC_NAMESPACE, UPNP_DIDL_DC_NAMESPACE_URI);
  resultDoc.AddNamespace(UPNP_DIDL_UPNP_NAMESPACE, UPNP_DIDL_UPNP_NAMESPACE_URI);

  // add the created object/item
  resultDoc.AddElement(createdItem->Clone());

  // serialize the created object/item
  if (!resultDoc.Serialize(result, serializationContext))
  {
    result.clear();

    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService::CreateObject: failed to serialize DIDL-Lite document for created object \"%s\"", objectID.c_str());
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

void COhUPnPContentDirectoryService::ContentDirectory::DestroyObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- DestroyObject(%s)", objectID.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: DestroyObject(%s) from %s (version %u; user-agent: %s)",
    objectID.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: DestroyObject is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> DestroyObject(%s)", objectID.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::UpdateObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& currentTagValue, const std::string& newTagValue)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- UpdateObject(%s, %s, %s)", objectID.c_str(), currentTagValue.c_str(), newTagValue.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: UpdateObject(%s, %s, %s) from %s (version %u; user-agent: %s)",
    objectID.c_str(), currentTagValue.c_str(), newTagValue.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: UpdateObject is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> UpdateObject(%s, %s, %s)",
    objectID.c_str(), currentTagValue.c_str(), newTagValue.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::MoveObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& newParentID, std::string& newObjectID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- MoveObject(%s, %s)", objectID.c_str(), newParentID.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: MoveObject(%s, %s) from %s (version %u; user-agent: %s)",
    objectID.c_str(), newParentID.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: MoveObject is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> MoveObject(%s, %s): new object ID = %s", objectID.c_str(), newParentID.c_str(), newObjectID.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::ImportResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& sourceURI, const std::string& destinationURI, uint32_t& transferID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- ImportResource(%s, %s)", sourceURI.c_str(), destinationURI.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::ImportResource(%s, %s) from %s (version %u; user-agent: %s)",
    sourceURI.c_str(), destinationURI.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  transferID = 0;

  if (sourceURI.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::ImportResource: unknown source URI");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_SOURCE_RESOURCE, "No such source resource");
    return;
  }

  if (destinationURI.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::ImportResource: unknown destination URI");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource");
    return;
  }

  // find the created object
  {
    CSingleLock lock(m_criticalCreatedObjects);
    const auto& createdObject = m_createdObjects.find(destinationURI);
    if (createdObject == m_createdObjects.end())
    {
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::ImportResource: destination URI hasn't been created");
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource");
      return;
    }

    // get a (matching) profile
    COhUPnPDeviceProfile profile;
    if (!COhUPnPDeviceProfilesManager::GetInstance().FindProfile(clientDevice, profile))
      CLog::Log(LOGINFO, "COhUPnPContentDirectoryService::CreateObject: client doesn't match any profiles");
    else
      CLog::Log(LOGINFO, "COhUPnPContentDirectoryService::CreateObject: client matches profile %s", profile.GetName().c_str());

    // create a transfer for the object/item to be imported
    CFileItem item;
    if (!createdObject->second->ToFileItem(item, { clientDevice, profile }))
    {
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::ImportResource: failed to determine item to be imported");
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource"); // TODO
      return;
    }

    if (!m_service.m_transferManager.Import(clientDevice, sourceURI, item, this, transferID))
    {
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::ImportResource: failed to create transfer");
      invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_DESTINATION_RESOURCE, "No such destination resource"); // TODO
      return;
    }

    // remember the active transfer
    m_createdObjectTransfers.insert({ transferID, destinationURI });
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

void COhUPnPContentDirectoryService::ContentDirectory::ExportResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& sourceURI, const std::string& destinationURI, uint32_t& transferID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- ExportResource(%s, %s)", sourceURI.c_str(), destinationURI.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: ExportResource(%s, %s) from %s (version %u; user-agent: %s)",
    sourceURI.c_str(), destinationURI.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: ExportResource is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> ExportResource(%s, %s): transfer ID = %u", sourceURI.c_str(), destinationURI.c_str(), transferID);
}

void COhUPnPContentDirectoryService::ContentDirectory::DeleteResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& resourceURI)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- DeleteResource(%s)", resourceURI.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: DeleteResource(%s) from %s (version %u; user-agent: %s)",
    resourceURI.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: DeleteResource is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> DeleteResource(%s)", resourceURI.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::StopTransferResource(OpenHome::Net::IDvInvocationStd& invocation, uint32_t transferID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- StopTransferResource(%u)", transferID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::StopTransferResource(%u) from %s (version %u; user-agent: %s)",
    transferID, COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  if (!m_service.m_transferManager.StopTransfer(transferID))
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::StopTransferResource: unable to stop unknown transfer");
    invocation.ReportError(UPNP_ERROR_CD_NO_SUCH_FILE_TRANSFER, "No such file transfer");
    return;
  }
}

void COhUPnPContentDirectoryService::ContentDirectory::GetTransferProgress(OpenHome::Net::IDvInvocationStd& invocation, uint32_t transferID, std::string& transferStatus, std::string& transferLength, std::string& transferTotal)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransferProgress(%u)", transferID);

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::GetTransferProgress(%u) from %s (version %u; user-agent: %s)",
    transferID, COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  ohUPnPTransferStatus status;
  uint64_t progress, total;
  if (!m_service.m_transferManager.GetTransferProgress(transferID, status, progress, total))
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService::GetTransferProgress: unable to monitor unknown transfer");
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

void COhUPnPContentDirectoryService::ContentDirectory::CreateReference(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& objectID, std::string& newID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- CreateReference(%s, %s)", containerID.c_str(), objectID.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: CreateReference(%s, %s) from %s (version %u; user-agent: %s)",
    containerID.c_str(), objectID.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: CreateReference is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> CreateReference(%s, %s): new ID = %u", containerID.c_str(), objectID.c_str(), newID.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::FreeFormQuery(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, uint32_t CDSView, const std::string& queryRequest, std::string& queryResult, uint32_t& updateID)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- FreeFormQuery(%s, %u, %s)", containerID.c_str(), CDSView, queryRequest.c_str());

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: FreeFormQuery(%s, %u, %s) from %s (version %u; user-agent: %s)",
    containerID.c_str(), CDSView, queryRequest.c_str(), COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: FreeFormQuery is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
  {
    CLog::Log(LOGDEBUG, "[ohNet] --> FreeFormQuery(%s, %u, %s): update ID = %u",
      containerID.c_str(), CDSView, queryRequest.c_str(), updateID);
    CLog::Log(LOGDEBUG, "[ohNet] --> FreeFormQuery(%s, %u, %s): result =\n%s",
      containerID.c_str(), CDSView, queryRequest.c_str(), queryResult.c_str());
  }
}

void COhUPnPContentDirectoryService::ContentDirectory::GetFreeFormQueryCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& ffqCapabilities)
{
  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] <-- GetFreeFormQueryCapabilities()");

  COhUPnPClientDevice clientDevice(invocation);

  TIpAddress ip;
  uint32_t port;
  invocation.GetClientEndpoint(ip, port);

  CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryService: GetFreeFormQueryCapabilities() from %s (version %u; user-agent: %s)",
    COhUtils::TIpAddressToString(ip).c_str(), invocation.Version(), clientDevice.GetUserAgent().c_str());

  CLog::Log(LOGWARNING, "COhUPnPContentDirectoryService: GetFreeFormQueryCapabilities is unsupported");
  invocation.ReportError(UPNP_ERROR_ACTION_NOT_IMPLEMENTED, "Action not implemented");

  if (g_advancedSettings.CanLogComponent(LOGUPNP))
    CLog::Log(LOGDEBUG, "[ohNet] --> GetFreeFormQueryCapabilities(): free form query capabilities = %s", ffqCapabilities.c_str());
}

void COhUPnPContentDirectoryService::ContentDirectory::OnTransferCompleted(uint32_t transferId)
{
  RemoveTransfer(transferId);
}

void COhUPnPContentDirectoryService::ContentDirectory::OnTransferFailed(uint32_t transferId)
{
  RemoveTransfer(transferId);
}

void COhUPnPContentDirectoryService::ContentDirectory::RemoveTransfer(uint32_t transferId)
{
  CSingleLock lock(m_criticalCreatedObjects);
  const auto& transfer = m_createdObjectTransfers.find(transferId);
  if (transfer == m_createdObjectTransfers.cend())
    return;

  const auto& createdObject = m_createdObjects.find(transfer->second);
  m_createdObjectTransfers.erase(transfer);

  if (createdObject == m_createdObjects.cend())
    return;

  m_createdObjects.erase(createdObject);
}
