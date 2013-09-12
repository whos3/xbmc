/*
 *      Copyright (C) 2013 Team XBMC
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

#include <string.h>

#include "system.h" // TODO
#include "MediaImportManager.h"
#include "interfaces/AnnouncementManager.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"
#include "media/import/MediaImportTaskProcessorJob.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "media/import/task/MediaImportSourceRegistrationTask.h"
#include "media/import/task/MediaImportSynchronisationTask.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/SpecialSort.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"

CMediaImportManager::CMediaImportManager()
  : CJobQueue(false, 1, CJob::PRIORITY_LOW_PAUSABLE)
{ }

CMediaImportManager::~CMediaImportManager()
{
  {
    CSingleLock lock(m_importersLock);
    for (std::map<const char*, const IMediaImporter*>::const_iterator it = m_importers.begin(); it != m_importers.end(); ++it)
      delete it->second;
    m_importers.clear();
  }

  {
    // TODO
    CSingleLock lock(m_importHandlersLock);
    for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
      delete *itHandler;
    m_importHandlers.clear();
    m_importHandlersMap.clear();
  }

  {
    CSingleLock lock(m_sourcesLock);
    for (std::map<std::string, bool>::const_iterator it = m_sources.begin(); it != m_sources.end(); ++it)
      UnregisterSource(it->first);
    m_sources.clear();
  }

  {
    CSingleLock lock(m_importRepositoriesLock);
    m_importRepositories.clear();
  }
}

CMediaImportManager& CMediaImportManager::Get()
{
  static CMediaImportManager instance;
  return instance;
}

void CMediaImportManager::RegisterSourceRepository(IMediaImportRepository* importRepository)
{
  if (importRepository == NULL ||
      !importRepository->Initialize())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: failed to register and initialize given import repository");
    return;
  }

  std::vector<CMediaImportSource> sources = importRepository->GetSources();

  CSingleLock repositoriesLock(m_importRepositoriesLock);
  m_importRepositories.insert(importRepository);
  
  // add the sources from the repository
  // TODO: handle sources in multiple repositories
  CSingleLock sourcesLock(m_sourcesLock);
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
    m_sources.insert(std::make_pair(itSource->GetIdentifier(), false));

  CLog::Log(LOGDEBUG, "CMediaImportManager: new import repository with %d sources added", sources.size());
}

bool CMediaImportManager::UnregisterSourceRepository(const IMediaImportRepository* importRepository)
{
  if (importRepository == NULL)
    return false;

  CSingleLock repositoriesLock(m_importRepositoriesLock);
  std::set<IMediaImportRepository*>::const_iterator it = m_importRepositories.find(const_cast<IMediaImportRepository*>(importRepository));
  if (it == m_importRepositories.end())
    return false;

  // remove all sources from that repository
  // TODO: handle sources in multiple repositories
  std::vector<CMediaImportSource> sources = importRepository->GetSources();
  CSingleLock sourceslock(m_sourcesLock);
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
    m_sources.erase(itSource->GetIdentifier());

  m_importRepositories.erase(it);
  CLog::Log(LOGDEBUG, "CMediaImportManager: import repository with %d sources removed", sources.size());

  return true;
}

void CMediaImportManager::RegisterImporter(const IMediaImporter* importer)
{
  if (importer == NULL)
    return;

  CSingleLock lock(m_importersLock);
  if (m_importers.find(importer->GetIdentification()) == m_importers.end())
  {
    m_importers.insert(std::make_pair(importer->GetIdentification(), importer));
    CLog::Log(LOGDEBUG, "CMediaImportManager: new importer %s added", importer->GetIdentification());
  }
}

bool CMediaImportManager::UnregisterImporter(const IMediaImporter* importer)
{
  CSingleLock lock(m_importersLock);
  std::map<const char*, const IMediaImporter*>::const_iterator it = m_importers.find(importer->GetIdentification());
  if (it == m_importers.end())
    return false;

  m_importers.erase(it);
  CLog::Log(LOGDEBUG, "CMediaImportManager: importer %s removed", importer->GetFriendlySourceName().c_str());
  lock.Leave();

  return true;
}

std::vector<const IMediaImporter*> CMediaImportManager::GetImporter() const
{
  std::vector<const IMediaImporter*> importer;

  CSingleLock lock(m_importersLock);
  for (std::map<const char*, const IMediaImporter*>::const_iterator it = m_importers.begin(); it != m_importers.end(); ++it)
    importer.push_back(it->second);

  return importer;
}

const IMediaImporter* CMediaImportManager::GetImporter(const std::string &path) const
{
  CSingleLock lock(m_importersLock);
  for (std::map<const char*, const IMediaImporter*>::const_iterator it = m_importers.begin(); it != m_importers.end(); ++it)
  {
    if (it->second->CanImport(path))
      return it->second;
  }

  return NULL;
}

void CMediaImportManager::RegisterMediaImportHandler(IMediaImportHandler *importHandler)
{
  if (importHandler == NULL)
    return;

  CSingleLock lock(m_importHandlersLock);
  if (m_importHandlersMap.find(importHandler->GetMediaType()) == m_importHandlersMap.end())
  {
    m_importHandlersMap.insert(make_pair(importHandler->GetMediaType(), importHandler));

    // build a dependency list
    std::vector< std::pair<MediaType, MediaType > > dependencies;
    for (std::map<MediaType, IMediaImportHandler*>::const_iterator itHandler = m_importHandlersMap.begin(); itHandler != m_importHandlersMap.end(); ++itHandler)
    {
      std::set<MediaType> mediaTypes = itHandler->second->GetDependencies();
      for (std::set<MediaType>::const_iterator itMediaType = mediaTypes.begin(); itMediaType != mediaTypes.end(); ++itMediaType)
        dependencies.push_back(make_pair(itHandler->first, *itMediaType));
    }

    // re-sort the import handlers and their dependencies
    std::vector<MediaType> result = SpecialSort::SortTopologically(dependencies);
    std::map<MediaType, IMediaImportHandler*> handlersCopy(m_importHandlersMap.begin(), m_importHandlersMap.end());
    m_importHandlers.clear();
    for (std::vector<MediaType>::const_iterator it = result.begin(); it != result.end(); ++it)
    {
      m_importHandlers.push_back(handlersCopy.find(*it)->second);
      handlersCopy.erase(*it);
    }
    for (std::map<MediaType, IMediaImportHandler*>::const_iterator itHandler = handlersCopy.begin(); itHandler != handlersCopy.end(); ++itHandler)
      m_importHandlers.push_back(itHandler->second);

    CLog::Log(LOGDEBUG, "CMediaImportManager: new import handler for %s added", importHandler->GetMediaType().c_str());
  }
}

void CMediaImportManager::UnregisterMediaImportHandler(IMediaImportHandler *importHandler)
{
  if (importHandler == NULL)
    return;

  CSingleLock lock(m_importHandlersLock);
  std::map<MediaType, IMediaImportHandler*>::const_iterator it = m_importHandlersMap.find(importHandler->GetMediaType());
  if (it == m_importHandlersMap.end() || it->second != importHandler)
    return;

  // remove the import handler from the map
  m_importHandlersMap.erase(it);
  // and from the sorted vector
  for (std::vector<IMediaImportHandler*>::const_iterator vecIt = m_importHandlers.begin(); vecIt != m_importHandlers.end(); ++vecIt)
  {
    if (*vecIt == importHandler)
    {
      m_importHandlers.erase(vecIt);
      CLog::Log(LOGDEBUG, "CMediaImportManager: import handler for %s removed", importHandler->GetMediaType().c_str());
      break;
    }
  }
}

void CMediaImportManager::RegisterSource(const std::string& sourceID, const std::string& friendlyName, const std::set<MediaType>& mediaTypes /* = std::set<MediaType>() */)
{
  if (sourceID.empty() || friendlyName.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to register source with invalid identifier \"%s\" or friendly name \"%s\"", sourceID.c_str(), friendlyName.c_str());
    return;
  }

  CMediaImportSource source(sourceID, friendlyName, mediaTypes);

  CSingleLock sourcesLock(m_sourcesLock);
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() || !FindSource(sourceID, source))
  {
    CSingleLock jobsLock(m_jobsLock);
    std::map<std::string, CJob*>::const_iterator itRegistrationJob = m_jobMap.find(sourceID);
    // if a registration job for this source is already running, there's nothing to do
    if (itRegistrationJob != m_jobMap.end())
    {
      CLog::Log(LOGDEBUG, "CMediaImportManager: source %s is already being registered, so nothing to do", sourceID.c_str());
      return;
    }

    CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(sourceID, this);

    CMediaImportSourceRegistrationTask *registrationTask =
      new CMediaImportSourceRegistrationTask(CMediaImport(sourceID, CMediaImportSource(sourceID, friendlyName, mediaTypes), mediaTypes));
    processorJob->SetTask(registrationTask);
    AddTaskProcessorJob(sourceID, processorJob);
    
    CLog::Log(LOGDEBUG, "CMediaImportManager: registration job for source %s queued", sourceID.c_str());
    return;
  }

  // update any possibly changed values
  CMediaImportSource updatedSource = source;
  updatedSource.SetFriendlyName(friendlyName);
  updatedSource.SetAvailableMediaTypes(mediaTypes);

  bool updated = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to update the source in at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->UpdateSource(updatedSource))
      updated = true;
  }
  repositoriesLock.Leave();

  if (updated)
    CLog::Log(LOGDEBUG, "CMediaImportManager: source %s updated", sourceID.c_str());

  // start processing all imports of the source
  ImportSource(sourceID);
}

