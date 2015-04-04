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

#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

class CUPnPResource : public IDidlLiteElement
{
public:
  class CProtocolInfo
  {
  public:
    class DLNA
    {
    public:
      static const std::string ProfileKey;

      enum class Operation
      {
        None = 0x00,
        Range = 0x01,
        TimeSeek = 0x10
      };
      static const std::string OperationKey;

      enum class PlaySpeed
      {
        Invalid = 0,
        Normal = 1
      };
      static const std::string PlaySpeedKey;

      enum class ConversionIndicator
      {
        None = 0,
        Transcoded = 1
      };
      static const std::string ConversionIndicatorKey;

      enum class Flags
      {
        None = 0,
        SenderPaces = (1 << 31),
        TimeBasedSeek = (1 << 30),
        ByteBasedSeek = (1 << 29),
        PlayContainer = (1 << 28),
        S0Increase = (1 << 27),
        SNIncrease = (1 << 26),
        RtspPause = (1 << 25),
        StreamingTransferMode = (1 << 24),
        InteractiveTransferMode = (1 << 23),
        BackgroundTransferMode = (1 << 22),
        ConnectionStall = (1 << 21),
        DlnaVersion15 = (1 << 20)
      };
      static const std::string FlagsKey;

      DLNA();
      explicit DLNA(const std::string& protocolInfoExtras);
      DLNA(const std::string& profile, Operation operation, Flags flags = Flags::None,
        PlaySpeed playSpeed = PlaySpeed::Invalid, ConversionIndicator conversionIndicator = ConversionIndicator::None);

      bool IsValid() const;

      std::string Get() const;
      bool Set(const std::string& protocolInfoExtras);
      const std::string& GetProfile() const { return m_profile; }
      void SetProfile(const std::string& profile) { m_profile = profile; }
      Operation GetOperation() const { return m_operation; }
      void SetOperation(Operation operation) { m_operation = operation; }
      Flags GetFlags() const { return m_flags; }
      void SetFlags(Flags flags) { m_flags = flags; }
      PlaySpeed GetPlaySpeed() const { return m_playSpeed; }
      void SetPlaySpeed(PlaySpeed playSpeed) { m_playSpeed = playSpeed; }
      ConversionIndicator GetConversionIndicator() const { return m_conversionIndicator; }
      void SetConversionIndicator(ConversionIndicator conversionIndicator) { m_conversionIndicator = conversionIndicator; }

    private:
      std::string m_profile;
      Operation m_operation;
      Flags m_flags;
      PlaySpeed m_playSpeed;
      ConversionIndicator m_conversionIndicator;
    };

  public:
    CProtocolInfo() { }
    explicit CProtocolInfo(const std::string& protocolInfo);
    CProtocolInfo(const std::string& protocol, const std::string& mask, const std::string& contentType);
    CProtocolInfo(const std::string& protocol, const std::string& mask, const std::string& contentType, const std::string& extras);
    CProtocolInfo(const std::string& protocol, const std::string& mask, const std::string& contentType, const DLNA& extras);

    std::string Get() const;
    const std::string& GetProtocol() const { return m_protocol; }
    const std::string& GetMask() const { return m_mask; }
    const std::string& GetContentType() const { return m_contentType; }
    const DLNA& GetExtras() const { return m_extras; }
    DLNA& GetExtras() { return m_extras; }

    bool Set(const std::string& protocolInfo);
    void SetProtocol(const std::string& protocol) { m_protocol = protocol; }
    void SetMask(const std::string& mask) { m_mask = mask; }
    void SetContentType(const std::string& contentType) { m_contentType = contentType; }
    void SetExtras(const std::string& extras) { ParseExtras(extras); }
    void SetExtras(const DLNA& extras) { m_extras = extras; }

  private:
    bool ParseExtras(const std::string& extras);

    std::string m_protocol;
    std::string m_mask;
    std::string m_contentType;
    DLNA m_extras;
  };

  class CResolution
  {
  public:
    CResolution();
    explicit CResolution(const std::string& resolution);
    CResolution(uint32_t width, uint32_t height);

    bool IsValid() const { return m_width > 0 && m_height > 0; }

