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

#include "Transcoder.h"

#include <stdio.h>

#include "URL.h"
#include "filesystem/File.h"
#include "utils/log.h"
#include "utils/URIUtils.h"

TranscoderIdentifier CTranscoder::s_identifier = 0;

CTranscoder::CTranscoder(const std::string& path, ITranscoderCallbacks* callbacks)
  : CThread(this, "Transcoder"),
    m_identifier(s_identifier++),
    m_callbacks(callbacks),
    m_path(path),
    m_transcodedPath(),
    m_transcodingOptions(),
    m_transcodingOptionsSet(false),
    m_iCurrentHLSSegmentNumber(1),
    m_iTotalHLSSegmentNumber(0),
    m_iDuration(0),
    m_iLastVideoPTS(0),
    m_iLastAudioPTS(0),
    packet(),
    frame(nullptr),
    ifmt_ctx(nullptr),
    ofmt_ctx(nullptr),
    filter_ctx(nullptr),
    m_bFoundVideoStream(false),
    m_iVideoStreamIndex(0),
    m_iVideoStreamDuration(0),
    m_bFoundAudioStream(false),
    m_iAudioStreamIndex(0),
    m_iAudioStreamDuration(0),
    m_iVideoWidth(0),
    m_iVideoHeight(0),
    m_eVideoPixelFormat(AV_PIX_FMT_NONE),
    sws_video_ctx(nullptr)
{
  packet.data = nullptr;
  packet.size = 0;
}

CTranscoder::~CTranscoder()
{
  CloseSwsContext();
  CloseOutputFile();
  CloseInputFile();
}

void CTranscoder::SetOptions(CTranscodingOptions transOpts)
{
  m_transcodingOptions = transOpts;
  m_transcodingOptionsSet = true;
}

TranscoderIdentifier CTranscoder::Start()
{
  if (m_path.empty())
    return -1;

  if (!m_transcodingOptionsSet)
    CLog::Log(LOGDEBUG, "CTranscoder: no transcoding options set");

  // TODO: make this generic
  if (m_transcodingOptions.GetStreamingMethod() == "hls")
    m_transcodedPath = TranscodePlaylistPath();
  else
    m_transcodedPath = TranscodePath();

  Create(false);
  return GetIdentifier();
}

void CTranscoder::Stop(bool wait /* = true */)
{
  StopThread(wait);
}

int CTranscoder::InitSwsContext()
{
  sws_video_ctx = sws_getContext(m_iVideoWidth,
    m_iVideoHeight,
    m_eVideoPixelFormat,
    GetTargetWidth(),
    GetTargetHeight(),
    m_transcodingOptions.GetPixelFormat(),
    m_transcodingOptions.GetSwsInterpolationMethod(),
    nullptr, nullptr, nullptr);

  int ret = 0;
  if (sws_video_ctx == nullptr)
    ret = AVERROR(ENOMEM);

  return ret;
}

int CTranscoder::CloseSwsContext()
{
  int ret = 0;
  if (sws_video_ctx)
    sws_freeContext(sws_video_ctx);
  return ret;
}

int CTranscoder::SwsScaleVideo(const AVFrame *src_frame, AVFrame **scaled_frame)
{
  int ret;

  // Allocate a new frame
  *scaled_frame = av_frame_alloc();
  if (*scaled_frame == 0)
  {
    CLog::Log(LOGERROR, "CTranscoder::SwsScaleVideo(): Could not allocate frame.");
    ret = AVERROR(ENOMEM);
    return ret;
  }

  // Allocate space for the picture data
  if ((ret = avpicture_alloc((AVPicture *)*scaled_frame, m_transcodingOptions.GetPixelFormat(),
    GetTargetWidth(), GetTargetHeight())) < 0)
  {
    CLog::Log(LOGERROR, "CTranscoder::SwsScaleVideo(): Could not allocate picture.");
    return ret;
  }

  sws_scale(sws_video_ctx, (uint8_t const * const *)src_frame->data, src_frame->linesize,
    0, m_iVideoHeight, (*scaled_frame)->data, (*scaled_frame)->linesize);
  // TODO: Find out which of the following properties need to be set at all
  (*scaled_frame)->width = GetTargetWidth();
  (*scaled_frame)->height = GetTargetHeight();
  (*scaled_frame)->format = m_transcodingOptions.GetPixelFormat();
  (*scaled_frame)->sample_aspect_ratio = src_frame->sample_aspect_ratio;
  (*scaled_frame)->pts = src_frame->pts;
  (*scaled_frame)->pkt_pts = src_frame->pkt_pts;
  (*scaled_frame)->pkt_dts = src_frame->pkt_dts;
  (*scaled_frame)->nb_samples = src_frame->nb_samples;
  (*scaled_frame)->key_frame = src_frame->key_frame;
  (*scaled_frame)->pict_type = src_frame->pict_type;
  (*scaled_frame)->coded_picture_number = src_frame->coded_picture_number;

  return 0;
}

