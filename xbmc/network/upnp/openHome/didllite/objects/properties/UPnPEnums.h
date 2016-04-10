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

enum class UPnPRadioBand
{
  AM,
  FM,
  Shortwave,
  Internet,
  Sattelite,
  VendorSpecific
};

enum class UPnPDayOfWeek
{
  Sunday,
  Monday,
  Tuesday,
  Wednesday,
  Thursday,
  Friday,
  Saturday
};

enum class UPnPRecordedEpisodeType
{
  All,
  FirstRun,
  Repeat
};

class UPnPEnums
{
public:
  static bool RadioBandToString(UPnPRadioBand radioBand, std::string& radioBandStr);
  static UPnPRadioBand RadioBandFromString(const std::string& radioBandStr);

  static bool DayOfWeekToString(UPnPDayOfWeek dayOfWeek, std::string& dayOfWeekStr);
  static UPnPDayOfWeek DayOfWeekFromString(const std::string& dayOfWeekStr);

  static bool RecordedEpisodeTypeToString(UPnPRecordedEpisodeType recordedEpisodeType, std::string& recordedEpisodeTypeStr);
  static UPnPRecordedEpisodeType RecordedEpisodeTypeFromString(const std::string& recordedEpisodeTypeStr);

private:
  UPnPEnums() = delete;
  ~UPnPEnums() = delete;
};
