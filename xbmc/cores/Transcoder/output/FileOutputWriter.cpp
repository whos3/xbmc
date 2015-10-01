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

#include "FileOutputWriter.h"

extern "C" {
#include "libavformat/avformat.h"
}

CFileOutputWriter::CFileOutputWriter(const std::string& file)
  : IOutputWriter(file),
    m_context(nullptr)
{ }

CFileOutputWriter::~CFileOutputWriter()
{
  if (m_context != nullptr)
  {
    // write the trailer
    av_write_trailer(m_context);

    // close all stream codecs
    for (unsigned int i = 0; i < m_context->nb_streams; i++)
    {
      if (m_context->streams[i] && m_context->streams[i]->codec)
        avcodec_close(m_context->streams[i]->codec);
    }

    // close the file
    avio_closep(&m_context->pb);

    // destroy the context
    avformat_free_context(m_context);
    m_context = nullptr;
  }
}

AVStream* CFileOutputWriter::AddStream()
{
  if (m_context == nullptr)
  {
    if (avformat_alloc_output_context2(&m_context, nullptr, nullptr, m_file.c_str()) != 0 || m_context == nullptr)
      return nullptr;

    if (avio_open(&m_context->pb, m_file.c_str(), AVIO_FLAG_WRITE) != 0)
    {
      avformat_free_context(m_context);
      m_context = nullptr;

      return nullptr;
    }
  }
  
  return avformat_new_stream(m_context, nullptr);
}

int CFileOutputWriter::WriteHeader(AVDictionary** header)
{
  if (m_context == nullptr || header == nullptr)
    return AVERROR(ENOMEM); // TODO

  return avformat_write_header(m_context, header);
}

int CFileOutputWriter::Write(const AVStream const* stream, AVFrame* frame)
{
  if (m_context == nullptr || stream == nullptr || frame == nullptr)
    return AVERROR(ENOMEM); // TODO

  auto encoder = (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) ? avcodec_encode_video2 : avcodec_encode_audio2;

  // prepare the encoded packet
  AVPacket encodedPacket;
  encodedPacket.data = nullptr;
  encodedPacket.size = 0;
  av_init_packet(&encodedPacket);

  // encode the frame into a packet
  int gotFrame = 0;
  int ret = encoder(stream->codec, &encodedPacket, frame, &gotFrame);
  av_frame_free(&frame);

  if (ret != 0)
    return ret;

  // if we got no packet there's nothing else to do
  if (gotFrame == 0)
    return 0;

  // prepare the packet for muxing
  encodedPacket.stream_index = stream->index;
  av_packet_rescale_ts(&encodedPacket, stream->codec->time_base, stream->time_base);

  // mux the packet
  return av_interleaved_write_frame(m_context, &encodedPacket);
}
