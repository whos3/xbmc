/*
 *      Copyright (C) 2015 Team XBMC
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

#include "DidlLiteUtils.h"
#include "utils/StringUtils.h"

bool DidlLiteUtils::IsAttributeName(const std::string& name)
{
  return StringUtils::StartsWith(name, "@");
}

std::string DidlLiteUtils::GetElementName(const std::string& ns, const std::string& name)
{
  std::string elementName = ns;
  if (!ns.empty() && !name.empty())
    elementName += ":";
  elementName += name;

  return elementName;
}

std::string DidlLiteUtils::GetAttributeName(const std::string& name)
{
  std::string attributeName = name;
  if (IsAttributeName(name))
    return StringUtils::Mid(name, 1);

  return name;
}

std::string DidlLiteUtils::GetDate(const CDateTime& date)
{
  return date.GetAsW3CDate();
}

CDateTime DidlLiteUtils::GetDate(const std::string& date)
{
  CDateTime dateObj;
  dateObj.SetFromW3CDate(date);

  return dateObj;
}

std::string DidlLiteUtils::GetDateTime(const CDateTime& dateTime)
{
  return dateTime.GetAsW3CDateTime();
}

CDateTime DidlLiteUtils::GetDateTime(const std::string& dateTime)
{
  CDateTime dateTimeObj;
  dateTimeObj.SetFromW3CDateTime(dateTime);

  return dateTimeObj;
}

int64_t DidlLiteUtils::GetDurationInSeconds(const std::string& duration)
{
  if (StringUtils::IsNaturalNumber(duration))
    return strtol(duration.c_str(), NULL, 0);

  int64_t durationSeconds = -1;

  std::string tmpDuration = duration;

  // try to parse the "P[0-9]D[0-9]{2}:[0-9]{2}:[0-9]" time format defined in the UPnP specification
  unsigned int days = 0;
  if (StringUtils::StartsWith(tmpDuration, "P"))
  {
    // remove the leading "P"
    std::string tmpDuration = StringUtils::Mid(tmpDuration, 1);

    // check if the next character is a natural number (0-9)
    if (StringUtils::IsNaturalNumber(std::string(1, tmpDuration.at(0))))
    {
      // extract the number of days
      days = static_cast<unsigned int>(strtoul(tmpDuration.c_str(), NULL, 0));

      // remove the number of days
      tmpDuration = StringUtils::Mid(tmpDuration, 1);

      // check if the next character is a "D" and remove it
      if (StringUtils::StartsWith(tmpDuration, "D"))
        tmpDuration = StringUtils::Mid(tmpDuration, 1);
      else
        days = 0;
    }
  }

  // try to parse the HH:MM:SS format
  CDateTimeSpan time;
  if (time.SetFromTimeString(tmpDuration))
  {
    time.SetDateTimeSpan(days + time.GetDays(), time.GetHours(), time.GetMinutes(), time.GetSeconds());

    durationSeconds = time.GetSecondsTotal();
  }

  return durationSeconds;
}

std::string DidlLiteUtils::GetDurationFromSeconds(int64_t duration)
{
  std::string strDuration;
  if (duration < 0)
    return "";

  CDateTimeSpan time(0, 0, 0, static_cast<int>(duration));

  return StringUtils::Format("P%dD%02d:%02d:%02d", time.GetDays(), time.GetHours(), time.GetMinutes(), time.GetSeconds());
}
