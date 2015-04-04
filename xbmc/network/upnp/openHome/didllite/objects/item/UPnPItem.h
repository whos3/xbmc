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

#include "network/upnp/openHome/didllite/objects/UPnPObject.h"

class CUPnPItem : public CUPnPObject
{
public:
  CUPnPItem();
  CUPnPItem(const std::string& classType, const std::string& className = "");
  CUPnPItem(const CUPnPItem& item);
  virtual ~CUPnPItem();

  // implementations of IDidlLiteElement
  virtual CUPnPItem* Create() const override { return Clone(); }
  virtual CUPnPItem* Clone() const override { return new CUPnPItem(*this); }
  virtual std::string GetIdentifier() const { return "Item"; }
  virtual std::set<std::string> Extends() const { return { "Object" }; }

  // specialization of IFileItemElement
  virtual bool CanHandleFileItem(const CFileItem& item) const override;
  virtual bool ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const override;
  virtual bool FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context) override;

  const std::string& GetRefID() const { return m_refID; }
  std::vector<const CUPnPResource*> GetResources() const { return CUPnPObject::GetResources(); }
  void SetResources(const std::vector<CUPnPResource*>& resources) { CUPnPObject::SetResources(resources); }
  void AddResource(CUPnPResource* resource) { CUPnPObject::AddResource(resource); }

protected:
  void initializeProperties();

private:
  std::string m_refID;
};
