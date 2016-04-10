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

#include "ohUPnPTransferManager.h"
#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferCallbacks.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferJob.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

COhUPnPTransferManager::COhUPnPTransferManager()
  : CManageableJobQueue(false, 2, CJob::PRIORITY_LOW) // TODO: 1 instead of 2
{ }

COhUPnPTransferManager::~COhUPnPTransferManager()
{
  StopAllTransfers();

  for (const auto& transfer : m_finishedTransfers)
    delete transfer.second;
  m_finishedTransfers.clear();
}

bool COhUPnPTransferManager::Import(const COhUPnPDevice& sourceDevice, const std::string& sourceUri, const CFileItem& item, IOhUPnPTransferCallbacks* callback, uint32_t& transferId)
{
  if (!sourceDevice.IsValid() || sourceUri.empty() || item.GetLabel().empty())
    return false;

  CSingleLock lock(m_transferLock);
  auto transfer = new COhUPnPImportTransferJob(sourceDevice, sourceUri, item, callback);
  if (!transfer->IsValid() || !AddJob(transfer))
  {
    delete transfer;
    return false;
  }
  
  transferId = transfer->GetTransferId();
  m_importTransfers.insert(TransferMap::value_type(transferId, transfer));

  return true;
}

bool COhUPnPTransferManager::Export(uint32_t transferId, const COhUPnPDevice& destinationDevice, const std::string& sourceUri, const CFileItem& item, IOhUPnPTransferCallbacks* callback)
{
  if (!destinationDevice.IsValid() || sourceUri.empty() || item.GetLabel().empty())
    return false;

  CSingleLock lock(m_transferLock);
  if (m_exportTransfers.find(transferId) != m_exportTransfers.end())
    return false;

  auto transfer = new COhUPnPExportTransferJob(transferId, destinationDevice, sourceUri, item, callback);
  if (!AddJob(transfer))
  {
    delete transfer;
    return false;
  }
  m_exportTransfers.insert(TransferMap::value_type(transferId, transfer));

  return true;
}

bool COhUPnPTransferManager::HasUnfinishedTransfers() const
{
  CSingleLock lock(m_transferLock);
  return !m_importTransfers.empty() || !m_exportTransfers.empty();
}

bool COhUPnPTransferManager::GetTransferProgress(uint32_t transferId, ohUPnPTransferStatus& status, uint64_t& progress, uint64_t& total) const
{
  CSingleLock lock(m_transferLock);
  auto& transfer = m_importTransfers.find(transferId);
  if (transfer == m_importTransfers.cend())
  {
    transfer = m_exportTransfers.find(transferId);

    if (transfer == m_exportTransfers.cend())
    {
      transfer = m_finishedTransfers.find(transferId);

      if (transfer == m_finishedTransfers.cend())
        return false;
    }
  }

  status = transfer->second->GetTransferStatus();
  progress = transfer->second->GetProgress();
  total = transfer->second->GetTotal();

  return true;
}

bool COhUPnPTransferManager::StopTransfer(uint32_t transferId)
{
  CSingleLock lock(m_transferLock);
  auto& transfer = m_importTransfers.find(transferId);
  if (transfer == m_importTransfers.cend())
    return false;

  CancelJob(transfer->second->GetIdentifier());
  return true;
}

void COhUPnPTransferManager::StopImportTransfers()
{
  TransferMap transfers;
  {
    CSingleLock lock(m_transferLock);
    transfers = m_importTransfers;
  }

  for (const auto& transfer : transfers)
    CancelJob(transfer.second->GetIdentifier());
}

void COhUPnPTransferManager::StopExportTransfers()
{
  TransferMap transfers;
  {
    CSingleLock lock(m_transferLock);
    transfers = m_exportTransfers;
  }

  for (const auto& transfer : transfers)
    CancelJob(transfer.second->GetIdentifier());
}

void COhUPnPTransferManager::StopAllTransfers()
{
  CancelJobs();
}

void COhUPnPTransferManager::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
  auto transferJob = dynamic_cast<COhUPnPTransferJob*>(job);
  if (transferJob == nullptr)
    return;

  const auto& transferId = transferJob->GetTransferId();
  auto callback = transferJob->GetCallback();
  if (callback != nullptr)
  {
    if (success)
      callback->OnTransferCompleted(transferId);
    else
      callback->OnTransferFailed(transferId);
  }

  COhUPnPTransferJob* finishedTransfer = nullptr;
  {
    CSingleLock lock(m_transferLock);
    auto& transfer = m_importTransfers.find(transferId);
    if (transfer != m_importTransfers.cend())
    {
      m_importTransfers.erase(transfer);

      finishedTransfer = new COhUPnPImportTransferJob(*transferJob);
    }
    else
    {
      transfer = m_exportTransfers.find(transferId);
      if (transfer != m_exportTransfers.cend())
      {
        m_exportTransfers.erase(transfer);

        finishedTransfer = new COhUPnPExportTransferJob(*transferJob);
      }
    }

    // remember the finished transfers
    m_finishedTransfers.insert(TransferMap::value_type(transferId, finishedTransfer));
  }

  CManageableJobQueue::OnJobComplete(jobID, success, job);
}
