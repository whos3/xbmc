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

#include "ohUPnPTransferJob.h"
#include "URL.h"
#include "Util.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "filesystem/CurlFile.h"
#include "filesystem/Directory.h"
#include "filesystem/FileFactory.h"
#include "guilib/GUIWindowManager.h"
#include "media/MediaType.h"
#include "music/tags/MusicInfoTag.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/controlpoints/ohUPnPContentDirectoryControlPoint.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "threads/Thread.h"
#include "utils/log.h"
#include "utils/Mime.h"
#include "utils/ScraperUtils.h"
#include "utils/SystemInfo.h"
#include "utils/URIUtils.h"
#include "video/Episode.h"
#include "video/VideoInfoTag.h"

static const size_t ImportReadBufferSize = 8192;

static const int GetTransferProgressIntervalMs = 1000; // TODO

uint32_t COhUPnPTransferJob::s_nextId = 0;

static std::string GetImportFilename(const XFILE::CCurlFile& httpFile, const CURL& sourceUrl, const CFileItem& item)
{
  // try to get the filename from the HTTP headers
  std::string filename = httpFile.GetHttpHeader().GetFilename();

  // if there's no filename in the Content-Disposition header
  if (filename.empty())
  {
    // try to get it from the MIME type
    if (!httpFile.GetMimeType().empty())
    {
      std::string extension = CMime::GetExtension(httpFile.GetMimeType());
      if (!extension.empty())
        filename = item.GetLabel() + "." + extension;
    }

    // if that doesn't work try to parse the URL
    if (filename.empty())
    {
      std::string urlFilename = URIUtils::GetFileName(CURL::Decode(sourceUrl.GetFileNameWithoutPath()));
      if (!urlFilename.empty() && URIUtils::HasExtension(urlFilename))
        filename = urlFilename;

      if (filename.empty())
        return filename;
    }
  }

  // make sure that the determined filename is only a filename and doesn't contain any path
  // and that it's a legal filename
  return CUtil::MakeLegalFileName(URIUtils::GetFileName(filename));
}

static std::string GetImportDestinationPath(const std::string& destinationDirectory, std::string filename, const CFileItem& item)
{
  std::string destinationPath = destinationDirectory;

  // figure out the media type of the item
  MediaType mediaType;
  if (item.HasVideoInfoTag())
    mediaType = item.GetVideoInfoTag()->m_type;
  else if (item.HasMusicInfoTag())
    mediaType = item.GetMusicInfoTag()->GetType();

  if (!mediaType.empty())
  {
    if (mediaType == MediaTypeMovie || mediaType == MediaTypeMusicVideo || mediaType == MediaTypePicture)
    {
      destinationPath = URIUtils::AddFileToFolder(destinationPath, CMediaTypes::ToPlural(mediaType));

      // for movies if available add the year to the end of the filename
      if (mediaType == MediaTypeMovie && item.GetVideoInfoTag()->m_iYear > 0)
      {
        const std::string extension = URIUtils::GetExtension(filename);
        const std::string name = filename.substr(0, filename.size() - extension.size());
        filename = StringUtils::Format("%s (%d)%s", name.c_str(), item.GetVideoInfoTag()->m_iYear, extension.c_str());
      }
    }
    else if (mediaType == MediaTypeEpisode)
    {
      destinationPath = URIUtils::AddFileToFolder(destinationPath, CMediaTypes::ToPlural(MediaTypeTvShow));
      if (!item.GetVideoInfoTag()->m_strShowTitle.empty())
      {
        destinationPath = URIUtils::AddFileToFolder(destinationPath, item.GetVideoInfoTag()->m_strShowTitle);
        // TODO: season directory?
      }

      // if the filename doesn't contain season and/or episode number we add them manually
      std::vector<VIDEO::EPISODE> episodes;
      if (item.GetVideoInfoTag()->m_iSeason >= 0 && item.GetVideoInfoTag()->m_iEpisode >= 0 &&
          !ScraperUtils::GetEpisodeDetailsFromName(filename, "", g_advancedSettings.m_tvshowEnumRegExps, g_advancedSettings.m_tvshowMultiPartEnumRegExp, episodes))
        filename = StringUtils::Format("S%02dE%02d - %s", item.GetVideoInfoTag()->m_iSeason, item.GetVideoInfoTag()->m_iEpisode, filename.c_str());
    }
    else if (mediaType == MediaTypeSong)
    {
      destinationPath = URIUtils::AddFileToFolder(destinationPath, CMediaTypes::ToPlural(MediaTypeMusic));
      const std::string artist = item.GetMusicInfoTag()->GetAlbumArtistString();
      const std::string& album = item.GetMusicInfoTag()->GetAlbum();

      if (!artist.empty())
        destinationPath = URIUtils::AddFileToFolder(destinationPath, artist);
      if (!album.empty())
        destinationPath = URIUtils::AddFileToFolder(destinationPath, album);
      if (item.GetMusicInfoTag()->GetDiscNumber() > 0)
        destinationPath = URIUtils::AddFileToFolder(destinationPath, StringUtils::Format("Disc %d", item.GetMusicInfoTag()->GetDiscNumber())); // TODO: localization???

      // TODO: check if the filename already contains track number
      if (item.GetMusicInfoTag()->GetTrackNumber() > 0)
        filename = StringUtils::Format("%02d - %s", item.GetMusicInfoTag()->GetTrackNumber(), filename.c_str());
    }
  }

  return URIUtils::AddFileToFolder(destinationPath, filename);
}

