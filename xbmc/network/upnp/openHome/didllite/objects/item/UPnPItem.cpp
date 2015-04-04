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

#include "UPnPItem.h"
#include "FileItem.h"

CUPnPItem::CUPnPItem()
  : CUPnPItem("object.item")
{ }

CUPnPItem::CUPnPItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPObject("item", classType, className)
{
  initializeProperties();
}

CUPnPItem::CUPnPItem(const CUPnPItem& item)
  : CUPnPObject(item),
    m_refID(item.m_refID)
{
  initializeProperties();
  copyPropertyValidity(&item);
}

CUPnPItem::~CUPnPItem()
{ }

bool CUPnPItem::CanHandleFileItem(const CFileItem& item) const
{
  return !item.m_bIsFolder;
}

bool CUPnPItem::ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const
{
  if (!CUPnPObject::ToFileItem(item, context))
    return false;

  if (!m_refID.empty())
    item.SetPath(m_refID);

  return true;
}

bool CUPnPItem::FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context)
{
  if (!CUPnPObject::FromFileItem(item, context))
    return false;

  m_refID = item.GetPath();
  setPropertyValidity("@refID", !m_refID.empty());

  return true;
}

void CUPnPItem::initializeProperties()
{
  // define all attributes
  addStringProperty("@refID", &m_refID).AsAttribute().SetOptional();
}
