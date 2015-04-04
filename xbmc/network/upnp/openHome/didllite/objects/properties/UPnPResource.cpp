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

#include "UPnPResource.h"
#include "network/upnp/openHome/ohUPnPDefinitions.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"

static const std::string ProtocolInfoSeparator = ":";
static const std::string ProtocolInfoDlnaOrgPrefix = "DLNA.ORG_";
static const std::string ProtocolInfoExtrasAssigner = "=";
static const std::string ProtocolInfoExtrasSeparator = ";";

const std::string CUPnPResource::CProtocolInfo::DLNA::ProfileKey = ProtocolInfoDlnaOrgPrefix + "PN";
const std::string CUPnPResource::CProtocolInfo::DLNA::OperationKey = ProtocolInfoDlnaOrgPrefix + "OP";
const std::string CUPnPResource::CProtocolInfo::DLNA::PlaySpeedKey = ProtocolInfoDlnaOrgPrefix + "PS";
const std::string CUPnPResource::CProtocolInfo::DLNA::ConversionIndicatorKey = ProtocolInfoDlnaOrgPrefix + "CI";
const std::string CUPnPResource::CProtocolInfo::DLNA::FlagsKey = ProtocolInfoDlnaOrgPrefix + "FLAGS";

CUPnPResource::CProtocolInfo::DLNA::DLNA()
  : DLNA("", Operation::None)
{ }

CUPnPResource::CProtocolInfo::DLNA::DLNA(const std::string& protocolInfoExtras)
  : DLNA("", Operation::None)
{
  Set(protocolInfoExtras);
}

CUPnPResource::CProtocolInfo::DLNA::DLNA(const std::string& profile, Operation operation, Flags flags /* = Flags::None */,
  PlaySpeed playSpeed /* = PlaySpeed::Invalid */, ConversionIndicator conversionIndicator /* = ConversionIndicator::None */)
  : m_profile(profile),
    m_operation(operation),
    m_flags(flags),
    m_playSpeed(playSpeed),
    m_conversionIndicator(conversionIndicator)
{ }

bool CUPnPResource::CProtocolInfo::DLNA::IsValid() const
{
  return !m_profile.empty() || m_operation != Operation::None || m_playSpeed != PlaySpeed::Invalid ||
    m_conversionIndicator != ConversionIndicator::None || m_flags != Flags::None;
}

std::string CUPnPResource::CProtocolInfo::DLNA::Get() const
{
  if (!IsValid())
    return "*";

  std::string extras;
  if (!m_profile.empty())
    extras += ProfileKey + ProtocolInfoExtrasAssigner + m_profile;
  
  if (!extras.empty())
    extras += ProtocolInfoExtrasSeparator;
  extras += OperationKey + ProtocolInfoExtrasAssigner + StringUtils::Format("%02X", static_cast<unsigned int>(m_operation));

  if (m_playSpeed != PlaySpeed::Invalid)
    extras += ProtocolInfoExtrasSeparator + PlaySpeedKey + ProtocolInfoExtrasAssigner + StringUtils::Format("%02X", static_cast<unsigned int>(m_playSpeed));

  extras += ProtocolInfoExtrasSeparator + ConversionIndicatorKey + ProtocolInfoExtrasAssigner + StringUtils::Format("%02X", static_cast<unsigned int>(m_conversionIndicator));

  if (m_flags != Flags::None)
    extras += ProtocolInfoExtrasSeparator + FlagsKey + ProtocolInfoExtrasAssigner + StringUtils::Format("%032X", static_cast<unsigned int>(m_flags));

  return extras;
}

