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

#include "UPnPAudioBookItem.h"
#include "FileItem.h"

CUPnPAudioBookItem::CUPnPAudioBookItem()
  : CUPnPAudioBookItem("object.item.audioItem.audioBook")
{ }

CUPnPAudioBookItem::CUPnPAudioBookItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPAudioItem(classType, className)
{ }

CUPnPAudioBookItem::CUPnPAudioBookItem(const CUPnPAudioBookItem& audioBookItem)
  : CUPnPAudioItem(audioBookItem)
{
  copyPropertyValidity(&audioBookItem);
}

CUPnPAudioBookItem::~CUPnPAudioBookItem()
{ }

bool CUPnPAudioBookItem::CanHandleFileItem(const CFileItem& item) const
{
  return false; // TODO
}

bool CUPnPAudioBookItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPAudioItem::ToFileItem(item, context))
    return false;

  // TODO

  return true;
}

bool CUPnPAudioBookItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (!CUPnPAudioItem::FromFileItem(item, context))
    return false;

  // TODO

  return true;
}
