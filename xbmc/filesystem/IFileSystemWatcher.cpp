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

#include "IFileSystemWatcher.h"
#include "Directory.h"
#include "utils/log.h"
#include "utils/URIUtils.h"

using namespace std;
using namespace XFILE;

IFileSystemWatcher::IFileSystemWatcher()
  : m_stop(false),
    m_recursive(false),
    m_type(FileSystemWatcherTypeNone),
    m_notification(NULL)
{ }

IFileSystemWatcher::IFileSystemWatcher(const std::string &path, IFileSystemWatcherNotification *notification, FileSystemWatcherType type /* = (WatcherType)0x1F */, bool recursive /* = false */)
  : m_stop(false),
    m_recursive(recursive),
    m_type(type),
    m_notification(notification)
{
  AddPath(path);
}

IFileSystemWatcher::IFileSystemWatcher(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, FileSystemWatcherType type /* = (WatcherType)0x1F */, bool recursive /* = false */)
  : m_stop(false),
    m_recursive(recursive),
    m_type(type),
    m_notification(notification)
{
  AddPaths(paths);
}

IFileSystemWatcher::~IFileSystemWatcher()
{
  Stop();
}

bool IFileSystemWatcher::AddPath(const std::string &path)
{
  if (path.empty())
    return false;

  if (!CDirectory::Exists(path))
  {
    CLog::Log(LOGWARNING, "IFileSystemWatcher: Cannot add non-existing path %s", path.c_str());
    return false;
  }

  for (unsigned int index = 0; index < m_paths.size(); index++)
  {
    // Check if the current path is already covered by another watched path
    if (m_recursive && URIUtils::IsInPath(path, m_paths.at(index)))
    {
      CLog::Log(LOGDEBUG, "IFileSystemWatcher: Path %s is already covered by %s", path.c_str(), m_paths.at(index).c_str());
      return true;
    }
  }

  m_paths.push_back(path);

  return true;
}

bool IFileSystemWatcher::AddPaths(const std::vector<std::string> &paths)
{
  if (paths.size() == 0)
    return false;

  bool ret = false;
  for (unsigned int index = 0; index < paths.size(); index++)
    ret |= AddPath(paths.at(index));

  return ret;
}
