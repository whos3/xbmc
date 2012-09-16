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

#include "LanguageInvokerThread.h"

CLanguageInvokerThread::CLanguageInvokerThread(ILanguageInvoker *invoker)
  : ILanguageInvoker(), CThread("CLanguageInvoker"),
    m_invoker(invoker)
{
  // TODO
}

CLanguageInvokerThread::~CLanguageInvokerThread()
{
  // TODO
  delete m_invoker;
}

bool CLanguageInvokerThread::Execute(const std::string &script, const std::vector<std::string> &arguments /* = std::vector<std::string>() */)
{
  if (m_invoker == NULL || m_addon == NULL || script.empty())
    return false;

  m_script = script;
  m_args = arguments;

  Create();
  return true;
}

bool CLanguageInvokerThread::Stop(bool wait /* = false */)
{
  if (m_invoker == NULL)
    return false;

  if (!CThread::IsRunning())
    return true;

  bool result = true;
  if (m_invoker->GetState() < InvokerStateDone)
    result = m_invoker->Stop(wait);

  CThread::StopThread(wait);
  
  return result;
}

InvokerState CLanguageInvokerThread::GetState()
{
  if (m_invoker == NULL)
    return InvokerStateFailed;

  return m_invoker->GetState();
}

void CLanguageInvokerThread::OnStartup()
{
  if (m_invoker == NULL)
    return;

  m_invoker->SetId(GetId());
  m_invoker->SetAddon(m_addon);
  // TODO
}

void CLanguageInvokerThread::Process()
{
  if (m_invoker == NULL)
    return;
  
  // TODO
  m_invoker->Execute(m_script, m_args);
}

void CLanguageInvokerThread::OnExit()
{
  if (m_invoker == NULL)
    return;

  // TODO

  m_invoker->OnDone();
}

void CLanguageInvokerThread::OnException()
{
  if (m_invoker == NULL)
    return;

  // TODO

  m_invoker->OnError();
}
