#pragma once
/*
 *      Copyright (C) 2016 Team Kodi
 *      http://kodi.tv
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <string>

#include "utils/ByteFormatter.h"

class TransferSpeedFormatter
{
public:
  template<class TTransferSpeed>
  static inline double ToKBps(const TTransferSpeed& speed)
  {
    return ByteFormatter::ToKB(speed.GetCurrentTransferSpeed());
  }

  template<class TTransferSpeed>
  static inline double ToMBps(const TTransferSpeed& speed)
  {
    return ByteFormatter::ToMB(speed.GetCurrentTransferSpeed());
  }

  template<class TTransferSpeed>
  static inline std::string ToBpsString(const TTransferSpeed& speed, uint8_t decimalPlaces)
  {
    return FormatString(ByteFormatter::ToBString(speed.GetCurrentTransferSpeed(), decimalPlaces));
  }

  template<class TTransferSpeed>
  static inline std::string ToKBpsString(const TTransferSpeed& speed, uint8_t decimalPlaces)
  {
    return FormatString(ByteFormatter::ToKBString(speed.GetCurrentTransferSpeed(), decimalPlaces));
  }

  template<class TTransferSpeed>
  static inline std::string ToMBpsString(const TTransferSpeed& speed, uint8_t decimalPlaces)
  {
    return FormatString(ByteFormatter::ToMBString(speed.GetCurrentTransferSpeed(), decimalPlaces));
  }

  template<class TTransferSpeed>
  static inline std::string ToString(const TTransferSpeed& speed, uint8_t decimalPlaces)
  {
    return FormatString(ByteFormatter::ToString(speed.GetCurrentTransferSpeed(), decimalPlaces));
  }

private:
  TransferSpeedFormatter() = delete;
  ~TransferSpeedFormatter() = delete;

  static inline std::string FormatString(const std::string& formattedString)
  {
    return !formattedString.empty() ? (formattedString + "/s") : formattedString; // TODO: localization
  }
};
