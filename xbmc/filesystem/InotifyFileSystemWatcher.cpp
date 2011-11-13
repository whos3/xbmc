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

#include <sys/inotify.h>
#include <fcntl.h>

#include "InotifyFileSystemWatcher.h"
#include "FileItem.h"
#include "Directory.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

#define MAX_EVENTS  32
#define BUFFER_SIZE ((sizeof(struct inotify_event) + FILENAME_MAX) * MAX_EVENTS)

using namespace std;
using namespace XFILE;

CInotifyFileSystemWatcher::CInotifyFileSystemWatcher()
  : m_fd(0),
    m_running(false)
{ }

CInotifyFileSystemWatcher::~CInotifyFileSystemWatcher()
{
  Stop();
}

void CInotifyFileSystemWatcher::Start()
{
  if (m_stop || m_fd > 0)
    return;

  m_fd = inotify_init();
  if (m_fd <= 0)
    return;
  
  fcntl(m_fd, F_SETFD, FD_CLOEXEC);
  
  int filterMask = 0;
  if (m_type & FileSystemWatcherTypeModified)
    filterMask |= IN_MODIFY;
  if (m_type & FileSystemWatcherTypeMoved)
    filterMask |= IN_MOVED_FROM | IN_MOVED_TO;
  if (m_type & FileSystemWatcherTypeRenamed)
    filterMask |= IN_MOVED_FROM | IN_MOVED_TO;
  if (m_type & FileSystemWatcherTypeDeleted)
    filterMask |= IN_DELETE | IN_DELETE_SELF;
  if (m_type & FileSystemWatcherTypeCreated)
    filterMask |= IN_CREATE;
   
  for (unsigned int index = 0; index < m_paths.size(); index++)
    addWatch(m_paths.at(index), m_recursive, filterMask);

  run();
}

void CInotifyFileSystemWatcher::Stop()
{
  if (m_stop)
    return;

  IFileSystemWatcher::Stop();
  
  CSingleLock lock(m_lock);
  
  if (!m_running || m_fd <= 0)
    return;
  
  for (map<int, string>::iterator it = m_watchDescriptors.begin(); it != m_watchDescriptors.end(); it++)
    inotify_rm_watch(m_fd, it->first);
    
  m_watchDescriptors.clear();
    
  // Triggers the blocking read() on m_fd in run()
  close(m_fd);
  m_fd = 0;
  
  // We need to leave the lock now
  lock.Leave();
  
  // Wait until run() has finished
  m_terminationEvent.Wait();
}

void CInotifyFileSystemWatcher::addWatch(std::string path, bool recursive, int filterMask)
{
  if (path.empty())
    return;
    
  if (!CDirectory::Exists(path))
    return;
    
  int wd = inotify_add_watch(m_fd, path.c_str(), filterMask);
  if (wd <= 0)
  {
    CLog::Log(LOGWARNING, "InotifyFileSystemWatcher: inotify_add_watch failed for %s (%i) returning %i", path.c_str(), m_fd, wd);
    return;
  }
  
  m_watchDescriptors[wd] = path;
  
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

void CInotifyFileSystemWatcher::run()
{
  if (m_stop || m_fd <= 0 || m_watchDescriptors.size() == 0)
    return;
    
  m_running = true;
    
  ssize_t len, pos = 0;
  char buffer[BUFFER_SIZE] = { 0 };
  
  while (!m_stop && m_fd > 0 && m_watchDescriptors.size() > 0)
  {
    len = read(m_fd, buffer, BUFFER_SIZE);
    
    CSingleLock lock(m_lock);
    if (len < 0)
      break;
    
    pos = 0;
    while (pos < len)
    {
      struct inotify_event *event = (struct inotify_event *)&buffer[pos];
      
      if (event->wd > 0 && m_watchDescriptors.find(event->wd) != m_watchDescriptors.end())
      {
        string path = m_watchDescriptors[event->wd];
        m_notification->FileSystemChanged(path);
        
        // Check whether the directory has been removed
        if (event->mask & IN_DELETE_SELF && !CDirectory::Exists(path))
        {
          inotify_rm_watch(m_fd, event->wd);
          m_watchDescriptors.erase(event->wd);
        }
      }
      else
        CLog::Log(LOGWARNING, "InotifyFileSystemWatcher: unknown watcher descriptor %i", event->wd);
        
      if (m_stop || m_fd <= 0 || m_watchDescriptors.size() == 0)
        break;
    
      pos += sizeof(struct inotify_event) + event->len;
    }
  }
  
  m_running = false;
  if (m_stop)
    m_terminationEvent.Set();
}
