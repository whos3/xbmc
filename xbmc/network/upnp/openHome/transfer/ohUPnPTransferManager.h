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
#include <map>
#include <string>

#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferJob.h"
#include "threads/CriticalSection.h"
#include "utils/JobManager.h"

class IOhUPnPTransferCallbacks;

class COhUPnPTransferManager : protected CJobQueue
{
public:
  COhUPnPTransferManager();
  virtual ~COhUPnPTransferManager();

  bool Import(const COhUPnPDevice& sourceDevice, const std::string& sourceUri, const CFileItem& item, IOhUPnPTransferCallbacks* callback, uint32_t& transferId);
  bool Export(uint32_t transferId, const COhUPnPDevice& destinationDevice, const std::string& sourceUri, const CFileItem& item, IOhUPnPTransferCallbacks* callback);

  bool HasUnfinishedTransfers() const;
  bool GetTransferProgress(uint32_t transferId, ohUPnPTransferStatus& status, uint64_t& progress, uint64_t& total) const;

  bool StopTransfer(uint32_t transferId);
  void StopImportTransfers();
  void StopExportTransfers();
  void StopAllTransfers();

protected:
  // specialization of CJobQueue
  virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job) override;

private:
  using TransferMap = std::map<uint32_t, COhUPnPTransferJob*>;
  TransferMap m_importTransfers;
  TransferMap m_exportTransfers;
  TransferMap m_finishedTransfers;
  CCriticalSection m_transferLock;
};