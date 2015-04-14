#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include <stdint.h>
#include <map>
#include <string>

#include "FileItem.h"
#include "utils/JobManager.h"

/*!
 \ingroup thumbs,jobs
 \brief Thumb extractor job class

 Used by the CVideoThumbLoader to perform asynchronous generation of thumbs

 \sa CVideoThumbLoader and CJob
 */
class CVideoThumbExtractor : public CJob
{
public:
  CVideoThumbExtractor(const CFileItem& item, const std::string& listpath, bool extracThumb, const std::string& strTarget = "", int64_t pos = -1, bool fillStreamDetails = true);
  virtual ~CVideoThumbExtractor();

  /*!
   \brief Work function that extracts thumb.
   */
  virtual bool DoWork();

  virtual const char* GetType() const
  {
    return kJobTypeMediaFlags;
  }

  virtual bool operator==(const CJob* job) const;

  std::string m_target; ///< thumbpath
  std::string m_listpath; ///< path used in fileitem list
  CFileItem m_item;
  bool m_extractThumb; ///< extract thumb?
  int64_t m_pos; ///< position to extract thumb from
  bool m_fillStreamDetails; ///< fill in stream details? 
};
