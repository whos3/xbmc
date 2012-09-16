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


#include "LanguageHook.h"

#include "interfaces/legacy/AddonUtils.h"
#include "utils/GlobalsHandling.h"

namespace XBMCAddon
{
  namespace JavaScript
  {
    static AddonClass::Ref<LanguageHook> instance;

    static CCriticalSection ccrit;
    static bool isInited = false;
    static xbmcutil::InitFlag flag(isInited);

    // vtab instantiation
    LanguageHook::~LanguageHook() { }

    struct MutableInteger
    {
      MutableInteger() : value(0) {}
      int value;
    };

    LanguageHook* LanguageHook::getInstance() 
    {
      if (!isInited) // in this case we're being called from a static initializer
      {
        if (instance.isNull())
          instance = new LanguageHook();
      }
      else
      {
        CSingleLock lock (ccrit);
        if (instance.isNull())
          instance = new LanguageHook();
      }

      return instance.get();
    }

    String LanguageHook::getAddonId()
    {
      // TODO
      return emptyString;
    }

    String LanguageHook::getAddonVersion()
    {
      // TODO
      return emptyString;
    }

    void LanguageHook::registerPlayerCallback(IPlayerCallback* player) { /* TODO */ }
    void LanguageHook::unregisterPlayerCallback(IPlayerCallback* player) { /* TODO */ }
    void LanguageHook::registerMonitorCallback(XBMCAddon::xbmc::Monitor* monitor) { /* TODO */ }
    void LanguageHook::unregisterMonitorCallback(XBMCAddon::xbmc::Monitor* monitor) { /* TODO */ }
    void LanguageHook::waitForEvent(CEvent& hEvent) { /* TODO */ }
  }
}