void CMediaImportManager::UnregisterSource(const std::string& sourceID)
{
  if (sourceID.empty())
    return;

  CSingleLock lock(m_sourcesLock);
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end())
    return;

  itSource->second = false;
  CLog::Log(LOGDEBUG, "CMediaImportManager: source %s disabled", sourceID.c_str());
  lock.Leave();

  // if there's a task for the source which hasn't started yet or is still
  // running, try to cancel it
  std::map<std::string, CJob*>::const_iterator itJob = m_jobMap.find(sourceID);
  if (itJob != m_jobMap.end())
  {
    if (itJob->second != NULL)
      CancelJob(itJob->second);

    m_jobMap.erase(sourceID);
    CLog::Log(LOGDEBUG, "CMediaImportManager: source registration task for %s cancelled", sourceID.c_str());
  }

  // TODO: disable all items from that source
  SetImportItemsEnabled(sourceID, false);
}

void CMediaImportManager::RemoveSource(const std::string& sourceID)
{
  if (sourceID.empty())
    return;

  CSingleLock sourcesLock(m_sourcesLock);
  std::map<std::string, bool>::const_iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end())
    return;
  
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
    (*itRepository)->RemoveSource(sourceID);

  m_sources.erase(itSource);
  CLog::Log(LOGDEBUG, "CMediaImportManager: source %s removed", sourceID.c_str());
}

