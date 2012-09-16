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

#include "interfaces/generic/ILanguageInvoker.h"
#include "threads/Event.h"

class CPythonInvoker : public ILanguageInvoker
{
public:
  CPythonInvoker();
  virtual ~CPythonInvoker();

  virtual bool Execute(const std::string &script, const std::vector<std::string> &arguments = std::vector<std::string>());
  virtual bool Stop(bool wait = false);

  virtual void OnError();
  virtual void OnDone();

private:
  bool execute(const std::string &code, const std::vector<std::string> &arguments);
  void addPath(const std::string path);

  char *m_source;
  unsigned int  m_argc;
  char **m_argv;
  std::string m_pythonPath;
  void *m_threadState;
  bool m_stop;
  CEvent m_stoppedEvent;
};
