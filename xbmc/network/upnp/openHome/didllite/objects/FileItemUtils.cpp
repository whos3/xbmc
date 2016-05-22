/*
 *      Copyright (C) 2015 Team XBMC
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
#include <map>
#include <vector>

#include "FileItemUtils.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/didllite/DidlLiteDocument.h"
#include "network/upnp/openHome/didllite/objects/FileItemElementFactory.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "network/upnp/openHome/didllite/objects/UPnPObject.h"
#include "network/upnp/openHome/didllite/objects/classmappers/UPnPClassMapping.h"
#include "utils/log.h"

bool FileItemUtils::DocumentToFileItemList(const CDidlLiteDocument& document, CFileItemList& items, const OhUPnPControlPointContext& context)
{
  items.ClearItems();

  std::map<std::string, size_t> classes;
  std::vector<const IDidlLiteElement*> elements = document.GetElements();
  for (const auto& element : elements)
  {
    const IFileItemElement* fileItemElement = dynamic_cast<const IFileItemElement*>(element);
    if (fileItemElement == nullptr)
    {
      CLog::Log(LOGWARNING, "FileItemUtils: failed to deserialize element <%s:%s>",
        element->GetElementNamespace().c_str(), element->GetElementName().c_str());
      continue;
    }

    CFileItemPtr fileItem = std::make_shared<CFileItem>();
    if (!fileItemElement->ToFileItem(*fileItem, context))
    {
      CLog::Log(LOGWARNING, "FileItemUtils: failed to deserialize element <%s:%s> into item",
        element->GetElementNamespace().c_str(), element->GetElementName().c_str());
      continue;
    }

    items.Add(fileItem);

    // count the number of items per media type
    std::string mediaType = context.profile.GetClassMapping().GetMediaType(fileItemElement);
    if (!mediaType.empty())
      classes[mediaType] += 1;
  }

  // determine the media type with the most items
  const auto& maximumClass = std::max_element(classes.begin(), classes.end(), classes.value_comp());
  if (maximumClass != classes.end())
    items.SetContent(maximumClass->first);

  return true;
}

bool FileItemUtils::FileItemListToDocument(const CFileItemList& items, CDidlLiteDocument& document, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, const std::string& parent /* = "" */)
{
  for (int index = 0; index < items.Size(); ++index)
    SerializeFileItem(*items.Get(index).get(), document, elementFactory, context, parent);

  return true;
}

bool FileItemUtils::ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, CUPnPObject*& upnpObj)
{
  return ConvertFileItem(item, elementFactory, device, profile, COhUPnP::GetInstance().GetResourceUriPrefix(), upnpObj);
}

bool FileItemUtils::ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix, CUPnPObject*& upnpObj)
{
  OhUPnPRootDeviceContext context(device, profile, resourceUriPrefix);

  return ConvertFileItem(item, elementFactory, context, upnpObj);
}

bool FileItemUtils::ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, CUPnPObject*& upnpObj)
{
  return ConvertFileItem(item, elementFactory, context, "", upnpObj);
}

bool FileItemUtils::SerializeObject(CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, std::string& result)
{
  return SerializeObject(upnpObj, elementFactory, device, profile, COhUPnP::GetInstance().GetResourceUriPrefix(), result);
}

bool FileItemUtils::SerializeObject(CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix, std::string& result)
{
  if (upnpObj == nullptr)
    return false;

  OhUPnPRootDeviceContext context(device, profile, resourceUriPrefix);

  return SerializeObject(upnpObj, elementFactory, context, result);
}

bool FileItemUtils::SerializeObject(CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, std::string& result)
{
  if (upnpObj == nullptr)
    return false;

  CDidlLiteDocument doc(elementFactory);
  doc.AddNamespace(UPNP_DIDL_LITE_NAMESPACE_URI);
  doc.AddNamespace(UPNP_DIDL_DC_NAMESPACE, UPNP_DIDL_DC_NAMESPACE_URI);
  doc.AddNamespace(UPNP_DIDL_UPNP_NAMESPACE, UPNP_DIDL_UPNP_NAMESPACE_URI);

  doc.AddElement(upnpObj);

  return doc.Serialize(result, context);
}

