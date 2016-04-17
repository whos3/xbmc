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

#include <algorithm>
#include <functional>

#include "ohUPnPDeviceProfile.h"
#include "network/upnp/openHome/ohUPnPDevice.h"
#include "network/upnp/openHome/didllite/objects/classmappers/SimpleUPnPClassMapper.h"
#include "utils/log.h"
#include "utils/Mime.h"
#include "utils/Regexp.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/XBMCTinyXML.h"
#include "utils/XMLUtils.h"

static const std::string RootElement = "profile";
static const std::string RootElementExtendsAttribute = "extends";
static const std::string NameElement = "name";

static const std::string DeviceIdentificationElement = "identification";
static const std::string DeviceIdentificationRuleElement = "rule";
static const std::string DeviceIdentificationRulePropertyAttribute = "property";
static const std::string DeviceIdentificationRuleTypeAttribute = "type";

static const std::string ProtocolInfoElement = "protocolinfo";

static const std::string MediaServerElement = "mediaserver";
static const std::string MediaServerTypeMappingElement = "typemapping";
static const std::string MediaServerTypeMappingDefaultAttribute = "default";
static const std::string MediaServerTypeMappingMapElement = "map";
static const std::string MediaServerTypeMappingMapTypeAttribute = "type";
static const std::string MediaServerTypeMappingMapSimpleClassAttribute = "class";
static const std::string MediaServerTypeMappingMapSimpleToAttribute = "to";
static const std::string MediaServerPathMappingElement = "pathmapping";
static const std::string MediaServerPathMappingMapElement = "map";
static const std::string MediaServerPathMappingMapIdAttribute = "id";
static const std::string MediaServerPathMappingMapToAttribute = "to";

static const std::string ContentDirectoryElement = "contentdirectory";
static const std::string ContentDirectoryVersionElement = "version";
static const uint32_t ContentDirectoryVersionMinimum = 1;
static const uint32_t ContentDirectoryVersionMaximum = 4;

static const std::string MediaRendererElement = "mediarenderer";

static const std::string MediaProfilesElement = "mediaprofiles";
static const std::string MediaProfileElement = "mediaprofile";
static const std::string MediaProfileContainersElement = "containers";
static const std::string MediaProfileContainerElement = "container";
static const std::string MediaProfileVideoCodecsElement = "videocodecs";
static const std::string MediaProfileVideoCodecElement = "videocodec";
static const std::string MediaProfileAudioCodecsElement = "audiocodecs";
static const std::string MediaProfileAudioCodecElement = "audiocodec";
static const std::string MediaProfileMimeTypeElement = "mimetype";

COhUPnPDeviceProfile::COhUPnPDeviceProfile()
  : m_loaded(false),
    m_contentDirectoryVersion(1)
{ }

bool COhUPnPDeviceProfile::Load(const std::string& profileLocation)
{
  if (profileLocation.empty())
    return false;

  CXBMCTinyXML profileDoc;
  if (!profileDoc.LoadFile(profileLocation))
  {
    CLog::Log(LOGERROR, "COhUPnPDeviceProfile: failed to load profile from %s", profileLocation.c_str());
    return false;
  }

  const TiXmlElement* root = profileDoc.RootElement();
  if (root == nullptr || root->ValueStr() != RootElement)
  {
    CLog::Log(LOGERROR, "COhUPnPDeviceProfile: missing root element <%s> in profile from %s", RootElement.c_str(), profileLocation.c_str());
    return false;
  }

  // handle extended profiles
  const std::string* extends = root->Attribute(RootElementExtendsAttribute);
  if (extends != nullptr && !extends->empty())
  {
    std::string profileDirectory = URIUtils::GetDirectory(profileLocation);
    std::string extendedProfileLocation = URIUtils::AddFileToFolder(profileDirectory, *extends + ".xml");
    if (!Load(extendedProfileLocation))
    {
      CLog::Log(LOGERROR, "COhUPnPDeviceProfile: %s extends %s but it failed to load", profileLocation.c_str(), extendedProfileLocation.c_str());
      return false;
    }
    else
    {
      CLog::Log(LOGDEBUG, "COhUPnPDeviceProfile: %s extends %s", profileLocation.c_str(), extendedProfileLocation.c_str());

      // reset m_loaded in case something else goes wrong
      m_loaded = false;
    }
  }

  // read the name
  const TiXmlElement* element = root->FirstChildElement(NameElement);
  if (element == nullptr || element->FirstChild() == nullptr)
  {
    CLog::Log(LOGERROR, "COhUPnPDeviceProfile: missing <%s> element in profile from %s", NameElement.c_str(), profileLocation.c_str());
    return false;
  }

  m_name = element->FirstChild()->ValueStr();
  if (m_name.empty())
  {
    CLog::Log(LOGERROR, "COhUPnPDeviceProfile: empty <%s> element in profile from %s", NameElement.c_str(), profileLocation.c_str());
    return false;
  }

  if (!LoadDeviceIdentification(root))
    return false;

  if (!LoadGeneral(root))
    return false;

  if (!LoadMediaServer(root))
    return false;

  if (!LoadMediaRenderer(root))
    return false;

  if (!LoadContentDirectory(root))
    return false;

  if (!LoadMediaProfiles(root))
    return false;

  // TODO

  m_loaded = true;
  return true;
}

