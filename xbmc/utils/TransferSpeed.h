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

#include <algorithm>

#include "threads/CriticalSection.h"
#include "threads/SingleLock.h"
#include "threads/Timer.h"
#include "utils/PerformanceMeasurement.h"

template<class TClock = std::chrono::high_resolution_clock>
class CTransferSpeed : public ITimerCallback
{
public:
  CTransferSpeed(uint64_t totalSizeBytes, uint32_t updateIntervalMs = 250)
    : CTransferSpeed(totalSizeBytes, 0LL, updateIntervalMs)
  { }
  CTransferSpeed(uint64_t totalSizeBytes, uint64_t initialBytesTransferred, uint32_t updateIntervalMs = 250)
    : m_total(totalSizeBytes)
    , m_transferred(0LL)
    , m_lastTransferred(0LL)
    , m_currentSpeed(0.0)
    , m_averageSpeed(0.0)
    , m_timeMeasurement()
    , m_updateIntervalMs(updateIntervalMs)
    , m_updateTimer(this)
  {
    Start(initialBytesTransferred);
  }
  ~CTransferSpeed() = default;

  void Start(uint64_t initialBytesTransferred)
  {
    CSingleLock lock(m_critical);

    if (m_updateTimer.IsRunning())
      return;

    // set the bytes that have already been initially transferred
    m_transferred = std::min(initialBytesTransferred, m_total);

    // nothing else to do if we reached the end
    if (m_total > 0 && m_transferred >= m_total)
      return;

    m_timeMeasurement.Restart();
    m_updateTimer.Start(m_updateIntervalMs, true);
  }

  void Transferred(uint64_t bytesTransferred)
  {
    CSingleLock lock(m_critical);

    // nothing to do
    //   if nothing has been transferred or
    //   if the total size has already been reached
    if (bytesTransferred == 0 ||
       (m_total > 0 && m_transferred >= m_total))
      return;

    // make sure that we don't go past the end
    bool update = false;
    m_transferred += bytesTransferred;
    if (m_total == 0 || m_transferred <= m_total)
      m_lastTransferred += bytesTransferred;
    else
    {
      m_lastTransferred += m_total - m_transferred;
      m_transferred = m_total;
    }
  }

  inline double GetCurrentTransferSpeed() const { return m_currentSpeed; }
  inline double GetAverageTransferSpeed() const { return m_averageSpeed; }

  inline uint64_t GetRemainingSeconds() const
  {
    CSingleLock lock(m_critical);

    if (m_total == 0)
      return 0;

    return static_cast<uint64_t>(static_cast<double>(m_total - m_transferred) / m_averageSpeed);
  }

protected:
  // implementation of ITimerCallback
  virtual void OnTimeout() override
  {
    static const double CurrentSpeedSmoothingFactor = 0.4;
    static const double AverageSpeedSmoothingFactor = 0.05;

    CSingleLock lock(m_critical);

    // get the time since the last update
    m_timeMeasurement.Stop();
    double duration = m_timeMeasurement.GetDurationInSeconds();

    // calculate the speed of the reported transfer
    double lastTransferSpeed = static_cast<double>(m_lastTransferred) / duration;

    // calculate the current speed and the average speed
    UpdateSpeed(m_currentSpeed, lastTransferSpeed, CurrentSpeedSmoothingFactor);
    UpdateSpeed(m_averageSpeed, lastTransferSpeed, AverageSpeedSmoothingFactor);

    m_lastTransferred = 0LL;

    // nothing else to do if we reached the end
    if (m_total > 0 && m_transferred >= m_total)
      return;

    // restart the time measurement
    m_timeMeasurement.Restart();
  }

private:
  static inline void UpdateSpeed(double& speed, const double lastSpeed, const double smoothingFactor)
  {
    speed = smoothingFactor * lastSpeed + (1 - smoothingFactor) * speed;
  }

  uint64_t m_total;
  uint64_t m_transferred;
  uint64_t m_lastTransferred;

  double m_currentSpeed;
  double m_averageSpeed;

  CPerformanceMeasurement<TClock> m_timeMeasurement;

  uint32_t m_updateIntervalMs;
  CTimer m_updateTimer;
  CCriticalSection m_critical;
};
