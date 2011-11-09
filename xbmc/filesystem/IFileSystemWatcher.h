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

#include <string>
#include <vector>

namespace XFILE
{
  enum FileSystemWatcherType
  {
    FileSystemWatcherTypeNone      = 0x00,
    FileSystemWatcherTypeModified  = 0x01,
    FileSystemWatcherTypeMoved     = 0x02,
    FileSystemWatcherTypeRenamed   = 0x04,
    FileSystemWatcherTypeDeleted   = 0x08,
    FileSystemWatcherTypeCreated   = 0x10
  };

  class IFileSystemWatcherNotification
  {
  public:
    virtual ~IFileSystemWatcherNotification() { }

    virtual void FileSystemChanged(const std::string &path) = 0;
  };

  class IFileSystemWatcher
  {
  public:
    IFileSystemWatcher();
    IFileSystemWatcher(const std::string &path, IFileSystemWatcherNotification *notification, FileSystemWatcherType type = (FileSystemWatcherType)0x1F, bool recursive = false);
    IFileSystemWatcher(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, FileSystemWatcherType type = (FileSystemWatcherType)0x1F, bool recursive = false);
    virtual ~IFileSystemWatcher();

    bool AddPath(const std::string &path);
    bool AddPaths(const std::vector<std::string> &paths);

    void SetRecursive(bool recursive) { m_recursive = recursive; }

    void SetType(FileSystemWatcherType type) { m_type = type; }
    void AddType(FileSystemWatcherType type) { m_type = (FileSystemWatcherType)(m_type | type); }

    void SetNotification(IFileSystemWatcherNotification *notification) { m_notification = notification; }

    virtual void Start() = 0;
    virtual void Stop() { m_stop = true; }

  protected:
    bool m_stop;
    bool m_recursive;
    std::vector<std::string> m_paths;
    FileSystemWatcherType m_type;
    IFileSystemWatcherNotification* m_notification;
  };
}