bool COhUPnPDeviceProfile::Matches(const COhUPnPDevice& device) const
{
  if (!device.IsValid())
    return false;

  for (const auto& rule : m_identificationRules)
  {
    if (rule.Matches(device))
      return true;
  }

  return false;
}

bool COhUPnPDeviceProfile::GetMappedPath(const std::string& objectId, std::string& path) const
{
  if (objectId.empty())
    return false;

  path = objectId;
  const auto& mapped = m_pathMapping.find(objectId);
  if (mapped == m_pathMapping.end())
    return false;

  path = mapped->second;
  return true;
}

std::string COhUPnPDeviceProfile::GetMimeType(std::string container) const
{
  StringUtils::Trim(container, ".");
  std::transform(container.begin(), container.end(), container.begin(), ::tolower);

  for (const auto& mediaProfile : m_mediaProfiles)
  {
    if (mediaProfile.HasMimeType() && mediaProfile.MatchesContainer(container))
      return mediaProfile.GetMimeType();
  }

  // fall back to the general MIME list
  return CMime::GetMimeType(container);
}

bool COhUPnPDeviceProfile::LoadDeviceIdentification(const TiXmlElement* root)
{
  const TiXmlElement* identification = root->FirstChildElement(DeviceIdentificationElement);
  if (identification != nullptr)
  {
    // load the identification rules
    const TiXmlNode* identificationRuleNode = identification->FirstChild(DeviceIdentificationRuleElement);
    while (identificationRuleNode != nullptr)
    {
      DeviceIdentificationRule rule;
      if (!rule.Deserialize(identificationRuleNode))
      {
        CLog::Log(LOGWARNING, "COhUPnPDeviceProfile: invalid <%s><%s> element in profile %s",
          DeviceIdentificationElement.c_str(), DeviceIdentificationRuleElement.c_str(), m_name.c_str());
      }
      else
        m_identificationRules.push_back(rule);

      identificationRuleNode = identificationRuleNode->NextSibling(DeviceIdentificationRuleElement);
    }
  }

  return true;
}

bool COhUPnPDeviceProfile::LoadGeneral(const TiXmlElement* root)
{
  if (root == nullptr)
    return false;

  XMLUtils::GetString(root, ProtocolInfoElement.c_str(), m_protocolInfo);

  return true;
}

bool COhUPnPDeviceProfile::LoadMediaServer(const TiXmlElement* root)
{
  const TiXmlElement* mediaServer = root->FirstChildElement(MediaServerElement);
  if (mediaServer == nullptr)
    return true;

  if (!LoadTypeMapping(mediaServer))
    return false;
  if (!LoadPathMapping(mediaServer))
    return false;

  return true;
}

