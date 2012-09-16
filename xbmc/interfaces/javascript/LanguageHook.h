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

#pragma once

#include "interfaces/legacy/LanguageHook.h"
#include "threads/ThreadLocal.h"
#include "threads/Event.h"

namespace XBMCAddon
{
  namespace JavaScript
  {
    struct MutableInteger;

    /**
     * This class supplies the python specific functionality for
     *  plugging into the API. It's got a static only implementation
     *  and uses the singleton pattern for access.
     */
    class LanguageHook : public XBMCAddon::LanguageHook
    {
      LanguageHook() : XBMCAddon::LanguageHook("JavaScript::LanguageHook")  {  }

      XbmcThreads::ThreadLocal<MutableInteger> tlsCount;
    public:

      virtual ~LanguageHook();

      // TODO: virtual void delayedCallOpen();
      // TODO: virtual void delayedCallClose();

      // TODO: virtual XBMCAddon::CallbackHandler* getCallbackHandler();

      virtual String getAddonId();
      virtual String getAddonVersion();

      virtual void registerPlayerCallback(IPlayerCallback* player);
      virtual void unregisterPlayerCallback(IPlayerCallback* player);
      virtual void registerMonitorCallback(XBMCAddon::xbmc::Monitor* monitor);
      virtual void unregisterMonitorCallback(XBMCAddon::xbmc::Monitor* monitor);
      virtual void waitForEvent(CEvent& hEvent);

      static LanguageHook* getInstance();
    };
  }
}

