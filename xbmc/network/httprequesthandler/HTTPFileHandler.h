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

#include <memory>
#include <string>

#include "XBDateTime.h"
#include "network/httprequesthandler/IHTTPRequestHandler.h"
#include "network/httprequesthandler/IMimeTypeGetter.h"

class CDefaultMimeTypeGetter : public IMimeTypeGetter
{
public:
  CDefaultMimeTypeGetter() = default;
  virtual ~CDefaultMimeTypeGetter() = default;

  // implementation of IMimeTypeGetter
  virtual std::string GetMimeTypeFromExtension(const std::string& extension, const HTTPRequest &request) const override;
};

class CHTTPFileHandler : public IHTTPRequestHandler
{
public:
  virtual ~CHTTPFileHandler() { }

  virtual int HandleRequest();

  virtual bool CanHandleRanges() const { return m_canHandleRanges; }
  virtual bool CanBeCached() const { return m_canBeCached; }
  virtual bool GetLastModifiedDate(CDateTime &lastModified) const;

  virtual std::string GetRedirectUrl() const { return m_url; }
  virtual std::string GetResponseFile() const { return m_url; }

protected:
  CHTTPFileHandler(std::shared_ptr<const IMimeTypeGetter> mimeTypeGetter = nullptr);
  CHTTPFileHandler(const HTTPRequest &request, std::shared_ptr<const IMimeTypeGetter> mimeTypeGetter = nullptr);
  CHTTPFileHandler(const HTTPRequest &request, const CHTTPFileHandler& other);

  void SetFile(const std::string& file, int responseStatus);

  void SetCanHandleRanges(bool canHandleRanges) { m_canHandleRanges = canHandleRanges; }
  void SetCanBeCached(bool canBeCached) { m_canBeCached = canBeCached; }
  void SetLastModifiedDate(CDateTime lastModified) { m_lastModified = lastModified; }

private:
  std::string m_url;

  bool m_canHandleRanges;
  bool m_canBeCached;

  CDateTime m_lastModified;

  const std::shared_ptr<const IMimeTypeGetter> m_mimeTypeGetter;
};
