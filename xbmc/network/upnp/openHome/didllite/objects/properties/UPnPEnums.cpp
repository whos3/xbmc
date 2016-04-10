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

#include <algorithm>
#include <set>

#include "UPnPEnums.h"

static const std::set<std::pair<UPnPRadioBand, std::string>> RadioBands = {
  { UPnPRadioBand::AM, "AM" },
  { UPnPRadioBand::FM, "FM" },
  { UPnPRadioBand::Shortwave, "Shortwave" },
  { UPnPRadioBand::Internet, "Internet" },
  { UPnPRadioBand::Sattelite, "Sattelite" },
  { UPnPRadioBand::VendorSpecific, "" }
};

static const std::set<std::pair<UPnPDayOfWeek, std::string>> DayOfWeek = {
  { UPnPDayOfWeek::Sunday, "SUN" },
  { UPnPDayOfWeek::Monday, "MON" },
  { UPnPDayOfWeek::Tuesday, "TUE" },
  { UPnPDayOfWeek::Wednesday, "WED" },
  { UPnPDayOfWeek::Thursday, "THU" },
  { UPnPDayOfWeek::Friday, "FRI" },
  { UPnPDayOfWeek::Saturday, "SAT" }
};

static const std::set<std::pair<UPnPRecordedEpisodeType, std::string>> RecordedEpisodeType = {
  { UPnPRecordedEpisodeType::All, "ALL" },
  { UPnPRecordedEpisodeType::FirstRun, "FIRST-RUN" },
  { UPnPRecordedEpisodeType::Repeat, "REPEAT" }
};

template<class TKey, class TValue>
bool FindEnumValue(const std::set<std::pair<TKey, TValue>>& enumValues, const TKey& key, TValue& value)
{
  const auto& it = std::find_if(enumValues.cbegin(), enumValues.cend(),
    [&key](const std::pair<TKey, TValue>& enumValue)
    {
      return enumValue.first == key;
    });

  if (it != enumValues.cend())
  {
    value = it->second;
    return true;
  }

  return false;
}

template<class TKey, class TValue>
bool FindEnumKey(const std::set<std::pair<TKey, TValue>>& enumValues, const TValue& value, TKey& key)
{
  const auto& it = std::find_if(enumValues.cbegin(), enumValues.cend(),
    [&value](const std::pair<TKey, TValue>& enumValue)
  {
    return enumValue.second == value;
  });

  if (it != enumValues.cend())
  {
    key = it->first;
    return true;
  }

  return false;
}

bool UPnPEnums::RadioBandToString(UPnPRadioBand radioBand, std::string& radioBandStr)
{
  return FindEnumValue(RadioBands, radioBand, radioBandStr);
}

UPnPRadioBand UPnPEnums::RadioBandFromString(const std::string& radioBandStr)
{
  UPnPRadioBand radioBand = UPnPRadioBand::VendorSpecific;
  FindEnumKey(RadioBands, radioBandStr, radioBand);

  return radioBand;
}

bool UPnPEnums::DayOfWeekToString(UPnPDayOfWeek dayOfWeek, std::string& dayOfWeekStr)
{
  return FindEnumValue(DayOfWeek, dayOfWeek, dayOfWeekStr);
}

UPnPDayOfWeek UPnPEnums::DayOfWeekFromString(const std::string& dayOfWeekStr)
{
  UPnPDayOfWeek dayOfWeek = UPnPDayOfWeek::Sunday;
  FindEnumKey(DayOfWeek, dayOfWeekStr, dayOfWeek);

  return dayOfWeek;
}

bool UPnPEnums::RecordedEpisodeTypeToString(UPnPRecordedEpisodeType recordedEpisodeType, std::string& recordedEpisodeTypeStr)
{
  return FindEnumValue(RecordedEpisodeType, recordedEpisodeType, recordedEpisodeTypeStr);
}
UPnPRecordedEpisodeType UPnPEnums::RecordedEpisodeTypeFromString(const std::string& recordedEpisodeTypeStr)
{
  UPnPRecordedEpisodeType recordedEpisodeType = UPnPRecordedEpisodeType::All;
  FindEnumKey(RecordedEpisodeType, recordedEpisodeTypeStr, recordedEpisodeType);

  return recordedEpisodeType;
}
