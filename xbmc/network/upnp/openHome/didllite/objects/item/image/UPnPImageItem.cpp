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

#include "UPnPImageItem.h"
#include "FileItem.h"

CUPnPImageItem::CUPnPImageItem()
  : CUPnPImageItem("object.item.imageItem")
{ }

CUPnPImageItem::CUPnPImageItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPItem(classType, className)
{ }

CUPnPImageItem::CUPnPImageItem(const CUPnPImageItem& imageItem)
  : CUPnPItem(imageItem)
{
  copyPropertyValidity(&imageItem);
}

CUPnPImageItem::~CUPnPImageItem()
{ }

bool CUPnPImageItem::CanHandleFileItem(const CFileItem& item) const
{
  return CUPnPItem::CanHandleFileItem(item) && item.HasPictureInfoTag();
}
