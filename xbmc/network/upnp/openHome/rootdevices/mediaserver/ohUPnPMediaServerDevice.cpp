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

#include "ohUPnPMediaServerDevice.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "network/upnp/openHome/rootdevices/mediaserver/ohUPnPContentDirectoryService.h"
#include "network/upnp/openHome/rootdevices/mediaserver/ohUPnPMediaServerConnectionManagerService.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"

static const size_t MaximumAttributeValueLength = 256;

COhUPnPMediaServerDevice::COhUPnPMediaServerDevice(const std::string& uuid,
  const CFileItemElementFactory& fileItemElementFactory,
  COhUPnPTransferManager& transferManager,
  COhUPnPResourceManager& resourceManager)
  : COhUPnPRootDevice(uuid, UPNP_DOMAIN_NAME, UPNP_DEVICE_TYPE_MEDIASERVER, 3,
                      fileItemElementFactory, transferManager, resourceManager),
    m_contentDirectory(nullptr),
    m_connectionManager(nullptr),
    m_supportImporting(false)
{ }

COhUPnPMediaServerDevice::~COhUPnPMediaServerDevice()
{ }

void COhUPnPMediaServerDevice::SetSupportImporting(bool supportImporting)
{
  if (m_supportImporting == supportImporting)
    return;

  m_supportImporting = supportImporting;

  if (IsRunning())
    Restart();
}

void COhUPnPMediaServerDevice::SetupDevice(OpenHome::Net::DvDeviceStdStandard* device)
{
  if (device == nullptr)
    return;

  SetModelDescription(StringUtils::Format("%s - Media Server", CSysInfo::GetAppName().c_str()));
}

bool COhUPnPMediaServerDevice::StartServices()
{
  // create and start the ConnectionManager service
  m_connectionManager = std::make_unique<COhUPnPMediaServerConnectionManagerService>(*this);
  m_connectionManager->Start();

  // create and start the ContentDirectory service
  m_contentDirectory = std::make_unique<COhUPnPContentDirectoryService>(*this, m_elementFactory, m_transferManager, m_resourceManager);
  m_contentDirectory->Start(m_supportImporting);

  return true;
}

bool COhUPnPMediaServerDevice::StopServices()
{
  // stop the ContentDirectory service
  m_contentDirectory->Stop();
  m_contentDirectory.reset();

  // stop the ConnectionManager service
  m_connectionManager->Stop();
  m_connectionManager.reset();

  return true;
}
