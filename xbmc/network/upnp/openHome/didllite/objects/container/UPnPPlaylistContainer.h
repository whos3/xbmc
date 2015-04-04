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

#include "network/upnp/openHome/didllite/objects/container/UPnPContainer.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPPerson.h"

class CUPnPPlaylistContainer : public CUPnPContainer
{
public:
  CUPnPPlaylistContainer();
  CUPnPPlaylistContainer(const std::string& classType, const std::string& className = "");
  CUPnPPlaylistContainer(const CUPnPPlaylistContainer& playlistContainer);
  virtual ~CUPnPPlaylistContainer();

  // implementations of IDidlLiteElement
  virtual CUPnPPlaylistContainer* Create() const override { return Clone(); }
  virtual CUPnPPlaylistContainer* Clone() const override { return new CUPnPPlaylistContainer(*this); }
  virtual std::string GetIdentifier() const { return "PlaylistContainer"; }
  virtual std::set<std::string> Extends() const { return { "Container" }; }

  // specializations of IFileItemElement
  virtual bool CanHandleFileItem(const CFileItem& item) const override { return false; /* TODO */ }

  // TODO
};