bool CUPnPResource::CProtocolInfo::DLNA::Set(const std::string& protocolInfoExtras)
{
  if (protocolInfoExtras == "*")
  {
    m_profile.clear();
    m_operation = Operation::None;
    m_playSpeed = PlaySpeed::Invalid;
    m_conversionIndicator = ConversionIndicator::None;
    m_flags = Flags::None;
    return true;
  }

  std::vector<std::string> parts = StringUtils::Split(protocolInfoExtras, ProtocolInfoExtrasSeparator);
  if (parts.empty())
    return false;

  for (const auto& part : parts)
  {
    std::vector<std::string> keyValue = StringUtils::Split(part, ProtocolInfoExtrasAssigner);
    if (keyValue.size() != 2)
      return false;

    std::string key = keyValue.at(0);
    std::string strValue = keyValue.at(1);

    if (key == ProfileKey)
      m_profile = strValue;
    else
    {
      std::string format = "%02X";
      if (key == FlagsKey)
        format = "%032X";

      unsigned int value = 0;
      if (sscanf(strValue.c_str(), format.c_str(), &value) != 1)
        return false;

      if (key == OperationKey)
        m_operation = static_cast<Operation>(value);
      else if (key == PlaySpeedKey)
        m_playSpeed = static_cast<PlaySpeed>(value);
      else if (key == ConversionIndicatorKey)
        m_conversionIndicator = static_cast<ConversionIndicator>(value);
      else if (key == FlagsKey)
        m_flags = static_cast<Flags>(value);
      else
        return false;
    }
  }

  return true;
}

CUPnPResource::CProtocolInfo::CProtocolInfo(const std::string& protocolInfo)
{
  Set(protocolInfo);
}

CUPnPResource::CProtocolInfo::CProtocolInfo(const std::string& protocol, const std::string& mask, const std::string& contentType)
  : CProtocolInfo(protocol, mask, contentType, "")
{ }

CUPnPResource::CProtocolInfo::CProtocolInfo(const std::string& protocol, const std::string& mask, const std::string& contentType, const std::string& extras)
  : m_protocol(protocol),
    m_mask(mask),
    m_contentType(contentType)
{
  ParseExtras(extras);
}

CUPnPResource::CProtocolInfo::CProtocolInfo(const std::string& protocol, const std::string& mask, const std::string& contentType, const DLNA& extras)
  : m_protocol(protocol),
    m_mask(mask),
    m_contentType(contentType),
    m_extras(extras)
{ }

std::string CUPnPResource::CProtocolInfo::Get() const
{
  std::string protocolInfo = m_protocol + ProtocolInfoSeparator;
  if (!m_mask.empty())
    protocolInfo += m_mask;
  else
    protocolInfo += "*";
  protocolInfo += ProtocolInfoSeparator;
  if (!m_contentType.empty())
    protocolInfo += m_contentType;
  else
    protocolInfo += "*";
  protocolInfo += ProtocolInfoSeparator + m_extras.Get();

  return protocolInfo;
}

bool CUPnPResource::CProtocolInfo::Set(const std::string& protocolInfo)
{
  std::vector<std::string> parts = StringUtils::Split(protocolInfo, ProtocolInfoSeparator);
  if (parts.size() != 4)
    return false;

  m_protocol = parts.at(0);
  m_mask = parts.at(1);
  m_contentType = parts.at(2);
  return ParseExtras(parts.at(3));
}

bool CUPnPResource::CProtocolInfo::ParseExtras(const std::string& extras)
{
  return m_extras.Set(extras);
}

CUPnPResource::CResolution::CResolution()
  : CResolution(0, 0)
{ }

CUPnPResource::CResolution::CResolution(const std::string& resolution)
  : CResolution(0, 0)
{
  Set(resolution);
}

CUPnPResource::CResolution::CResolution(uint32_t width, uint32_t height)
  : m_width(width),
    m_height(height)
{ }

std::string CUPnPResource::CResolution::Get() const
{
  return StringUtils::Format("%ux%u", m_width, m_height);
}

