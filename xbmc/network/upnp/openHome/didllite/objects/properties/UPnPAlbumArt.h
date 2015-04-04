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

#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

#include <string>

class CUPnPAlbumArt : public IDidlLiteElement
{
public:
  CUPnPAlbumArt();
  explicit CUPnPAlbumArt(const std::string &url, const std::string &dlnaProfileID = "");
  CUPnPAlbumArt(const CUPnPAlbumArt& upnpalbumArt);
  virtual ~CUPnPAlbumArt();

  // implementations of IDidlLiteElement
  virtual CUPnPAlbumArt* Create() const override { return new CUPnPAlbumArt(); }
  virtual CUPnPAlbumArt* Clone() const override { return new CUPnPAlbumArt(*this); }
  virtual std::string GetIdentifier() const { return "AlbumArt"; }
  virtual std::set<std::string> Extends() const { return{}; }

  const std::string& GetUrl() const { return m_url; }
  const std::string& GetDlnaProfileID() const { return m_dlnaProfileID; }

private:
  void initializeProperties();

  std::string m_url;
  std::string m_dlnaProfileID;
  std::string m_dlnaNamespace;
};
