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

#include "TranscoderManager.h"
#include "cores/Transcoder/Transcoder.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

CTranscoderManager::CTranscoderManager()
  : m_transcoders()
{ }


CTranscoderManager::~CTranscoderManager()
{
  CSingleLock lock(m_transcodersLock);
  for (const auto& transcoder : m_transcoders)
    Stop(transcoder.first, true);

  m_transcoders.clear();
}

CTranscoderManager& CTranscoderManager::GetInstance()
{
  static CTranscoderManager instance;
  return instance;
}

TranscoderPtr CTranscoderManager::Start(const std::string& path)
{
  if (path.empty())
    return false;

  TranscoderPtr transcoder = std::make_shared<CTranscoder>(path, this);
  TranscoderIdentifier identifier = transcoder->Start();

  CSingleLock lock(m_transcodersLock);
  m_transcoders.insert(std::make_pair(identifier, transcoder));

  return transcoder;
}

TranscoderPtr CTranscoderManager::Start(const std::string& path, const CTranscodingOptions& options)
{
  if (path.empty())
    return false;

  // TODO: get rid of code duplication
  TranscoderPtr transcoder = std::make_shared<CTranscoder>(path, this);
  transcoder->SetOptions(options);
  TranscoderIdentifier identifier = transcoder->Start();

  CSingleLock lock(m_transcodersLock);
  m_transcoders.insert(std::make_pair(identifier, transcoder));

  return transcoder;
}

TranscoderPtr CTranscoderManager::Get(TranscoderIdentifier identifier) const
{
  CSingleLock lock(m_transcodersLock);
  const auto& transcoder = m_transcoders.find(identifier);
  if (transcoder == m_transcoders.end())
    return TranscoderPtr();

  return transcoder->second;
}

bool CTranscoderManager::GetTranscodedPath(TranscoderIdentifier identifier, std::string& transcodedPath) const
{
  TranscoderPtr transcoder = Get(identifier);
  if (transcoder == nullptr)
    return false;

  transcodedPath = transcoder->GetTranscodedPath();

  return true;
}

void CTranscoderManager::Stop(TranscoderIdentifier identifier, bool wait /* = true */)
{
  TranscoderPtr transcoder = Get(identifier);
  if (transcoder == nullptr)
    return;

  transcoder->Stop(wait);
}

void CTranscoderManager::OnTranscodingError(TranscoderIdentifier identifier)
{
  CLog::Log(LOGERROR, "CTranscoderManager: transcoder %lu failed", identifier);

  CSingleLock lock(m_transcodersLock);
  m_transcoders.erase(identifier);
}

void CTranscoderManager::OnTranscodingStopped(TranscoderIdentifier identifier)
{
  CLog::Log(LOGERROR, "CTranscoderManager: transcoder %lu has been stopped", identifier);

  CSingleLock lock(m_transcodersLock);
  m_transcoders.erase(identifier);
}

void CTranscoderManager::OnTranscodingFinished(TranscoderIdentifier identifier)
{
  CLog::Log(LOGERROR, "CTranscoderManager: transcoder %lu has finished", identifier);

  // TODO: what to do here?
}
