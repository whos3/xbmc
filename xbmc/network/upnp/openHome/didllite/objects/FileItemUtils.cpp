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
#include "network/upnp/openHome/ohUPnPContext.h"
#include "network/upnp/openHome/didllite/DidlLiteDocument.h"
#include "network/upnp/openHome/didllite/objects/UPnPObject.h"
#include "network/upnp/openHome/didllite/objects/FileItemElementFactory.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
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
  {
    const CFileItem& item = *items.Get(index).get();

    IFileItemElement* element = elementFactory.GetElement(item);
    if (element == nullptr)
    {
      CLog::Log(LOGWARNING, "FileItemUtils: failed to find element for item \"%s\" (%s)",
        item.GetLabel().c_str(), item.GetPath().c_str());
      continue;
    }

    if (!element->FromFileItem(item, context))
    {
      CLog::Log(LOGWARNING, "FileItemUtils: failed to serialize item \"%s\" (%s) into element <%s:%s>",
        item.GetLabel().c_str(), item.GetPath().c_str(),
        element->GetElementNamespace().c_str(), element->GetElementName().c_str());
      continue;
    }

    // overwrite the parent ID if provided
    if (!parent.empty())
    {
      CUPnPObject* upnpObject = dynamic_cast<CUPnPObject*>(element);
      if (upnpObject != nullptr)
        upnpObject->SetParentId(parent);
    }

    document.AddElement(element);
  }

  return true;
}
