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
#include <map>
#include <string>
#include <vector>

#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/didllite/objects/classmappers/UPnPClassMapping.h"
#include "utils/IXmlDeserializable.h"

class TiXmlElement;

class COhUPnPDeviceProfile
{
public:
  COhUPnPDeviceProfile();

  bool Load(const std::string& profileLocation);

  bool IsLoaded() const { return m_loaded; }
  bool Matches(const COhUPnPDevice& device) const;

  const std::string& GetName() const { return m_name; }

  const CUPnPClassMapping& GetClassMapping() const { return m_classMapping; }

  bool GetMappedPath(const std::string& objectId, std::string& path) const;

  uint32_t GetContentDirectoryVersion() const { return m_contentDirectoryVersion; }

  bool ExpectsSimpleVideoItems() const { return m_simpleVideoItems; }
  bool ExpectsSimpleStorageFolders() const { return m_simpleStorageFolders; }

  const std::string& GetProtocolInfo() const { return m_protocolInfo; }

  // TODO

  std::string GetMimeType(std::string container) const;

  // TODO

private:
  bool LoadDeviceIdentification(const TiXmlElement* root);
  bool LoadGeneral(const TiXmlElement* root);
  bool LoadMediaServer(const TiXmlElement* root);
  bool LoadTypeMapping(const TiXmlElement* root);
  bool LoadPathMapping(const TiXmlElement* root);
  bool LoadMediaRenderer(const TiXmlElement* root);
  bool LoadContentDirectory(const TiXmlElement* root);
  bool LoadMediaProfiles(const TiXmlElement* root);

  bool m_loaded;
  std::string m_name;

  class DeviceIdentificationRule : public IXmlDeserializable
  {
  public:
    DeviceIdentificationRule();

    // implementation of IXmlDeserializable
    bool Deserialize(const TiXmlNode* node) override;

    bool Matches(const COhUPnPDevice& device) const;

  private:
    using PropertyGetter = const std::string&(COhUPnPDevice::*)() const;
    enum class MatchType
    {
      Exact,
      Substring,
      Regex
    };

    MatchType m_type;
    std::string m_value;
    PropertyGetter m_property;
  };
  std::vector<DeviceIdentificationRule> m_identificationRules;

  CUPnPClassMapping m_classMapping;

  using PathMapping = std::map<std::string, std::string>;
  PathMapping m_pathMapping;

  uint32_t m_contentDirectoryVersion;

  bool m_simpleVideoItems;
  bool m_simpleStorageFolders;

  std::string m_protocolInfo;

  class MediaProfile : public IXmlDeserializable
  {
  public:
    MediaProfile() = default;

    // implementation of IXmlDeserializable
    bool Deserialize(const TiXmlNode* node) override;

    inline bool HasMimeType() const { return !m_mimeType.empty(); }

    inline bool MatchesContainer(const std::string& container, bool matchEmpty = false) const { return MatchesSet(m_containers, container, matchEmpty); }
    inline bool MatchesVideoCodec(const std::string& videoCodec, bool matchEmpty = false) const { return MatchesSet(m_videoCodecs, videoCodec, matchEmpty); }
    inline bool MatchesAudioCodec(const std::string& audioCodec, bool matchEmpty = false) const { return MatchesSet(m_audioCodecs, audioCodec, matchEmpty); }

    // TODO

    const std::string& GetMimeType() const { return m_mimeType; }

  private:
    inline static bool MatchesSet(const std::set<std::string>& set, const std::string& value, bool matchEmpty) { return (matchEmpty && set.empty()) || set.find(value) != set.cend(); }
    std::set<std::string> m_containers;
    std::set<std::string> m_videoCodecs;
    std::set<std::string> m_audioCodecs;
    std::string m_mimeType;
  };
  std::vector<MediaProfile> m_mediaProfiles;
};