int CTranscoder::GetTargetWidth() const
{
  int result = m_transcodingOptions.GetWidth();
  if (result != 0)
    return result;

  return m_iVideoWidth;
}

int CTranscoder::GetTargetHeight() const
{
  int result = m_transcodingOptions.GetHeight();
  if (result != 0)
    return result;

  return m_iVideoHeight;
}

int CTranscoder::CreateMediaPlaylist(const char* filename)
{
  int ret = 0;
  if (XFILE::CFile::Exists(filename))
  {
    CLog::Log(LOGWARNING, "CTranscoder::CreateMediaPlaylist(): Playlist already exists: %s", filename);
    return -1;
  }

  XFILE::CFile file;
  if (!file.OpenForWrite(filename, false))
  {
    CLog::Log(LOGWARNING, "CTranscoder::CreateMediaPlaylist(): Playlist could not be created: %s", filename);
    return -2;
  }

  // Precompute number of total segments
  m_iTotalHLSSegmentNumber = static_cast<int>((m_iDuration / AV_TIME_BASE) / m_transcodingOptions.GetSegmentDuration() + 1);

  // Write contents
  std::string playlistHeader = "#EXTM3U\n";
  file.Write(playlistHeader.c_str(), playlistHeader.length());
  //std::string playlistType = "#EXT-X-PLAYLIST-TYPE:VOD\n";
  //file.Write(playlistType.c_str(), playlistType.length());
  std::string playlistTargetDuration = "#EXT-X-TARGETDURATION:" + std::to_string(m_transcodingOptions.GetSegmentDuration()) + "\n";
  file.Write(playlistTargetDuration.c_str(), playlistTargetDuration.length());
  //std::string playlistVersion = "#EXT-X-VERSION:3\n";
  //file.Write(playlistVersion.c_str(), playlistVersion.length());
  std::string playlistMediaSequence = "#EXT-X-MEDIA-SEQUENCE:1\n";
  file.Write(playlistMediaSequence.c_str(), playlistMediaSequence.length());
  for (int s = 1; s <= m_iTotalHLSSegmentNumber; ++s)
  {
    int segmentDuration;
    if (s == m_iTotalHLSSegmentNumber)
    {
      segmentDuration = (m_iDuration / AV_TIME_BASE) % m_transcodingOptions.GetSegmentDuration();
      segmentDuration++;
    }
    else
      segmentDuration = m_transcodingOptions.GetSegmentDuration();

    std::string playlistEntry = "#EXTINF:" + std::to_string(segmentDuration) + ",Description\n";
    file.Write(playlistEntry.c_str(), playlistEntry.length());
    
    std::string playlistEntryFile = TranscodeSegmentPath(s);
    //std::string::size_type beginning = playlistEntryFile.find_last_of("/");
    //if (beginning == std::string::npos)
    //  beginning = playlistEntryFile.find_last_of("\\");
    //if (beginning == std::string::npos)
    //  beginning = -1;
    //beginning++;
    playlistEntryFile = CURL::Encode(playlistEntryFile.c_str()) + "\n";
    file.Write(playlistEntryFile.c_str(), playlistEntryFile.length());
  }
  
  std::string playlistEnd = "#EXT-X-ENDLIST\n";
  file.Write(playlistEnd.c_str(), playlistEnd.length());

  file.Close();
  return ret;
}

int CTranscoder::ShouldStartNewSegment(int64_t time_stamp, const AVRational& time_base)
{
  int ret = 0;
  if (m_iCurrentHLSSegmentNumber >= m_iTotalHLSSegmentNumber)
  {
    // The last segment is currently being created. Don't start a new one!
    ret = 0;
  }
  else if (time_stamp * time_base.num / time_base.den
    >= m_iCurrentHLSSegmentNumber * m_transcodingOptions.GetSegmentDuration())
  {
    m_iCurrentHLSSegmentNumber++;
    ret = 1;
  }

  return ret;
}

void CTranscoder::LogError(int errnum)
{
	char* buf = (char*) malloc(AV_ERROR_MAX_STRING_SIZE * sizeof(char));
	CLog::Log(LOGERROR, "AVERROR 0x%08x: %s", errnum, av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, errnum));
	free(buf);
}

void CTranscoder::ResetAVDictionary(AVDictionary **dict)
{
  if (*dict)
  {
    av_free(*dict);
    *dict = nullptr;
  }
}