    std::string Get() const;
    bool Set(const std::string& resolution);
    const uint32_t GetWidth() const { return m_width; }
    void SetWidth(uint32_t width) { m_width = width; }
    const uint32_t GetHeight() const { return m_height; }
    void SetHeight(uint32_t height) { m_height = height; }

  private:
    uint32_t m_width;
    uint32_t m_height;
  };

public:
  CUPnPResource();
  explicit CUPnPResource(const std::string &resourceUri);
  CUPnPResource(const CUPnPResource& upnpResource);
  virtual ~CUPnPResource();

  // implementations of IDidlLiteElement
  virtual CUPnPResource* Create() const override { return new CUPnPResource(); }
  virtual CUPnPResource* Clone() const override { return new CUPnPResource(*this); }
  virtual std::string GetIdentifier() const { return "Resource"; }
  virtual std::set<std::string> Extends() const { return { }; }

  const std::string& GetId() const { return m_id; }
  void SetId(const std::string& id);
  const std::string& GetUri() const { return m_uri; }
  const CProtocolInfo& GetProtocolInfo() const { return m_protocolInfoDetails; }
  void SetProtocolInfo(const CProtocolInfo& protocolInfo);
  const std::string& GetImportUri() const { return m_importUri; }
  void SetImportUri(const std::string& importUri);
  uint64_t GetSize() const { return m_size; }
  void SetSize(uint64_t size);
  double GetDuration() const { return GetDurationInSeconds(m_duration); }
  void SetDuration(double duration);
  const std::string& GetProtection() const { return m_protection; }
  void SetProtection(const std::string& protection);
  uint32_t GetBitrate() const { return m_bitrate; }
  void SetBitrate(uint32_t bitrate);
  uint32_t GetBitsPerSample() const { return m_bitsPerSample; }
  void SetBitsPerSample(uint32_t bitsPerSample);
  uint32_t GetSampleFrequency() const { return m_sampleFrequency; }
  void SetSampleFrequency(uint32_t sampleFrequency);
  uint32_t GetNumberOfAudioChannels() const { return m_nrAudioChannels; }
  void SetNumberOfAudioChannels(uint32_t nrAudioChannels);
  const CResolution& GetResolution() const { return m_resolutionDetails; }
  void SetResolution(const CResolution& resolution);

protected:
  // implementations of IDidlLiteElement
  virtual bool deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context) override;

private:
  void initializeProperties();

  /*!
   * \brief Parse the resource duration format.
   *
   * The duration format is specified in the UPnP ContentDirectory specification
   * as either H+:MM:SS[.F+] or H+:MM:SS[.F0/F1] where H+ is any number of hours,
   * MM the number of minutes, SS the number of seconds and the optional F+ and
   * F0/F1 (with F0 < F1) the number of fractions in the second.
   */
  static double GetDurationInSeconds(std::string duration);
  static std::string GetDurationFromSeconds(double duration);

  std::string m_id;
  std::string m_uri;
  std::string m_protocolInfo;
  CProtocolInfo m_protocolInfoDetails;
  std::string m_importUri;
  uint64_t m_size;
  std::string m_duration;
  std::string m_protection;
  uint32_t m_bitrate;
  uint32_t m_bitsPerSample;
  uint32_t m_sampleFrequency;
  uint32_t m_nrAudioChannels;
  std::string m_resolution;
  CResolution m_resolutionDetails;
  uint32_t m_colorDepth;
  std::string m_tspec;
  std::string m_allowedUse;
  std::string m_validityStart;
  std::string m_validityEnd;
  std::string m_remainingTime;
  std::string m_usageInfo;
  std::string m_rightsInfoURI;
  std::string m_contentInfoURI;
  std::string m_recordQuality;
  std::string m_daylightSaving;
  std::string m_framerate;
};

class CUPnPResourceFinder
{
public:
  static CUPnPResourceFinder ByProtocol(const std::string& protocol);
  static CUPnPResourceFinder ByContentType(const std::string& contentType);
  static CUPnPResourceFinder ByProtocolAndContentType(const std::string& protocol, const std::string& contentType);

  bool operator()(const CUPnPResource* resource);

private:
  CUPnPResourceFinder(const std::string& protocol, const std::string& contentType);

  std::string m_protocol;
  std::string m_contentType;
};
