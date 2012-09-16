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

#include "interfaces/generic/ILanguageInvoker.h"
#include "threads/Event.h"

class CJavaScriptInvoker : public ILanguageInvoker
{
public:
  CJavaScriptInvoker();
  virtual ~CJavaScriptInvoker();

  virtual bool Execute(const std::string &script, const std::vector<std::string> &arguments = std::vector<std::string>());
  virtual bool Stop(bool wait = false);

private:
  bool initializeBridge();

  static v8::Handle<v8::Value> getAbortRequested(v8::Local<v8::String> property, const v8::AccessorInfo& info);

  v8::Handle<v8::ObjectTemplate> m_tmplGlobal;
  v8::Persistent<v8::Context> m_context;
  v8::Isolate *m_isolate;

  bool m_stop;
  CEvent m_stoppedEvent;
};