int CTranscoder::OpenInputFile(const char *filename)
{
	int ret;
	ifmt_ctx = nullptr;

	if ((ret = avformat_open_input(&ifmt_ctx, filename, nullptr, nullptr)) < 0)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenInputFile(): Cannot open input file '%s'.", filename);
		return ret;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, nullptr)) < 0)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenInputFile(): Cannot find stream information.");
		return ret;
	}

  int duration_time_base = AV_TIME_BASE;
  m_iDuration = ifmt_ctx->duration;
  CLog::Log(LOGDEBUG, "CTranscoder::OpenInputFile(): Duration of the media file: %d/%d",
    m_iDuration, duration_time_base);

  AVDurationEstimationMethod dem = ifmt_ctx->duration_estimation_method;
  if (dem == AVFMT_DURATION_FROM_PTS)
    CLog::Log(LOGDEBUG, "CTranscoder::OpenInputFile(): Duration estimation method: AVFMT_DURATION_FROM_PTS");
  else if (dem == AVFMT_DURATION_FROM_STREAM)
    CLog::Log(LOGDEBUG, "CTranscoder::OpenInputFile(): Duration estimation method: AVFMT_DURATION_FROM_STREAM");
  else if (dem == AVFMT_DURATION_FROM_BITRATE)
    CLog::Log(LOGDEBUG, "CTranscoder::OpenInputFile(): Duration estimation method: AVFMT_DURATION_FROM_BITRATE");
  else
    CLog::Log(LOGERROR, "CTranscoder::OpenInputFile(): Duration estimation method: UNKNOWN");
	
	/* Open video and audio decoders */
  unsigned int i;
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
  {
    AVStream *stream = ifmt_ctx->streams[i];
    AVCodecContext *codec_ctx = stream->codec;
    AVMediaType codec_type = codec_ctx->codec_type;
		if (codec_type == AVMEDIA_TYPE_VIDEO	|| codec_type == AVMEDIA_TYPE_AUDIO)
    {
      AVCodecID codec_id = codec_ctx->codec_id;
      if ((ret = avcodec_open2(codec_ctx, avcodec_find_decoder(codec_id), nullptr)) < 0)
      {
        CLog::Log(LOGERROR, "CTranscoder::OpenInputFile(): Failed to open decoder for stream #%u.", i);
				return ret;
			}
      if (!m_bFoundVideoStream && codec_type == AVMEDIA_TYPE_VIDEO)
      {
        m_bFoundVideoStream = true;
        m_iVideoStreamIndex = i;
        m_iVideoStreamDuration = stream->duration;
        m_iVideoWidth = codec_ctx->width;
        m_iVideoHeight = codec_ctx->height;
        m_eVideoPixelFormat = codec_ctx->pix_fmt;
      }
      if (!m_bFoundAudioStream && codec_type == AVMEDIA_TYPE_AUDIO)
      {
        m_bFoundAudioStream = true;
        m_iAudioStreamIndex = i;
        m_iAudioStreamDuration = stream->duration;
      }
		}
	}

	av_dump_format(ifmt_ctx, 0, filename, 0);
	return ret;
}

int CTranscoder::CloseInputFile()
{
  if (ifmt_ctx)
  {
    for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++)
      avcodec_close(ifmt_ctx->streams[i]->codec);

    avformat_close_input(&ifmt_ctx);
    ifmt_ctx = nullptr;
  }

  return 0;
}

int CTranscoder::OpenOutputFile(const char *filename)
{
	ofmt_ctx = nullptr;
	avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename);
	if (!ofmt_ctx)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile: Could not create output context.");
		return AVERROR_UNKNOWN;
	}

	AVStream *in_stream;
	AVStream *out_stream;
  AVCodecContext *dec_ctx;
  AVCodecContext *enc_ctx;

	int ret = 0;
	unsigned int i;
  for (i = 0; i < ifmt_ctx->nb_streams; i++)
  {
    if (!(out_stream = avformat_new_stream(ofmt_ctx, nullptr)))
    {
      CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile: Failed allocating output stream.");
      return AVERROR_UNKNOWN;
    }

    in_stream = ifmt_ctx->streams[i];
    dec_ctx = in_stream->codec;
    enc_ctx = out_stream->codec;

    AVMediaType codec_type = dec_ctx->codec_type;
    if (codec_type == AVMEDIA_TYPE_VIDEO)
    {
      if ((ret = OpenVideoEncoder(enc_ctx, dec_ctx)) != 0)
      {
        CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile(): Can't open video encoder for stream #%d.", i);
        return ret;
      }
    }
    else if (codec_type == AVMEDIA_TYPE_AUDIO)
    {
      if ((ret = OpenAudioEncoder(enc_ctx, dec_ctx)) != 0)
      {
        CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile(): Can't open audio encoder for stream #%d.", i);
        return ret;
      }
    }
		else if (codec_type == AVMEDIA_TYPE_UNKNOWN)
    {
      CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile(): Elementary stream #%d is of unknown type, cannot proceed!", i);
			return AVERROR_INVALIDDATA;
		}
		else
    {
			if ((ret = avcodec_copy_context(enc_ctx, dec_ctx)) < 0)
      {
        CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile(): Copying stream context failed!");
				return ret;
			}
		}

    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
      enc_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
  {
    if ((ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE)) < 0)
    {
      CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile(): Could not open output file '%s'.", filename);
			return ret;
		}
	}

  // TODO: Set metadata of the ofmt_ctx, i.e. output file

	/* init muxer, write output file header */

  AVDictionary *dictionary = nullptr;
  if (strcmp(ofmt_ctx->oformat->name, "mp4") == 0)
    av_dict_set(&dictionary, "movflags", "faststart", 0);

  if ((ret = avformat_write_header(ofmt_ctx, &dictionary)) < 0)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenOutputFile(): Error occurred when opening output file:");
    LogError(ret);
	}

  if (dictionary)
    av_dict_free(&dictionary);

	return ret;
}

