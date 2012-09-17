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

#include <v8-debug.h>

#include "JavaScriptDebugger.h"

#define APP_NAME  "xbmc"

using namespace v8;

CJavaScriptDebugger::CJavaScriptDebugger()
  : m_initialized(false), m_port(5858)
{ }

CJavaScriptDebugger::~CJavaScriptDebugger()
{
  Uninitialize();
}

CJavaScriptDebugger& CJavaScriptDebugger::Get()
{
  static CJavaScriptDebugger s_instance;
  return s_instance;
}

bool CJavaScriptDebugger::Initialize(uint16_t port /* = 5858 */)
{
  if (m_initialized)
  {
    if (m_port == port)
      return true;

    Uninitialize();
  }

  m_port = port;

  /* TODO: get a proper context
  m_debugContext = Persistent<Context>::New(context);
  */

  Debug::SetDebugMessageDispatchHandler(debugMessageHandler, true);
    
  if (!Debug::EnableAgent(APP_NAME, m_port, true))
    return false;

  m_initialized = true;
  return true;
}

void CJavaScriptDebugger::Uninitialize()
{
  if (!m_initialized)
    return;

  Debug::DisableAgent();
}

void CJavaScriptDebugger::debugMessageHandler()
{
  Context::Scope scope(Debug::GetDebugContext());

  Debug::ProcessDebugMessages();
}
