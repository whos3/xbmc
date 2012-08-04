#pragma once
/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <string>

class CVariant;

namespace API
{
  typedef enum
  {
    TransportLayerCapabilityResponse              = 0x1,
    TransportLayerCapabilityAnnouncing            = 0x2,
    TransportLayerCapabilityFileDownloadRedirect  = 0x4,
    TransportLayerCapabilityFileDownloadDirect    = 0x8
  } TransportLayerCapability;

  const int TransportLayerCapabilityAll = (TransportLayerCapabilityResponse | TransportLayerCapabilityAnnouncing |
                                           TransportLayerCapabilityFileDownloadRedirect | TransportLayerCapabilityFileDownloadDirect);

  class ITransportLayer
  {
  public:
    virtual ~ITransportLayer() { };
    virtual bool PrepareDownload(const char *path, CVariant &details, std::string &protocol) = 0;
    virtual bool Download(const char *path, CVariant &result) = 0;
    virtual int GetCapabilities() = 0;

    /*!
     \brief Returns a TransportLayerCapability value of the given string representation
     \param transport String representation of the TransportLayerCapability
     \return TransportLayerCapability value of the given string representation
     */
    static inline TransportLayerCapability StringToTransportLayer(const std::string &transport)
    {
      if (transport.compare("Announcing") == 0)
        return TransportLayerCapabilityAnnouncing;
      if (transport.compare("FileDownloadDirect") == 0)
        return TransportLayerCapabilityFileDownloadDirect;
      if (transport.compare("FileDownloadRedirect") == 0)
        return TransportLayerCapabilityFileDownloadRedirect;

      return TransportLayerCapabilityResponse;
    }
  };
}
