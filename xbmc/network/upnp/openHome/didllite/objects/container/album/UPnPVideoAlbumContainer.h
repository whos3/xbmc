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

#include "network/upnp/openHome/didllite/objects/container/album/UPnPAlbumContainer.h"

class CUPnPVideoAlbumContainer : public CUPnPAlbumContainer
{
public:
  CUPnPVideoAlbumContainer();
  CUPnPVideoAlbumContainer(const std::string& classType, const std::string& className = "");
  CUPnPVideoAlbumContainer(const CUPnPVideoAlbumContainer& videoAlbumContainer);
  virtual ~CUPnPVideoAlbumContainer();

  // implementations of IDidlLiteElement
  virtual CUPnPVideoAlbumContainer* Create() const override { return Clone(); }
  virtual CUPnPVideoAlbumContainer* Clone() const override { return new CUPnPVideoAlbumContainer(*this); }

  // specializations of IFileItemElement
  virtual bool ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const override;
  virtual bool FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context) override;
  virtual std::string GetIdentifier() const { return "VideoAlbumContainer"; }
  virtual std::set<std::string> Extends() const { return { "AlbumContainer" }; }

  // specializations of IFileItemElement
  virtual bool CanHandleFileItem(const CFileItem& item) const override;
};
