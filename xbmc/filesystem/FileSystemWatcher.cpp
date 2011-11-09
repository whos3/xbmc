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

#include "FileSystemWatcher.h"
#include "utils/log.h"

#if defined(TARGET_WINDOWS)
#include "Win32FileSystemWatcher.h"
#elif defined(TARGET_DARWIN) || defined(__FreeBSD__)
#include "KqueueFileSystemWatcher.h"
#elif defined(TARGET_LINUX)
#include "InotifyFileSystemWatcher.h"
#endif

using namespace std;
using namespace XFILE;

CFileSystemWatcher::CFileSystemWatcher()
  : CThread("FileSystemWatcher"),
    m_implementation(NULL)
{ }

CFileSystemWatcher::CFileSystemWatcher(const std::string &path, IFileSystemWatcherNotification *notification, FileSystemWatcherType type /* = (WatcherType)0x1F */, bool recursive /* = false */)
  : IFileSystemWatcher(path, notification, type, recursive),
    CThread("FileSystemWatcher"),
    m_implementation(NULL)
{ }

CFileSystemWatcher::CFileSystemWatcher(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, FileSystemWatcherType type /* = (WatcherType)0x1F */, bool recursive /* = false */)
  : IFileSystemWatcher(paths, notification, type, recursive),
    CThread("FileSystemWatcher"),
    m_implementation(NULL)
{ }

CFileSystemWatcher::~CFileSystemWatcher()
{
  if (m_implementation)
  {
    delete m_implementation;
    m_implementation = NULL;
  }
}

void CFileSystemWatcher::Start()
{
  CThread::Create(false);
}

void CFileSystemWatcher::Stop()
{
  if (m_implementation)
    m_implementation->Stop();

  CThread::StopThread(true);
}

void CFileSystemWatcher::OnStartup()
{
#if defined(TARGET_WINDOWS)
  m_implementation = new CWin32FileSystemWatcher();
#elif defined(TARGET_DARWIN) || defined(__FreeBSD__)
  m_implementation = new CKqueueFileSystemWatcher();
#elif defined(TARGET_LINUX)
  m_implementation = new CInotifyFileSystemWatcher();
#endif

  if (m_implementation == NULL)
  {
    CLog::Log(LOGERROR, "FileSystemWatcher: No native implementation found");
    return;
  }
  
  m_implementation->SetRecursive(m_recursive);
  m_implementation->AddPaths(m_paths);
  m_implementation->SetNotification(m_notification);
  m_implementation->SetType(m_type);
}

void CFileSystemWatcher::Process()
{
  if (m_implementation)
    m_implementation->Start();
}

void CFileSystemWatcher::OnExit()
{
  if (m_implementation)
    m_implementation->Stop();
}
