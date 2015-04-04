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

#include <stdint.h>
#include <string>

class CUPnPIcon : public IDidlLiteElement
{
public:
  CUPnPIcon();
  CUPnPIcon(const std::string &url, const std::string &mimetype, uint32_t width, uint32_t height, uint32_t depth);
  CUPnPIcon(const CUPnPIcon& upnpIcon);
  virtual ~CUPnPIcon();

  // implementations of IDidlLiteElement
  virtual CUPnPIcon* Create() const override { return new CUPnPIcon(); }
  virtual CUPnPIcon* Clone() const override { return new CUPnPIcon(*this); }
  virtual std::string GetIdentifier() const { return "Icon"; }
  virtual std::set<std::string> Extends() const { return{}; }

  bool operator<(const CUPnPIcon &other) const;

  const std::string& GetUrl() const { return m_url; }
  const std::string& GetMimeType() const { return m_mimetype; }
  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }
  uint32_t GetDepth() const { return m_depth; }

private:
  void initializeProperties();

  std::string m_url;
  std::string m_mimetype;
  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_depth;
};

class CUPnPIconList : public IDidlLiteElement
{
public:
  CUPnPIconList();
  explicit CUPnPIconList(const std::vector<CUPnPIcon>& icons);
  CUPnPIconList(const CUPnPIconList& upnpIcons);
  virtual ~CUPnPIconList();

  // implementations of IDidlLiteElement
  virtual CUPnPIconList* Create() const override { return new CUPnPIconList(); }
  virtual CUPnPIconList* Clone() const override { return new CUPnPIconList(*this); }
  virtual std::string GetIdentifier() const { return "IconList"; }
  virtual std::set<std::string> Extends() const { return{}; }

  std::vector<const CUPnPIcon*> GetIcons() const;

private:
  void initializeProperties();

  std::vector<CUPnPIcon*> m_icons;
};
