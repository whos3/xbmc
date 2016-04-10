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

#include "threads/CriticalSection.h"
#include "utils/ProgressJob.h"
#include "utils/JobManager.h"

class CFileItem;

enum class ManageableJobStatus
{
  InProgress,
  Queued,
  Completed,
  Stopped
};

ManageableJobStatus ManageableJobStatusFromString(const std::string& status);
std::string ManageableJobStatusToString(ManageableJobStatus status);

class CManageableJob : public CProgressJob
{
public:
  virtual ~CManageableJob() = default;

  // implementation of CJob
  virtual const char *GetType() const override { return "ManageableJob"; }
  virtual bool operator==(const CJob* job) const override;
  virtual bool ShouldCancel(uint64_t progress, uint64_t total) const override;

  virtual bool operator<(const CManageableJob* job) const;

  inline const std::string GetIdentifier() const { return m_id; }
  inline const std::string GetLabel() const { return m_label; }

  inline uint64_t GetProgress() const { return m_progress; }
  inline uint64_t GetTotal() const { return m_total; }
  inline uint8_t GetProgressPercentage() const { return static_cast<uint8_t>((100.0 * m_progress) / m_total); }

  inline ManageableJobStatus GetStatus() const { return m_status; }

  virtual bool UpdateFileItem(CFileItem* fileItem) const;

  CManageableJob* Finish(ManageableJobStatus status) const;

  static const std::string FileItemPropertyStatus;

protected:
  CManageableJob(const std::string& label = "");
  CManageableJob(const CManageableJob& other) = default;
  CManageableJob(const CManageableJob& other, ManageableJobStatus status);

  void SetLabel(const std::string& label) { m_label = label; }

  virtual CManageableJob* Clone(ManageableJobStatus status) const = 0;

private:
  friend class CManageableJobQueue;

  const std::string m_id;
  std::string m_label;

  mutable uint64_t m_progress;
  mutable uint64_t m_total;
  mutable ManageableJobStatus m_status;
};

class IManageableJobQueueObserver
{
public:
  virtual ~IManageableJobQueueObserver() = default;

  virtual void OnJobAdded(const CManageableJob* job) { }
  virtual void OnJobProgress(const CManageableJob* job) { }
  virtual void OnJobEnded(const CManageableJob* job) { }
  virtual void OnJobRemoved(const std::string& jobIdentifier) { }

protected:
  IManageableJobQueueObserver() = default;
};

class CManageableJobQueue : protected CJobQueue, protected IManageableJobQueueObserver
{
public:
  virtual ~CManageableJobQueue();

  void SetObserver(IManageableJobQueueObserver* observer);

  inline bool IsProcessingJobs() const { return CJobQueue::IsProcessing(); }
  inline size_t GetActiveJobCount() const { return m_jobs.size(); }
  inline size_t GetEndedJobCount() const { return m_finishedJobs.size(); }
  inline size_t GetJobCount() const { return m_jobs.size() + m_finishedJobs.size(); }

  std::vector<const CManageableJob* const> GetAllJobs() const;
  std::vector<const CManageableJob* const> GetActiveJobs() const;
  std::vector<const CManageableJob* const> GetEndedJobs() const;

  bool AddJob(CManageableJob* job);
  void CancelJob(const std::string& jobIdentifier);
  void CancelAllJobs();

  void RemoveEndedJob(const std::string& jobIdentifier);
  void ClearEndedJobs();

protected:
  CManageableJobQueue(bool lifo = false, unsigned int jobsAtOnce = 1, CJob::PRIORITY priority = CJob::PRIORITY_LOW);

  // implementations of IManageableJobQueueObserver
  virtual void OnJobAdded(const CManageableJob* job) override;
  virtual void OnJobProgress(const CManageableJob* job) override;
  virtual void OnJobEnded(const CManageableJob* job) override;
  virtual void OnJobRemoved(const std::string& jobIdentifier) override;

  // specialization of IJobCallback
  virtual void OnJobProgress(unsigned int jobID, uint64_t progress, uint64_t total, const CJob *job) override;

  // specialization of CJobQueue
  virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job) override;

private:
  using ManagedJobs = std::map<std::string, CManageableJob*>;

  ManagedJobs::const_iterator FindJob(const CManageableJob* job);
  const CManageableJob* EndJob(const CManageableJob* job, ManageableJobStatus status);

  ManagedJobs m_jobs;
  std::map<std::string, std::shared_ptr<const CManageableJob>> m_finishedJobs;
  CCriticalSection m_criticalJobs;

  IManageableJobQueueObserver* m_observer;
  CCriticalSection m_criticalObserver;
};
