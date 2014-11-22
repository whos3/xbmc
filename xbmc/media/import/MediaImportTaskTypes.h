#pragma once
/*
*      Copyright (C) 2014 Team XBMC
*      http://xbmc.org
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

typedef enum MediaImportTaskType
{
  MediaImportTaskNone = 0,
  MediaImportTaskSourceRegistration,
  MediaImportTaskRetrieval,
  MediaImportTaskChangeset,
  MediaImportTaskScraping,
  MediaImportTaskSynchronisation,
  MediaImportTaskCleanup,
  MediaImportTaskUpdate
} MediaImportTaskType;

class MediaImportTaskTypes
{
public:
  static std::string ToString(MediaImportTaskType taskType)
  {
    switch (taskType)
    {
      case MediaImportTaskSourceRegistration:
        return "source registration";

      case MediaImportTaskRetrieval:
        return "retrieval";

      case MediaImportTaskChangeset:
        return "changeset";

      case MediaImportTaskScraping:
        return "scraping";

      case MediaImportTaskSynchronisation:
        return "synchronisation";

      case MediaImportTaskCleanup:
        return "cleanup";

      case MediaImportTaskUpdate:
        return "update";

      default:
        break;
    }

    return "unknown";
  }

private:
  MediaImportTaskTypes();
};