int CTranscoder::OpenVideoEncoder(AVCodecContext* encodingContext, AVCodecContext* decodingContext)
{
  int ret = 0;

  // Use H.264 for video encoding
  AVCodec* encoder = nullptr;
  encoder = avcodec_find_encoder(AV_CODEC_ID_H264);

  if (!encoder)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenVideoEncoder: Neccessary video encoder not found!");
    return AVERROR_ENCODER_NOT_FOUND;
  }
  av_opt_set(encodingContext->priv_data, "profile", "high", AV_OPT_SEARCH_CHILDREN);

  encodingContext->profile = FF_PROFILE_H264_HIGH;
  encodingContext->height = GetTargetHeight();
  encodingContext->width = GetTargetWidth();
  encodingContext->pix_fmt = m_transcodingOptions.GetPixelFormat();
  AVRational sar; sar.num = 1; sar.den = 1;
  encodingContext->sample_aspect_ratio = sar;
  encodingContext->time_base = decodingContext->time_base;
  encodingContext->max_b_frames = 0;
  encodingContext->bit_rate = m_transcodingOptions.GetVideoBitrate();
  encodingContext->bit_rate_tolerance = 4 * m_transcodingOptions.GetVideoBitrate();
  encodingContext->rc_max_rate = 2 * m_transcodingOptions.GetVideoBitrate();
  encodingContext->rc_min_rate = 0;
  encodingContext->rc_buffer_size = 1 * 1000 * 1000;
  // TODO: Some of the following settings are needed for a correctly working encoder.
  // Find out which one or which combination of them!
  encodingContext->flags = 2143289344;
  encodingContext->gop_size = -1;
  encodingContext->b_frame_strategy = -1;
  encodingContext->coder_type = -1;
  encodingContext->me_method = -1;
  encodingContext->me_subpel_quality = -1;
  encodingContext->me_cmp = -1;
  encodingContext->me_range = -1;
  encodingContext->scenechange_threshold = -1;
  encodingContext->i_quant_factor = -1;
  encodingContext->qcompress = -1;
  encodingContext->qmin = -1;
  encodingContext->qmax = -1;
  encodingContext->max_qdiff = -1;

  AVDictionary* dictionary = nullptr;
  av_dict_set(&dictionary, "preset", "slow", 0);
  av_dict_set(&dictionary, "vprofile", "high", 0);
  if ((ret = avcodec_open2(encodingContext, encoder, &dictionary)) < 0)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenVideoEncoder(): Cannot open video encoder!");
    LogError(ret);
  }
  av_dict_free(&dictionary);

  return ret;
}

int CTranscoder::OpenAudioEncoder(AVCodecContext* encodingContext, AVCodecContext* decodingContext)
{
  int ret = 0;

  // Use AAC for audio encoding
  AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_AAC);
  if (encoder == nullptr)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenAudioEncoder(): Neccessary audio encoder not found!");
    return AVERROR_ENCODER_NOT_FOUND;
  }

  encodingContext->sample_rate = decodingContext->sample_rate;
  encodingContext->channel_layout = decodingContext->channel_layout;
  encodingContext->channels = av_get_channel_layout_nb_channels(encodingContext->channel_layout);
  encodingContext->sample_fmt = encoder->sample_fmts[0];
  encodingContext->time_base = decodingContext->time_base;
  encodingContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

  if ((ret = avcodec_open2(encodingContext, encoder, nullptr)) < 0)
  {
    CLog::Log(LOGERROR, "CTranscoder::OpenAudioEncoder(): Cannot open audio encoder!");
    LogError(ret);
  }

  return ret;
}

int CTranscoder::CloseOutputFile()
{
  int ret = 0;
  if (ifmt_ctx && ofmt_ctx)
  {
    FlushFiltersAndEncoders();
    av_write_trailer(ofmt_ctx);
  }

  if (frame)
    av_frame_free(&frame);

  if (ifmt_ctx)
  {
    unsigned int i;
    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
      if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && ofmt_ctx->streams[i]->codec)
        avcodec_close(ofmt_ctx->streams[i]->codec);

      if (filter_ctx && filter_ctx[i].filter_graph)
        avfilter_graph_free(&filter_ctx[i].filter_graph);
    }
  }

  if (ofmt_ctx)
  {
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
      avio_closep(&ofmt_ctx->pb);

    avformat_free_context(ofmt_ctx);
    ofmt_ctx = nullptr;
  }

  if (filter_ctx)
  {
    av_free(filter_ctx);
    filter_ctx = nullptr;
  }

  return ret;
}