bool CMediaImportManager::Import(const std::string& sourceID, const std::string& path, const std::set<MediaType>& mediaTypes)
{
  if (sourceID.empty() || path.empty() || mediaTypes.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to import from invalid source \"%s\", path \"%s\" or without media types (%zu)",
              sourceID.c_str(), path.c_str(), mediaTypes.size());
    return false;
  }

  CMediaImport import(path, sourceID, mediaTypes);
  if (FindImport(path, import))
  {
    // make sure the source of the import matches the given source identifier
    if (import.GetSource().GetIdentifier() != sourceID)
    {
      CLog::Log(LOGERROR, "CMediaImportManager: source \"%s\" of given path \"%s\" of existing import doesn't match \"%s\"",
                sourceID.c_str(), path.c_str(), import.GetSource().GetIdentifier().c_str());
      return false;
    }

    Import(import);
    return true;
  }

  CMediaImportSource source(sourceID);
  CSingleLock sourcesLock(m_sourcesLock);
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() ||  // source doesn't exist
      !itSource->second ||            // source hasn't been registered yet
      !FindSource(sourceID, source))
  {
    CLog::Log(LOGWARNING, "CMediaImportManager:: trying to import from unknown or disabled source \"%s\"", sourceID.c_str());
    return false;
  }

  return ImportNew(source, path, mediaTypes);
}

bool CMediaImportManager::Import(const std::string& path)
{
  if (path.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to import from an empty path");
    return false;
  }

  CMediaImport import(path);
  if (!FindImport(path, import))
    return false;

  Import(import);
  return true;
}

bool CMediaImportManager::Import(const std::string& sourceID, const MediaType& mediaType, const CFileItemPtr& item)
{
  if (sourceID.empty() || item == NULL)
    return false;

  CFileItemList items;
  items.Add(item);
  return Import(sourceID, mediaType, items);
}

