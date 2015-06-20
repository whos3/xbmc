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

extern "C" {
#include "libavutil/pixdesc.h" // TODO
}

class CTranscodingOptions
{
public:
  CTranscodingOptions();
  virtual ~CTranscodingOptions() { }

  inline const std::string& GetFileExtension() const { return m_containerFormat; }
  inline void SetFileExtension(const std::string& fileExtension) { m_containerFormat = fileExtension; }

  inline const std::string& GetStreamingMethod() const { return m_streamingMethod; }
  void SetStreamingMethod(const std::string& streamingMethod);

  inline int GetWidth() const { return m_width; }
  inline void SetWidth(int width) { m_width = width; }

  inline int GetHeight() const { return m_height; }
  inline void SetHeight(int height) { m_height = height; }

  inline int GetVideoBitrate() const { return m_videoBitrate; }
  inline void SetVideoBitrate(int videoBitrate) { m_videoBitrate = videoBitrate; }

  inline int GetSegmentDuration() const { return m_segmentDuration; }
  inline void SetSegmentDuration(int segmentDuration) { m_segmentDuration = segmentDuration; }

  inline AVPixelFormat GetPixelFormat() const { return m_pixelFormat; }
  inline void SetPixelFormat(AVPixelFormat pixelFormat) { m_pixelFormat = pixelFormat; }

  inline int GetSwsInterpolationMethod() const { return m_swsInterpolationMethod; }
  inline void SetSwsInterpolationMethod(int swsInterpolationMethod) { m_swsInterpolationMethod = swsInterpolationMethod; }

private:
  std::string m_containerFormat;
  std::string m_streamingMethod;
  int m_width;
  int m_height;
  int m_videoBitrate;
  int m_segmentDuration;
  AVPixelFormat m_pixelFormat;
  int m_swsInterpolationMethod;

};