int CTranscoder::InitFilter(FilteringContext* fctx, AVCodecContext *dec_ctx,
	AVCodecContext *enc_ctx, const char *filter_spec)
{
	char args[512];
	int ret = 0;
	AVFilter *buffersrc = nullptr;
	AVFilter *buffersink = nullptr;
	AVFilterContext *buffersrc_ctx = nullptr;
	AVFilterContext *buffersink_ctx = nullptr;
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	AVFilterGraph *filter_graph = avfilter_graph_alloc();

	if (!outputs || !inputs || !filter_graph)
  {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
  {
		buffersrc = avfilter_get_by_name("buffer");
		buffersink = avfilter_get_by_name("buffersink");
		if (!buffersrc || !buffersink) 
    {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Filtering source or sink element not found.");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
      GetTargetWidth(), GetTargetHeight(), m_transcodingOptions.GetPixelFormat(),
			dec_ctx->time_base.num, dec_ctx->time_base.den,
			dec_ctx->sample_aspect_ratio.num,
			dec_ctx->sample_aspect_ratio.den);

		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, nullptr, filter_graph);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot create buffer source.");
			goto end;
		}

		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			nullptr, nullptr, filter_graph);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot create buffer sink.");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "pix_fmts",
			(uint8_t*)&enc_ctx->pix_fmt, sizeof(enc_ctx->pix_fmt),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot set output pixel format.");
			goto end;
		}
	}
	else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
		buffersrc = avfilter_get_by_name("abuffer");
		buffersink = avfilter_get_by_name("abuffersink");
		if (!buffersrc || !buffersink) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Filtering source or sink element not found.");
			ret = AVERROR_UNKNOWN;
			goto end;
		}

		if (!dec_ctx->channel_layout)
			dec_ctx->channel_layout =
			av_get_default_channel_layout(dec_ctx->channels);
		snprintf(args, sizeof(args),
			"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
			dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
			av_get_sample_fmt_name(dec_ctx->sample_fmt),
			dec_ctx->channel_layout);
		ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
			args, nullptr, filter_graph);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot create audio buffer source.");
			goto end;
		}

		ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
			nullptr, nullptr, filter_graph);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot create audio buffer sink.");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
			(uint8_t*)&enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot set output sample format.");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "channel_layouts",
			(uint8_t*)&enc_ctx->channel_layout,
			sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot set output channel layout.");
			goto end;
		}

		ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
			(uint8_t*)&enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate),
			AV_OPT_SEARCH_CHILDREN);
		if (ret < 0) {
      CLog::Log(LOGERROR, "CTranscoder::InitFilter(): Cannot set output sample rate.");
			goto end;
		}
	}
	else {
		ret = AVERROR_UNKNOWN;
		goto end;
	}

	// Endpoints for the filter graph
	outputs->name = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = nullptr;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = nullptr;

	if (!outputs->name || !inputs->name) {
		ret = AVERROR(ENOMEM);
		goto end;
	}

	if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
		&inputs, &outputs, nullptr)) < 0)
		goto end;

	if ((ret = avfilter_graph_config(filter_graph, nullptr)) < 0)
		goto end;

	// Fill filtering context
	fctx->buffersrc_ctx = buffersrc_ctx;
	fctx->buffersink_ctx = buffersink_ctx;
	fctx->filter_graph = filter_graph;

end:
	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	return ret;
}

int CTranscoder::InitFilters()
{
	int ret = 0;
	filter_ctx = (FilteringContext*) av_malloc_array(ifmt_ctx->nb_streams, sizeof(*filter_ctx));
  if (!filter_ctx)
		return AVERROR(ENOMEM);

	unsigned int i;
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		filter_ctx[i].buffersrc_ctx = nullptr;
		filter_ctx[i].buffersink_ctx = nullptr;
		filter_ctx[i].filter_graph = nullptr;
    AVMediaType codec_type = ifmt_ctx->streams[i]->codec->codec_type;
	  const char *filter_spec;
    if (codec_type == AVMEDIA_TYPE_VIDEO)
			filter_spec = "null"; /* passthrough (dummy) filter for video */
    else if (codec_type == AVMEDIA_TYPE_AUDIO)
      filter_spec = "anull"; /* passthrough (dummy) filter for audio */
    else
      continue;

    if ((ret = InitFilter(&filter_ctx[i], ifmt_ctx->streams[i]->codec,
			ofmt_ctx->streams[i]->codec, filter_spec)) != 0)
      break;
	}
	return ret;
}