bool COhUPnPDeviceProfile::LoadTypeMapping(const TiXmlElement* root)
{
  const TiXmlElement* typeMapping = root->FirstChildElement(MediaServerTypeMappingElement);
  if (typeMapping != nullptr)
  {
    // load the "default" attribute
    const std::string* defaultAttribute = typeMapping->Attribute(MediaServerTypeMappingDefaultAttribute);
    if (defaultAttribute != nullptr)
      m_classMapping.SetDefaultMediaType(*defaultAttribute);

    // load all mappings
    const TiXmlElement* typeMapElement = typeMapping->FirstChildElement(MediaServerTypeMappingMapElement);
    while (typeMapElement != nullptr)
    {
      // load the type of the mapping
      const std::string* typeAttribute = typeMapElement->Attribute(MediaServerTypeMappingMapTypeAttribute);
      if (typeAttribute == nullptr || *typeAttribute == "simple")
      {
        const std::string* classAttribute = typeMapElement->Attribute(MediaServerTypeMappingMapSimpleClassAttribute);
        const std::string* toAttribute = typeMapElement->Attribute(MediaServerTypeMappingMapSimpleToAttribute);
        if (classAttribute != nullptr && !classAttribute->empty() && toAttribute != nullptr && !toAttribute->empty())
          m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper(*classAttribute, *toAttribute));
        else
        {
          CLog::Log(LOGWARNING, "COhUPnPDeviceProfile: simple type mapping element with invalid \"%s\" or \"%s\" attribute in profile %s",
            MediaServerTypeMappingMapSimpleClassAttribute.c_str(), MediaServerTypeMappingMapSimpleToAttribute.c_str(), m_name.c_str());
        }
      }
      else
      {
        CLog::Log(LOGWARNING, "COhUPnPDeviceProfile: <%s><%s> element with unknown type \"%s\" in profile %s",
          MediaServerTypeMappingElement.c_str(), MediaServerTypeMappingMapElement.c_str(), typeAttribute->c_str(), m_name.c_str());
      }

      typeMapElement = typeMapElement->NextSiblingElement(MediaServerTypeMappingMapElement);
    }
  }

  return true;
}

bool COhUPnPDeviceProfile::LoadPathMapping(const TiXmlElement* root)
{
  const TiXmlElement* pathMapping = root->FirstChildElement(MediaServerPathMappingElement);
  if (pathMapping != nullptr)
  {
    // load all mappings
    const TiXmlElement* pathMapElement = pathMapping->FirstChildElement(MediaServerPathMappingMapElement);
    while (pathMapElement != nullptr)
    {
      const std::string* idAttribute = pathMapElement->Attribute(MediaServerPathMappingMapIdAttribute);
      if (idAttribute == nullptr || idAttribute->empty())
      {
        CLog::Log(LOGWARNING, "COhUPnPDeviceProfile: path mapping element with invalid \"%s\" attribute in profile %s",
          MediaServerPathMappingMapIdAttribute.c_str(), m_name.c_str());
      }
      else
      {
        const std::string* toAttribute = pathMapElement->Attribute(MediaServerPathMappingMapToAttribute);
        if (toAttribute != nullptr && !toAttribute->empty())
          m_pathMapping.insert(std::make_pair(*idAttribute, *toAttribute));
        else
        {
          CLog::Log(LOGWARNING, "COhUPnPDeviceProfile: path mapping element with invalid \"%s\" attribute in profile %s",
            MediaServerPathMappingMapToAttribute.c_str(), m_name.c_str());
        }
      }

      pathMapElement = pathMapElement->NextSiblingElement(MediaServerPathMappingMapElement);
    }
  }

  return true;
}

bool COhUPnPDeviceProfile::LoadMediaRenderer(const TiXmlElement* root)
{
  const TiXmlElement* mediaRenderer = root->FirstChildElement(MediaRendererElement);
  if (mediaRenderer == nullptr)
    return true;

  // TODO

  return true;
}

bool COhUPnPDeviceProfile::LoadContentDirectory(const TiXmlElement* root)
{
  const TiXmlElement* contentDirectory = root->FirstChildElement(ContentDirectoryElement);
  if (contentDirectory == nullptr)
    return true;

  XMLUtils::GetUInt(contentDirectory, ContentDirectoryVersionElement.c_str(), m_contentDirectoryVersion, ContentDirectoryVersionMinimum, ContentDirectoryVersionMaximum);

  return true;
}

bool COhUPnPDeviceProfile::LoadMediaProfiles(const TiXmlElement* root)
{
  const TiXmlElement* mediaProfilesNode = root->FirstChildElement(MediaProfilesElement);
  if (mediaProfilesNode != nullptr)
  {
    // load the identification rules
    const TiXmlNode* mediaProfileNode = mediaProfilesNode->FirstChild(MediaProfileElement);
    while (mediaProfileNode != nullptr)
    {
      MediaProfile mediaProfile;
      if (!mediaProfile.Deserialize(mediaProfileNode))
      {
        CLog::Log(LOGWARNING, "COhUPnPDeviceProfile: invalid <%s><%s> element in profile %s",
          MediaProfilesElement.c_str(), MediaProfileElement.c_str(), m_name.c_str());
      }
      else
        m_mediaProfiles.push_back(mediaProfile);

      mediaProfileNode = mediaProfileNode->NextSibling(MediaProfileElement);
    }
  }

  return true;
}

COhUPnPDeviceProfile::DeviceIdentificationRule::DeviceIdentificationRule()
  : m_type(MatchType::Exact),
    m_property(nullptr)
{ }

