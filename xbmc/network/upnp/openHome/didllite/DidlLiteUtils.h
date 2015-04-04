#pragma once
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

#include <stdint.h>
#include <string>

#include "XBDateTime.h"

class DidlLiteUtils
{
public:
  static bool IsAttributeName(const std::string& name);
  static std::string GetElementName(const std::string& ns, const std::string& name);
  static std::string GetAttributeName(const std::string& name);

  static std::string GetDate(const CDateTime& date);
  static CDateTime GetDate(const std::string& date);
  static std::string GetDateTime(const CDateTime& dateTime);
  static CDateTime GetDateTime(const std::string& dateTime);
  static int64_t GetDurationInSeconds(const std::string& duration);
  static std::string GetDurationFromSeconds(int64_t duration);

private:
  DidlLiteUtils() { }
};