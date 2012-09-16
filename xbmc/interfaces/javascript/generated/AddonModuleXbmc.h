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

#include <v8.h>

#include "addons/IAddon.h"

namespace JavaScriptBindings
{
  class Xbmc
  {
  public:
    Xbmc(ADDON::AddonPtr addon);
    static v8::Handle<v8::FunctionTemplate> Init(v8::Isolate *isolate);

  private:
    static v8::Handle<v8::Value> log(const v8::Arguments& args);

    static v8::Handle<v8::Value> getAddonId(v8::Local<v8::String> property, const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> getApiVersion(v8::Local<v8::String> property, const v8::AccessorInfo& info);

    ADDON::AddonPtr m_addon;
  };
}
