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
#include <string>
#include <stdint.h>

class IOhUPnPService
{
public:
  virtual ~IOhUPnPService() { }

protected:
  IOhUPnPService(const std::string &domain, const std::string &serviceName, uint8_t version)
    : m_domain(domain),
      m_serviceName(serviceName),
      m_serviceVersion(version)
  {
    if (m_domain.empty() || m_serviceName.empty())
      return;

    if (version <= 0)
      version = 1;
  }

  std::string m_domain;
  std::string m_serviceName;
  uint8_t m_serviceVersion;
};
