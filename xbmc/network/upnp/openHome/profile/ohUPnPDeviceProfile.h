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

  // TODO

private:
  bool LoadIdentification(const TiXmlElement* root);
  bool LoadMediaServer(const TiXmlElement* root);
  bool LoadTypeMapping(const TiXmlElement* root);
  bool LoadPathMapping(const TiXmlElement* root);
  bool LoadContentDirectory(const TiXmlElement* root);

  bool m_loaded;
  std::string m_name;

  class IdentificationRule : public IXmlDeserializable
  {
  public:
    IdentificationRule();

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
  std::vector<IdentificationRule> m_identificationRules;

  CUPnPClassMapping m_classMapping;

  using PathMapping = std::map<std::string, std::string>;
  PathMapping m_pathMapping;

  uint32_t m_contentDirectoryVersion;
};
