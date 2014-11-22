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

#include "media/MediaType.h"
#include "media/import/MediaImportSource.h"
#include "media/import/IMediaImportTask.h"

class CFileItemList;

// job is deleted after work is done, so we need proxy job to trigger IMediaImport::DoImport
class CMediaImportSourceRegistrationTask : public IMediaImportTask
{
public:
  CMediaImportSourceRegistrationTask(const CMediaImportSource &source);
  virtual ~CMediaImportSourceRegistrationTask() { }

  /*!
   * \brief Returns the source to be registered/added.
   */
  const CMediaImportSource& GetImportSource() const { return m_source; }

  /*!
  * \brief Returns true if the source should be imported otherwise false.
  */
  bool GetImportDecision() const { return m_importDecision; }

  // implementation of IMediaImportTask
  virtual MediaImportTaskType GetType() { return MediaImportTaskSourceRegistration; }
  virtual bool DoWork();

protected:
  CMediaImportSource m_source;
  bool m_importDecision;
};
