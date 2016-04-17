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
#include <map>
#include <string>

#include <OpenHome/Net/Cpp/DvUpnpOrgContentDirectory3.h>

#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/ohUPnPService.h"
#include "network/upnp/openHome/rootdevices/ohUPnPRootDevice.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferCallbacks.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferManager.h"
#include "threads/CriticalSection.h"

class CFileItemElementFactory;
class COhUPnPRootDevice;
class CUPnPItem;

class COhUPnPContentDirectoryService : public IOhUPnPService
{
public:
  COhUPnPContentDirectoryService(COhUPnPRootDevice& device,
    const CFileItemElementFactory& fileItemElementFactory,
    COhUPnPTransferManager& transferManager,
    COhUPnPResourceManager& resourceManager);
  virtual ~COhUPnPContentDirectoryService();

  void Start(bool supportImporting);
  void Stop();
  bool IsRunning() const;

private:
  class ContentDirectory : public IOhUPnPTransferCallbacks, public OpenHome::Net::DvProviderUpnpOrgContentDirectory3Cpp
  {
  public:
    ContentDirectory(COhUPnPContentDirectoryService& service, OpenHome::Net::DvDeviceStd& device, bool supportImporting);
    virtual ~ContentDirectory();

  protected:
    void GetSearchCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& searchCaps) override;
    void GetSortCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& sortCaps) override;
    void GetSortExtensionCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& sortExtensionCaps) override;
    void GetFeatureList(OpenHome::Net::IDvInvocationStd& invocation, std::string& featureList) override;
    void GetSystemUpdateID(OpenHome::Net::IDvInvocationStd& invocation, uint32_t& id) override;
    void GetServiceResetToken(OpenHome::Net::IDvInvocationStd& invocation, std::string& resetToken) override;
    void Browse(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& browseFlag,
      const std::string& filter, uint32_t startingIndex, uint32_t requestedCount, const std::string& sortCriteria,
      std::string& result, uint32_t& numberReturned, uint32_t& totalMatches, uint32_t& updateID) override;
    void Search(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& searchCriteria,
      const std::string& filter, uint32_t startingIndex, uint32_t requestedCount, const std::string& sortCriteria,
      std::string& result, uint32_t& numberReturned, uint32_t& totalMatches, uint32_t& updateID) override;
    void CreateObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& elements,
      std::string& objectID, std::string& result) override;
    void DestroyObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID) override;
    void UpdateObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& currentTagValue, const std::string& newTagValue) override;
    void MoveObject(OpenHome::Net::IDvInvocationStd& invocation, const std::string& objectID, const std::string& newParentID, std::string& newObjectID) override;
    void ImportResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& sourceURI, const std::string& destinationURI, uint32_t& transferID) override;
    void ExportResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& sourceURI, const std::string& destinationURI, uint32_t& transferID) override;
    void DeleteResource(OpenHome::Net::IDvInvocationStd& invocation, const std::string& resourceURI) override;
    void StopTransferResource(OpenHome::Net::IDvInvocationStd& invocation, uint32_t transferID) override;
    void GetTransferProgress(OpenHome::Net::IDvInvocationStd& invocation, uint32_t transferID, std::string& transferStatus, std::string& transferLength, std::string& transferTotal) override;
    void CreateReference(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, const std::string& objectID, std::string& newID) override;
    void FreeFormQuery(OpenHome::Net::IDvInvocationStd& invocation, const std::string& containerID, uint32_t CDSView, const std::string& queryRequest, std::string& queryResult, uint32_t& updateID) override;
    void GetFreeFormQueryCapabilities(OpenHome::Net::IDvInvocationStd& invocation, std::string& FFQCapabilities) override;

    // implementations of IOhUPnPTransferCallbacks
    virtual void OnTransferCompleted(uint32_t transferId) override;
    virtual void OnTransferFailed(uint32_t transferId) override;

    void RemoveTransfer(uint32_t transferID);

  private:
    COhUPnPContentDirectoryService& m_service;

    std::map<std::string, std::shared_ptr<CUPnPItem>> m_createdObjects;
    std::map<uint32_t, std::string> m_createdObjectTransfers;
    uint64_t m_createdObjectID;
    CCriticalSection m_criticalCreatedObjects;
  };

  friend class ContentDirectory;

  COhUPnPRootDevice& m_device;
  std::unique_ptr<ContentDirectory> m_contentDirectory;

  COhUPnPTransferManager& m_transferManager;
  COhUPnPResourceManager& m_resourceManager;
  const CFileItemElementFactory& m_elementFactory;
};
