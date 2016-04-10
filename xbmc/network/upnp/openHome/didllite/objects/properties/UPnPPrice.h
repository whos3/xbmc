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

class CUPnPPrice : public IDidlLiteElement
{
public:
  CUPnPPrice();
  explicit CUPnPPrice(float price);
  CUPnPPrice(float price, const std::string& currency);
  CUPnPPrice(const CUPnPPrice& upnpPrice);
  virtual ~CUPnPPrice();

  // implementations of IDidlLiteElement
  virtual CUPnPPrice* Create() const override { return new CUPnPPrice(); }
  virtual CUPnPPrice* Clone() const override { return new CUPnPPrice(*this); }
  virtual std::string GetIdentifier() const { return "Price"; }
  virtual std::set<std::string> Extends() const { return { }; }

  float GetPrice() const { return m_price; }
  void SetPrice(float price) { m_price = price; }
  const std::string& GetCurrency() const { return m_currency; }
  void SetCurrency(const std::string& currency) { m_currency = currency; }

private:
  void initializeProperties();

  float m_price;
  std::string m_currency;
};
