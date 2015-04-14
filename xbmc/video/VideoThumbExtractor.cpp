/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include <cstdlib>

#include "VideoThumbExtractor.h"
#include "TextureCache.h"
#include "TextureCacheJob.h"
#include "URL.h"
#include "cores/dvdplayer/DVDFileInfo.h"
#include "filesystem/StackDirectory.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"
#include "video/VideoInfoTag.h"

CVideoThumbExtractor::CVideoThumbExtractor(const CFileItem& item, const std::string& listpath, bool extracThumb,
                                           const std::string& target /* = "" */, int64_t pos /* = -1 */,
                                           bool fillStreamDetails /* = true */)
  : m_target(target),
    m_listpath(listpath),
    m_item(item),
    m_extractThumb(extracThumb),
    m_pos(pos),
    m_fillStreamDetails(fillStreamDetails)
{
  if (item.IsVideoDb() && item.HasVideoInfoTag())
    m_item.SetPath(item.GetVideoInfoTag()->m_strFileNameAndPath);

  if (m_item.IsStack())
    m_item.SetPath(XFILE::CStackDirectory::GetFirstStackedFile(m_item.GetPath()));
}

CVideoThumbExtractor::~CVideoThumbExtractor()
{ }

bool CVideoThumbExtractor::operator==(const CJob* job) const
{
  if (strcmp(job->GetType(), GetType()) == 0)
  {
    const CVideoThumbExtractor* jobExtract = dynamic_cast<const CVideoThumbExtractor*>(job);
    if (jobExtract != nullptr &&
        jobExtract->m_listpath == m_listpath &&
        jobExtract->m_target == m_target)
      return true;
  }

  return false;
}

bool CVideoThumbExtractor::DoWork()
{
  if (m_item.IsLiveTV() ||
      URIUtils::IsUPnP(m_item.GetPath()) ||
      m_item.IsDVD() ||
      m_item.IsDiscImage() ||
      m_item.IsDVDFile(false, true) ||
      m_item.IsInternetStream() ||
      m_item.IsDiscStub() ||
      m_item.IsPlayList())
    return false;

  // For HTTP/FTP we only allow extraction when on a LAN
  if (URIUtils::IsRemote(m_item.GetPath()) && !URIUtils::IsOnLAN(m_item.GetPath()) &&
     (URIUtils::IsFTP(m_item.GetPath()) || URIUtils::IsHTTP(m_item.GetPath())))
    return false;

  bool result = false;
  if (m_extractThumb)
  {
    CLog::Log(LOGDEBUG,"%s - trying to extract thumb from video file %s", __FUNCTION__, CURL::GetRedacted(m_item.GetPath()).c_str());

    // construct the thumb cache file
    CTextureDetails details;
    details.file = CTextureCache::GetCacheFile(m_target) + ".jpg";
    result = CDVDFileInfo::ExtractThumb(m_item.GetPath(), details, m_fillStreamDetails ? &m_item.GetVideoInfoTag()->m_streamDetails : NULL, (int)m_pos);
    if (result)
    {
      CTextureCache::Get().AddCachedTexture(m_target, details);
      m_item.SetProperty("HasAutoThumb", true);
      m_item.SetProperty("AutoThumbImage", m_target);
      m_item.SetArt("thumb", m_target);

      CVideoInfoTag* info = m_item.GetVideoInfoTag();
      if (info->m_iDbId > 0 && !info->m_type.empty())
      {
        CVideoDatabase db;
        if (db.Open())
        {
          db.SetArtForItem(info->m_iDbId, info->m_type, "thumb", m_item.GetArt("thumb"));
          db.Close();
        }
      }
    }
  }
  else if (!m_item.HasVideoInfoTag() || !m_item.GetVideoInfoTag()->HasStreamDetails())
  {
    // No tag or no details set, so extract them
    CLog::Log(LOGDEBUG,"%s - trying to extract filestream details from video file %s", __FUNCTION__, CURL::GetRedacted(m_item.GetPath()).c_str());
    result = CDVDFileInfo::GetFileStreamDetails(&m_item);
  }

  if (!result)
    return false;

  CVideoDatabase db;
  if (!db.Open())
    return false;

  CVideoInfoTag* info = m_item.GetVideoInfoTag();

  if (URIUtils::IsStack(m_listpath))
  {
    // Don't know the total time of the stack, so set duration to zero to avoid confusion
    info->m_streamDetails.SetVideoDuration(0, 0);

    // Restore original stack path
    m_item.SetPath(m_listpath);
  }

  if (info->m_iFileId < 0)
    db.SetStreamDetailsForFile(info->m_streamDetails, !info->m_strFileNameAndPath.empty() ? info->m_strFileNameAndPath : m_item.GetPath());
  else
    db.SetStreamDetailsForFileId(info->m_streamDetails, info->m_iFileId);

  // overwrite the runtime value if the one from streamdetails is available
  if (info->m_iDbId > 0 && info->m_duration != static_cast<int>(info->GetDuration()))
  {
    info->m_duration = static_cast<int>(info->GetDuration());

    // store the updated information in the database
    db.SetDetailsForItem(info->m_iDbId, info->m_type, *info, m_item.GetArt());
  }

  return true;
}
