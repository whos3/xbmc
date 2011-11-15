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

#include "FileSystemWatcherManager.h"

using namespace std;
using namespace XFILE;

CFileSystemWatcherManager::~CFileSystemWatcherManager()
{
  m_watchers.clear();
}

CFileSystemWatcherManager& CFileSystemWatcherManager::Get()
{
  static CFileSystemWatcherManager sManager;
  return sManager;
}

CFileSystemWatcherPtr CFileSystemWatcherManager::Watch(const std::string &path, IFileSystemWatcherNotification *notification, FileSystemWatcherType type /* = (FileSystemWatcherType)0x1F */, bool recursive /* = false */)
{
  CFileSystemWatcherPtr watcher(new CFileSystemWatcher(path, notification, type, recursive));
  m_watchers.insert(watcher);
  watcher->Start();

  return watcher;
}

CFileSystemWatcherPtr CFileSystemWatcherManager::Watch(const std::string &path, IFileSystemWatcherNotification *notification, bool recursive /* = false */, FileSystemWatcherType type /* = (FileSystemWatcherType)0x1F */)
{
  return Watch(path, notification, type, recursive);
}

CFileSystemWatcherPtr CFileSystemWatcherManager::Watch(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, FileSystemWatcherType type /* = (FileSystemWatcherType)0x1F */, bool recursive /* = false */)
{
  CFileSystemWatcherPtr watcher(new CFileSystemWatcher(paths, notification, type, recursive));
  m_watchers.insert(watcher);
  watcher->Start();

  return watcher;
}

CFileSystemWatcherPtr CFileSystemWatcherManager::Watch(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, bool recursive /* = false */, FileSystemWatcherType type /* = (FileSystemWatcherType)0x1F */)
{
  return Watch(paths, notification, type, recursive);
}

void CFileSystemWatcherManager::Stop(CFileSystemWatcherPtr watcher)
{
  if (watcher.get() == NULL)
    return;

  set<CFileSystemWatcherPtr>::iterator it = m_watchers.find(watcher);
  if (it == m_watchers.end())
    return;

  (*it)->Stop();
  m_watchers.erase(it);
}

