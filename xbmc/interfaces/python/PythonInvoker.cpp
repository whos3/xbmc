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

#include <Python.h>
#include <osdefs.h>

#include "PythonInvoker.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "addons/AddonManager.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "filesystem/File.h"
#include "filesystem/SpecialProtocol.h"
#include "guilib/GraphicContext.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/legacy/CallbackHandler.h"
#include "interfaces/legacy/Exception.h"
#include "interfaces/python/pythreadstate.h"
#include "interfaces/python/swig.h"
#include "interfaces/python/XBPython.h"
#include "utils/CharsetConverter.h"
#include "utils/log.h"
#include "utils/StdString.h"
#include "utils/URIUtils.h"

#ifdef TARGET_WINDOWS
extern "C" FILE *fopen_utf8(const char *_Filename, const char *_Mode);
#else
#define fopen_utf8 fopen
#endif

#define PY_PATH_SEP DELIM

// Time before ill-behaved scripts are terminated
#define PYTHON_SCRIPT_TIMEOUT 5000 // ms

CPythonInvoker::CPythonInvoker()
  : ILanguageInvoker(),
    m_source(NULL), m_argc(0), m_argv(NULL),
    m_threadState(NULL), m_stop(false)
{ }

CPythonInvoker::~CPythonInvoker()
{
  Stop(true);
  g_pythonParser.PulseGlobalEvent();

  delete [] m_source;
  if (m_argv != NULL)
  {
    for (unsigned int i = 0; i < m_argc; i++)
      delete [] m_argv[i];
    delete [] m_argv;
  }
}

bool CPythonInvoker::Execute(const std::string &script, const std::vector<std::string> &arguments /* = std::vector<std::string>() */)
{
  return execute(script, arguments);
}

bool CPythonInvoker::Stop(bool wait /* = false */)
{
  CSingleLock lock(g_pythonParser.m_critSection);
  m_stop = true;

  if (!IsRunning())
    return false;

  setState(InvokerStateStopping);

  if (m_threadState)
  {
    PyEval_AcquireLock();
    PyThreadState* old = PyThreadState_Swap((PyThreadState*)m_threadState);

    //tell xbmc.Monitor to call onAbortRequested()
    if (m_addon)
      g_pythonParser.OnAbortRequested(m_addon->ID());

    PyObject *m;
    m = PyImport_AddModule((char*)"xbmc");
    if(!m || PyObject_SetAttrString(m, (char*)"abortRequested", PyBool_FromLong(1)))
      CLog::Log(LOGERROR, "CPythonInvoker: failed to set abortRequested");

    PyThreadState_Swap(old);
    PyEval_ReleaseLock();

    XbmcThreads::EndTime timeout(PYTHON_SCRIPT_TIMEOUT);
    while (!m_stoppedEvent.WaitMSec(15))
    {
      if (timeout.IsTimePast())
      {
        CLog::Log(LOGERROR, "CPythonInvoker: script didn't stop in %d seconds - let's kill it", PYTHON_SCRIPT_TIMEOUT / 1000);
        break;
      }
      // We can't empty-spin in the main thread and expect scripts to be able to
      // dismantle themselves. Python dialogs aren't normal XBMC dialogs, they rely
      // on TMSG_GUI_PYTHON_DIALOG messages, so pump the message loop.
      if (g_application.IsCurrentThread())
      {
        CSingleExit ex(g_graphicsContext);
        CApplicationMessenger::Get().ProcessMessages();
      }
    }
    // Useful for add-on performance metrics
    if (!timeout.IsTimePast())
      CLog::Log(LOGDEBUG, "CPythonInvoker: script termination took %d ms", PYTHON_SCRIPT_TIMEOUT - timeout.MillisLeft());
    
    //everything which didn't exit by now gets killed
    PyEval_AcquireLock();
    old = PyThreadState_Swap((PyThreadState*)m_threadState);    
    for(PyThreadState* state = ((PyThreadState*)m_threadState)->interp->tstate_head; state; state = state->next)
    {
      // Raise a SystemExit exception in python threads
      Py_XDECREF(state->async_exc);
      state->async_exc = PyExc_SystemExit;
      Py_XINCREF(state->async_exc);
    }
    // If a dialog entered its doModal(), we need to wake it to see the exception
    g_pythonParser.PulseGlobalEvent();

    PyThreadState_Swap(old);
    PyEval_ReleaseLock();
  }

  return true;
}

void CPythonInvoker::OnDone()
{
  g_pythonParser.setDone(GetId());
}

void CPythonInvoker::OnError()
{
  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  setState(InvokerStateFailed);
  
  CLog::Log(LOGERROR, "CPythonInvoker: abnormally terminating python thread");
  CSingleLock lock(g_pythonParser.m_critSection);
  m_threadState = NULL;
  OnDone();
}

