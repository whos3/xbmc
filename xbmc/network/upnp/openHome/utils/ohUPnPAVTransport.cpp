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

#include "ohUPnPAVTransport.h"

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportSeekUnit> COhUPnPAVTransportSeekUnit::OhUPnPAVTransportSeekUnitDefinition = {
  { OhUPnPAVTransportSeekUnit::Unknown, "" },
  { OhUPnPAVTransportSeekUnit::TrackNumber, "TRACK_NR" },
  { OhUPnPAVTransportSeekUnit::AbsoluteTime, "ABS_TIME" },
  { OhUPnPAVTransportSeekUnit::RelativeTime, "REL_TIME" },
  { OhUPnPAVTransportSeekUnit::AbsoluteCount, "ABS_COUNT" },
  { OhUPnPAVTransportSeekUnit::RelativeCount, "REL_COUNT" },
  { OhUPnPAVTransportSeekUnit::RelativeFrame, "REL_FRAME" },
  { OhUPnPAVTransportSeekUnit::Frame, "FRAME" },
  { OhUPnPAVTransportSeekUnit::TapeIndex, "TAPE-INDEX" },
  { OhUPnPAVTransportSeekUnit::RelativeTapeIndex, "REL_TAPE-INDEX" },
};

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportPlayMode> COhUPnPAVTransportPlayMode::OhUPnPAVTransportPlayModeDefinition = {
  { OhUPnPAVTransportPlayMode::Normal, "NORMAL" },
  { OhUPnPAVTransportPlayMode::Shuffle, "SHUFFLE" },
  { OhUPnPAVTransportPlayMode::RepeatOne, "REPEAT_ONE" },
  { OhUPnPAVTransportPlayMode::RepeatAll, "REPEAT_ALL" },
  { OhUPnPAVTransportPlayMode::Random, "RANDOM" },
  { OhUPnPAVTransportPlayMode::Direct1, "DIRECT_1" },
  { OhUPnPAVTransportPlayMode::Intro, "INTRO" },
};

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportRecordQualityMode> COhUPnPAVTransportRecordQualityMode::OhUPnPAVTransportRecordQualityModeDefinition = {
  { OhUPnPAVTransportRecordQualityMode::NotImplemented, "NOT_IMPLEMENTED" },
  { OhUPnPAVTransportRecordQualityMode::EP, "0:EP" },
  { OhUPnPAVTransportRecordQualityMode::LP, "1:LP" },
  { OhUPnPAVTransportRecordQualityMode::SP, "2:SP" },
  { OhUPnPAVTransportRecordQualityMode::Basic, "0:BASIC" },
  { OhUPnPAVTransportRecordQualityMode::Medium, "1:MEDIUM" },
  { OhUPnPAVTransportRecordQualityMode::High, "2:HIGH" },
};

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportTransportAction> COhUPnPAVTransportTransportAction::OhUPnPAVTransportTransportActionDefinition = {
  { OhUPnPAVTransportTransportAction::Unknown, "" },
  { OhUPnPAVTransportTransportAction::Play, "PLAY" },
  { OhUPnPAVTransportTransportAction::Stop, "STOP" },
  { OhUPnPAVTransportTransportAction::Pause, "PAUSE" },
  { OhUPnPAVTransportTransportAction::Seek, "SEEK" },
  { OhUPnPAVTransportTransportAction::Next, "NEXT" },
  { OhUPnPAVTransportTransportAction::Previous, "PREVIOUS" },
  { OhUPnPAVTransportTransportAction::Record, "RECORD" },
};

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportTransportState> COhUPnPAVTransportTransportState::OhUPnPAVTransportTransportStateDefinition = {
  { OhUPnPAVTransportTransportState::Stopped, "STOPPED" },
  { OhUPnPAVTransportTransportState::Playing, "PLAYING" },
  { OhUPnPAVTransportTransportState::Transitioning, "TRANSITIONING" },
  { OhUPnPAVTransportTransportState::PausedPlayback, "PAUSED_PLAYBACK" },
  { OhUPnPAVTransportTransportState::PausedRecording, "PAUSED_RECORDING" },
  { OhUPnPAVTransportTransportState::Recording, "RECORDING" },
  { OhUPnPAVTransportTransportState::NoMediaPresent, "NO_MEDIA_PRESENT" },
};

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportTransportStatus> COhUPnPAVTransportTransportStatus::OhUPnPAVTransportTransportStatusDefinition = {
  { OhUPnPAVTransportTransportStatus::OK, "OK" },
  { OhUPnPAVTransportTransportStatus::ErrorOccured, "ERROR_OCCURRED" },
};

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportStorageMedium> COhUPnPAVTransportStorageMedium::OhUPnPAVTransportStorageMediumDefinition = {
  { OhUPnPAVTransportStorageMedium::Unknown, "UNKNOWN" },
  { OhUPnPAVTransportStorageMedium::DV, "DV" },
  { OhUPnPAVTransportStorageMedium::Mini_DV, "MINI-DV" },
  { OhUPnPAVTransportStorageMedium::VHS, "VHS" },
  { OhUPnPAVTransportStorageMedium::W_VHS, "W-VHS" },
  { OhUPnPAVTransportStorageMedium::S_VHS, "S-VHS" },
  { OhUPnPAVTransportStorageMedium::D_VHS, "D-VHS" },
  { OhUPnPAVTransportStorageMedium::VHSC, "VHSC" },
  { OhUPnPAVTransportStorageMedium::Video8, "VIDEO8" },
  { OhUPnPAVTransportStorageMedium::Hi8, "HI8" },
  { OhUPnPAVTransportStorageMedium::CD_ROM, "CD-ROM" },
  { OhUPnPAVTransportStorageMedium::CD_DA, "CD-DA" },
  { OhUPnPAVTransportStorageMedium::CD_R, "CD-R" },
  { OhUPnPAVTransportStorageMedium::CD_RW, "CD-RW" },
  { OhUPnPAVTransportStorageMedium::Video_CD, "VIDEO-CD" },
  { OhUPnPAVTransportStorageMedium::SACD, "SACD" },
  { OhUPnPAVTransportStorageMedium::MD_Audio, "MD-AUDIO" },
  { OhUPnPAVTransportStorageMedium::MD_Picture, "MD-PICTURE" },
  { OhUPnPAVTransportStorageMedium::DVD_ROM, "DVD-ROM" },
  { OhUPnPAVTransportStorageMedium::DVD_Video, "DVD-VIDEO" },
  { OhUPnPAVTransportStorageMedium::DVD_Plus_R, "DVD+R" },
  { OhUPnPAVTransportStorageMedium::DVD_R, "DVD-R" },
  { OhUPnPAVTransportStorageMedium::DVD_Plus_RW, "DVD+RW" },
  { OhUPnPAVTransportStorageMedium::DVD_RW, "DVD-RW" },
  { OhUPnPAVTransportStorageMedium::DVD_RAM, "DVD-RAM" },
  { OhUPnPAVTransportStorageMedium::DVD_Audio, "DVD-AUDIO" },
  { OhUPnPAVTransportStorageMedium::DAT, "DAT" },
  { OhUPnPAVTransportStorageMedium::LD, "LD" },
  { OhUPnPAVTransportStorageMedium::HDD, "HDD" },
  { OhUPnPAVTransportStorageMedium::Micro_MV, "MICRO-MV" },
  { OhUPnPAVTransportStorageMedium::Network, "NETWORK" },
  { OhUPnPAVTransportStorageMedium::None, "NONE" },
  { OhUPnPAVTransportStorageMedium::NotImplemented, "NOT_IMPLEMENTED" },
  { OhUPnPAVTransportStorageMedium::SD, "SD" },
  { OhUPnPAVTransportStorageMedium::PC_Card, "PC-CARD" },
  { OhUPnPAVTransportStorageMedium::MMC, "MMC" },
  { OhUPnPAVTransportStorageMedium::CF, "CF" },
  { OhUPnPAVTransportStorageMedium::BD, "BD" },
  { OhUPnPAVTransportStorageMedium::MS, "MS" },
  { OhUPnPAVTransportStorageMedium::HD_DVD, "HD_DVD" },
};

const OhUPnPEnumValueDefinitions<OhUPnPAVTransportWriteStatus> COhUPnPAVTransportWriteStatus::OhUPnPAVTransportWriteStatusDefinition = {
  { OhUPnPAVTransportWriteStatus::Unknown, "UNKNOWN" },
  { OhUPnPAVTransportWriteStatus::NotImplemented, "NOT_IMPLEMENTED" },
  { OhUPnPAVTransportWriteStatus::Writable, "WRITABLE" },
  { OhUPnPAVTransportWriteStatus::Protected, "PROTECTED" },
  { OhUPnPAVTransportWriteStatus::NotWritable, "NOT_WRITABLE" },
};