int CTranscoder::EncodeWriteFrame(AVFrame *filt_frame, unsigned int stream_index, int *got_frame) {
	int ret;
	int got_frame_local;
	AVPacket enc_pkt;
	int(*enc_func)(AVCodecContext *, AVPacket *, const AVFrame *, int *) =
		(ifmt_ctx->streams[stream_index]->codec->codec_type ==
		AVMEDIA_TYPE_VIDEO) ? avcodec_encode_video2 : avcodec_encode_audio2;

	if (!got_frame)
		got_frame = &got_frame_local;

  //CLog::Log(LOGINFO, "Encoding frame\n");
	/* encode filtered frame */
	enc_pkt.data = nullptr;
	enc_pkt.size = 0;
	av_init_packet(&enc_pkt);
	ret = enc_func(ofmt_ctx->streams[stream_index]->codec, &enc_pkt,
		filt_frame, got_frame);
	av_frame_free(&filt_frame);
	if (ret < 0)
		return ret;
	if (!(*got_frame))
		return 0;

	/* prepare packet for muxing */
	enc_pkt.stream_index = stream_index;
	av_packet_rescale_ts(&enc_pkt,
		ofmt_ctx->streams[stream_index]->codec->time_base,
		ofmt_ctx->streams[stream_index]->time_base);

  //CLog::Log(LOGDEBUG, "Muxing frame\n");
	/* mux encoded frame */
	ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
	return ret;
}

int CTranscoder::FilterEncodeWriteFrame(AVFrame *frame, unsigned int stream_index)
{
	int ret;
	AVFrame *filt_frame;

  //CLog::Log(LOGINFO, "Pushing decoded frame to filters\n");
	/* push the decoded frame into the filtergraph */
	ret = av_buffersrc_add_frame_flags(filter_ctx[stream_index].buffersrc_ctx,
		frame, 0);
	if (ret < 0) {
    CLog::Log(LOGERROR, "Error while feeding the filtergraph.");
		return ret;
	}

	/* pull filtered frames from the filtergraph */
	while (1) {
		filt_frame = av_frame_alloc();
		if (!filt_frame) {
			ret = AVERROR(ENOMEM);
			break;
		}
    //CLog::Log(LOGINFO, "Pulling filtered frame from filters\n");
		ret = av_buffersink_get_frame(filter_ctx[stream_index].buffersink_ctx,
			filt_frame);
		if (ret < 0) {
			/* if no more frames for output - returns AVERROR(EAGAIN)
			* if flushed and no more frames for output - returns AVERROR_EOF
			* rewrite retcode to 0 to show it as normal procedure completion
			*/
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				ret = 0;
			av_frame_free(&filt_frame);
			break;
		}

		filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
		ret = EncodeWriteFrame(filt_frame, stream_index, nullptr);
		if (ret < 0)
			break;
	}

	return ret;
}