bool CPythonInvoker::execute(const std::string &code, const std::vector<std::string> &arguments)
{
  if (code.empty())
    return false;

  if (!XFILE::CFile::Exists(code))
  {
    CLog::Log(LOGERROR, "CPythonInvoker: python script \"%s\" does not exist", CSpecialProtocol::TranslatePath(code).c_str());
    return false;
  }

  if (!g_pythonParser.Initialize())
    return false;

  // copy the code/script into a local string buffer
#ifdef TARGET_WINDOWS
  CStdString strsrc = code;
  g_charsetConverter.utf8ToSystem(strsrc);
  m_source  = new char[strsrc.length() + 1];
  strcpy(m_source, strsrc);
#else
  m_source  = new char[code.length() + 1];
  strcpy(m_source, code.c_str());
#endif

  // copy the arguments into a local buffer
  m_argc = arguments.size();
  m_argv = new char*[m_argc];
  for(unsigned int i = 0; i < m_argc; i++)
  {
    m_argv[i] = new char[arguments.at(i).length() + 1];
    strcpy(m_argv[i], arguments.at(i).c_str());
  }

  CLog::Log(LOGDEBUG,"CPythonInvoker: start processing");

  int m_Py_file_input = Py_file_input;

  // get the global lock
  PyEval_AcquireLock();
  PyThreadState* state = Py_NewInterpreter();
  if (!state)
  {
    PyEval_ReleaseLock();
    CLog::Log(LOGERROR,"CPythonInvoker: FAILED to get thread state!");
    return false;
  }
  // swap in my thread state
  PyThreadState_Swap(state);

  g_pythonParser.InitializeInterpreter(m_addon);
  setState(InvokerStateInitialized);

  CLog::Log(LOGDEBUG, "CPythonInvoker: The source file to load is %s", m_source);

  // get path from script file name and add python path's
  // this is used for python so it will search modules from script path first
  CStdString scriptDir;
  URIUtils::GetDirectory(CSpecialProtocol::TranslatePath(m_source), scriptDir);
  URIUtils::RemoveSlashAtEnd(scriptDir);
  addPath(scriptDir);

  // add on any addon modules the user has installed
  ADDON::VECADDONS addons;
  ADDON::CAddonMgr::Get().GetAddons(ADDON::ADDON_SCRIPT_MODULE, addons);
  for (unsigned int i = 0; i < addons.size(); ++i)
    addPath(CSpecialProtocol::TranslatePath(addons[i]->LibPath()));

  // we want to use sys.path so it includes site-packages
  // if this fails, default to using Py_GetPath
  PyObject *sysMod(PyImport_ImportModule((char*)"sys")); // must call Py_DECREF when finished
  PyObject *sysModDict(PyModule_GetDict(sysMod)); // borrowed ref, no need to delete
  PyObject *pathObj(PyDict_GetItemString(sysModDict, "path")); // borrowed ref, no need to delete

  if (pathObj && PyList_Check(pathObj))
  {
    for (int i = 0; i < PyList_Size(pathObj); i++)
    {
      PyObject *e = PyList_GetItem(pathObj, i); // borrowed ref, no need to delete
      if (e && PyString_Check(e))
        addPath(PyString_AsString(e)); // returns internal data, don't delete or modify
    }
  }
  else
    addPath(Py_GetPath());

  Py_DECREF(sysMod); // release ref to sysMod

  // set current directory and python's path.
  if (m_argv != NULL)
    PySys_SetArgv(m_argc, m_argv);

  CLog::Log(LOGDEBUG, "CPythonInvoker: setting the Python path to %s", m_pythonPath.c_str());
  PySys_SetPath((char *)m_pythonPath.c_str());

  CLog::Log(LOGDEBUG, "CPythonInvoker: entering source directory %s", scriptDir.c_str());
  PyObject* module = PyImport_AddModule((char*)"__main__");
  PyObject* moduleDict = PyModule_GetDict(module);

  // when we are done initing we store thread state so we can be aborted
  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  // we need to check if we was asked to abort before we had inited
  bool stopping = false;
  { CSingleLock lock(g_pythonParser.m_critSection);
    m_threadState = state;
    stopping = m_stop;
  }

  PyEval_AcquireLock();
  PyThreadState_Swap(state);

  if (!stopping)
  {
    try
    {
      // run script from file
      // We need to have python open the file because on Windows the DLL that python
      //  is linked against may not be the DLL that xbmc is linked against so
      //  passing a FILE* to python from an fopen has the potential to crash.
      PyObject* file = PyFile_FromString((char *) CSpecialProtocol::TranslatePath(m_source).c_str(), (char*)"r");
      FILE *fp = PyFile_AsFile(file);

      if (fp)
      {
        PyObject *f = PyString_FromString(CSpecialProtocol::TranslatePath(m_source).c_str());
        PyDict_SetItemString(moduleDict, "__file__", f);

        if (m_addon.get() != NULL)
        {
          PyObject *pyaddonid = PyString_FromString(m_addon->ID().c_str());
          PyDict_SetItemString(moduleDict, "__xbmcaddonid__", pyaddonid);

          CStdString version = ADDON::GetXbmcApiVersionDependency(m_addon);
          PyObject *pyxbmcapiversion = PyString_FromString(version.c_str());
          PyDict_SetItemString(moduleDict, "__xbmcapiversion__", pyxbmcapiversion);

          CLog::Log(LOGDEBUG, "CPythonInvoker: instantiating addon using automatically obtained id of \"%s\" dependent on version %s of the xbmc.python api", m_addon->ID().c_str(), version.c_str());
        }

        Py_DECREF(f);
        setState(InvokerStateRunning);
        PyRun_FileExFlags(fp, CSpecialProtocol::TranslatePath(m_source).c_str(), m_Py_file_input, moduleDict, moduleDict, 1, NULL);
      }
      else
        CLog::Log(LOGERROR, "CPythonInvoker: %s not found!", m_source);

      /* run script
      setState(InvokerStateRunning);
      PyRun_String(m_source, m_Py_file_input, moduleDict, moduleDict);
      */
    }
    catch (const XbmcCommons::Exception& e)
    {
      setState(InvokerStateFailed);
      e.LogThrowMessage();
    }
    catch (...)
    {
      setState(InvokerStateFailed);
      CLog::Log(LOGERROR, "CPythonInvoker: failure in %s", m_source);
    }
  }

  if (!PyErr_Occurred())
  {
    CLog::Log(LOGINFO, "CPythonInvoker: script successfully run");
    setState(InvokerStateDone);
  }
  else if (PyErr_ExceptionMatches(PyExc_SystemExit))
  {
    CLog::Log(LOGINFO, "CPythonInvoker: script aborted");
    setState(InvokerStateFailed);
  }
  else
  {
    setState(InvokerStateFailed);

    PythonBindings::PythonToCppException e;
    e.LogThrowMessage();

    {
      CPyThreadState releaseGil;
      CSingleLock gc(g_graphicsContext);

      CGUIDialogKaiToast *pDlgToast = (CGUIDialogKaiToast*)g_windowManager.GetWindow(WINDOW_DIALOG_KAI_TOAST);
      if (pDlgToast)
      {
        CStdString desc;
        CStdString path;
        CStdString script;
        URIUtils::Split(m_source, path, script);
        if (script.Equals("default.py"))
        {
          CStdString path2;
          URIUtils::RemoveSlashAtEnd(path);
          URIUtils::Split(path, path2, script);
        }

        desc.Format(g_localizeStrings.Get(2100), script);
        pDlgToast->QueueNotification(CGUIDialogKaiToast::Error, g_localizeStrings.Get(257), desc);
      }
    }
  }

  PyObject *m = PyImport_AddModule((char*)"xbmc");
  if (!m || PyObject_SetAttrString(m, (char*)"abortRequested", PyBool_FromLong(1)))
    CLog::Log(LOGERROR, "CPythonInvoker: failed to set abortRequested");

  // make sure all sub threads have finished
  for (PyThreadState* s = state->interp->tstate_head, *old = NULL; s; )
  {
    if (s == state)
    {
      s = s->next;
      continue;
    }
    if (old != s)
    {
      CLog::Log(LOGINFO, "CPythonInvoker: waiting on thread %"PRIu64, (uint64_t)s->thread_id);
      old = s;
    }

    CPyThreadState pyState;
    Sleep(100);
    pyState.Restore();

    s = state->interp->tstate_head;
  }

  // pending calls must be cleared out
  XBMCAddon::RetardedAsynchCallbackHandler::clearPendingCalls(state);

  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  //set stopped event - this allows ::stop to run and kill remaining threads
  //this event has to be fired without holding m_pExecuter->m_critSection
  //before
  //Also the GIL (PyEval_AcquireLock) must not be held
  //if not obeyed there is still no deadlock because ::stop waits with timeout (smart one!)
  m_stoppedEvent.Set();

  { CSingleLock lock(g_pythonParser.m_critSection);
    m_threadState = NULL;
  }

  PyEval_AcquireLock();
  PyThreadState_Swap(state);

  g_pythonParser.DeInitializeInterpreter();

  Py_EndInterpreter(state);
  PyThreadState_Swap(NULL);

  PyEval_ReleaseLock();

  return true;
}

void CPythonInvoker::addPath(const std::string path)
{
  if (path.empty())
    return;

  if (!m_pythonPath.empty())
    m_pythonPath += PY_PATH_SEP;

#ifdef TARGET_WINDOWS
  CStdString tmp(path);
  g_charsetConverter.utf8ToSystem(tmp);
  m_pythonPath += tmp;
#else
  m_pythonPath += path;
#endif
}
