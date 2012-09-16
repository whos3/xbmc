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

#include <map>

#include "LanguageInvokerThread.h"
#include "threads/CriticalSection.h"

class ScriptInvocationManager
{
public:
  static ScriptInvocationManager& Get();

  int Execute(const std::string &script, ADDON::AddonPtr addon, const std::vector<std::string> &arguments = std::vector<std::string>());
  bool Stop(int scriptId);

  ILanguageInvoker* GetInvoker(const std::string &script);

  // TODO

private:
  ScriptInvocationManager();
  ScriptInvocationManager(const ScriptInvocationManager&);
  ScriptInvocationManager const& operator=(ScriptInvocationManager const&);
  virtual ~ScriptInvocationManager();

  CCriticalSection m_critSection;
  std::map<int, CLanguageInvokerThread*> m_scripts;
  int m_id;
};