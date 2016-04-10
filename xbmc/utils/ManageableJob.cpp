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

#include "ManageableJob.h"
#include "FileItem.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"
#include "utils/Variant.h"

static const std::string FileItemPropertyPrefix = "ManageableJob.";
static const std::string FileItemPropertyProgressPercentage = FileItemPropertyPrefix + "ProgressPercentage";

const std::string CManageableJob::FileItemPropertyStatus = FileItemPropertyPrefix + "Status";

ManageableJobStatus ManageableJobStatusFromString(const std::string& status)
{
  if (status == "InProgress")
    return ManageableJobStatus::InProgress;
  if (status == "Completed")
    return ManageableJobStatus::Completed;
  if (status == "Stopped")
    return ManageableJobStatus::Stopped;

  return ManageableJobStatus::Queued;
}

std::string ManageableJobStatusToString(ManageableJobStatus status)
{
  switch (status)
  {
  case ManageableJobStatus::InProgress:
    return "InProgress";

  case ManageableJobStatus::Completed:
    return "Completed";

  case ManageableJobStatus::Stopped:
    return "Stopped";

  case ManageableJobStatus::Queued:
  default:
    return "Queued";
  }
}

CManageableJob::CManageableJob(const std::string& label /* = "" */)
  : CProgressJob()
  , m_id(StringUtils::CreateUUID())
  , m_label(label)
  , m_progress(0LL)
  , m_total(0LL)
  , m_status(ManageableJobStatus::Queued)
{ }

CManageableJob::CManageableJob(const CManageableJob& other, ManageableJobStatus status)
  : CProgressJob(other)
  , m_id(other.m_id)
  , m_label(other.m_label)
  , m_progress(other.m_progress)
  , m_total(other.m_total)
  , m_status(status)
{ }

bool CManageableJob::operator==(const CJob* job) const
{
  if (job == nullptr)
    return false;

  const CManageableJob* manageableJob = dynamic_cast<const CManageableJob*>(job);
  if (manageableJob == nullptr)
    return false;

  return m_id == manageableJob->m_id;
}

bool CManageableJob::ShouldCancel(uint64_t progress, uint64_t total) const
{
  m_status = ManageableJobStatus::InProgress;

  m_progress = progress;
  m_total = total;

  if (m_progress > m_total)
    m_progress = m_total;

  return CProgressJob::ShouldCancel(progress, total);
}

bool CManageableJob::operator<(const CManageableJob* job) const
{
  if (job == nullptr)
    return true;

  return m_status < job->m_status;
}

bool CManageableJob::UpdateFileItem(CFileItem* fileItem) const
{
  if (fileItem == nullptr)
    return false;

  // make sure not to update a different manageable job
  bool changed = false;
  if (!fileItem->GetPath().empty())
  {
    if (fileItem->GetPath() != m_id)
      return false;
  }
  else
  {
    // only set these unchanging properties once
    fileItem->SetPath(m_id);
    fileItem->SetLabel(m_label);
    changed = true;
  }

  uint64_t oldProgressPercentage = fileItem->GetProperty(FileItemPropertyProgressPercentage).asUnsignedInteger();
  uint8_t currentProgressPercentage = GetProgressPercentage();
  if (oldProgressPercentage != currentProgressPercentage)
  {
    fileItem->SetProperty(FileItemPropertyProgressPercentage, { currentProgressPercentage });
    changed = true;
  }

  const std::string oldStatus = fileItem->GetProperty(FileItemPropertyStatus).asString();
  const std::string currentStatus = ManageableJobStatusToString(m_status);
  if (oldStatus != currentStatus)
  {
    fileItem->SetProperty(FileItemPropertyStatus, { currentStatus });
    changed = true;
  }

  return changed;
}

CManageableJob* CManageableJob::Finish(ManageableJobStatus status) const
{
  return Clone(status);
}

CManageableJobQueue::CManageableJobQueue(bool lifo /* = false */, unsigned int jobsAtOnce /* = 1 */, CJob::PRIORITY priority /* = CJob::PRIORITY_LOW */)
  : CJobQueue(lifo, jobsAtOnce, priority)
{ }

CManageableJobQueue::~CManageableJobQueue()
{
  CancelAllJobs();

  m_finishedJobs.clear();
}

void CManageableJobQueue::SetObserver(IManageableJobQueueObserver* observer)
{
  CSingleLock lock(m_criticalObserver);
  m_observer = observer;
}

std::vector<const CManageableJob* const> CManageableJobQueue::GetAllJobs() const
{
  std::vector<const CManageableJob* const> jobs;

  auto activeJobs = GetActiveJobs();
  auto endedJobs = GetEndedJobs();

  for (const auto& job : activeJobs)
    jobs.push_back(job);
  for (const auto& job : endedJobs)
    jobs.push_back(job);

  return jobs;
}

std::vector<const CManageableJob* const> CManageableJobQueue::GetActiveJobs() const
{
  std::vector<const CManageableJob* const> jobs;

  {
    CSingleLock lock(m_criticalJobs);
    for (const auto& job : m_jobs)
      jobs.push_back(job.second);
  }

  return jobs;
}

std::vector<const CManageableJob* const> CManageableJobQueue::GetEndedJobs() const
{
  std::vector<const CManageableJob* const> jobs;

  {
    CSingleLock lock(m_criticalJobs);
    for (const auto& job : m_finishedJobs)
      jobs.push_back(job.second.get());
  }

  return jobs;
}

