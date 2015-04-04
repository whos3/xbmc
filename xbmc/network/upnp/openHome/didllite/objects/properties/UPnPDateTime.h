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

#include "XBDateTime.h"
#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

enum class UPnPDaylightSaving
{
  Unknown,
  Standard,
  DaylightSaving
};

class CUPnPDate : public IDidlLiteElement
{
public:
  CUPnPDate();
  CUPnPDate(const std::string& date, UPnPDaylightSaving daylightSaving = UPnPDaylightSaving::Unknown);
  explicit CUPnPDate(const CDateTime& date);
  CUPnPDate(const CUPnPDate& upnpDate);
  virtual ~CUPnPDate();

  // implementations of IDidlLiteElement
  virtual CUPnPDate* Create() const override { return new CUPnPDate(); }
  virtual CUPnPDate* Clone() const override { return new CUPnPDate(*this); }
  virtual std::string GetIdentifier() const { return "Date"; }
  virtual std::set<std::string> Extends() const { return { }; }

  const CDateTime& GetDate() const { return m_date; }
  void SetDate(const CDateTime& date) { m_date = date; }

protected:
  // implementations of IDidlLiteElement
  virtual bool serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const override;
  virtual bool deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context) override;

private:
  CDateTime m_date;
  UPnPDaylightSaving m_daylightSaving;
};

class CUPnPDateTime : public IDidlLiteElement
{
public:
  explicit CUPnPDateTime(const std::string& elementName);
  CUPnPDateTime(const std::string& elementName, const std::string& dateTime, UPnPDaylightSaving daylightSaving = UPnPDaylightSaving::Unknown);
  CUPnPDateTime(const std::string& elementName, const CDateTime& date);
  CUPnPDateTime(const CUPnPDateTime& upnpDateTime);
  virtual ~CUPnPDateTime();

  // implementations of IDidlLiteElement
  virtual CUPnPDateTime* Create() const override { return new CUPnPDateTime(""); } // TODO
  virtual CUPnPDateTime* Clone() const override { return new CUPnPDateTime(*this); }
  virtual std::string GetIdentifier() const { return "DateTime"; }
  virtual std::set<std::string> Extends() const { return { }; }

  const CDateTime& GetDateTime() const { return m_dateTime; }
  void SetDateTime(const CDateTime& dateTime) { m_dateTime = dateTime; }

protected:
  // implementations of IDidlLiteElement
  virtual bool serialize(TiXmlNode* node, const OhUPnPRootDeviceContext& context) const override;
  virtual bool deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context) override;

private:
  CDateTime m_dateTime;
  UPnPDaylightSaving m_daylightSaving;
};