static bool PrepareDestinationPath(const std::string& destinationPath)
{
  const std::string destinationDirectory = URIUtils::GetDirectory(destinationPath);

  auto directories = URIUtils::SplitPath(destinationDirectory);
  if (directories.empty())
    return false;

  // create all missing intermediate directories
  std::string intermediateDirectory = directories.front();
  for (auto it = ++directories.cbegin(); it != directories.cend(); ++it)
  {
    intermediateDirectory = URIUtils::AddFileToFolder(intermediateDirectory, *it);
    if (XFILE::CDirectory::Exists(intermediateDirectory))
      continue;

    if (!XFILE::CDirectory::Create(intermediateDirectory))
      return false;
  }

  return true;
}

COhUPnPTransferJob::COhUPnPTransferJob(const COhUPnPDevice& device, const std::string& sourceUri,
  const CFileItem& item, IOhUPnPTransferCallbacks* callback)
  : COhUPnPTransferJob(s_nextId, device, sourceUri, item, callback)
{ }

COhUPnPTransferJob::COhUPnPTransferJob(uint32_t id, const COhUPnPDevice& device, const std::string& sourceUri,
  const CFileItem& item, IOhUPnPTransferCallbacks* callback)
  : CProgressJob()
  , m_id(id)
  , m_device(device)
  , m_sourceUri(sourceUri)
  , m_item(item)
  , m_progress(0L)
  , m_total(0L)
  , m_speed(nullptr)
  , m_status(ohUPnPTransferStatus::InProgress)
  , m_callback(callback)
{
  m_label = item.GetLabel();

  // for episodes adjust the label to contain the tvshow, season and episode number
  // TODO: localization
  if (item.HasVideoInfoTag() && item.GetVideoInfoTag()->m_type == MediaTypeEpisode)
  {
    if (!item.GetVideoInfoTag()->m_strShowTitle.empty())
      m_label = item.GetVideoInfoTag()->m_strShowTitle + ": ";

    bool needSeparator = false;
    if (item.GetVideoInfoTag()->m_iSeason >= 0)
    {
      m_label += StringUtils::Format("S%02d", item.GetVideoInfoTag()->m_iSeason);
      needSeparator = true;
    }
    if (item.GetVideoInfoTag()->m_iEpisode >= 0)
    {
      m_label += StringUtils::Format("E%02d", item.GetVideoInfoTag()->m_iEpisode);
      needSeparator = true;
    }

    if (needSeparator)
      m_label += " - ";

    if (!item.GetVideoInfoTag()->m_strTitle.empty())
      m_label += item.GetVideoInfoTag()->m_strTitle;
    else
      m_label += item.GetLabel();
  }
}

COhUPnPTransferJob::COhUPnPTransferJob(const COhUPnPTransferJob& other)
  : CProgressJob()
  , m_id(other.m_id)
  , m_device(other.m_device)
  , m_sourceUri(other.m_sourceUri)
  , m_item(other.m_item)
  , m_label(other.m_label)
  , m_progress(other.m_progress)
  , m_total(other.m_total)
  , m_speed(other.m_speed)
  , m_status(other.m_status)
  , m_callback(other.m_callback)
{ }

COhUPnPTransferJob::~COhUPnPTransferJob()
{
  delete m_speed;
  m_speed = nullptr;
}

double COhUPnPTransferJob::GetTransferSpeed() const
{
  if (m_speed == nullptr)
    return 0.0;

  return m_speed->GetCurrentTransferSpeed();
}