bool CUPnPResource::CResolution::Set(const std::string& resolution)
{
  std::vector<std::string> parts = StringUtils::Split(resolution, "x");
  if (parts.size() != 2)
    return false;

  uint32_t width = static_cast<uint32_t>(strtoul(parts.at(0).c_str(), nullptr, 10));
  uint32_t height = static_cast<uint32_t>(strtoul(parts.at(1).c_str(), nullptr, 10));

  if (width == 0 || height == 0)
    return false;

  m_width = width;
  m_height = height;
  return true;
}

CUPnPResource::CUPnPResource()
  : CUPnPResource("")
{ }

CUPnPResource::CUPnPResource(const std::string& resourceUri)
  : IDidlLiteElement("res"),
    m_uri(resourceUri),
    m_size(0),
    m_bitrate(0),
    m_bitsPerSample(0),
    m_sampleFrequency(0),
    m_nrAudioChannels(0),
    m_colorDepth(0)
{
  initializeProperties();

  setPropertyValid();
}

CUPnPResource::CUPnPResource(const CUPnPResource& upnpResource)
  : IDidlLiteElement(upnpResource),
    m_id(upnpResource.m_id),
    m_uri(upnpResource.m_uri),
    m_protocolInfo(upnpResource.m_protocolInfo),
    m_protocolInfoDetails(upnpResource.m_protocolInfoDetails),
    m_importUri(upnpResource.m_importUri),
    m_size(upnpResource.m_size),
    m_duration(upnpResource.m_duration),
    m_protection(upnpResource.m_protection),
    m_bitrate(upnpResource.m_bitrate),
    m_bitsPerSample(upnpResource.m_bitsPerSample),
    m_sampleFrequency(upnpResource.m_sampleFrequency),
    m_nrAudioChannels(upnpResource.m_nrAudioChannels),
    m_resolution(upnpResource.m_resolution),
    m_colorDepth(upnpResource.m_colorDepth),
    m_tspec(upnpResource.m_tspec),
    m_allowedUse(upnpResource.m_allowedUse),
    m_validityStart(upnpResource.m_validityStart),
    m_validityEnd(upnpResource.m_validityEnd),
    m_remainingTime(upnpResource.m_remainingTime),
    m_usageInfo(upnpResource.m_usageInfo),
    m_rightsInfoURI(upnpResource.m_rightsInfoURI),
    m_contentInfoURI(upnpResource.m_contentInfoURI),
    m_recordQuality(upnpResource.m_recordQuality),
    m_daylightSaving(upnpResource.m_daylightSaving),
    m_framerate(upnpResource.m_framerate)
{
  initializeProperties();
  copyPropertyValidity(&upnpResource);
}

CUPnPResource::~CUPnPResource()
{ }

void CUPnPResource::SetId(const std::string& id)
{
  m_id = id;
  setPropertyValidity("@id", !m_id.empty());
}

void CUPnPResource::SetProtocolInfo(const CProtocolInfo& protocolInfo)
{
  m_protocolInfoDetails = protocolInfo;
  m_protocolInfo = protocolInfo.Get();
  setPropertyValidity("@protocolInfo", !m_protocolInfo.empty());
}

void CUPnPResource::SetImportUri(const std::string& importUri)
{
  m_importUri = importUri;
  setPropertyValidity("@importUri", !m_importUri.empty());
}

void CUPnPResource::SetSize(uint64_t size)
{
  m_size = size;
  setPropertyValidity("@size", m_size > 0);
}

void CUPnPResource::SetDuration(double duration)
{
  m_duration = GetDurationFromSeconds(duration);
  setPropertyValidity("@duration", !m_duration.empty());
}

void CUPnPResource::SetProtection(const std::string& protection)
{
  m_protection = protection;
  setPropertyValidity("@protection", !m_protection.empty());
}

void CUPnPResource::SetBitrate(uint32_t bitrate)
{
  m_bitrate = bitrate;
  setPropertyValidity("@bitrate", m_bitrate > 0);
}

