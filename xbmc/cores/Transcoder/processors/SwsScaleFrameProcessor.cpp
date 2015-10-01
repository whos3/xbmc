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

#include "SwsScaleFrameProcessor.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libswscale/swscale.h"
}

CSwsScaleFrameProcessor::CSwsScaleFrameProcessor(const CTranscodingOptions& originalOptions, const CTranscodingOptions& targetOptions)
  : IFrameProcessor(originalOptions, targetOptions),
    m_needsScaling(NeedsScaling(originalOptions, targetOptions)),
    m_context(nullptr)
{ }

CSwsScaleFrameProcessor::~CSwsScaleFrameProcessor()
{
  if (m_context != nullptr)
  {
    sws_freeContext(m_context);
    m_context = nullptr;
  }
}

int CSwsScaleFrameProcessor::Process(AVFrame* frame, AVFrame** processedFrame, bool& newFrame)
{
  newFrame = false;

  if (processedFrame == nullptr)
    return AVERROR(ENOMEM);

  // if we don't need to perform any scaling, just forward the input frame
  if (!m_needsScaling)
  {
    *processedFrame = frame;
    return 0;
  }

  // make sure that the context is initialized
  int ret = InitializeContext();
  if (ret != 0)
    return ret;

  // allocate a new frame
  *processedFrame = av_frame_alloc();
  if (*processedFrame == nullptr)
    return AVERROR(ENOMEM);

  newFrame = true;

  // allocate space for the picture data
  if ((ret = avpicture_alloc(reinterpret_cast<AVPicture*>(*processedFrame),
    m_targetOptions.GetPixelFormat(), m_targetOptions.GetWidth(), m_targetOptions.GetHeight())) < 0)
  {
    av_frame_free(processedFrame);
    newFrame = false;

    return ret;
  }

  // resize the frame/picture
  sws_scale(m_context, (uint8_t const * const *)frame->data, frame->linesize,
    0, m_originalOptions.GetHeight(), (*processedFrame)->data, (*processedFrame)->linesize);

  // TODO: find out which of the following properties need to be set at all
  (*processedFrame)->width = m_targetOptions.GetWidth();
  (*processedFrame)->height = m_targetOptions.GetHeight();
  (*processedFrame)->format = m_targetOptions.GetPixelFormat();
  (*processedFrame)->sample_aspect_ratio = frame->sample_aspect_ratio;
  (*processedFrame)->pts = frame->pts;
  (*processedFrame)->pkt_pts = frame->pkt_pts;
  (*processedFrame)->pkt_dts = frame->pkt_dts;
  (*processedFrame)->nb_samples = frame->nb_samples;
  (*processedFrame)->key_frame = frame->key_frame;
  (*processedFrame)->pict_type = frame->pict_type;
  (*processedFrame)->coded_picture_number = frame->coded_picture_number;

  return 0;
}

void CSwsScaleFrameProcessor::FreeProcessedFrame(AVFrame** processedFrame, bool newFrame)
{
  if (!newFrame || processedFrame == nullptr || *processedFrame == nullptr)
    return;

  avpicture_free((AVPicture *)*processedFrame);
  av_frame_free(processedFrame);
}

int CSwsScaleFrameProcessor::InitializeContext()
{
  if (m_context != nullptr)
    return 0;

  m_context = sws_getContext(m_originalOptions.GetWidth(), m_originalOptions.GetHeight(), m_originalOptions.GetPixelFormat(),
    m_targetOptions.GetWidth(), m_targetOptions.GetHeight(), m_targetOptions.GetPixelFormat(), m_targetOptions.GetSwsInterpolationMethod(),
    nullptr, nullptr, nullptr);

  if (m_context == nullptr)
    return AVERROR(ENOMEM);

  return 0;
}

bool CSwsScaleFrameProcessor::NeedsScaling(const CTranscodingOptions& originalOptions, const CTranscodingOptions& targetOptions)
{
  return targetOptions.GetWidth() != originalOptions.GetWidth() ||
         targetOptions.GetHeight() != originalOptions.GetHeight() ||
         targetOptions.GetPixelFormat() != originalOptions.GetPixelFormat();
}