bool COhUPnPDeviceProfile::DeviceIdentificationRule::Deserialize(const TiXmlNode* node)
{
  if (node == nullptr || node->Type() != TiXmlNode::TINYXML_ELEMENT || node->FirstChild() == nullptr)
    return false;

  m_value = node->FirstChild()->ValueStr();
  if (m_value.empty())
    return false;

  const TiXmlElement* element = node->ToElement();
  const std::string* type = element->Attribute(DeviceIdentificationRuleTypeAttribute);
  if (type == nullptr || *type == "exact")
    m_type = MatchType::Exact;
  else if (*type == "substring")
    m_type = MatchType::Substring;
  else if (*type == "regex")
    m_type = MatchType::Regex;
  else
    return false;

  const std::string* prop = element->Attribute(DeviceIdentificationRulePropertyAttribute);
  if (prop == nullptr || prop->empty())
    return false;

  if (StringUtils::EqualsNoCase(prop->c_str(), "friendlyname"))
    m_property = &COhUPnPDevice::GetFriendlyName;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "manufacturer"))
    m_property = &COhUPnPDevice::GetManufacturer;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "manufacturerurl"))
    m_property = &COhUPnPDevice::GetManufacturerUrl;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "modelname"))
    m_property = &COhUPnPDevice::GetModelName;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "modelnumber"))
    m_property = &COhUPnPDevice::GetModelNumber;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "modeldescription"))
    m_property = &COhUPnPDevice::GetModelDescription;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "modelurl"))
    m_property = &COhUPnPDevice::GetModelUrl;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "serialnumber"))
    m_property = &COhUPnPDevice::GetSerialNumber;
  else if (StringUtils::EqualsNoCase(prop->c_str(), "user-agent"))
    m_property = &COhUPnPDevice::GetUserAgent;
  else
    return false;

  return true;
}

bool COhUPnPDeviceProfile::DeviceIdentificationRule::Matches(const COhUPnPDevice& device) const
{
  if (!device.IsValid() || m_value.empty())
    return false;

  auto propertyFun = std::mem_fn(m_property);
  const std::string& propertyValue = propertyFun(device);

  switch (m_type)
  {
    case MatchType::Exact:
      return propertyValue == m_value;

    case MatchType::Substring:
      return propertyValue.find(m_value.c_str()) != std::string::npos;

    case MatchType::Regex:
    {
      CRegExp regex;
      if (!regex.RegComp(m_value))
      {
        CLog::Log(LOGWARNING, "COhUPnPDeviceProfile: invalid regular expression \"%s\" in identification rule", m_value.c_str());
        return false;
      }

      // regular expression must match the whole property value
      int result = regex.RegFind(propertyValue);
      return regex.RegFind(propertyValue) == 0 && regex.GetFindLen() == propertyValue.size();
    }
  }

  return false;
}

bool COhUPnPDeviceProfile::MediaProfile::Deserialize(const TiXmlNode* node)
{
  if (node == nullptr)
    return false;

  const TiXmlNode* containersNode = node->FirstChild(MediaProfileContainersElement);
  if (containersNode != nullptr)
  {
    std::vector<std::string> containers;
    if (XMLUtils::GetStringArray(containersNode, MediaProfileContainerElement.c_str(), containers))
      m_containers.insert(containers.cbegin(), containers.cend());
  }

  const TiXmlNode* videoCodecsNode = node->FirstChild(MediaProfileVideoCodecsElement);
  if (videoCodecsNode != nullptr)
  {
    std::vector<std::string> videoCodecs;
    if (XMLUtils::GetStringArray(videoCodecsNode, MediaProfileVideoCodecElement.c_str(), videoCodecs))
      m_videoCodecs.insert(videoCodecs.cbegin(), videoCodecs.cend());
  }

  const TiXmlNode* audioCodecsNode = node->FirstChild(MediaProfileAudioCodecsElement);
  if (audioCodecsNode != nullptr)
  {
    std::vector<std::string> audioCodecs;
    if (XMLUtils::GetStringArray(audioCodecsNode, MediaProfileAudioCodecElement.c_str(), audioCodecs))
      m_audioCodecs.insert(audioCodecs.cbegin(), audioCodecs.cend());
  }

  if (m_containers.empty() && m_videoCodecs.empty() && m_audioCodecs.empty())
    return false;

  XMLUtils::GetString(node, MediaProfileMimeTypeElement.c_str(), m_mimeType);

  return !m_mimeType.empty();
}