bool CMediaImportManager::Import(const std::string& sourceID, const MediaType& mediaType, const CFileItemList& items)
{
  if (sourceID.empty() || items.Size() <= 0 || items.GetPath().empty())
    return false;

  CSingleLock sourcesLock(m_sourcesLock);
  std::map<std::string, bool>::iterator itSource = m_sources.find(sourceID);
  if (itSource == m_sources.end() ||  // source doesn't exist
      !itSource->second)              // source hasn't been registered yet
    return false;

  /* TODO
  CSingleLock handlersLock(m_importHandlersLock);
  // loop through all available media import handlers and handle the imported items
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
  {
    IMediaImportHandler* handler = *itHandler;
    if (handler->GetMediaType() != mediaType)
      continue;

    CLog::Log(LOGINFO, "CMediaImportManager: handling %d imported %s items from %s", items.Size(), mediaType.c_str(), sourceName.c_str());
    // handle the imported items of a specific media type
    handler->HandleImportedItems(sourceID, sourceName, items, NULL);

    return true;
  }
  */
  
  CLog::Log(LOGWARNING, "CMediaImportManager: importing %d %s items from source %s failed (no media type handler)", items.Size(), mediaType, sourceID);
  return false;
}

bool CMediaImportManager::ImportSource(const std::string &sourceID)
{
  if (sourceID.empty())
  {
    CLog::Log(LOGWARNING, "CMediaImportManager: unable to import from an empty source identifier");
    return false;
  }

  // TODO: enable all items from that source
  SetImportItemsEnabled(sourceID, true);

  // get all the imports for the source
  std::vector<CMediaImport> imports;
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    std::vector<CMediaImport> repoImports = (*itRepository)->GetImports(sourceID);
    imports.insert(imports.end(), repoImports.begin(), repoImports.end());
  }

  // start importing media items from all the defined imports for the source
  for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    Import(*itImport);

  return true;
}

bool CMediaImportManager::ImportNew(const CMediaImportSource &source, const std::string& path, const std::set<MediaType>& mediaTypes)
{
  if (path.empty() || mediaTypes.empty())
  {
    CLog::Log(LOGERROR, "CMediaImportManager: unable to create new import with empty path \"%s\" or media types (%zu)", path.c_str(), mediaTypes.size());
    return false;
  }

  CMediaImport import(path, source, mediaTypes);

  if (!AddImport(import))
  {
    CLog::Log(LOGERROR, "CMediaImportManager: failed to add new import %s to any import repository", path.c_str());
    return false;
  }

  Import(import);
  return true;
}

void CMediaImportManager::Import(const CMediaImport &import)
{
  CMediaImportTaskProcessorJob *processorJob = new CMediaImportTaskProcessorJob(import.GetPath(), this);

  CMediaImportRetrievalTask *retrievalTask = new CMediaImportRetrievalTask(import);
  processorJob->SetTask(retrievalTask);
  AddTaskProcessorJob(import.GetPath(), processorJob);

  CLog::Log(LOGINFO, "CMediaImportManager: import retrieval task for %s (%s) started", import.GetSource().GetIdentifier().c_str(), import.GetPath().c_str());
}

bool CMediaImportManager::AddSource(const CMediaImportSource &source)
{
  bool added = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->AddSource(source))
      added = true;
  }

  return added;
}

bool CMediaImportManager::FindSource(const std::string &sourceID, CMediaImportSource &source)
{
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->GetSource(sourceID, source))
      return true;
  }

  return false;
}

bool CMediaImportManager::AddImport(const CMediaImport &import)
{
  bool added = false;
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->AddImport(import))
      added = true;
  }

  return added;
}

bool CMediaImportManager::FindImport(const std::string &path, CMediaImport &import)
{
  CSingleLock repositoriesLock(m_importRepositoriesLock);
  // try to add the source to at least one of the repositories
  for (std::set<IMediaImportRepository*>::const_iterator itRepository = m_importRepositories.begin(); itRepository != m_importRepositories.end(); ++itRepository)
  {
    if ((*itRepository)->GetImport(path, import))
      return true;
  }

  return false;
}

void CMediaImportManager::SetImportItemsEnabled(const std::string& sourceID, bool enabled)
{
  if (sourceID.empty())
    return;

  CSingleLock handlersLock(m_importHandlersLock);
  // loop through all available media import handlers and disable the imported items
  for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
    (*itHandler)->SetImportedItemsEnabled(sourceID, enabled);
}

void CMediaImportManager::AddTaskProcessorJob(const std::string &path, CJob *job)
{
  m_jobMap.insert(std::make_pair(path, job));
  AddJob(job);
}

