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

#include "TranscoderOperations.h"
#include "URL.h"
#include "cores/Transcoder/Transcoder.h"
#include "cores/Transcoder/TranscoderManager.h"
#include "filesystem/File.h"
#include "interfaces/json-rpc/VideoLibrary.h"
#include "utils/log.h"

using namespace JSONRPC;

JSONRPC_STATUS CTranscoderOperations::Transcode(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CFileItemList items;
  CVideoLibrary::FillFileItemList(parameterObject["item"], items);

  if (items.Size() != 1)
    return InternalError;

  CFileItemPtr item = items[0];
  std::string path = item->GetPath();

  TranscoderPtr transcoder = CTranscoderManager::GetInstance().Start(path, ParseOptions(parameterObject["options"]));
  if (transcoder == nullptr)
    return InvalidParams;

  std::string transcodedPath = transcoder->GetTranscodedPath();

  result["protocol"] = "http";

  if ((transport->GetCapabilities() & FileDownloadDirect) == FileDownloadDirect)
    result["mode"] = "direct";
  else
    result["mode"] = "redirect";

  std::string url = "vfs/";
  url += CURL::Encode(transcodedPath);
  result["details"]["path"] = url;

  return OK;
}

CTranscodingOptions CTranscoderOperations::ParseOptions(const CVariant& options)
{
  CTranscodingOptions transcodingOptions;

  const CVariant& containerFormat = options["container"];
  if (!containerFormat.empty())
    transcodingOptions.SetFileExtension(containerFormat.asString());

  const CVariant& streamingMethod = options["streaming"];
  if (!streamingMethod.empty())
  {
    std::string strStreamingMethod = streamingMethod.asString();
    // check if a valid streaming method was provided
    if (strStreamingMethod.compare("http") == 0 || strStreamingMethod.compare("hls") == 0)
      transcodingOptions.SetStreamingMethod(strStreamingMethod);
  }

  const CVariant& width = options["width"];
  if (width.asInteger() > 0)
    transcodingOptions.SetWidth(static_cast<int>(width.asInteger()));

  const CVariant& height = options["height"];
  if (height.asInteger() > 0)
    transcodingOptions.SetHeight(static_cast<int>(height.asInteger()));

  const CVariant& videoBitrate = options["videobitrate"];
  if (videoBitrate.asInteger() > 0)
    transcodingOptions.SetVideoBitrate(static_cast<int>(videoBitrate.asInteger()));

  const CVariant& segmentDuration = options["segmentduration"];
  if (segmentDuration.asInteger() > 0)
    transcodingOptions.SetSegmentDuration(static_cast<int>(segmentDuration.asInteger()));

  return transcodingOptions;
}
