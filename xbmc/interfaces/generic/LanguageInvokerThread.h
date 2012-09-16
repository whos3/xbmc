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

#include "ILanguageInvoker.h"
#include "threads/Thread.h"

class CLanguageInvokerThread : public ILanguageInvoker, protected CThread
{
public:
  CLanguageInvokerThread(ILanguageInvoker *invoker);
  ~CLanguageInvokerThread();

  virtual bool Execute(const std::string &script, const std::vector<std::string> &arguments = std::vector<std::string>());
  virtual bool Stop(bool wait = false);

  virtual InvokerState GetState();

protected:
  virtual void OnStartup();
  virtual void Process();
  virtual void OnExit();
  virtual void OnException();

private:
  ILanguageInvoker *m_invoker;
  std::string m_script;
  std::vector<std::string> m_args;
};
