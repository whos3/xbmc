#pragma once
/*
 *      Copyright (C) 2015 Team XBMC
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

#include "cores/Transcoder/IOutputWriter.h"

struct AVFormatContext;

class CFileOutputWriter : public IOutputWriter
{
public:
  CFileOutputWriter(const std::string& file);
  virtual ~CFileOutputWriter();

  // implementations of IOutputWriter
  virtual AVStream* AddStream() override;
  virtual int WriteHeader(AVDictionary** header) override;
  virtual int Write(const AVStream const* stream, AVFrame* frame) override;

protected:
  AVFormatContext* m_context;
};