void CUPnPResource::SetBitsPerSample(uint32_t bitsPerSample)
{
  m_bitsPerSample = bitsPerSample;
  setPropertyValidity("@bitsPerSample", m_bitsPerSample > 0);
}

void CUPnPResource::SetSampleFrequency(uint32_t sampleFrequency)
{
  m_sampleFrequency = sampleFrequency;
  setPropertyValidity("@sampleFrequency", m_sampleFrequency > 0);
}

void CUPnPResource::SetNumberOfAudioChannels(uint32_t nrAudioChannels)
{
  m_nrAudioChannels = nrAudioChannels;
  setPropertyValidity("@nrAudioChannels", m_nrAudioChannels > 0);
}

void CUPnPResource::SetResolution(const CResolution& resolution)
{
  m_resolutionDetails = resolution;
  m_resolution = resolution.Get();
  setPropertyValidity("@resolution", !m_resolution.empty());
}

bool CUPnPResource::deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context)
{
  setPropertyValid();

  if (!m_protocolInfo.empty() && !m_protocolInfoDetails.Set(m_protocolInfo))
    return false;

  if (!m_resolution.empty() && !m_resolutionDetails.Set(m_resolution))
    return false;

  return true;
}

void CUPnPResource::initializeProperties()
{
  addStringProperty(&m_uri).SetRequired();
  addStringProperty("@id", &m_id).AsAttribute().SetOptional().SetMinimumVersion(4);
  addStringProperty("@protocolInfo", &m_protocolInfo).AsAttribute().SetRequired();
  addStringProperty("@importUri", &m_importUri).AsAttribute().SetOptional();
  addUnsignedLongProperty("@size", &m_size).AsAttribute().SetOptional();
  addStringProperty("@duration", &m_duration).AsAttribute().SetOptional();
  addStringProperty("@protection", &m_protection).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@bitrate", &m_bitrate).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@bitsPerSample", &m_bitsPerSample).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@sampleFrequency", &m_sampleFrequency).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@nrAudioChannels", &m_nrAudioChannels).AsAttribute().SetOptional();
  addStringProperty("@resolution", &m_resolution).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@colorDepth", &m_colorDepth).AsAttribute().SetOptional();
  addStringProperty("@tspec", &m_tspec).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@allowedUse", &m_allowedUse).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@validityStart", &m_validityStart).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@validityEnd", &m_validityEnd).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@remainingTime", &m_remainingTime).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@usageInfo", &m_usageInfo).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@rightsInfoURI", &m_rightsInfoURI).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@contentInfoURI", &m_contentInfoURI).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@recordQuality", &m_recordQuality).AsAttribute().SetOptional().SetMinimumVersion(2);
  addStringProperty("@daylightSaving", &m_daylightSaving).AsAttribute().SetOptional().SetMinimumVersion(3);
  addStringProperty("@framerate", &m_framerate).AsAttribute().SetOptional().SetMinimumVersion(4);
  // TODO: @updateCount (min: 3)
}

