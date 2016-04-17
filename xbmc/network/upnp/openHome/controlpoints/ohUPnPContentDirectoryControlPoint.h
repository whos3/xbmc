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

#include <OpenHome/Net/Cpp/CpUpnpOrgContentDirectory3.h>

#include "network/upnp/openHome/controlpoints/ohUPnPControlPoint.h"
#include "network/upnp/openHome/transfer/ohUPnPTransferStatus.h"

class CDidlLiteDocument;
class CFileItem;
class CFileItemElementFactory;
class CFileItemList;
class COhUPnPResourceManager;

class COhUPnPContentDirectoryControlPoint : public IOhUPnPControlPoint<OpenHome::Net::CpProxyUpnpOrgContentDirectory3Cpp>
{
public:
  COhUPnPContentDirectoryControlPoint(const CFileItemElementFactory& fileItemElementFactory, COhUPnPResourceManager& resourceManager);
  virtual ~COhUPnPContentDirectoryControlPoint();

  const COhUPnPDevice& GetLocalDevice() const { return m_localDevice; }

  bool SupportsSearch(const std::string& uuid);
  bool SupportsCreateObject(const std::string& uuid);
  bool SupportsImportResource(const std::string& uuid);

  bool BrowseSync(const std::string& uuid, const std::string& objectId, CFileItemList& items,
                  uint32_t start = 0, uint32_t count = 0, const std::string& filter = "", const std::string& sorting = "") const;
  bool BrowseMetadataSync(const std::string& uuid, const std::string& objectId, CFileItem& item, const std::string& filter = "") const;

  bool CreateObject(const std::string& uuid, const std::string& containerId, const CFileItem& item, std::string& objectId, std::string& importUri) const;

  bool ImportResource(const std::string& uuid, const std::string& sourceUri, const std::string& destinationUri, uint32_t& transferId) const;
  bool StopTransferResource(const std::string& uuid, uint32_t transferId) const;
  bool GetTransferProgress(const std::string& uuid, uint32_t transferId, ohUPnPTransferStatus& status, uint64_t& length, uint64_t& total) const;

protected:
  // specialization of IOhUPnPControlPoint
  void OnServiceAdded(const UPnPControlPointService &service) override;
  void OnServiceRemoved(const UPnPControlPointService &service) override;

  void updateUPnPPath();

  bool browseSync(const UPnPControlPointService& service, const std::string& objectId, bool metadata, std::string& result, uint32_t& resultCount, uint32_t& resultTotal,
                  uint32_t start, uint32_t count, const std::string& filter, const std::string& sorting) const;

  bool resultToFileItemList(const UPnPControlPointService& service, const std::string& result, uint32_t resultCount, uint32_t resultTotal, CFileItemList& items) const;

private:
  COhUPnPControlPointDevice m_localDevice;
  const CFileItemElementFactory& m_elementFactory;
  COhUPnPResourceManager& m_resourceManager;
};
