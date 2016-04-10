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

#include <algorithm>

#include <openHome/Net/Cpp/CpProxy.h>

#include "ohUPnPContentDirectoryControlPoint.h"
#include "FileItem.h"
#include "URL.h"
#include "media/MediaType.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPResourceManager.h"
#include "network/upnp/openHome/didllite/DidlLiteDocument.h"
#include "network/upnp/openHome/didllite/IDidlLiteElement.h"
#include "network/upnp/openHome/didllite/objects/FileItemElementFactory.h"
#include "network/upnp/openHome/didllite/objects/FileItemUtils.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "network/upnp/openHome/didllite/objects/item/UPnPItem.h"
#include "network/upnp/openHome/didllite/objects/classmappers/SimpleUPnPClassMapper.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPResource.h"
#include "network/upnp/openHome/utils/ohUtils.h"
#include "settings/AdvancedSettings.h"
#include "utils/PerformanceMeasurement.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"

static bool IsUPnPActionSupported(uint32_t errorCode)
{
  return errorCode != UPNP_ERROR_INVALID_ACTION && errorCode != UPNP_ERROR_ACTION_NOT_IMPLEMENTED && errorCode != UPNP_ERROR_ACTION_FAILED;
}

class CResourcePrioritySorter
{
public:
  explicit CResourcePrioritySorter(const IFileItemElement* item)
  {
    const auto type = item->GetType();
    if (StringUtils::StartsWith(type, "object.item.audioItem"))
      m_content = "audio";
    else if (StringUtils::StartsWith(type, "object.item.imageItem"))
      m_content = "image";
    else if (StringUtils::StartsWith(type, "object.item.videoItem"))
      m_content = "video";
  }

  int GetPriority(const CUPnPResource* res) const
  {
    const CUPnPResource::CProtocolInfo& protocolInfo = res->GetProtocolInfo();
    int prio = 0;

    if (!m_content.empty() && StringUtils::StartsWith(protocolInfo.GetContentType(), m_content))
      prio += 400;

    CURL url(res->GetUri());
    if (URIUtils::IsHostOnLAN(url.GetHostName(), false))
      prio += 300;

    if (protocolInfo.GetProtocol() == "xbmc-get")
      prio += 200;
    else if (protocolInfo.GetProtocol() == "http-get")
      prio += 100;

    return prio;
  }

  bool operator()(const CUPnPResource* lh, const CUPnPResource* rh) const
  {
    return GetPriority(lh) < GetPriority(rh);
  }

private:
  std::string m_content;
};

COhUPnPContentDirectoryControlPoint::COhUPnPContentDirectoryControlPoint(const CFileItemElementFactory& fileItemElementFactory, COhUPnPResourceManager& resourceManager)
  : IOhUPnPControlPoint<OpenHome::Net::CpProxyUpnpOrgContentDirectory3Cpp>(UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_CONTENTDIRECTORY, 1),
    m_localDevice(CSysInfo::GetDeviceName()),
    m_elementFactory(fileItemElementFactory),
    m_resourceManager(resourceManager)
{ }

COhUPnPContentDirectoryControlPoint::~COhUPnPContentDirectoryControlPoint()
{ }

bool COhUPnPContentDirectoryControlPoint::SupportsSearch(const std::string& uuid)
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to check support for Search() on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  if (service.device.SupportsCreateObject() == COhUPnPControlPointDevice::SupportsMethod::Unknown)
  {
    bool supported = true;
    try
    {
      std::string result;
      uint32_t returned, total, updateId;
      service.service->SyncSearch("", "", "", 0, 0, "", result, returned, total, updateId);
    }
    catch (OpenHome::Net::ProxyError& ex)
    {
      supported = IsUPnPActionSupported(ex.Code());
    }
    catch (OpenHome::Exception&)
    {
    }

    service.device.SetSupportsCreateObject(supported);
  }

  return service.device.SupportsCreateObject() == COhUPnPControlPointDevice::SupportsMethod::Yes;
}

bool COhUPnPContentDirectoryControlPoint::SupportsCreateObject(const std::string& uuid)
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to check support for CreateObject() on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  if (service.device.SupportsCreateObject() == COhUPnPControlPointDevice::SupportsMethod::Unknown)
  {
    bool supported = true;
    try
    {
      std::string objectId, result;
      service.service->SyncCreateObject("", "", objectId, result);
    }
    catch (OpenHome::Net::ProxyError& ex)
    {
      supported = IsUPnPActionSupported(ex.Code());
    }
    catch (OpenHome::Exception&)
    { }

    service.device.SetSupportsCreateObject(supported);
  }

  return service.device.SupportsCreateObject() == COhUPnPControlPointDevice::SupportsMethod::Yes;
}

