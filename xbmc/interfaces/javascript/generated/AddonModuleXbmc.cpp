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

#include "AddonModuleXbmc.h"
#include "interfaces/javascript/LanguageHook.h"
#include "commons/Exception.h"
#include "interfaces/legacy/ModuleXbmc.h"

#include "utils/log.h"

using namespace v8;
using namespace ADDON;

namespace JavaScriptBindings
{
  Xbmc::Xbmc(AddonPtr addon)
    : m_addon(addon)
  { }

  Handle<FunctionTemplate> Xbmc::Init(v8::Isolate *isolate)
  {
    Locker lock(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope;

    // Create a new function template
    Handle<FunctionTemplate> tmplXbmc = FunctionTemplate::New();
    tmplXbmc->SetClassName(String::New("xbmc"));

    // Get the ObjectTemplate
    Local<ObjectTemplate> instXbmc = tmplXbmc->InstanceTemplate();
    instXbmc->SetInternalFieldCount(1);
    instXbmc->SetAccessor(String::New("addonid"), getAddonId);
    instXbmc->SetAccessor(String::New("apiversion"), getApiVersion);

    // Get the prototype template
    Handle<ObjectTemplate> prtXbmc = tmplXbmc->PrototypeTemplate();
    prtXbmc->Set("log", FunctionTemplate::New(Xbmc::log));

    return handle_scope.Close(tmplXbmc);
  }

  V8_FUNCTION(Xbmc::log)
  {
    if (args.Length() > 0 && args[0]->IsString())
    {
      String::Utf8Value msgValue(args[0]);
      std::string msg = *msgValue;

      int level = LOGNOTICE;
      if (args.Length() > 1 && args[1]->IsInt32())
        level = args[1]->Int32Value();

      try
      {
        XBMCAddon::SetLanguageHookGuard slhg(XBMCAddon::JavaScript::LanguageHook::getInstance());
        XBMCAddon::xbmc::log(msg.c_str(), level);
      }
      catch (const XbmcCommons::Exception& e)
      { 
        CLog::Log(LOGERROR, "EXCEPTION: %s", e.GetMessage());
        // TODO: create a javascript exception
      }
      catch (...)
      {
        CLog::Log(LOGERROR, "EXCEPTION: Unknown exception thrown from the call \"XBMCAddon::xbmc::log\"");
        // TODO: create a javascript exception
      }
    }
    
    return Handle<Value>();
  }

  V8_GETTER(Xbmc::getAddonId)
  {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    return String::New(static_cast<Xbmc*>(ptr)->m_addon->ID().c_str());
  }

  V8_GETTER(Xbmc::getApiVersion)
  {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    return String::New(ADDON::GetXbmcApiVersionDependency(static_cast<Xbmc*>(ptr)->m_addon).c_str());
  }
}