int CTranscoder::FlushEncoder(unsigned int stream_index)
{
	int ret;
	int got_frame;

	if (!(ofmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;

	while (1) {
    //CLog::Log(LOGINFO, "Flushing stream #%u encoder\n", stream_index);
    ret = EncodeWriteFrame(nullptr, stream_index, &got_frame);
		if (ret < 0)
			break;
		if (!got_frame)
			return 0;
	}
	return ret;
}

int CTranscoder::FlushFiltersAndEncoders()
{
  int ret = 0;
  unsigned int i;
  for (i = 0; i < ifmt_ctx->nb_streams; i++)
  {
    // Flush filter
    if (!filter_ctx[i].filter_graph)
    {
      continue;
    }
    if ((ret = FilterEncodeWriteFrame(nullptr, i)) < 0)
    {
      CLog::Log(LOGERROR, "CTranscoder::FlushFiltersAndEncoders(): Flushing filter failed.");
      LogError(ret);
      break;
    }

    // Flush encoder
    if ((ret = FlushEncoder(i)) < 0)
    {
      CLog::Log(LOGERROR, "CTranscoder::FlushFiltersAndEncoders(): Flushing encoder failed.");
      LogError(ret);
      break;
    }
  }
  return ret;
}

void CTranscoder::Run()
{
  int ret;
  CLog::Log(LOGDEBUG, "CTranscoder::Run() was called.");
  
  if (m_path.empty()) {
    CLog::Log(LOGERROR, "CTranscoder::Run(): Path to input file must not be empty.");
    return;
  }

  CLog::Log(LOGDEBUG, "CTranscoder::Run(): Input file: %s", m_path.c_str());

  avfilter_register_all();
  avcodec_register_all();
  av_register_all();

  // TODO: Get rid of code duplication for HTTP and HLS transcoding
  // HTTP Live Streaming
  if (m_transcodingOptions.GetStreamingMethod() == "hls")
  {
    // Input file and sws context are only created once
    if ((ret = OpenInputFile(m_path.c_str())) < 0)
      goto end;
    if ((ret = InitSwsContext()) < 0)
      goto end;

    // Output file and filters need to be created for every media segment file
    std::string pathSegment = TranscodeSegmentPath();
    if ((ret = OpenOutputFile(pathSegment.c_str())) < 0)
      goto end;
    if ((ret = InitFilters()) < 0)
      goto end;

    CLog::Log(LOGDEBUG, "CTranscoder::Run(): Output file: %s", m_transcodedPath.c_str());
    CreateMediaPlaylist(m_transcodedPath.c_str());

    unsigned int stream_index;
    int got_frame;

    while (1)
    {
      if (m_bStop)
        CLog::Log(LOGDEBUG, "CTranscoder::Run(): Transcoder asked to stop!");
      if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
      {
        if (ret == AVERROR_EOF)
        {
          CLog::Log(LOGDEBUG, "CTranscoder::Run(): Reached end of file!");
          ret = 0;
        }
        break;
      }
      stream_index = packet.stream_index;
      AVStream *stream = ifmt_ctx->streams[stream_index];
      AVCodecContext *codec_ctx = stream->codec;
      AVMediaType codec_type = codec_ctx->codec_type;
      //CLog::Log(LOGDEBUG, "CTranscoder::Run(): Demuxer gave frame of stream_index %u\n", stream_index);

      if (filter_ctx[stream_index].filter_graph)
      {
        //CLog::Log(LOGDEBUG, "CTranscoder::Run(): Going to reencode and filter the frame\n"); 
        if ((frame = av_frame_alloc()) == nullptr)
        {
          ret = AVERROR(ENOMEM);
          break;
        }
        av_packet_rescale_ts(&packet, stream->time_base, codec_ctx->time_base);
        if (codec_type == AVMEDIA_TYPE_VIDEO)
          ret = avcodec_decode_video2(codec_ctx, frame, &got_frame, &packet);
        else if (codec_type == AVMEDIA_TYPE_AUDIO)
          ret = avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);
        else
        {
          CLog::Log(LOGERROR, "CTranscoder::Run(): Got packet of unexpected media type.");
          ret = -1;
        }
        if (ret < 0) {
          av_frame_free(&frame);
          CLog::Log(LOGERROR, "CTranscoder::Run(): Decoding failed.");
          break;
        }

        if (got_frame)
        {
          frame->pts = av_frame_get_best_effort_timestamp(frame);
          int64_t last_pts = frame->pts;
          if (codec_type == AVMEDIA_TYPE_VIDEO)
          {
            m_iLastVideoPTS = last_pts;
            // Rescale the video frame
            AVFrame *scaledFrame;
            if ((ret = SwsScaleVideo(frame, &scaledFrame)) == 0)
            {
              ret = FilterEncodeWriteFrame(scaledFrame, stream_index);
              avpicture_free((AVPicture *)scaledFrame);
              av_frame_free(&scaledFrame);
              av_frame_free(&frame);
            }
            else
            {
              CLog::Log(LOGERROR, "CTranscoder::Run(): Scaling of video frame failed.");
              if (scaledFrame)
              {
                avpicture_free((AVPicture *)scaledFrame);
                av_frame_free(&scaledFrame);
              }
              av_frame_free(&frame);
            }

            // See if it's time to start a new segment
            if (ShouldStartNewSegment(last_pts, codec_ctx->time_base))
            {
              CLog::Log(LOGDEBUG, "CTranscoder::Run(): Time to start segment %d.", m_iCurrentHLSSegmentNumber);
              CloseOutputFile();

              pathSegment = TranscodeSegmentPath();
              if ((ret = OpenOutputFile(pathSegment.c_str())) < 0)
                goto end;
              if ((ret = InitFilters()) < 0)
                goto end;
            }
          }
          else if (codec_type == AVMEDIA_TYPE_AUDIO)
          {
            m_iLastAudioPTS = last_pts;
            ret = FilterEncodeWriteFrame(frame, stream_index);
            av_frame_free(&frame);
          }
          // This should not happen
          else
            CLog::Log(LOGERROR, "CTranscoder::Run(): Only video and audio frames are sent to the filter graph.");
          if (ret < 0)
            goto end;
        }
        else
          av_frame_free(&frame);
      }
      else
      {
        /* remux this frame without reencoding */
        av_packet_rescale_ts(&packet,
          ifmt_ctx->streams[stream_index]->time_base,
          ofmt_ctx->streams[stream_index]->time_base);

        ret = av_interleaved_write_frame(ofmt_ctx, &packet);
        if (ret < 0)
          goto end;
      }
      av_free_packet(&packet);
    }

    // Done reading input file
    return;
  }

  // HTTP single file
  CLog::Log(LOGDEBUG, "CTranscoder::Run(): Output file: %s", m_transcodedPath.c_str());

  if ((ret = OpenInputFile(m_path.c_str())) < 0)
    goto end;
  if ((ret = InitSwsContext()) < 0)
    goto end;
  if ((ret = OpenOutputFile(m_transcodedPath.c_str())) < 0)
    goto end;
  if ((ret = InitFilters()) < 0)
    goto end;

  unsigned int stream_index;
  int got_frame;

  while (1)
  {
    if (m_bStop)
      CLog::Log(LOGDEBUG, "CTranscoder::Run(): Transcoder asked to stop!");
    if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
    {
      if (ret == AVERROR_EOF)
      {
        CLog::Log(LOGDEBUG, "CTranscoder::Run(): Reached end of file!");
        ret = 0;
      }
      break;
    }
    stream_index = packet.stream_index;
    AVStream *stream = ifmt_ctx->streams[stream_index];
    AVCodecContext *codec_ctx = stream->codec;
    AVMediaType codec_type = codec_ctx->codec_type;
    //CLog::Log(LOGDEBUG, "CTranscoder::Run(): Demuxer gave frame of stream_index %u\n", stream_index);

    if (filter_ctx[stream_index].filter_graph)
    {
      //CLog::Log(LOGDEBUG, "CTranscoder::Run(): Going to reencode and filter the frame\n"); 
      if ((frame = av_frame_alloc()) == nullptr)
      {
        ret = AVERROR(ENOMEM);
        break;
      }
      av_packet_rescale_ts(&packet, stream->time_base, codec_ctx->time_base);
      if (codec_type == AVMEDIA_TYPE_VIDEO)
        ret = avcodec_decode_video2(codec_ctx, frame, &got_frame, &packet);
      else if (codec_type == AVMEDIA_TYPE_AUDIO)
        ret = avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);
      else
      {
        CLog::Log(LOGERROR, "CTranscoder::Run(): Got packet of unexpected media type.");
        ret = -1;
      }
      if (ret < 0) {
        av_frame_free(&frame);
        CLog::Log(LOGERROR, "CTranscoder::Run(): Decoding failed.");
        break;
      }

      if (got_frame)
      {
        frame->pts = av_frame_get_best_effort_timestamp(frame);
        if (codec_type == AVMEDIA_TYPE_VIDEO)
        {
          // Rescale the video frame
          AVFrame *scaledFrame;
          if ((ret = SwsScaleVideo(frame, &scaledFrame)) == 0)
          {
            ret = FilterEncodeWriteFrame(scaledFrame, stream_index);
            avpicture_free((AVPicture *)scaledFrame);
            av_frame_free(&scaledFrame);
            av_frame_free(&frame);
          }
          else
          {
            CLog::Log(LOGERROR, "CTranscoder::Run(): Scaling of video frame failed.");
            if (scaledFrame)
            {
              avpicture_free((AVPicture *)scaledFrame);
              av_frame_free(&scaledFrame);
            }
            av_frame_free(&frame);
          }
        }
        else
        {
          ret = FilterEncodeWriteFrame(frame, stream_index);
          av_frame_free(&frame);
        }
        if (ret < 0)
          goto end;
      }
      else
        av_frame_free(&frame);
    }
    else
    {
      /* remux this frame without reencoding */
      av_packet_rescale_ts(&packet,
        ifmt_ctx->streams[stream_index]->time_base,
        ofmt_ctx->streams[stream_index]->time_base);

      ret = av_interleaved_write_frame(ofmt_ctx, &packet);
      if (ret < 0)
        goto end;
    }
    av_free_packet(&packet);
  }

end:
  if (ret < 0)
    LogError(ret);
}

