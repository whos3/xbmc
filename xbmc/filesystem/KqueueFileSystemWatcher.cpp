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

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>

#include "KqueueFileSystemWatcher.h"
#include "Directory.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

using namespace std;
using namespace XFILE;

CKqueueFileSystemWatcher::CKqueueFileSystemWatcher()
  : m_kqfd(-1),
    m_kqpipe({-1}),
    m_filterMask(0),
    m_running(false)
{ }

CKqueueFileSystemWatcher::~CKqueueFileSystemWatcher()
{
  Stop();
}

void CKqueueFileSystemWatcher::Start()
{
  if (m_stop || m_kqfd > -1)
    return;

  m_kqfd = kqueue();
  if (m_kqfd < 0)
    return;
  
  fcntl(m_kqfd, F_SETFD, FD_CLOEXEC);

  if (pipe(m_kqpipe) == -1) 
  {
    CLog::Log(LOGERROR, "KqueueFileSystemWatcher: pipe() failed");
    close (m_kqfd);
    m_kqfd = m_kqpipe[0] = m_kqpipe[1] = -1;
    return;
  }
  fcntl(m_kqpipe[0], F_SETFD, FD_CLOEXEC);
  fcntl(m_kqpipe[1], F_SETFD, FD_CLOEXEC);

  struct kevent kev;
  EV_SET(&kev, m_kqpipe[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

  if (kevent(m_kqfd, &kev, 1, 0, 0, 0) == -1) 
  {
    CLog::Log(LOGERROR, "KqueueFileSystemWatcher: cannot watch pipe, kevent() failed");
    close (m_kqfd);
    close (m_kqpipe[0]);
    close (m_kqpipe[1]);
    m_kqfd = m_kqpipe[0] = m_kqpipe[1] = -1;
    return;
  }
  
  m_filterMask = 0;
  if (m_type & FileSystemWatcherTypeModified)
    m_filterMask |= NOTE_WRITE | NOTE_EXTEND;
  if (m_type & FileSystemWatcherTypeMoved)
    m_filterMask |= NOTE_RENAME | NOTE_WRITE | NOTE_DELETE;
  if (m_type & FileSystemWatcherTypeRenamed)
    m_filterMask |= NOTE_RENAME;
  if (m_type & FileSystemWatcherTypeDeleted)
    m_filterMask |= NOTE_DELETE | NOTE_REVOKE;
  if (m_type & FileSystemWatcherTypeCreated)
    m_filterMask |= NOTE_WRITE;
   
  for (unsigned int index = 0; index < m_paths.size(); index++)
    addWatch(m_paths.at(index), m_recursive, filterMask);

  run();
}

void CKqueueFileSystemWatcher::Stop()
{
  if (m_stop)
    return;

  IFileSystemWatcher::Stop();
  
  CSingleLock lock(m_lock);
  
  if (!m_running || m_kqfd < 0)
    return;

  // Trigger the pipe to quit run()
  write(m_kqpipe[1], 'q', 1);
  
  // We need to leave the lock now
  lock.Leave();
  
  // Wait until run() has finished
  if (m_running)
    m_terminationEvent.Wait();
  
  for (map<int, string>::iterator it = m_kqueueDescriptors.begin(); it != m_kqueueDescriptors.end(); it++)
    close(it->first);
    
  m_kqueueDescriptors.clear();
    
  close(m_kqfd);
  close(m_kqpipe[0]);
  close(m_kqpipe[1]);
  m_kqfd = m_kqpipe[0] = m_kqpipe[1] = -1;
}

void CKqueueFileSystemWatcher::addWatch(std::string path, bool recursive)
{
  if (path.empty())
    return;
    
  if (!CDirectory::Exists(path))
    return;
    
  int fd;
#ifdef O_EVTONLY
  fd = open(path, O_EVTONLY);
#else
  fd = open(path, O_RDONLY);
#endif
  if (fd <= 0)
  {
    CLog::Log(LOGWARNING, "KqueueFileSystemWatcher: open failed for %s returning %i", path.c_str(), fd);
    return;
  }

  struct kevent kev;
  EV_SET(&kev, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_ONESHOT, m_filterMask, 0, 0);
  if (kevent(m_kqfd, &kev, 1, 0, 0, 0) == -1) 
  {
    CLog::Log(LOGWARNING, "KqueueFileSystemWatcher: kevent failed for %s", path.c_str());
    close(fd);
    return;
  }
  
  m_kqueueDescriptors[fd] = path;
  
  if (!recursive)
    return;
    
  CFileItemList items;
  if (!CDirectory::GetDirectory(path, items) || items.GetFolderCount() <= 0)
    return;
    
  for (int index = 0; index < items.Size(); index++)
  {
    if (items[index]->m_bIsFolder)
      addWatch(items[index]->GetPath(), recursive, filterMask);
  }
}

void CKqueueFileSystemWatcher::run()
{
  if (m_stop || m_kqfd > -1 || m_kqueueDescriptors.size() == 0)
    return;
    
  m_running = true;
  
  while (!m_stop && m_kqfd >= 0 && m_kqueueDescriptors.size() > 0)
  {
    struct kevent kev;
    int ret = kevent(m_kqfd, 0, 0, &kev, 1, NULL);
    
    CSingleLock lock(m_lock);
    // An error occured
    if (ret < 0)
      break;
    // Timeout occured or no event is available
    if (ret == 0)
      continue;
    
    int fd = kev.ident;
    if (fd == m_kqpipe[0])
    {
      char c;
      if (read(m_kqpipe[0], &c, 1) != 1)
      {
        CLog::Log(LOGERROR, "KqueueFileSystemWatcher: reading kqueue pipe failed");
        break;
      }

      // We need to quit
      if (c == 'q')
        break;
    }
    else if (kev.filter == EVFILT_VNODE)
    {
      if (m_kqueueDescriptors.find(fd) != m_kqueueDescriptors.end())
      {
        string path = m_kqueueDescriptors[fd];
        m_notification->FileSystemChanged(path);
        
        // Check whether the directory has been removed
        if ((kev.fflags & (NOTE_DELETE | NOTE_REVOKE | NOTE_RENAME)) && !CDirectory::Exists(path))
        {
          close(fd)
          m_kqueueDescriptors.erase(fd);
        }
        else
        {
          //re-enable watching the path
          EV_SET(&kev, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_ONESHOT, m_filterMask, 0, 0);
          if (kevent(m_kqfd, &kev, 1, 0, 0, 0) == -1)
          {
            CLog::Log(LOGWARNING, "KqueueFileSystemWatcher: kevent for %s failed", path.c_str());
            close(fd)
            m_kqueueDescriptors.erase(fd);
          }
        }
      }
      else
        CLog::Log(LOGWARNING, "KqueueFileSystemWatcher: unknown kqueue descriptor %i", fd);
        
      if (m_stop || m_kqfd <= 0 || m_kqueueDescriptors.size() == 0)
        break;
    }
    else
      CLog::Log(LOGWARNING, "KqueueFileSystemWatcher: unknown filter %i received", (int)kev.filter);
  }
  
  m_running = false;
  if (m_stop)
    m_terminationEvent.Set();
}
