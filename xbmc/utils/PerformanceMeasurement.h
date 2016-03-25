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

#include <chrono>

#include "utils/StringUtils.h"

template<class TClock = std::chrono::high_resolution_clock>
class CPerformanceMeasurement
{
public:
  CPerformanceMeasurement()
    : m_start(TClock::now())
  { }

  ~CPerformanceMeasurement()
  {
    Stop();
  }

  void Stop()
  {
    m_duration = GetCurrentDuration();
  }

  void Restart()
  {
    m_start = TClock::now();
  }

  typename TClock::duration GetCurrentDuration() const
  {
    return TClock::now() - m_start;
  }

  typename TClock::duration GetDuration() const
  {
    return m_duration;
  }

  double GetCurrentDurationInSeconds() const
  {
    return GetDurationInSeconds(GetCurrentDuration());
  }

  double GetDurationInSeconds() const
  {
    return GetDurationInSeconds(m_duration);
  }

private:
  static double GetDurationInSeconds(const typename TClock::duration& duration)
  {
    return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
  }

  typename TClock::time_point m_start;
  typename TClock::duration m_duration;
};
