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

#include "TranscodingOptions.h"

#include "utils/log.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

CTranscodingOptions::CTranscodingOptions()
  : m_containerFormat("mp4"),
    m_width(0),
    m_height(0),
    m_videoBitrate(500 * 1000), // TODO: ???
    m_segmentDuration(10), // TODO: ???
    m_pixelFormat(AV_PIX_FMT_YUV420P),
    m_swsInterpolationMethod(SWS_BILINEAR)
{
  SetStreamingMethod("http");
}

void CTranscodingOptions::SetStreamingMethod(const std::string& streamingMethod)
{
  m_streamingMethod = streamingMethod;

  // in case of HLS as streaming method an MPEG transport stream is required
  if (m_streamingMethod.compare("hls") == 0 && m_containerFormat.compare("ts") != 0)
  {
    CLog::Log(LOGWARNING, "TranscodingOptions::SetStreamingMethod(): HTTP Live Streaming doesn't support the chosen container format. Using .ts instead");
    m_containerFormat = "ts";
  }
}

void CTranscodingOptions::SetFromCodec(const AVCodecContext* codec)
{
  if (codec == nullptr)
    return;

  switch (codec->codec_type)
  {
  case AVMEDIA_TYPE_VIDEO:
    SetWidth(codec->width);
    SetHeight(codec->height);
    SetPixelFormat(codec->pix_fmt);
    SetVideoBitrate(codec->bit_rate);
    break;

  default:
    break;
  }
}