void CTranscoder::OnStartup()
{
  CLog::Log(LOGDEBUG, "CTranscoder::OnStartup() was called.");
}

void CTranscoder::OnExit()
{
  CLog::Log(LOGDEBUG, "CTranscoder::OnExit() was called.");
}

void CTranscoder::OnTranscodingError()
{
  OnTranscodingError(GetIdentifier());
}

void CTranscoder::OnTranscodingStopped()
{
  OnTranscodingStopped(GetIdentifier());
}

void CTranscoder::OnTranscodingFinished()
{
  OnTranscodingFinished(GetIdentifier());
}

void CTranscoder::OnTranscodingError(TranscoderIdentifier identifier)
{
  if (m_callbacks == nullptr)
    return;

  m_callbacks->OnTranscodingError(identifier);
}

void CTranscoder::OnTranscodingStopped(TranscoderIdentifier identifier)
{
  if (m_callbacks == nullptr)
    return;

  m_callbacks->OnTranscodingStopped(identifier);
}

void CTranscoder::OnTranscodingFinished(TranscoderIdentifier identifier)
{
  if (m_callbacks == nullptr)
    return;

  m_callbacks->OnTranscodingFinished(identifier);
}

std::string CTranscoder::TranscodePath() const
{
  std::string path = m_path;
  URIUtils::RemoveExtension(path);

  return path + "-transcoded." +
    m_transcodingOptions.GetFileExtension();
}

std::string CTranscoder::TranscodePlaylistPath() const
{
  std::string path = m_path;
  URIUtils::RemoveExtension(path);

  return path + "-transcoded.m3u8";
}

std::string CTranscoder::TranscodeSegmentPath(int segment /* = 0 */) const
{
  std::string path = m_path;
  URIUtils::RemoveExtension(path);
  int segmentNumber = (segment == 0) ? m_iCurrentHLSSegmentNumber : segment;

  return path + "-transcoded" +
    std::to_string((segment == 0) ? m_iCurrentHLSSegmentNumber : segment) + "." +
    m_transcodingOptions.GetFileExtension();
}
