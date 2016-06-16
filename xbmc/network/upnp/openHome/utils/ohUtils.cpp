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

#include "ohUtils.h"
#include "utils/StringUtils.h"

static const std::string CSVSeparator = ",";

std::string COhUtils::TIpAddressToString(TIpAddress address)
{
  return StringUtils::Format("%d.%d.%d.%d", (address & 0xFF), ((address >> 8) & 0xFF), ((address >> 16) & 0xFF), ((address >> 24) & 0xFF));
}

int64_t COhUtils::GetDurationInSeconds(const std::string& duration)
{
  if (StringUtils::IsNaturalNumber(duration))
    return strtol(duration.c_str(), NULL, 0);

  int64_t durationSeconds = -1;

  std::string tmpDuration = duration;

  // handle the case where the hours are defined with a single digit
  if (tmpDuration.find(':') == 1)
    tmpDuration = "0" + tmpDuration;

  // try to parse the "[0-9]+:[0-9]{2}:[0-9]{2}" time format defined in the UPnP specification
  CDateTimeSpan time;
  if (time.SetFromTimeString(tmpDuration))
    durationSeconds = time.GetSecondsTotal();

  return durationSeconds;
}

std::string COhUtils::GetDurationFromSeconds(int64_t duration)
{
  std::string strDuration;
  if (duration < 0)
    return "";

  CDateTimeSpan time(0, 0, 0, static_cast<int>(duration));

  return StringUtils::Format("%02d:%02d:%02d", time.GetHours(), time.GetMinutes(), time.GetSeconds());
}

std::string COhUtils::ToCSV(const std::vector<std::string>& list)
{
  return StringUtils::Join(list, CSVSeparator);
}

std::vector<std::string> COhUtils::SplitCSV(const std::string& csvList)
{
  return StringUtils::Split(csvList, CSVSeparator);
}