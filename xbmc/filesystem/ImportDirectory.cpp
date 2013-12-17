/*
 *      Copyright (C) 2011-2013 Team XBMC
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

#include "ImportDirectory.h"
#include "DatabaseManager.h"
#include "DbUrl.h"
#include "FileItem.h"
#include "guilib/LocalizeStrings.h"
#include "filesystem/Directory.h"
#include "media/MediaType.h"
#include "media/import/IMediaImportRepository.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDbUrl.h"

using namespace std;
using namespace XFILE;

CImportDirectory::CImportDirectory(void)
{
}

CImportDirectory::~CImportDirectory(void)
{
}

bool CImportDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  CURL url(strPath);
  IMediaImportRepository *repository = NULL;
  CDbUrl *dbUrl = NULL;
  string library = url.GetHostName();
  if (library == "video")
  {
    repository = &CDatabaseManager::Get().GetVideoImportRepository();
    dbUrl = new CVideoDbUrl();
  }

  if (repository == NULL ||
      dbUrl == NULL)
  {
    delete dbUrl;
    return false;
  }

  // get the file name for determining the directory level
  string filename = url.GetFileName();
  URIUtils::RemoveSlashAtEnd(filename);
  vector<string> parts = StringUtils::Split(filename, string(1, url.GetDirectorySeparator()));
  if (!parts.empty() && parts.back().empty())
    parts.erase(parts.begin() + parts.size() - 1);

  // check if the first part is a media type
  MediaType mediaType;
  size_t index = 0;
  if (!parts.empty() && MediaTypes::IsValidMediaType(parts.at(0)))
  {
    mediaType = parts.at(0);
    index = 1;
  }
  
  // try to retrieve source, import and media type
  string strSource, strImport;
  if (parts.size() > index)
  {
    strSource = CURL::Decode(parts.at(index));
    index += 1;

    if (parts.size() > index)
    {
      strImport = CURL::Decode(parts.at(index));
      index += 1;

      if (parts.size() > index && mediaType.empty())
        mediaType = parts.at(index);
    }
  }

  bool result = false;

  // list all sources
  if (strSource.empty())
  {
    items.SetLabel(g_localizeStrings.Get(752));

    result = GetSources(strPath, repository, mediaType, items);
  }
  // list all imports for a source
  else if (strImport.empty())
  {
    CMediaImportSource source(strSource, "");
    if (repository->GetSource(strSource, source))
    {
      items.SetLabel(source.GetFriendlyName());

      result = GetImports(strPath, repository, strSource, mediaType, items);
    }
  }
  // list all media types for an import
  else if (mediaType.empty())
  {
    CMediaImport import(strImport, strSource);
    if (repository->GetImport(strImport, import))
    {
      set<MediaType> mediaTypes = import.GetImportedMediaTypes();
      for (set<MediaType>::const_iterator itMediaType = mediaTypes.begin(); itMediaType != mediaTypes.end(); ++itMediaType)
      {
        string sourcePath = URIUtils::AddFileToFolder(strPath, *itMediaType);
        URIUtils::AddSlashAtEnd(sourcePath);
        CFileItemPtr item(new CFileItem(sourcePath, true));
        item->SetLabel(*itMediaType); // TODO: localization
        item->m_dateTime = import.GetLastSynced();

        items.Add(item);
      }
    }

    result = true;
  }
  // list all items for a media type of an import
  else
  {
    string dbPath = library + "db://";
    if (mediaType == "episode")
      dbPath += "tvshows/titles/-1/-1";
    else
      dbPath += MediaTypes::ToPlural(mediaType) + "/titles";

    if (dbUrl->FromString(dbPath))
    {
      dbUrl->AddOption("source", strSource);
      dbUrl->AddOption("import", strImport);

      result = CDirectory::GetDirectory(dbUrl->ToString(), items);
    }
  }

  delete dbUrl;
  return result;
}

bool CImportDirectory::Exists(const char* strPath)
{
  if (strPath == NULL)
    return false;

  // TODO

  return false;
}

bool CImportDirectory::GetSources(const std::string &strPath, const IMediaImportRepository *repository, const MediaType &mediaType, CFileItemList &items)
{
  if (repository == NULL)
    return false;

  std::vector<CMediaImportSource> sources = repository->GetSources(mediaType);
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
  {
    string sourcePath = URIUtils::AddFileToFolder(strPath, CURL::Encode(itSource->GetIdentifier()));
    URIUtils::AddSlashAtEnd(sourcePath);
    CFileItemPtr item(new CFileItem(sourcePath, true));
    item->SetLabel(itSource->GetFriendlyName());
    item->m_dateTime = itSource->GetLastSynced();

    items.Add(item);
  }

  return true;
}

bool CImportDirectory::GetImports(const std::string &strPath, const IMediaImportRepository *repository, const std::string &sourceIdentifier, const MediaType &mediaType, CFileItemList &items)
{
  if (repository == NULL || sourceIdentifier.empty())
    return false;

  std::vector<CMediaImport> imports = repository->GetImports(sourceIdentifier);
  for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
  {
    // filter by imported media type if necessary
    if (!mediaType.empty() &&
        itImport->GetImportedMediaTypes().find(mediaType) == itImport->GetImportedMediaTypes().end())
      continue;

    string sourcePath = URIUtils::AddFileToFolder(strPath, CURL::Encode(itImport->GetPath()));
    URIUtils::AddSlashAtEnd(sourcePath);
    CFileItemPtr item(new CFileItem(sourcePath, true));
    if (itImport->GetPath() == sourceIdentifier)
      item->SetLabel(g_localizeStrings.Get(593));
    else
      item->SetLabel(itImport->GetPath());
    item->m_dateTime = itImport->GetLastSynced();

    items.Add(item);
  }

  return true;
}
