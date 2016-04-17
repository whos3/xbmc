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

#include "ohUPnPRenderingControl.h"

const OhUPnPEnumValueDefinitions<OhUPnPRenderingControlChannel> COhUPnPRenderingControlChannel::OhUPnPRenderingControlChannelDefinition = {
  { OhUPnPRenderingControlChannel::Master, "Master" },
  { OhUPnPRenderingControlChannel::LF, "LF" },
  { OhUPnPRenderingControlChannel::RF, "RF" },
  { OhUPnPRenderingControlChannel::CF, "CF" },
  { OhUPnPRenderingControlChannel::LFE, "LFE" },
  { OhUPnPRenderingControlChannel::LS, "LS" },
  { OhUPnPRenderingControlChannel::RS, "RS" },
  { OhUPnPRenderingControlChannel::LFC, "LFC" },
  { OhUPnPRenderingControlChannel::RFC, "RFC" },
  { OhUPnPRenderingControlChannel::SD, "SD" },
  { OhUPnPRenderingControlChannel::SL, "SL" },
  { OhUPnPRenderingControlChannel::SR, "SR" },
  { OhUPnPRenderingControlChannel::T, "T" },
  { OhUPnPRenderingControlChannel::B, "B" },
  { OhUPnPRenderingControlChannel::BC, "BC" },
  { OhUPnPRenderingControlChannel::BL, "BL" },
  { OhUPnPRenderingControlChannel::BR, "BR" },
};
