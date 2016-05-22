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

#include "ohUPnPMediaRendererDevice.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/rootdevices/mediarenderer/ohUPnPMediaRendererAVTransportService.h"
#include "network/upnp/openHome/rootdevices/mediarenderer/ohUPnPMediaRendererConnectionManagerService.h"
#include "network/upnp/openHome/rootdevices/mediarenderer/ohUPnPRenderingControlService.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"

static const size_t MaximumAttributeValueLength = 256;

COhUPnPMediaRendererDevice::COhUPnPMediaRendererDevice(const std::string& uuid,
  const CFileItemElementFactory& fileItemElementFactory,
  COhUPnPTransferManager& transferManager,
  COhUPnPResourceManager& resourceManager)
  : COhUPnPRootDevice(uuid, UPNP_DOMAIN_NAME, UPNP_DEVICE_TYPE_MEDIARENDERER, 3,
                      fileItemElementFactory, transferManager, resourceManager)
  , m_avTransport(nullptr)
  , m_renderingControl(nullptr)
  , m_connectionManager(nullptr)
{ }

COhUPnPMediaRendererDevice::~COhUPnPMediaRendererDevice()
{ }

void COhUPnPMediaRendererDevice::UpdateState()
{
  if (m_avTransport != nullptr)
    m_avTransport->UpdateState();
}

void COhUPnPMediaRendererDevice::SetupDevice(OpenHome::Net::DvDeviceStdStandard* device)
{
  if (device == nullptr)
    return;

  SetModelDescription(StringUtils::Format("%s - Media Renderer", CSysInfo::GetAppName().c_str()));
}

bool COhUPnPMediaRendererDevice::StartServices()
{
  // create and start the ConnectionManager service
  m_connectionManager = std::make_unique<COhUPnPMediaRendererConnectionManagerService>(*this);
  m_connectionManager->Start();

  // create and start RenderingControl service
  m_renderingControl = std::make_unique<COhUPnPRenderingControlService>(*this);
  m_renderingControl->Start();

  // create and start AVTransport service
  m_avTransport = std::make_unique<COhUPnPMediaRendererAVTransportService>(*this, m_elementFactory);
  m_avTransport->Start();

  return true;
}

bool COhUPnPMediaRendererDevice::StopServices()
{
  // stop the AVTransport service
  m_avTransport->Stop();
  m_avTransport.reset();

  // stop the RenderingControl service
  m_renderingControl->Stop();
  m_renderingControl.reset();

  // stop the ConnectionManager service
  m_connectionManager->Stop();
  m_connectionManager.reset();

  return true;
}
