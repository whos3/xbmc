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

#include "cores/Transcoder/TranscodingOptions.h"

struct AVFrame;

class IFrameProcessor
{
public:
  virtual ~IFrameProcessor() { }

  virtual int Process(AVFrame* frame, AVFrame** processedFrame, bool& newFrame) = 0;
  virtual void FreeProcessedFrame(AVFrame** processedFrame, bool newFrame) = 0;

protected:
  IFrameProcessor(const CTranscodingOptions& originalOptions, const CTranscodingOptions& targetOptions)
    : m_originalOptions(originalOptions),
      m_targetOptions(targetOptions)
  { }

  CTranscodingOptions m_originalOptions;
  CTranscodingOptions m_targetOptions;
};