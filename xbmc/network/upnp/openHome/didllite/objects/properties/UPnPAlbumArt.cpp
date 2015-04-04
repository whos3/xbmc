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

#include "UPnPAlbumArt.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPAlbumArt::CUPnPAlbumArt()
  : CUPnPAlbumArt("")
{ }

CUPnPAlbumArt::CUPnPAlbumArt(const std::string &url, const std::string &dlnaProfileID /* = "" */)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, "albumArtURI"),
    m_url(url),
    m_dlnaProfileID(dlnaProfileID),
    m_dlnaNamespace(UPNP_DIDL_DLNA_NAMESPACE_URI)
{
  initializeProperties();

  setPropertyValidity(!m_url.empty());
  setPropertyValidity("@dlna:profileID", !m_dlnaProfileID.empty());
  setPropertyValidity("@xmlns:" UPNP_DIDL_DLNA_NAMESPACE, !m_dlnaProfileID.empty());
}

CUPnPAlbumArt::CUPnPAlbumArt(const CUPnPAlbumArt& upnpAlbumArt)
  : IDidlLiteElement(upnpAlbumArt),
    m_url(upnpAlbumArt.m_url),
    m_dlnaProfileID(upnpAlbumArt.m_dlnaProfileID)
{
  initializeProperties();
  copyPropertyValidity(&upnpAlbumArt);
}

CUPnPAlbumArt::~CUPnPAlbumArt()
{ }

void CUPnPAlbumArt::initializeProperties()
{
  addStringProperty(&m_url).SetRequired();
  addStringProperty("@dlna:profileID", &m_dlnaProfileID).SetOptional();
  addStringProperty("@xmlns:" UPNP_DIDL_DLNA_NAMESPACE, &m_dlnaNamespace).SetOptional();
}