double CUPnPResource::GetDurationInSeconds(std::string duration)
{
  StringUtils::Trim(duration);
  if (duration.empty())
    return 0.0;

  bool positive = true;

  // handle leading +/- sign
  if (StringUtils::StartsWith(duration, "+"))
    duration = duration.substr(1);
  else if (StringUtils::StartsWith(duration, "-"))
  {
    positive = false;
    duration = duration.substr(1);
  }

  //
  std::vector<std::string> parts = StringUtils::Split(duration, ":");
  if (parts.size() < 3)
    return 0.0;

  std::string strHours = parts.at(0);
  std::string strMinutes = parts.at(1);
  std::string strSeconds = parts.at(2);
  std::string strFractions;

  // check if the seconds part contains a fraction
  if (strSeconds.find('.') != std::string::npos)
  {
    std::vector<std::string> secondsParts = StringUtils::Split(strSeconds, ".");
    if (secondsParts.size() > 2)
      return 0.0;

    strSeconds = secondsParts.at(0);
    if (secondsParts.size() > 1)
      strFractions = secondsParts.at(1);
  }

  // make sure we only have numbers and that they have the right length
  if (!StringUtils::IsNaturalNumber(strHours) || !StringUtils::IsNaturalNumber(strMinutes) ||
      !StringUtils::IsNaturalNumber(strSeconds) || strMinutes.size() != 2 || strSeconds.size() != 2)
    return 0.0;

  // parse hours, minutes and seconds
  uint64_t hours = strtoull(strHours.c_str(), nullptr, 10);
  uint8_t minutes = static_cast<uint8_t>(strtoul(strMinutes.c_str(), nullptr, 10));
  uint8_t seconds = static_cast<uint8_t>(strtoul(strSeconds.c_str(), nullptr, 10));

  // handle fractions in second
  double fractions = 0.0;
  if (!strFractions.empty())
  {
    // check if the fractions in second is expressed as a fraction
    if (strFractions.find("/") != std::string::npos)
    {
      std::vector<std::string> fractionParts = StringUtils::Split(strFractions, "/");
      if (fractionParts.size() != 2 || !StringUtils::IsNaturalNumber(fractionParts.at(0)) ||
          !StringUtils::IsNaturalNumber(fractionParts.at(1)))
        return 0.0;

      // parse nominator and denominator of the fraction
      uint64_t fractionNominator = strtoull(fractionParts.at(0).c_str(), nullptr, 10);
      uint64_t fractionDenominator = strtoull(fractionParts.at(1).c_str(), nullptr, 10);

      // the denominator must be bigger than the nominator
      if (fractionNominator >= fractionDenominator)
        return 0.0;

      fractions = static_cast<double>(fractionNominator) / static_cast<double>(fractionDenominator);
    }
    else if (StringUtils::IsNaturalNumber(strFractions))
    {
      strFractions = "0." + strFractions;
      fractions = strtod(strFractions.c_str(), nullptr);
    }
    else
      return 0.0;
  }

  double durationInSeconds = hours * 60 * 60 + minutes * 60 + seconds + fractions;
  if (!positive)
    durationInSeconds = -durationInSeconds;

  return durationInSeconds;
}

std::string CUPnPResource::GetDurationFromSeconds(double duration)
{
  int minutes = static_cast<int>(duration / 60);
  double seconds = duration - 60 * minutes;
  int hours = minutes / 60;
  minutes %= 60;

  std::string strDuration;
  if (hours > 0)
    strDuration = StringUtils::Format("%d:", hours);
  strDuration += StringUtils::Format("%02d:%02.03f", minutes, seconds);

  return strDuration;
}

CUPnPResourceFinder::CUPnPResourceFinder(const std::string& protocol, const std::string& contentType)
  : m_protocol(protocol),
    m_contentType(contentType)
{ }

CUPnPResourceFinder CUPnPResourceFinder::ByProtocol(const std::string& protocol)
{
  return CUPnPResourceFinder(protocol, "");
}

CUPnPResourceFinder CUPnPResourceFinder::ByContentType(const std::string& contentType)
{
  return CUPnPResourceFinder("", contentType);
}

CUPnPResourceFinder CUPnPResourceFinder::ByProtocolAndContentType(const std::string& protocol, const std::string& contentType)
{
  return CUPnPResourceFinder(protocol, contentType);
}

bool CUPnPResourceFinder::operator()(const CUPnPResource* resource)
{
  if (resource == nullptr)
    return false;

  const CUPnPResource::CProtocolInfo& protocolInfo = resource->GetProtocolInfo();
  if (!m_protocol.empty() && !StringUtils::EqualsNoCase(protocolInfo.GetProtocol(), m_protocol))
    return false;
  if (!m_contentType.empty() && !StringUtils::StartsWithNoCase(protocolInfo.GetContentType(), m_contentType))
    return false;

  return true;
}