void CMediaImportManager::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  CSingleLock jobsLock(m_jobsLock);
  if (job != NULL)
  {
    CMediaImportTaskProcessorJob *processorJob = static_cast<CMediaImportTaskProcessorJob*>(job);
    if (processorJob != NULL)
      m_jobMap.erase(processorJob->GetPath());
  }

  CJobQueue::OnJobComplete(jobID, success, job);
}

void CMediaImportManager::OnJobProgress(unsigned int jobID, unsigned int progress, unsigned int total, const CJob *job)
{
  if (strcmp(job->GetType(), "MediaImportTaskProcessorJob") == 0)
  {
    const CMediaImportTaskProcessorJob* processorJob = static_cast<const CMediaImportTaskProcessorJob*>(job);
    if (processorJob->GetCurrentTask() != NULL &&
        processorJob->GetCurrentTask()->GetProgressBarHandle() != NULL)
      processorJob->GetCurrentTask()->GetProgressBarHandle()->SetProgress(progress, total);
  }

  CJobQueue::OnJobProgress(jobID, progress, total, job);
}

std::vector<IMediaImportTask*> CMediaImportManager::OnTaskComplete(bool success, IMediaImportTask *task)
{
  std::vector<IMediaImportTask*> nextTasks;
  if (task == NULL)
    return nextTasks;

  std::string taskType = task->GetType();
  if (taskType.compare("MediaImportSourceRegistrationTask") == 0)
  {
    CMediaImportSourceRegistrationTask *registrationTask = static_cast<CMediaImportSourceRegistrationTask*>(task);
    if (registrationTask == NULL)
      return nextTasks;

    const CMediaImportSource &source = registrationTask->GetImportSource();
    m_jobMap.erase(source.GetIdentifier());

    // nothing to do if the import job failed
    if (!success)
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: source registration task for %s failed", source.GetIdentifier().c_str());
      return nextTasks;
    }

    if (!AddSource(source))
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: source %s couldn't be added to any import repository", source.GetIdentifier().c_str());
      return nextTasks;
    }

    m_sources.insert(std::make_pair(source.GetIdentifier(), true));
    CLog::Log(LOGDEBUG, "CMediaImportManager: new source %s added", source.GetIdentifier().c_str());

    bool added = false;
    // try to add the imports to at least one of the repositories
    const std::vector<CMediaImport> &imports = registrationTask->GetImports();
    std::vector<CMediaImport> imported;
    for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
    {
      bool success = AddImport(*itImport);
      if (success)
        imported.push_back(*itImport);

      added |= success;
    }

    if (!added)
    {
      if (!imports.empty())
        CLog::Log(LOGWARNING, "CMediaImportManager: failed to add %zu imports for source %s to any import repository", imports.size(), source.GetIdentifier().c_str());
      return nextTasks;
    }
    
    // start processing all imports of the source
    for (std::vector<CMediaImport>::const_iterator itImport = imported.begin(); itImport != imported.end(); ++itImport)
      nextTasks.push_back(new CMediaImportRetrievalTask(*itImport));
  }
  else if (taskType.compare("MediaImportRetrievalTask") == 0)
  {
    CMediaImportRetrievalTask* retrievalTask = static_cast<CMediaImportRetrievalTask*>(task);
    if (retrievalTask == NULL)
      return nextTasks;

    const IMediaImporter* importer = retrievalTask->GetImporter();
    if (importer == NULL)
      return nextTasks;

    // nothing to do if the import job failed
    if (!success)
    {
      CLog::Log(LOGWARNING, "CMediaImportManager: media retrieval task for %s using %s failed", importer->GetSourceIdentifier().c_str(), importer->GetIdentification());
      return nextTasks;
    }
  
    CLog::Log(LOGINFO, "CMediaImportManager: media retrieval task for %s using %s successfully finished", importer->GetSourceIdentifier().c_str(), importer->GetIdentification());

    CSingleLock handlersLock(m_importHandlersLock);
    // loop through all available media import handlers and handle the imported items
    for (std::vector<IMediaImportHandler*>::const_iterator itHandler = m_importHandlers.begin(); itHandler != m_importHandlers.end(); ++itHandler)
    {
      IMediaImportHandler* handler = *itHandler;
      CFileItemList importedItems;
      // get the list of imported items of a specific media type
      if (!retrievalTask->GetImportedMedia(handler->GetMediaType(), importedItems))
        continue;

      nextTasks.push_back(new CMediaImportSynchronisationTask(retrievalTask->GetImport(), handler->Create(), importedItems));
    }
  }

  return nextTasks;
}