bool COhUPnPContentDirectoryControlPoint::SupportsImportResource(const std::string& uuid)
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to check support for ImportResource() on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  if (service.device.SupportsImportResource() == COhUPnPControlPointDevice::SupportsMethod::Unknown)
  {
    bool supported = true;
    try
    {
      uint32_t transferId;
      service.service->SyncImportResource("", "", transferId);
    }
    catch (OpenHome::Net::ProxyError& ex)
    {
      supported = IsUPnPActionSupported(ex.Code());
    }
    catch (OpenHome::Exception&)
    { }

    service.device.SetSupportsImportResource(supported);
  }

  return service.device.SupportsImportResource() == COhUPnPControlPointDevice::SupportsMethod::Yes;
}

bool COhUPnPContentDirectoryControlPoint::BrowseSync(const std::string& uuid, const std::string& objectId, CFileItemList& items,
                                                     uint32_t start /* = 0 */, uint32_t count /* = 0 */,
                                                     const std::string& filter /* = "" */, const std::string& sorting /* = "" */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to browse an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  uint32_t currentStart = start;
  uint32_t total = count;
  // TODO
  if (count == 0)
    count = 200;
  else
    count = std::min(200u, count);

  do {
    std::string result;
    uint32_t resultCount, resultTotal;

    {
      CPerformanceMeasurement<> browsePerformance;

      if (!browseSync(service, objectId, false, result, resultCount, resultTotal, currentStart, count, filter, sorting))
        return false;

      browsePerformance.Stop();
      CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryControlPoint: [Performance] Browse(): %fs", browsePerformance.GetDurationInSeconds());
    }

    if (total <= 0 && resultTotal > 0)
      total = resultTotal;
    currentStart += resultCount;

    CFileItemList tmpItems;
    if (!resultToFileItemList(service, result, resultCount, resultTotal, tmpItems))
      return false;

    items.Append(tmpItems);
    // TODO: handle the case where we have to make multiple requests which results in multiple content type evaluations
    items.SetContent(tmpItems.GetContent());
  } while (static_cast<uint32_t>(items.Size()) < total);

  items.SetProperty("total", total);
  return true;
}

bool COhUPnPContentDirectoryControlPoint::BrowseMetadataSync(const std::string& uuid, const std::string& objectId, CFileItem& item, const std::string& filter /* = "" */) const
{
  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to browse metadata on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  std::string result;
  uint32_t resultCount, resultTotal;
  if (!browseSync(service, objectId, true, result, resultCount, resultTotal, 0, 1, filter, "") ||
      resultCount != 1 || resultTotal != resultCount || result.empty())
    return false;

  // parse and deserialize the result as a DIDL-Lite document
  OhUPnPControlPointContext context{ service.device, service.profile };
  CDidlLiteDocument doc(m_elementFactory);
  if (!doc.Deserialize(result, context))
    return false;

  auto elements = doc.GetElements();
  if (elements.size() != 1)
    return false;

  auto element = elements.front();
  const CUPnPItem* upnpItem = dynamic_cast<const CUPnPItem*>(element);
  if (upnpItem == nullptr)
    return false;

  if (!upnpItem->ToFileItem(item, context))
    return false;

  // store original path so we remember it
  item.SetProperty("original_listitem_url", item.GetPath());
  item.SetProperty("original_listitem_mime", item.GetMimeType());

  // get all available resources
  auto resources = upnpItem->GetResources();
  if (resources.empty())
    return false;

  // sort the resources
  // TODO: make configurable (per profile or globally)?
  std::stable_sort(resources.begin(), resources.end(), CResourcePrioritySorter(upnpItem));

  // grab the best resource from the end of the sorted resources
  const auto bestResource = resources.back();

  // update the item's path to the resource's URI
  item.SetPath(bestResource->GetUri());

  // update the item's MIME type
  if (!bestResource->GetProtocolInfo().GetContentType().empty())
    item.SetMimeType(bestResource->GetProtocolInfo().GetContentType());

  // look for subtitles
  std::vector<const CUPnPResource*> subtitleResources;
  std::copy_if(resources.begin(), resources.end(), std::back_inserter(subtitleResources), CUPnPResourceFinder::ByContentType("text/")); // TODO: "text/srt", "text/ssa", "text/sub", "text/idx"

  uint32_t subsIndex = 1;
  for (const auto& subtitle : subtitleResources)
  {
    std::string subtitleProperty = StringUtils::Format("subtitle:%u", subsIndex++);
    item.SetProperty(subtitleProperty, subtitle->GetUri());
  }

  return true;
}