bool CManageableJobQueue::AddJob(CManageableJob* job)
{
  if (job == nullptr)
    return false;

  {
    CSingleLock lock(m_criticalJobs);
    const auto& managedJob = FindJob(job);
    if (managedJob != m_jobs.cend())
      return false;

    // add the job to the queue
    if (!CJobQueue::AddJob(job))
      return false;

    job->m_status = ManageableJobStatus::Queued;
    m_jobs.insert({ job->GetIdentifier(), job });
  }

  // notify the observer
  OnJobAdded(job);

  return true;
}

void CManageableJobQueue::CancelJob(const std::string& jobIdentifier)
{
  if (jobIdentifier.empty())
    return;

  const CManageableJob* job = nullptr;
  CManageableJob const* endedJob = nullptr;
  {
    CSingleLock lock(m_criticalJobs);
    const auto& it = m_jobs.find(jobIdentifier);
    if (it == m_jobs.cend())
      return;

    job = it->second;
    endedJob = EndJob(job, ManageableJobStatus::Stopped);
    m_jobs.erase(it);
  }

  // cancel the job
  CJobQueue::CancelJob(job);

  // notify the observer
  OnJobEnded(endedJob);
}

void CManageableJobQueue::CancelAllJobs()
{
  std::set<std::string> cancelJobIdentifiers;
  {
    CSingleLock lock(m_criticalJobs);
    for (const auto& job : m_jobs)
      cancelJobIdentifiers.insert(job.second->GetIdentifier());
  }

  // cancel all jobs
  for (const auto& jobIdentifier : cancelJobIdentifiers)
    CancelJob(jobIdentifier);
}

void CManageableJobQueue::RemoveEndedJob(const std::string& jobIdentifier)
{
  if (jobIdentifier.empty())
    return;

  {
    CSingleLock lock(m_criticalJobs);
    const auto& job = m_finishedJobs.find(jobIdentifier);
    if (job == m_finishedJobs.cend())
      return;

    m_finishedJobs.erase(job);
  }

  OnJobRemoved(jobIdentifier);
}

void CManageableJobQueue::ClearEndedJobs()
{
  std::set<std::string> removedJobIdentifiers;
  {
    CSingleLock lock(m_criticalJobs);
    for (const auto& job : m_finishedJobs)
      removedJobIdentifiers.insert(job.second->GetIdentifier());
    m_finishedJobs.clear();
  }

  for (const auto& jobIdentifier : removedJobIdentifiers)
    OnJobRemoved(jobIdentifier);
}

void CManageableJobQueue::OnJobAdded(const CManageableJob* job)
{
  if (job == nullptr)
    return;

  IManageableJobQueueObserver* observer = nullptr;
  {
    CSingleLock lock(m_criticalObserver);
    if (m_observer == nullptr)
      return;

    observer = m_observer;
  }

  observer->OnJobAdded(job);
}

void CManageableJobQueue::OnJobProgress(const CManageableJob* job)
{
  if (job == nullptr)
    return;

  IManageableJobQueueObserver* observer = nullptr;
  {
    CSingleLock lock(m_criticalObserver);
    if (m_observer == nullptr)
      return;

    observer = m_observer;
  }

  observer->OnJobProgress(job);
}

void CManageableJobQueue::OnJobEnded(const CManageableJob* job)
{
  if (job == nullptr)
    return;

  IManageableJobQueueObserver* observer = nullptr;
  {
    CSingleLock lock(m_criticalObserver);
    if (m_observer == nullptr)
      return;

    observer = m_observer;
  }

  observer->OnJobEnded(job);
}

void CManageableJobQueue::OnJobRemoved(const std::string& jobIdentifier)
{
  if (jobIdentifier.empty())
    return;

  IManageableJobQueueObserver* observer = nullptr;
  {
    CSingleLock lock(m_criticalObserver);
    if (m_observer == nullptr)
      return;

    observer = m_observer;
  }

  observer->OnJobRemoved(jobIdentifier);
}

void CManageableJobQueue::OnJobProgress(unsigned int jobID, uint64_t progress, uint64_t total, const CJob *job)
{
  const CManageableJob* manageableJob = dynamic_cast<const CManageableJob*>(job);
  if (manageableJob == nullptr)
    return;

  {
    CSingleLock lock(m_criticalJobs);
    const auto& managedJob = FindJob(manageableJob);
    if (managedJob == m_jobs.cend())
      return;
  }

  // notify the observer
  OnJobProgress(manageableJob);
}

void CManageableJobQueue::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  const CManageableJob* manageableJob = dynamic_cast<const CManageableJob*>(job);
  if (manageableJob == nullptr)
    return;

  CManageableJob const* endedJob = nullptr;
  {
    CSingleLock lock(m_criticalJobs);
    const auto& managedJob = FindJob(manageableJob);
    if (managedJob == m_jobs.cend())
      return;

    endedJob = EndJob(manageableJob, success ? ManageableJobStatus::Completed : ManageableJobStatus::Stopped);
    m_jobs.erase(managedJob);
  }

  OnJobEnded(endedJob);
}

CManageableJobQueue::ManagedJobs::const_iterator CManageableJobQueue::FindJob(const CManageableJob* job)
{
  return m_jobs.find(job->GetIdentifier());
}

const CManageableJob* CManageableJobQueue::EndJob(const CManageableJob* job, ManageableJobStatus status)
{
  // create a copy of the job
  std::shared_ptr<const CManageableJob> endedJob(job->Finish(status));
  m_finishedJobs.insert({ endedJob->GetIdentifier(), endedJob });

  return endedJob.get();
}