bool FileItemUtils::DeserializeObject(const std::string& document, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const CUPnPObject*& upnpObj)
{
  OhUPnPControlPointContext context(device, profile);

  return DeserializeObject(document, elementFactory, context, upnpObj);
}

bool FileItemUtils::DeserializeObject(const std::string& document, const CFileItemElementFactory& elementFactory, const OhUPnPControlPointContext& context, const CUPnPObject*& upnpObj)
{
  if (document.empty())
    return false;

  CDidlLiteDocument doc(elementFactory);
  if (!doc.Deserialize(document, context))
    return false;

  std::vector<const IDidlLiteElement*> elements = doc.GetElements();
  if (elements.size() != 1)
    return false;

  upnpObj = dynamic_cast<const CUPnPObject*>(elements.front());
  return upnpObj != nullptr;
}

bool FileItemUtils::ConvertFileItem(const CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, CFileItem& item)
{
  if (upnpObj == nullptr)
    return false;

  OhUPnPControlPointContext context(device, profile);

  return ConvertFileItem(upnpObj, elementFactory, context, item);
}

bool FileItemUtils::ConvertFileItem(const CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const OhUPnPControlPointContext& context, CFileItem& item)
{
  if (upnpObj == nullptr)
    return false;

  if (!upnpObj->ToFileItem(item, context))
    return false;

  return true;
}

bool FileItemUtils::DeserializeFileItem(const std::string& document, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, CFileItem& item)
{
  OhUPnPControlPointContext context(device, profile);

  return DeserializeFileItem(document, elementFactory, context, item);
}

bool FileItemUtils::DeserializeFileItem(const std::string& document, const CFileItemElementFactory& elementFactory, const OhUPnPControlPointContext& context, CFileItem& item)
{
  if (document.empty())
    return false;

  CDidlLiteDocument doc(elementFactory);
  if (!doc.Deserialize(document, context))
    return false;

  CFileItemList items;
  if (!DocumentToFileItemList(doc, items, context) || items.Size() != 1)
    return false;

  item = *items.Get(0).get();
  return true;
}

bool FileItemUtils::SerializeFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, std::string& result)
{
  return SerializeFileItem(item, elementFactory, device, profile, COhUPnP::GetInstance().GetResourceUriPrefix(), result);
}

bool FileItemUtils::SerializeFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix, std::string& result)
{
  OhUPnPRootDeviceContext context(device, profile, resourceUriPrefix);

  return SerializeFileItem(item, elementFactory, context, result);
}

bool FileItemUtils::SerializeFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, std::string& result)
{
  CUPnPObject* upnpObj = nullptr;
  if (!ConvertFileItem(item, elementFactory, context, upnpObj))
    return false;

  return SerializeObject(upnpObj, elementFactory, context, result);
}

bool FileItemUtils::ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, const std::string& parent, CUPnPObject*& upnpObj)
{
  IFileItemElement* element = elementFactory.GetElement(item);
  if (element == nullptr)
  {
    CLog::Log(LOGWARNING, "FileItemUtils: failed to find element for item \"%s\" (%s)",
      item.GetLabel().c_str(), item.GetPath().c_str());
    return false;
  }

  upnpObj = dynamic_cast<CUPnPObject*>(element);
  if (upnpObj == nullptr)
  {
    CLog::Log(LOGWARNING, "FileItemUtils: item \"%s\" (%s) of type %s is not a UPnP item",
      item.GetLabel().c_str(), item.GetPath().c_str(), element->GetType().c_str());
    return false;
  }

  if (!element->FromFileItem(item, context))
  {
    CLog::Log(LOGWARNING, "FileItemUtils: failed to serialize item \"%s\" (%s) into element <%s:%s>",
      item.GetLabel().c_str(), item.GetPath().c_str(),
      element->GetElementNamespace().c_str(), element->GetElementName().c_str());
    return false;
  }

  // overwrite the parent ID if provided
  if (!parent.empty())
    upnpObj->SetParentId(parent);

  return true;
}

bool FileItemUtils::SerializeFileItem(const CFileItem& item, CDidlLiteDocument& document, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, const std::string& parent)
{
  CUPnPObject* upnpObj = nullptr;
  if (!ConvertFileItem(item, elementFactory, context, parent, upnpObj))
    return false;

  document.AddElement(upnpObj);

  return true;
}