uint64_t COhUPnPTransferJob::GetRemainingSeconds() const
{
  if (m_speed == nullptr)
    return 0LL;

  return m_speed->GetRemainingSeconds();
}

bool COhUPnPTransferJob::operator==(const CJob* job) const
{
  const COhUPnPTransferJob* transferJob = dynamic_cast<const COhUPnPTransferJob*>(job);
  if (transferJob == nullptr)
    return false;

  return m_device.GetUuid() == transferJob->m_device.GetUuid() &&
    m_sourceUri == transferJob->m_sourceUri &&
    m_item.IsSamePath(&transferJob->m_item);
}

bool COhUPnPTransferJob::DoWork()
{
  m_status = ohUPnPTransferStatus::InProgress;

  if (!Transfer())
  {
    if (m_status != ohUPnPTransferStatus::Stopped)
      m_status = ohUPnPTransferStatus::Error;

    return false;
  }

  m_status = ohUPnPTransferStatus::Completed;
  return true;
}

bool COhUPnPTransferJob::ShouldCancel(uint64_t progress, uint64_t total) const
{
  m_progress = progress;
  m_total = total;

  if (!CProgressJob::ShouldCancel(progress, total))
    return false;

  m_status = ohUPnPTransferStatus::Stopped;
  return true;
}

COhUPnPImportTransferJob::COhUPnPImportTransferJob(const COhUPnPDevice& device, const std::string& sourceUri,
  const CFileItem& item, IOhUPnPTransferCallbacks* callback)
  : COhUPnPTransferJob(device, sourceUri, item, callback),
    m_destinationDirectory(CSettings::GetInstance().GetString(CSettings::SETTING_SERVICES_UPNPTRANSFERIMPORTPATH))
{ }

COhUPnPImportTransferJob::COhUPnPImportTransferJob(const COhUPnPTransferJob& other)
  : COhUPnPTransferJob(other),
    m_destinationDirectory(CSettings::GetInstance().GetString(CSettings::SETTING_SERVICES_UPNPTRANSFERIMPORTPATH))
{ }

bool COhUPnPImportTransferJob::IsValid() const
{
  // check if the file can be written to the destination path
  if (m_destinationDirectory.empty() || !XFILE::CDirectory::Exists(m_destinationDirectory))
    return false;

  // check if source URI exists
  XFILE::CCurlFile sourceFile;
  if (!sourceFile.Exists(CURL(m_sourceUri)))
    return false;

  return true;
}

