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

#include "JavaScriptInvoker.h"
#include "swig.h"
#include "generated/AddonModuleXbmc.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "filesystem/File.h"
#include "guilib/GraphicContext.h"
#include "utils/log.h"

// Time before ill-behaved scripts are terminated
#define JS_SCRIPT_TIMEOUT 5000 // ms

using namespace v8;
using namespace JavaScriptBindings;

CJavaScriptInvoker::CJavaScriptInvoker()
  : ILanguageInvoker(),
    m_isolate(NULL), m_stop(false)
{
  // TODO
}

CJavaScriptInvoker::~CJavaScriptInvoker()
{
  Stop(true);
}

bool CJavaScriptInvoker::Execute(const std::string &script, const std::vector<std::string> &arguments /* = std::vector<std::string>() */)
{
  if (script.empty() || !XFILE::CFile::Exists(script, false))
    return false;

  std::string code;
  XFILE::CFile file;
  if (!file.Open(script))
    return false;

  char buffer[1024];
  int read = 0;
  while ((read = file.Read(buffer, 1023)) > 0)
  {
    buffer[read] = '\0';
    code.append(buffer);
  }
  
  // Get a new isolate for this thread
  m_isolate = Isolate::New();
  if (m_isolate == NULL)
    CLog::Log(LOGWARNING, "CJavaScriptInvoker: unable to retrieve a thread-specific isolate");
  
  {
    // get a v8 lock for this execution
    Locker lock(m_isolate);
  
    Isolate::Scope isolate_scope(m_isolate);

    // Create a stack-allocated handle scope.
    HandleScope handle_scope;

    // Create a new global object
    m_tmplGlobal = ObjectTemplate::New();
    m_tmplGlobal->SetInternalFieldCount(1);
    m_tmplGlobal->SetAccessor(V8_STR("abortRequested"), getAbortRequested);

    // Create a new context.
    m_context = Context::New(NULL, m_tmplGlobal);

    {
      // Enter the created context for compiling and
      // running the hello world script. 
      Context::Scope context_scope(m_context);

      Local<Object> objGlobal = m_tmplGlobal->NewInstance();
      objGlobal->SetInternalField(0, External::New(this));
      m_context->Global()->Set(String::New("script"), objGlobal);

      // expose objects and methods to v8
      if (!initializeBridge())
        return false;

      setState(InvokerStateInitialized);
    
      // make the code available for v8
      Handle<String> source = V8_STR(code.c_str());

      // Compile the source code.
      TryCatch tryCatch;
      Handle<Script> script = Script::Compile(source);
      if (script.IsEmpty()) 
      {
        CLog::Log(LOGERROR, "CJavaScriptInvoker: error during compilation");
        setState(InvokerStateFailed);

        JavaScriptToCppException e(tryCatch);
        e.LogThrowMessage();
        return false;
      }

      setState(InvokerStateRunning);
      Handle<Value> result = script->Run();
      if (result.IsEmpty()) 
      {
        CLog::Log(LOGERROR, "CJavaScriptInvoker: error during execution");
        setState(InvokerStateFailed);

        JavaScriptToCppException e(tryCatch);
        e.LogThrowMessage();
        return false;
      }

      CLog::Log(LOGDEBUG, "CJavaScriptInvoker: Result = %s", *String::Utf8Value(result));
    }

    if (!m_context.IsEmpty())
      m_context.Dispose();
  }

  setState(InvokerStateDone);
  m_stoppedEvent.Set();

  m_isolate->Dispose();

  return true;
}

bool CJavaScriptInvoker::Stop(bool wait /* = false */)
{
  m_stop = true;

  if (!IsRunning())
    return false;

  setState(InvokerStateStopping);
  
  // TODO: call OnAbortRequested
  
  XbmcThreads::EndTime timeout(JS_SCRIPT_TIMEOUT);
  while (!m_stoppedEvent.WaitMSec(15))
  {
    if (timeout.IsTimePast())
    {
      CLog::Log(LOGERROR, "CJavaScriptInvoker: script didn't stop in %d seconds - let's kill it", JS_SCRIPT_TIMEOUT / 1000);
      V8::TerminateExecution(m_isolate);
      break;
    }
    // We can't empty-spin in the main thread and expect scripts to be able to
    // dismantle themselves. Addon dialogs aren't normal XBMC dialogs, they rely
    // on TMSG_GUI_PYTHON_DIALOG messages, so pump the message loop.
    if (g_application.IsCurrentThread())
    {
      CSingleExit ex(g_graphicsContext);
      CApplicationMessenger::Get().ProcessMessages();
    }
  }
  // Useful for add-on performance metrics
  if (!timeout.IsTimePast())
    CLog::Log(LOGDEBUG, "CJavaScriptInvoker: script termination took %d ms", JS_SCRIPT_TIMEOUT - timeout.MillisLeft());

  // TODO: cleanup

  return true;
}

bool CJavaScriptInvoker::initializeBridge()
{
  {
    Locker lock(m_isolate);
    Isolate::Scope isolate_scope(m_isolate);
    HandleScope handle_scope;

    // demo: xbmc module
    Handle<FunctionTemplate> tmplXbmc = JavaScriptBindings::Xbmc::Init(m_isolate);

    JavaScriptBindings::Xbmc *xbmc = new JavaScriptBindings::Xbmc(m_addon);

    // Pass the Point object to v8
    Handle<Function> ctorXbmc = tmplXbmc->GetFunction();
    Local<Object> objXbmc = ctorXbmc->NewInstance();
    objXbmc->SetInternalField(0, External::New(xbmc));

    m_context->Global()->Set(String::New("xbmc"), objXbmc);
  }

  return true;
}

V8_GETTER(CJavaScriptInvoker::getAbortRequested)
{
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();

  return Boolean::New(static_cast<CJavaScriptInvoker*>(ptr)->m_stop);
}
