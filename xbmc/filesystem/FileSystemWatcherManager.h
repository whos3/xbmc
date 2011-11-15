#pragma once
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

#include <set>

#include "FileSystemWatcher.h"

namespace XFILE
{
  class CFileSystemWatcherManager
  {
  public:
    static CFileSystemWatcherManager& Get();

    CFileSystemWatcherPtr Watch(const std::string &path, IFileSystemWatcherNotification *notification, FileSystemWatcherType type = (FileSystemWatcherType)0x1F, bool recursive = false);
    CFileSystemWatcherPtr Watch(const std::string &path, IFileSystemWatcherNotification *notification, bool recursive = false, FileSystemWatcherType type = (FileSystemWatcherType)0x1F);
    CFileSystemWatcherPtr Watch(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, FileSystemWatcherType type = (FileSystemWatcherType)0x1F, bool recursive = false);
    CFileSystemWatcherPtr Watch(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, bool recursive = false, FileSystemWatcherType type = (FileSystemWatcherType)0x1F);
    
    void Stop(CFileSystemWatcherPtr watcher);

  private:
    CFileSystemWatcherManager() { }
    ~CFileSystemWatcherManager();

    std::set<CFileSystemWatcherPtr> m_watchers;
  };
}