bool COhUPnPImportTransferJob::Transfer()
{
  CURL sourceUrl(m_sourceUri);
  XFILE::CCurlFile sourceFile;
  sourceFile.SetUserAgent(CSysInfo::GetUserAgent());
  
  // force a HEAD requests to get the proper length
  uint64_t progress = 0, length = 0;
  struct __stat64 sourceFileStat;
  if (sourceFile.Stat(sourceUrl, &sourceFileStat) == 0)
    length = std::max(sourceFileStat.st_size, 0LL);

  // issue GET request
  if (!sourceFile.Open(sourceUrl))
  {
    CLog::Log(LOGERROR, "COhUPnPImportTransferJob: unable to access \"%s\" through HTTP GET", m_label.c_str());
    return false;
  }

  // try to get the filename
  std::string filename = GetImportFilename(sourceFile, sourceUrl, m_item);
  if (filename.empty())
  {
    CLog::Log(LOGERROR, "COhUPnPImportTransferJob: unable to determine a filename for \"%s\"", m_label.c_str());
    return false;
  }
 
  // get the destination path for the local file
  std::string destinationFilePath = GetImportDestinationPath(m_destinationDirectory, filename, m_item);

  // make sure the destination path exists and is writable
  if (!PrepareDestinationPath(destinationFilePath))
  {
    CLog::Log(LOGERROR, "COhUPnPImportTransferJob: unable to import \"%s\" to \"%s\"", m_label.c_str(), destinationFilePath.c_str());
    return false;
  }

  CURL destinationFileUrl(destinationFilePath);
  // create a loader for the destination file
  std::unique_ptr<XFILE::IFile> destinationFile(XFILE::CFileFactory::CreateLoader(destinationFileUrl));
  if (destinationFile == nullptr)
  {
    CLog::Log(LOGERROR, "COhUPnPImportTransferJob: unable to create file loader for \"%s\" with \"%s\"", m_label.c_str(), destinationFilePath.c_str());
    return false;
  }

  if (!destinationFile->OpenForWrite(destinationFileUrl, true))
  {
    CLog::Log(LOGERROR, "COhUPnPImportTransferJob: unable to open file at \"%s\" for \"%s\"", destinationFilePath.c_str(), m_label.c_str());
    return false;
  }

  // make sure the file is empty
  if (destinationFile->GetLength() > 0)
    destinationFile->Truncate(0);

  if (CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_UPNPTRANSFERPROGRESS))
  {
    CGUIDialogExtendedProgressBar* dialog =
      (CGUIDialogExtendedProgressBar*)g_windowManager.GetWindow(WINDOW_DIALOG_EXT_PROGRESS);
    SetProgressBar(dialog->GetHandle("UPnP import")); // TODO: localization
    SetText(StringUtils::Format("Importing %s...", m_label.c_str())); // TODO: localization
  }

  m_speed = new CTransferSpeed<>(length);
  char buffer[ImportReadBufferSize] = { 0 };
  ssize_t read = 0;
  while ((read = sourceFile.Read(buffer, sizeof(buffer))) > 0)
  {
    if (ShouldCancel(progress, length))
      break;

    if (destinationFile->Write(buffer, read) != read)
    {
      CLog::Log(LOGERROR, "COhUPnPImportTransferJob: error writing imported data to \"%s\" for \"%s\"", destinationFilePath.c_str(), m_label.c_str());
      break;
    }

    progress += read;
    memset(buffer, 0, sizeof(buffer));

    if (ShouldCancel(progress, length))
      break;

    m_speed->Transferred(read);

    if (progress % 20971520 < ImportReadBufferSize)
    {
      CLog::Log(LOGDEBUG, "[TRANSFER SPEED] current: %f MB/s; average: %f MB/s; remaining: %" PRIu64 " s",
        (m_speed->GetCurrentTransferSpeed() / (1024 * 1024)), (m_speed->GetAverageTransferSpeed() / (1024 * 1024)), m_speed->GetRemainingSeconds());
    }
  }

  if (length > 0 && progress != length)
  {
    // try to close the destination file
    destinationFile->Close();

    // try to delete the destination file
    if (!destinationFile->Delete(destinationFileUrl))
      CLog::Log(LOGWARNING, "COhUPnPImportTransferJob: failed to delete partially imported \"%s\" at \"%s\"", m_label.c_str(), destinationFilePath.c_str());

    return false;
  }

  CLog::Log(LOGINFO, "COhUPnPImportTransferJob: successfully imported \"%s\" to \"%s\"", m_label.c_str(), destinationFilePath.c_str());
  return true;
}

COhUPnPExportTransferJob::COhUPnPExportTransferJob(uint32_t id, const COhUPnPDevice& device, const std::string& sourceUri,
  const CFileItem& item, IOhUPnPTransferCallbacks* callback)
  : COhUPnPTransferJob(id, device, sourceUri, item, callback)
{ }

COhUPnPExportTransferJob::COhUPnPExportTransferJob(const COhUPnPTransferJob& other)
  : COhUPnPTransferJob(other)
{ }

bool COhUPnPExportTransferJob::Transfer()
{
  const auto& contentDirectoryControlPoint = COhUPnP::GetInstance().GetContentDirectoryClient();

  if (CSettings::GetInstance().GetBool(CSettings::SETTING_SERVICES_UPNPTRANSFERPROGRESS))
  {
    CGUIDialogExtendedProgressBar* dialog =
      (CGUIDialogExtendedProgressBar*)g_windowManager.GetWindow(WINDOW_DIALOG_EXT_PROGRESS);
    SetProgressBar(dialog->GetHandle("UPnP export")); // TODO: localization
    SetText(StringUtils::Format("Exporting %s to %s...", m_label.c_str(), m_device.GetFriendlyName().c_str())); // TODO: localization
  }

  const auto& uuid = m_device.GetUuid();
  m_status = ohUPnPTransferStatus::InProgress;

  uint64_t lastProgress = 0;
  while (m_status == ohUPnPTransferStatus::InProgress)
  {
    if (m_speed )
    if (!contentDirectoryControlPoint.GetTransferProgress(uuid, m_id, m_status, m_progress, m_total) ||
        ShouldCancel(m_progress, m_total))
      return false;

    if (m_speed == nullptr)
      m_speed = new CTransferSpeed<>(m_total, m_progress);
    else
      m_speed->Transferred(m_progress - lastProgress);

    lastProgress = m_progress;

    Sleep(GetTransferProgressIntervalMs);
  }

  return m_status == ohUPnPTransferStatus::Completed;
}
