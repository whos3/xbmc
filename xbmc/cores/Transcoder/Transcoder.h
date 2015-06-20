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

#include <string>

#include "cores/Transcoder/TranscodingOptions.h"
#include "threads/Thread.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavfilter/avfiltergraph.h" // TODO
#include "libavfilter/avcodec.h" // TODO
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h" // TODO
}

class CTranscoder : public IRunnable, protected CThread
{
public:
  CTranscoder();
  virtual ~CTranscoder();

  /** Starts transcoding in a new thread
   * \param path Path to the video to be transcoded.
   * \return Returns always 1.
   */
  int Transcode(std::string path);

  /** Get path of the transcoded video.
   * \param path Path to the video to be transcoded.
   * \return Returns the path to the transcoded video.
   */
  std::string TranscodePath(const std::string &path) const;

  /** Get path of the HLS media playlist of the transcoded video.
   * \param path Path to the video to be transcoded.
   * \return Returns the path to the media playlist.
   */
  std::string TranscodePlaylistPath(const std::string &path) const;

  /** Get path of a transcoded HLS media segment.
   * \param path Path to the video to be transcoded.
   * \param segment Number of the media segment.
   * \return Returns the path to the transcoded media segment.
   */
  std::string TranscodeSegmentPath(const std::string &path, int segment = 0) const;

  /** Set transcoding options for this transcoder. SHOULD be called before
   * Transcode(std::string path).
   * \param transOpts Options to be used for transcoding.
   */
  void SetTranscodingOptions(CTranscodingOptions transOpts);

protected:
  // implementation of IRunnable
  virtual void Run() override;

  // specializations of CThread
  virtual void OnStartup() override;
  virtual void OnExit() override;

private:
  CTranscodingOptions m_transcodingOptions;
  bool m_transcodingOptionsSet;

  typedef struct FilteringContext
  {
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
  } FilteringContext;

  int OpenInputFile(const char *filename);
  int CloseInputFile();

  int OpenOutputFile(const char *filename);
  int CloseOutputFile();

  int OpenVideoEncoder(AVCodecContext* encodingContext, AVCodecContext* decodingContext);
  int OpenAudioEncoder(AVCodecContext* encodingContext, AVCodecContext* decodingContext);

  int InitFilter(FilteringContext* fctx, AVCodecContext *dec_ctx,
    AVCodecContext *enc_ctx, const char *filter_spec);
  int InitFilters(void);
  int EncodeWriteFrame(AVFrame *filt_frame, unsigned int stream_index, int *got_frame);
  int FilterEncodeWriteFrame(AVFrame *frame, unsigned int stream_index);
  int FlushEncoder(unsigned int stream_index);
  int FlushFiltersAndEncoders();

  int InitSwsContext();
  int SwsScaleVideo(const AVFrame *src_frame, AVFrame **scaled_frame);
  int CloseSwsContext();

  int GetTargetWidth() const;
  int GetTargetHeight() const;

  // HTTP Live Streaming related members

  /** Writes the media playlist into a file.
   * \param filename File name of the playlist to create.
   * \return Returns 0 on success, a negative value otherwise.
   */
  int CreateMediaPlaylist(const char* filename);
  /** Number of the media segment that is currently being created.*/
  int m_iCurrentHLSSegmentNumber;
  /** Number of media segments that will be created.*/
  int m_iTotalHLSSegmentNumber;
  /** Duration of the input file and the total duration of all output segments.*/
  int64_t m_iDuration;
  /** PTS of the last decoded video frame.*/
  int64_t m_iLastVideoPTS;
  /** PTS of the last decoded audio frame.*/
  int64_t m_iLastAudioPTS;
  int ShouldStartNewSegment(int64_t time_stamp, const AVRational& time_base);

  /** Log an AVERROR code in a more readable way.
   * \param errnum The AVERROR code to be logged.
   */
  static void LogError(int errnum);
  static void ResetAVDictionary(AVDictionary **dict);

  /** Path to the input file.*/
  std::string path;

  // TODO: Make this a local variable if possible
  AVPacket packet;

  // TODO: Make this a local variable if possible
  AVFrame *frame;

  /** Format context of the input file.*/
  AVFormatContext *ifmt_ctx;
  /** Format context of the output file.*/
  AVFormatContext *ofmt_ctx;

  /** Filter graph and filter contexts of the transcoder.*/
  FilteringContext *filter_ctx;

  /** True iff the input file contains a video stream that we can decode.*/
  bool m_bFoundVideoStream;
  /** Index of the input video stream in the input format context.*/
  unsigned int m_iVideoStreamIndex;
  int64_t m_iVideoStreamDuration;
  /** True iff the input file contains an audio stream that we can decode.*/
  bool m_bFoundAudioStream;
  /** Index of the input audio stream in the input format context.*/
  unsigned int m_iAudioStreamIndex;
  int64_t m_iAudioStreamDuration;
  /** Width of the input video stream.*/
  int m_iVideoWidth;
  /** Height of the input video stream.*/
  int m_iVideoHeight;
  /** Pixel format of the input video stream.*/
  AVPixelFormat m_eVideoPixelFormat;
  /** Sws context used for scaling the video frames.*/
  SwsContext *sws_video_ctx;
};