bool COhUPnPContentDirectoryControlPoint::CreateObject(const std::string& uuid, const std::string& containerId, const CFileItem& item, std::string& objectId, std::string& importUri) const
{
  if (uuid.empty() || containerId.empty() || item.GetPath().empty())
    return false;

  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to create object on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  // serialize the item into DIDL-Lite
  IFileItemElement* element = m_elementFactory.GetElement(item);
  if (element == nullptr)
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to create object for an unknown item");
    return false;
  }

  CUPnPItem* upnpItem = dynamic_cast<CUPnPItem*>(element);
  if (upnpItem == nullptr)
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to create object for an unsupported item");
    delete element;
    return false;
  }

  OhUPnPRootDeviceContext serializationContext = { service.device, service.profile, COhUPnP::GetInstance().GetResourceUriPrefix() };

  // fill the available properties from the item
  upnpItem->FromFileItem(item, serializationContext);

  // set the necessary properties of the object/item to be created
  upnpItem->SetId("");
  upnpItem->SetParentId(containerId);
  upnpItem->SetRestricted(false);
  upnpItem->SetTitle(item.GetLabel());

  // remove any resources
  // TODO: is this necessary?
  upnpItem->SetResources({ });

  // prepare the DIDL-Lite document for serialization of the created object/item
  CDidlLiteDocument elementsDoc(m_elementFactory);
  elementsDoc.AddNamespace(UPNP_DIDL_LITE_NAMESPACE_URI);
  elementsDoc.AddNamespace(UPNP_DIDL_DC_NAMESPACE, UPNP_DIDL_DC_NAMESPACE_URI);
  elementsDoc.AddNamespace(UPNP_DIDL_UPNP_NAMESPACE, UPNP_DIDL_UPNP_NAMESPACE_URI);

  // add the created object/item
  elementsDoc.AddElement(upnpItem);

  // serialize the created object/item
  std::string elements;
  if (!elementsDoc.Serialize(elements, serializationContext))
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: failed to serialize DIDL-Lite document for object to be created");
    return false;
  }

  std::string result;
  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> CreateObject(%s, %s)", containerId.c_str(), elements.c_str());

    service.service->SyncCreateObject(containerId, elements, objectId, result);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- CreateObject(%s, %s): object ID = %s", containerId.c_str(), elements.c_str(), objectId.c_str());
      CLog::Log(LOGDEBUG, "[ohNet] <-- CreateObject(%s, %s): result =\n%s", containerId.c_str(), elements.c_str(), result.c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: error %u calling SyncCreateObject(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: exception calling SyncCreateObject(): %s", ex.Message());
    return false;
  }

  // deserialize the result
  OhUPnPControlPointContext deserializationContext = { service.device, service.profile };

  // parse and deserialize the result as a DIDL-Lite document
  CDidlLiteDocument resultDoc(m_elementFactory);
  if (!resultDoc.Deserialize(result, deserializationContext))
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: failed to deserialize DIDL-Lite document for created object");
    return false;
  }

  const auto& items = resultDoc.GetElements();
  if (items.size() != 1)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: invalid number of created objects");
    return false;
  }

  const auto& resultItem = items.front();
  const CUPnPItem* upnpResultItem = dynamic_cast<const CUPnPItem*>(resultItem);
  if (upnpResultItem == nullptr)
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryControlPoint: unknown created object <%s>",
      DidlLiteUtils::GetElementName(resultItem->GetElementNamespace(), resultItem->GetElementName()).c_str());
    return false;
  }

  const auto& resources = upnpResultItem->GetResources();
  if (resources.size() != 1)
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryControlPoint: invalid number of <res> tags in created object");
    return false;
  }

  const auto& resource = resources.front();
  importUri = resource->GetImportUri();

  if (importUri.empty())
  {
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryControlPoint: missing importUri attribute in <res> property of created object");
    return false;
  }

  return true;
}

