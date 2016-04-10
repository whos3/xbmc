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

class ByteFormatter
{
public:
  static double ToKB(double bps);
  static double ToMB(double bps);
  static double ToGB(double bps);
  static double ToTB(double bps);

  static std::string ToBString(double bps, uint8_t decimalPlaces);
  static std::string ToKBString(double bps, uint8_t decimalPlaces);
  static std::string ToMBString(double bps, uint8_t decimalPlaces);
  static std::string ToGBString(double bps, uint8_t decimalPlaces);
  static std::string ToTBString(double bps, uint8_t decimalPlaces);
  static std::string ToString(double bps, uint8_t decimalPlaces);

private:
  ByteFormatter() = delete;
  ~ByteFormatter() = delete;

  static std::string GetFormat(uint8_t decimalPlaces);
};
