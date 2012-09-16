#pragma once
/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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
#include <vector>

#include "addons/IAddon.h"

typedef enum {
  InvokerStateUninitialized,
  InvokerStateInitialized,
  InvokerStateRunning,
  InvokerStateStopping,
  InvokerStateDone,
  InvokerStateFailed
} InvokerState;

class ILanguageInvoker
{
public:
  ILanguageInvoker()
    : m_id(-1), m_state(InvokerStateUninitialized)
  { }
  virtual ~ILanguageInvoker() { }

  virtual bool Execute(const std::string &script, const std::vector<std::string> &arguments = std::vector<std::string>()) = 0;
  virtual bool Stop(bool abort = false) = 0;
  
  void SetId(int id) { m_id = id; }
  int GetId() const { return m_id; }
  void SetAddon(ADDON::AddonPtr addon) { m_addon = addon; }
  virtual InvokerState GetState() { return m_state; }
  virtual bool IsActive() { return GetState() > InvokerStateUninitialized && GetState() < InvokerStateDone; }
  virtual bool IsRunning() { return GetState() == InvokerStateRunning; }
  virtual bool IsStopping() { return GetState() == InvokerStateStopping; }

  virtual void OnError() { }
  virtual void OnDone() { }

protected:
  void setState(InvokerState state)
  {
    if (state <= m_state)
      return;

    m_state = state;
  }
  
  ADDON::AddonPtr m_addon;

private:
  int m_id;
  InvokerState m_state;
};
