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

#include <map>
#include <memory>

#include "cores/Transcoder/ITranscoderCallbacks.h"
#include "cores/Transcoder/TranscodingOptions.h"
#include "threads/CriticalSection.h"

class CTranscoder;
using TranscoderPtr = std::shared_ptr<CTranscoder>;

class CTranscoderManager : public ITranscoderCallbacks
{
public:
  CTranscoderManager(const CTranscoderManager&) = delete;
  CTranscoderManager& operator=(const CTranscoderManager&) = delete;
  virtual ~CTranscoderManager();

  static CTranscoderManager& GetInstance();

  TranscoderPtr Start(const std::string& path);
  TranscoderPtr Start(const std::string& path, const CTranscodingOptions& options);
  TranscoderPtr Get(TranscoderIdentifier identifier) const;
  bool GetTranscodedPath(TranscoderIdentifier identifier, std::string& transcodedPath) const;
  void Stop(TranscoderIdentifier identifier, bool wait = true);

protected:
  // implementations of ITranscoderCallbacks
  virtual void OnTranscodingError(TranscoderIdentifier identifier) override;
  virtual void OnTranscodingStopped(TranscoderIdentifier identifier) override;
  virtual void OnTranscodingFinished(TranscoderIdentifier identifier) override;

private:
  CTranscoderManager();

  // TODO

  std::map<TranscoderIdentifier, TranscoderPtr> m_transcoders;
  CCriticalSection m_transcodersLock;
};

