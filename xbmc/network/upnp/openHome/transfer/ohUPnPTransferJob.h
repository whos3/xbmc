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
#include <string>

#include "FileItem.h"
#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferStatus.h"
#include "utils/ProgressJob.h"
#include "utils/TransferSpeed.h"

class IOhUPnPTransferCallbacks;

class COhUPnPTransferJob : public CProgressJob
{
public:
  COhUPnPTransferJob(const COhUPnPTransferJob& other);
  virtual ~COhUPnPTransferJob();

  inline uint32_t GetId() const { return m_id; }
  inline uint64_t GetProgress() const { return m_progress; }
  inline uint64_t GetTotal() const { return m_total; }
  inline ohUPnPTransferStatus GetStatus() const { return m_status; }
  inline IOhUPnPTransferCallbacks* GetCallback() const { return m_callback; }

  double GetTransferSpeed() const;
  uint64_t GetRemainingSeconds() const;

  virtual bool IsValid() const = 0;

  // specializations of CProgressJob
  virtual const char *GetType() const override { return "UPnPTransferJob"; };
  virtual bool operator==(const CJob* job) const override;

protected:
  COhUPnPTransferJob(const COhUPnPDevice& device, const std::string& sourceUri,
    const CFileItem& item, IOhUPnPTransferCallbacks* callback);
  COhUPnPTransferJob(uint32_t id, const COhUPnPDevice& device, const std::string& sourceUri,
    const CFileItem& item, IOhUPnPTransferCallbacks* callback);

  // specializations of CProgressJob
  virtual bool ShouldCancel(uint64_t progress, uint64_t total) const override;

  // implementations of CJob
  virtual bool DoWork() override;

  virtual bool Transfer() = 0;

  uint32_t m_id;
  COhUPnPDevice m_device;
  std::string m_sourceUri;

  CFileItem m_item;
  std::string m_label;

  mutable uint64_t m_progress;
  mutable uint64_t m_total;
  mutable CTransferSpeed<>* m_speed;
  mutable ohUPnPTransferStatus m_status;

  IOhUPnPTransferCallbacks* m_callback;

  static uint32_t s_nextId;
};

class COhUPnPImportTransferJob : public COhUPnPTransferJob
{
public:
  COhUPnPImportTransferJob(const COhUPnPDevice& device, const std::string& sourceUri,
    const CFileItem& item, IOhUPnPTransferCallbacks* callback);
  COhUPnPImportTransferJob(const COhUPnPTransferJob& other);
  virtual ~COhUPnPImportTransferJob() = default;

  virtual bool IsValid() const override;

protected:
  // implementation of COhUPnPTransferJob
  virtual bool Transfer() override;

private:
  std::string m_destinationDirectory;
};

class COhUPnPExportTransferJob : public COhUPnPTransferJob
{
public:
  COhUPnPExportTransferJob(uint32_t id, const COhUPnPDevice& device, const std::string& sourceUri,
    const CFileItem& item, IOhUPnPTransferCallbacks* callback);
  COhUPnPExportTransferJob(const COhUPnPTransferJob& other);
  virtual ~COhUPnPExportTransferJob() = default;

  virtual bool IsValid() const override { return true; }

protected:
  // implementation of COhUPnPTransferJob
  virtual bool Transfer() override;
};