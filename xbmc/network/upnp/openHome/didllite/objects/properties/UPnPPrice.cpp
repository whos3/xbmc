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

#include "UPnPPrice.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPPrice::CUPnPPrice()
  : CUPnPPrice(0.0f)
{ }

CUPnPPrice::CUPnPPrice(float price)
  : CUPnPPrice(price, "")
{ }

CUPnPPrice::CUPnPPrice(float price, const std::string& currency)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, "genre"),
    m_price(price),
    m_currency(currency)
{
  initializeProperties();

  setPropertyValidity(m_price > 0.0f);
  setPropertyValidity("@currency", !m_currency.empty());
}

CUPnPPrice::CUPnPPrice(const CUPnPPrice& upnpPrice)
  : IDidlLiteElement(upnpPrice),
    m_price(upnpPrice.m_price),
    m_currency(upnpPrice.m_currency)
{
  initializeProperties();
  copyPropertyValidity(&upnpPrice);
}

CUPnPPrice::~CUPnPPrice()
{ }

void CUPnPPrice::initializeProperties()
{
  addNumberProperty(&m_price).SetRequired();
  addStringProperty("@currency", &m_currency).AsAttribute().SetRequired();
}
