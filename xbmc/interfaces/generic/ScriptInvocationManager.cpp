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

#include "ScriptInvocationManager.h"
#include "filesystem/File.h"
#include "threads/SingleLock.h"
#include "utils/URIUtils.h"

#include "interfaces/python/PythonInvoker.h"
#include "interfaces/javascript/JavaScriptInvoker.h"

ScriptInvocationManager::ScriptInvocationManager()
  : m_id(0)
{ }

ScriptInvocationManager::~ScriptInvocationManager()
{
  CSingleLock lock(m_critSection);
  for (std::map<int, CLanguageInvokerThread*>::iterator script = m_scripts.begin(); script != m_scripts.end(); script++)
    delete script->second;

  m_scripts.clear();
}

ScriptInvocationManager& ScriptInvocationManager::Get()
{
  static ScriptInvocationManager s_instance;
  return s_instance;
}

int ScriptInvocationManager::Execute(const std::string &script, ADDON::AddonPtr addon, const std::vector<std::string> &arguments /* = std::vector<std::string>() */)
{
  if (script.empty() || !XFILE::CFile::Exists(script, false) || addon.get() == NULL)
    return -1;

  ILanguageInvoker *invoker = GetInvoker(script);
  if (invoker == NULL)
    return -1;

  CLanguageInvokerThread *invokerThread = new CLanguageInvokerThread(invoker);
  if (invokerThread == NULL)
    return -1;
  
  invokerThread->SetAddon(addon);

  CSingleLock lock(m_critSection);
  invokerThread->SetId(m_id);
  m_id++;
  lock.Leave();

  m_scripts[invokerThread->GetId()] = invokerThread;
  invokerThread->Execute(script, arguments);

  return invokerThread->GetId();
}

bool ScriptInvocationManager::Stop(int scriptId)
{
  if (scriptId < 0)
    return true;

  CSingleLock lock(m_critSection);
  std::map<int, CLanguageInvokerThread*>::const_iterator script = m_scripts.find(scriptId);
  if (script == m_scripts.end())
    return true;

  return script->second->Stop();
}

ILanguageInvoker* ScriptInvocationManager::GetInvoker(const std::string &script)
{
  std::string extension = URIUtils::GetExtension(script);
  if (strnicmp(extension.c_str(), ".py", 3) == 0)
    return new CPythonInvoker();
  else if (strnicmp(extension.c_str(), ".js", 3) == 0)
    return new CJavaScriptInvoker();

  return NULL;
}
