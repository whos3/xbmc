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

#include "ByteFormatter.h"
#include "utils/StringUtils.h"

static const uint16_t Factor = 1024;

double ByteFormatter::ToKB(double bps)
{
  return bps / Factor;
}

double ByteFormatter::ToMB(double bps)
{
  return ToKB(bps) / Factor;
}

double ByteFormatter::ToGB(double bps)
{
  return ToMB(bps) / Factor;
}

double ByteFormatter::ToTB(double bps)
{
  return ToGB(bps) / Factor;
}

std::string ByteFormatter::ToBString(double bps, uint8_t decimalPlaces)
{
  return StringUtils::Format(GetFormat(decimalPlaces).c_str(), bps) + " B"; // TODO: localization
}

std::string ByteFormatter::ToKBString(double bps, uint8_t decimalPlaces)
{
  return StringUtils::Format(GetFormat(decimalPlaces).c_str(), ToKB(bps)) + " KB"; // TODO: localization
}

std::string ByteFormatter::ToMBString(double bps, uint8_t decimalPlaces)
{
  return StringUtils::Format(GetFormat(decimalPlaces).c_str(), ToMB(bps)) + " MB"; // TODO: localization
}

std::string ByteFormatter::ToGBString(double bps, uint8_t decimalPlaces)
{
  return StringUtils::Format(GetFormat(decimalPlaces).c_str(), ToGB(bps)) + " GB"; // TODO: localization
}

std::string ByteFormatter::ToTBString(double bps, uint8_t decimalPlaces)
{
  return StringUtils::Format(GetFormat(decimalPlaces).c_str(), ToTB(bps)) + " TB"; // TODO: localization
}

std::string ByteFormatter::ToString(double bps, uint8_t decimalPlaces)
{
  if (bps < Factor)
    return ToBString(bps, decimalPlaces);

  auto kbps = ToKB(bps);
  if (kbps < Factor)
    return ToKBString(bps, decimalPlaces);

  auto mbps = ToMB(bps);
  if (mbps < Factor)
    return ToMBString(bps, decimalPlaces);

  auto gbps = ToGB(bps);
  if (gbps < Factor)
    return ToGBString(bps, decimalPlaces);

  return ToTBString(bps, decimalPlaces);
}

std::string ByteFormatter::GetFormat(uint8_t decimalPlaces)
{
  std::string format = "%.";
  if (decimalPlaces == 0)
    format += "0";
  else
    format += StringUtils::Format("%hhu", decimalPlaces);
  format += "f";

  return format;
}
