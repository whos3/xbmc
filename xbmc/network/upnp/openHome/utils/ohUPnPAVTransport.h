#pragma once
/*
 *      Copyright (C) 2016 Team XBMC
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

#include "network/upnp/openHome/utils/ohUPnPEnum.h"

enum class OhUPnPAVTransportSeekUnit
{
  Unknown,
  TrackNumber,
  AbsoluteTime,
  RelativeTime,
  AbsoluteCount,
  RelativeCount,
  RelativeFrame,
  Frame,
  TapeIndex,
  RelativeTapeIndex
};

class COhUPnPAVTransportSeekUnit : public COhUPnPEnum<OhUPnPAVTransportSeekUnit, OhUPnPAVTransportSeekUnit::Unknown>
{
public:
  COhUPnPAVTransportSeekUnit()
    : COhUPnPEnum(OhUPnPAVTransportSeekUnitDefinition)
  { }

  COhUPnPAVTransportSeekUnit(OhUPnPAVTransportSeekUnit value)
    : COhUPnPEnum(OhUPnPAVTransportSeekUnitDefinition, value)
  { }

  explicit COhUPnPAVTransportSeekUnit(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportSeekUnitDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportSeekUnit> OhUPnPAVTransportSeekUnitDefinition;
};

enum class OhUPnPAVTransportPlayMode
{
  Normal,
  Shuffle,
  RepeatOne,
  RepeatAll,
  Random,
  Direct1,
  Intro
};

class COhUPnPAVTransportPlayMode : public COhUPnPEnum<OhUPnPAVTransportPlayMode, OhUPnPAVTransportPlayMode::Normal>
{
public:
  COhUPnPAVTransportPlayMode()
    : COhUPnPEnum(OhUPnPAVTransportPlayModeDefinition)
  { }

  COhUPnPAVTransportPlayMode(OhUPnPAVTransportPlayMode value)
    : COhUPnPEnum(OhUPnPAVTransportPlayModeDefinition, value)
  { }

  explicit COhUPnPAVTransportPlayMode(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportPlayModeDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportPlayMode> OhUPnPAVTransportPlayModeDefinition;
};

enum class OhUPnPAVTransportRecordQualityMode
{
  NotImplemented,
  EP,
  LP,
  SP,
  Basic,
  Medium,
  High
};

class COhUPnPAVTransportRecordQualityMode : public COhUPnPEnum<OhUPnPAVTransportRecordQualityMode, OhUPnPAVTransportRecordQualityMode::NotImplemented>
{
public:
  COhUPnPAVTransportRecordQualityMode()
    : COhUPnPEnum(OhUPnPAVTransportRecordQualityModeDefinition)
  { }

  COhUPnPAVTransportRecordQualityMode(OhUPnPAVTransportRecordQualityMode value)
    : COhUPnPEnum(OhUPnPAVTransportRecordQualityModeDefinition, value)
  { }

  explicit COhUPnPAVTransportRecordQualityMode(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportRecordQualityModeDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportRecordQualityMode> OhUPnPAVTransportRecordQualityModeDefinition;
};

enum class OhUPnPAVTransportTransportAction
{
  Unknown,
  Play,
  Stop,
  Pause,
  Seek,
  Next,
  Previous,
  Record
};

class COhUPnPAVTransportTransportAction : public COhUPnPEnum<OhUPnPAVTransportTransportAction, OhUPnPAVTransportTransportAction::Unknown>
{
public:
  COhUPnPAVTransportTransportAction()
    : COhUPnPEnum(OhUPnPAVTransportTransportActionDefinition)
  { }

  COhUPnPAVTransportTransportAction(OhUPnPAVTransportTransportAction value)
    : COhUPnPEnum(OhUPnPAVTransportTransportActionDefinition, value)
  { }

  explicit COhUPnPAVTransportTransportAction(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportTransportActionDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportTransportAction> OhUPnPAVTransportTransportActionDefinition;
};

enum class OhUPnPAVTransportTransportState
{
  Stopped,
  Playing,
  Transitioning,
  PausedPlayback,
  PausedRecording,
  Recording,
  NoMediaPresent
};

class COhUPnPAVTransportTransportState : public COhUPnPEnum<OhUPnPAVTransportTransportState, OhUPnPAVTransportTransportState::NoMediaPresent>
{
public:
  COhUPnPAVTransportTransportState()
    : COhUPnPEnum(OhUPnPAVTransportTransportStateDefinition)
  { }

  COhUPnPAVTransportTransportState(OhUPnPAVTransportTransportState value)
    : COhUPnPEnum(OhUPnPAVTransportTransportStateDefinition, value)
  { }

  explicit COhUPnPAVTransportTransportState(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportTransportStateDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportTransportState> OhUPnPAVTransportTransportStateDefinition;
};

enum class OhUPnPAVTransportTransportStatus
{
  OK,
  ErrorOccured
};

class COhUPnPAVTransportTransportStatus : public COhUPnPEnum<OhUPnPAVTransportTransportStatus, OhUPnPAVTransportTransportStatus::OK>
{
public:
  COhUPnPAVTransportTransportStatus()
    : COhUPnPEnum(OhUPnPAVTransportTransportStatusDefinition)
  { }

  COhUPnPAVTransportTransportStatus(OhUPnPAVTransportTransportStatus value)
    : COhUPnPEnum(OhUPnPAVTransportTransportStatusDefinition, value)
  { }

  explicit COhUPnPAVTransportTransportStatus(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportTransportStatusDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportTransportStatus> OhUPnPAVTransportTransportStatusDefinition;
};

enum class OhUPnPAVTransportStorageMedium
{
  Unknown,
  DV,
  Mini_DV,
  VHS,
  W_VHS,
  S_VHS,
  D_VHS,
  VHSC,
  Video8,
  Hi8,
  CD_ROM,
  CD_DA,
  CD_R,
  CD_RW,
  Video_CD,
  SACD,
  MD_Audio,
  MD_Picture,
  DVD_ROM,
  DVD_Video,
  DVD_Plus_R,
  DVD_R,
  DVD_Plus_RW,
  DVD_RW,
  DVD_RAM,
  DVD_Audio,
  DAT,
  LD,
  HDD,
  Micro_MV,
  Network,
  None,
  NotImplemented,
  SD,
  PC_Card,
  MMC,
  CF,
  BD,
  MS,
  HD_DVD
};

class COhUPnPAVTransportStorageMedium : public COhUPnPEnum<OhUPnPAVTransportStorageMedium, OhUPnPAVTransportStorageMedium::NotImplemented>
{
public:
  COhUPnPAVTransportStorageMedium()
    : COhUPnPEnum(OhUPnPAVTransportStorageMediumDefinition)
  { }

  COhUPnPAVTransportStorageMedium(OhUPnPAVTransportStorageMedium value)
    : COhUPnPEnum(OhUPnPAVTransportStorageMediumDefinition, value)
  { }

  explicit COhUPnPAVTransportStorageMedium(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportStorageMediumDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportStorageMedium> OhUPnPAVTransportStorageMediumDefinition;
};

enum class OhUPnPAVTransportWriteStatus
{
  Unknown,
  NotImplemented,
  Writable,
  Protected,
  NotWritable
};

class COhUPnPAVTransportWriteStatus : public COhUPnPEnum<OhUPnPAVTransportWriteStatus, OhUPnPAVTransportWriteStatus::NotImplemented>
{
public:
  COhUPnPAVTransportWriteStatus()
    : COhUPnPEnum(OhUPnPAVTransportWriteStatusDefinition)
  { }

  COhUPnPAVTransportWriteStatus(OhUPnPAVTransportWriteStatus value)
    : COhUPnPEnum(OhUPnPAVTransportWriteStatusDefinition, value)
  { }

  explicit COhUPnPAVTransportWriteStatus(const std::string& value)
    : COhUPnPEnum(OhUPnPAVTransportWriteStatusDefinition, value)
  { }

private:
  static const OhUPnPEnumValueDefinitions<OhUPnPAVTransportWriteStatus> OhUPnPAVTransportWriteStatusDefinition;
};