bool COhUPnPContentDirectoryControlPoint::ImportResource(const std::string& uuid, const std::string& sourceUri, const std::string& destinationUri, uint32_t& transferId) const
{
  if (uuid.empty() || sourceUri.empty() || destinationUri.empty())
    return false;

  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to import resource on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> ImportResource(%s, %s)", sourceUri.c_str(), destinationUri.c_str());

    service.service->SyncImportResource(sourceUri, destinationUri, transferId);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] <-- ImportResource(%s, %s): transferID = %u", sourceUri.c_str(), destinationUri.c_str(), transferId);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: error %u calling SyncImportResource(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: exception calling SyncImportResource(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPContentDirectoryControlPoint::StopTransferResource(const std::string& uuid, uint32_t transferId) const
{
  if (uuid.empty())
    return false;

  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to stop transfer on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> SyncStopTransferResource(%u)", transferId);

    service.service->SyncStopTransferResource(transferId);
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: error %u calling SyncStopTransferResource(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: exception calling SyncStopTransferResource(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPContentDirectoryControlPoint::GetTransferProgress(const std::string& uuid, uint32_t transferId, ohUPnPTransferStatus& status, uint64_t& length, uint64_t& total) const
{
  if (uuid.empty())
    return false;

  UPnPControlPointService service;
  if (!findService(uuid, service))
  {
    CLog::Log(LOGERROR, "COhUPnPContentDirectoryControlPoint: trying to stop transfer on an unknown service with UUID \"%s\"", uuid.c_str());
    return false;
  }

  std::string strStatus, strLength, strTotal;
  try
  {
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
      CLog::Log(LOGDEBUG, "[ohNet] --> GetTransferProgress(%u)", transferId);

    service.service->SyncGetTransferProgress(transferId, strStatus, strLength, strTotal);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- GetTransferProgress(%u): status = %s; length = %s; total = %s",
        transferId, strStatus.c_str(), strLength.c_str(), strTotal.c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: error %u calling SyncStopTransferResource(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: exception calling SyncStopTransferResource(): %s", ex.Message());
    return false;
  }

  if (strStatus == "IN_PROGRESS")
    status = ohUPnPTransferStatus::InProgress;
  else if (strStatus == "COMPLETED")
    status = ohUPnPTransferStatus::Completed;
  else if (strStatus == "STOPPED")
    status = ohUPnPTransferStatus::Stopped;
  else if (strStatus == "ERROR")
    status = ohUPnPTransferStatus::Error;
  else
    return false;

  length = strtoull(strLength.c_str(), nullptr, 0);
  total = strtoull(strTotal.c_str(), nullptr, 0);

  return true;
}

bool COhUPnPContentDirectoryControlPoint::browseSync(const UPnPControlPointService& service, const std::string& objectId, bool metadata,
                                               std::string& result, uint32_t& resultCount, uint32_t& resultTotal,
                                               uint32_t start, uint32_t count, const std::string& filter, const std::string& sorting) const
{
  uint32_t updateId;
  try
  {
    const char* browseFlag = metadata ? "BrowseMetadata" : "BrowseDirectChildren";
    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] --> Browse(%s, %s, %s, %u, %u, %s)",
        objectId.c_str(), browseFlag, filter.c_str(), start, count, sorting.c_str());
    }

    service.service->SyncBrowse(objectId, metadata ? "BrowseMetadata" : "BrowseDirectChildren",
      filter, start, count, sorting, result, resultCount, resultTotal, updateId);

    if (g_advancedSettings.CanLogComponent(LOGUPNP))
    {
      CLog::Log(LOGDEBUG, "[ohNet] <-- Browse(%s, %s, %s, %u, %u, %s): count = %u; total = %u",
        objectId.c_str(), browseFlag, filter.c_str(), start, count, sorting.c_str(), resultCount, resultTotal);
      CLog::Log(LOGDEBUG, "[ohNet] <-- Browse(%s, %s, %s, %u, %u, %s): result =\n%s",
        objectId.c_str(), browseFlag, filter.c_str(), start, count, sorting.c_str(), result.c_str());
    }
  }
  catch (OpenHome::Net::ProxyError& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: error %u calling SyncBrowse(): %s", ex.Code(), ex.Message());
    return false;
  }
  catch (OpenHome::Exception& ex)
  {
    CLog::Log(LOGWARNING, "COhUPnPContentDirectoryControlPoint: exception calling SyncBrowse(): %s", ex.Message());
    return false;
  }

  return true;
}

bool COhUPnPContentDirectoryControlPoint::resultToFileItemList(const UPnPControlPointService& service, const std::string& result, uint32_t resultCount, uint32_t resultTotal, CFileItemList& items) const
{
  if (result.empty())
    return false;

  items.ClearItems();
  if (resultCount == 0)
    return true;

  OhUPnPControlPointContext context{ service.device, service.profile };

  // parse and deserialize the result as a DIDL-Lite document
  CDidlLiteDocument doc(m_elementFactory);
  {
    CPerformanceMeasurement<> deserializationPerformance;

    if (!doc.Deserialize(result, context))
      return false;

    deserializationPerformance.Stop();
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryControlPoint: [Performance] CDidlLiteDocument deserialization: %fs", deserializationPerformance.GetDurationInSeconds());
  }

  {
    CPerformanceMeasurement<> fileItemsConversionPerformance;

    // turn the deserialized DIDL-Lite document into a list of items
    if (!FileItemUtils::DocumentToFileItemList(doc, items, context) || static_cast<uint32_t>(items.Size()) != resultCount)
      return false;

    fileItemsConversionPerformance.Stop();
    CLog::Log(LOGDEBUG, "COhUPnPContentDirectoryControlPoint: [Performance] CDidlLiteDocument to CFileItemList: %fs", fileItemsConversionPerformance.GetDurationInSeconds());
  }

  items.SetProperty("total", resultTotal);
  return true;
}
