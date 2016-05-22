#pragma once
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

#include <string>

struct OhUPnPControlPointContext;
struct OhUPnPRootDeviceContext;

class CDidlLiteDocument;
class CFileItem;
class CFileItemElementFactory;
class CFileItemList;
class COhUPnPDevice;
class COhUPnPDeviceProfile;
class CUPnPClassMapping;
class CUPnPObject;

class FileItemUtils
{
public:
  static bool DocumentToFileItemList(const CDidlLiteDocument& document, CFileItemList& items, const OhUPnPControlPointContext& context);
  static bool FileItemListToDocument(const CFileItemList& items, CDidlLiteDocument& document, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, const std::string& parent = "");

  static bool ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, CUPnPObject*& upnpObj);
  static bool ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix, CUPnPObject*& upnpObj);
  static bool ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, CUPnPObject*& upnpObj);

  static bool SerializeObject(CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, std::string& result);
  static bool SerializeObject(CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix, std::string& result);
  static bool SerializeObject(CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, std::string& result);

  static bool SerializeFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, std::string& result);
  static bool SerializeFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const std::string& resourceUriPrefix, std::string& result);
  static bool SerializeFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, std::string& result);

  static bool DeserializeObject(const std::string& document, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, const CUPnPObject*& upnpObj);
  static bool DeserializeObject(const std::string& document, const CFileItemElementFactory& elementFactory, const OhUPnPControlPointContext& context, const CUPnPObject*& upnpObj);

  static bool ConvertFileItem(const CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, CFileItem& item);
  static bool ConvertFileItem(const CUPnPObject* upnpObj, const CFileItemElementFactory& elementFactory, const OhUPnPControlPointContext& context, CFileItem& item);

  static bool DeserializeFileItem(const std::string& document, const CFileItemElementFactory& elementFactory, const COhUPnPDevice& device, const COhUPnPDeviceProfile& profile, CFileItem& item);
  static bool DeserializeFileItem(const std::string& document, const CFileItemElementFactory& elementFactory, const OhUPnPControlPointContext& context, CFileItem& item);

private:
  FileItemUtils() = default;

  static bool ConvertFileItem(const CFileItem& item, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, const std::string& parent, CUPnPObject*& upnpObj);
  static bool SerializeFileItem(const CFileItem& item, CDidlLiteDocument& document, const CFileItemElementFactory& elementFactory, const OhUPnPRootDeviceContext& context, const std::string& parent);
};