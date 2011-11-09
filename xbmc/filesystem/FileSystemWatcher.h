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

#include <boost/shared_ptr.hpp>

#include "IFileSystemWatcher.h"
#include "threads/Thread.h"

namespace XFILE
{
  class CFileSystemWatcher : public IFileSystemWatcher, private CThread
  {
  public:
    CFileSystemWatcher();
    CFileSystemWatcher(const std::string &path, IFileSystemWatcherNotification *notification, FileSystemWatcherType type = (FileSystemWatcherType)0x1F, bool recursive = false);
    CFileSystemWatcher(const std::vector<std::string> &paths, IFileSystemWatcherNotification *notification, FileSystemWatcherType type = (FileSystemWatcherType)0x1F, bool recursive = false);
    virtual ~CFileSystemWatcher();
    
    virtual void Start();
    virtual void Stop();

  protected:
    virtual void OnStartup();
    virtual void Process();
    virtual void OnExit();

  private:
    IFileSystemWatcher *m_implementation;

    void stop(bool wait);
  };

  typedef boost::shared_ptr<CFileSystemWatcher> CFileSystemWatcherPtr;
}
