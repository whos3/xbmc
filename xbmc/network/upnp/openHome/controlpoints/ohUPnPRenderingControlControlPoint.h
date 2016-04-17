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

#include <OpenHome/Net/Cpp/CpUpnpOrgRenderingControl2.h>

#include "network/upnp/openHome/controlpoints/ohUPnPControlPoint.h"
#include "network/upnp/openHome/utils/ohUPnPRenderingControl.h"

class COhUPnPRenderingControlControlPoint;

class IOhUPnPRenderingControlControlPointAsync
{
public:
  virtual ~IOhUPnPRenderingControlControlPointAsync() = default;

  virtual void GetMuteResult(bool success, bool mute) { }
  virtual void SetMuteResult(bool success) { }
  virtual void GetVolumeResult(bool success, uint32_t volume) { }
  virtual void SetVolumeResult(bool success) { }
  virtual void GetVolumeDBResult(bool success, int32_t volumeDB) { }
  virtual void GetVolumeDBRangeResult(bool success, int32_t minVolumeDB, int32_t maxVolumeDB) { }
  virtual void SetVolumeDBResult(bool success) { }

protected:
  IOhUPnPRenderingControlControlPointAsync() = default;
};

class COhUPnPRenderingControlControlPointManager
  : public IOhUPnPControlPointManager<OpenHome::Net::CpProxyUpnpOrgRenderingControl2Cpp, COhUPnPRenderingControlControlPointManager, COhUPnPRenderingControlControlPoint, IOhUPnPRenderingControlControlPointAsync>
{
public:
  COhUPnPRenderingControlControlPointManager(const std::string& deviceType);
  virtual ~COhUPnPRenderingControlControlPointManager();

protected:
  friend class COhUPnPRenderingControlControlPoint;

  bool GetMuteSync(const std::string& uuid, bool& mute, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetMuteAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetMuteAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, bool& mute) const;

  bool SetMuteSync(const std::string& uuid, bool mute, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetMuteAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, bool mute, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetMuteAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool GetVolumeSync(const std::string& uuid, uint32_t& volume, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, uint32_t& volume) const;

  bool SetVolumeSync(const std::string& uuid, uint32_t volume, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetVolumeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, uint32_t volume, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetVolumeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  bool GetVolumeDBSync(const std::string& uuid, int32_t& volumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeDBAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeDBAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, int32_t& volumeDB) const;

  bool GetVolumeDBRangeSync(const std::string& uuid, int32_t& minVolumeDB, int32_t& maxVolumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeDBRangeAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeDBRangeAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid, int32_t& minVolumeDB, int32_t& maxVolumeDB) const;

  bool SetVolumeDBSync(const std::string& uuid, int32_t volumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetVolumeDBAsync(OpenHome::Net::FunctorAsync& callback, const std::string& uuid, int32_t volumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetVolumeDBAsyncResult(OpenHome::Net::IAsync& result, const std::string& uuid) const;

  // TODO
};

class COhUPnPRenderingControlControlPoint : public IOhUPnPControlPointInstance<COhUPnPRenderingControlControlPointManager, IOhUPnPRenderingControlControlPointAsync>
{
public:
  virtual ~COhUPnPRenderingControlControlPoint() = default;

  // implementation of IOhUPnPControlPointInstance
  const COhUPnPDeviceProfile& GetProfile() const override { return m_profile; }

  bool GetMuteSync(bool& mute, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetMuteAsync(COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0);

  bool SetMuteSync(bool mute, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetMuteAsync(bool mute, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0);

  bool GetVolumeSync(uint32_t& volume, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeAsync(COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0);

  bool SetVolumeSync(uint32_t volume, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetVolumeAsync(uint32_t volume, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0);

  bool GetVolumeDBSync(int32_t& volumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeDBAsync(COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0);

  bool GetVolumeDBRangeSync(int32_t& minVolumeDB, int32_t& maxVolumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool GetVolumeDBRangeAsync(COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0);

  bool SetVolumeDBSync(int32_t volumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0) const;
  bool SetVolumeDBAsync(int32_t volumeDB, COhUPnPRenderingControlChannel channel = OhUPnPRenderingControlChannel::Master, uint32_t instanceId = 0);

protected:
  friend class IOhUPnPControlPointManager<OpenHome::Net::CpProxyUpnpOrgRenderingControl2Cpp, COhUPnPRenderingControlControlPointManager, COhUPnPRenderingControlControlPoint, IOhUPnPRenderingControlControlPointAsync>;

  COhUPnPRenderingControlControlPoint(const COhUPnPRenderingControlControlPointManager* manager,
    const std::string& uuid, IOhUPnPRenderingControlControlPointAsync* callback);

  void GetMuteCallback(OpenHome::Net::IAsync& result);
  void SetMuteCallback(OpenHome::Net::IAsync& result);
  void GetVolumeCallback(OpenHome::Net::IAsync& result);
  void SetVolumeCallback(OpenHome::Net::IAsync& result);
  void GetVolumeDBCallback(OpenHome::Net::IAsync& result);
  void GetVolumeDBRangeCallback(OpenHome::Net::IAsync& result);
  void SetVolumeDBCallback(OpenHome::Net::IAsync& result);

private:
  COhUPnPDeviceProfile m_profile;
};
