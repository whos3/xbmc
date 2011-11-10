/*
 *      Copyright (C) 2011 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "Win32FileSystemWatcher.h"
#include "Directory.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

using namespace std;
using namespace XFILE;

CWin32FileSystemWatcher::CWin32FileSystemWatcher()
  : m_running(false)
{ }

CWin32FileSystemWatcher::~CWin32FileSystemWatcher()
{
  Stop();
}

void CWin32FileSystemWatcher::Start()
{
  if (m_stop)
    return;

  HANDLE handle = CreateEvent(0, false, false, NULL);
  if (handle == INVALID_HANDLE_VALUE)
  {
    CLog::Log(LOGERROR, "Win32FileSystemHandler: CreateEvent failed");
    return;
  }

  PathHandle eventHandle = { handle, "" };
  m_handles.push_back(eventHandle);

  DWORD filters = 0;
  if (m_type & FileSystemWatcherTypeModified)
    filters |= FILE_NOTIFY_CHANGE_LAST_WRITE;
  if (m_type & (FileSystemWatcherTypeMoved | FileSystemWatcherTypeRenamed | FileSystemWatcherTypeDeleted | FileSystemWatcherTypeCreated))
    filters |= FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;

  for (unsigned int index = 0; index < m_paths.size(); index++)
  {
    HANDLE handle = FindFirstChangeNotification((TCHAR *)m_paths.at(index).c_str(), m_recursive, filters);
    if (handle == INVALID_HANDLE_VALUE)
    {
      CLog::Log(LOGWARNING, "Win32FileSystemHandler: FindFirstChangeNotification failed for %s", m_paths.at(index).c_str());
      continue;
    }

    PathHandle pathHandle = { handle, m_paths.at(index) };
    m_handles.push_back(pathHandle);
  }

  run();
}

void CWin32FileSystemWatcher::Stop()
{
  if (m_stop)
    return;

  IFileSystemWatcher::Stop();

  if (!m_running || m_handles.size() < 1)
    return;

  SetEvent(m_handles[0].handle);
  if (m_running)
    m_terminationEvent.Wait();

  CSingleLock lock(m_lock);
  CloseHandle(m_handles.at(0).handle);
  m_handles.at(0).handle = INVALID_HANDLE_VALUE;

  for (unsigned int index = 1; index < m_handles.size(); index++)
  {
    if (m_handles.at(index).handle != INVALID_HANDLE_VALUE)
      FindCloseChangeNotification(m_handles.at(index).handle);
  }

  m_handles.clear();
}

void CWin32FileSystemWatcher::run()
{
  if (m_handles.size() <= 1)
    return;

  m_running = true;

  CSingleLock lock(m_lock);
  unsigned int size = m_handles.size();
  HANDLE *handles = new HANDLE[size];
  for (unsigned int index = 0; index < size; index++)
    handles[index] = m_handles.at(index).handle;
  lock.Leave();

  DWORD result = WaitForMultipleObjects(size, handles, false, INFINITE);
  while (result != WAIT_TIMEOUT && result >= WAIT_OBJECT_0 && result < (WAIT_OBJECT_0 + m_handles.size()))
  {
    if (result == WAIT_OBJECT_0)
    {
      if (m_stop)
        break;
    }
    else
    {
      lock.Enter();
      unsigned int index = result - WAIT_OBJECT_0;
      if (index >= m_handles.size())
        continue;

      if (!FindNextChangeNotification(handles[index]))
        CLog::Log(LOGWARNING, "Win32FileSystemWatcher: FindNextChangeNotification failed");

      string path = m_handles.at(index).path;
      m_notification->FileSystemChanged(path);

      // Check whether the directory has been removed
      if (!CDirectory::Exists(path))
      {
        FindCloseChangeNotification(handles[index]);
        m_handles.erase(m_handles.begin() + index);
      }
      lock.Leave();
    }
    
    lock.Enter();
    if (m_handles.size() <= 1)
    {
      lock.Leave();
      break;
    }

    // We need to copy the HANDLEs again as some might have been removed
    size = m_handles.size();
    for (unsigned int index = 0; index < size; index++)
      handles[index] = m_handles.at(index).handle;
    lock.Leave();

    result = WaitForMultipleObjects(size, handles, false, INFINITE);
  }

  m_running = false;
  if (m_stop)
    m_terminationEvent.Set();
}