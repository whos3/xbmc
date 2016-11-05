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

#include "SettingDefinitionXmlDeserializer.h"
#include "settings/lib/ISetting.h"
#include "settings/lib/Setting.h"
#include "settings/lib/SettingsManager.h"
#include "utils/log.h"
#include "utils/XBMCTinyXML.h"
#include "utils/XMLUtils.h"

static const char* AttributeId = "id";
static const char* AttributeVisible = "visible";
static const char* AttributeLabel = "label";
static const char* AttributeHelp = "help";
static const char* AttributeRequirement = "requirement";
static const char* AttributeParent = "parent";
static const char* AttributeLevel = "level";
static const char* AttributeDependencies = "dependencies";
static const char* AttributeDependency = "dependency";

bool CSettingDefinitionXmlDeserializer::Deserialize(const TiXmlElement* element, ISetting* setting, CSettingsManager* settingsManager, bool update /* = false */) const
{
  if (element == nullptr || setting == nullptr)
    return false;

  // deserialize visibility
  bool value;
  if (XMLUtils::GetBoolean(element, AttributeVisible, value))
    setting->SetVisible(value);

  // deserialize label and help
  int iValue = -1;
  if (element->QueryIntAttribute(AttributeLabel, &iValue) == TIXML_SUCCESS && iValue > 0)
    setting->SetLabel(iValue);
  if (element->QueryIntAttribute(AttributeHelp, &iValue) == TIXML_SUCCESS && iValue > 0)
    setting->SetHelp(iValue);

  // deserialize requirements
  CSettingRequirement requirements;
  const TiXmlNode *requirementNode = element->FirstChild(AttributeRequirement);
  if (requirementNode != nullptr && !requirements.Deserialize(requirementNode))
  {
    CLog::Log(LOGERROR, "CSettingDefinitionXmlDeserializer: error reading <requirement> tag of \"%s\"", setting->GetId().c_str());
    return false;
  }
  setting->SetRequirements(requirements);

  // if the setting is a CSetting deserialize additional properties
  CSetting* csetting = dynamic_cast<CSetting*>(setting);
  if (csetting != nullptr)
  {
    // deserialize parent setting
    const char *parentSetting = element->Attribute(AttributeParent);
    if (parentSetting != nullptr)
      csetting->SetParent(parentSetting);

    //deserialize level
    int level = -1;
    if (XMLUtils::GetInt(element, AttributeLevel, level))
      csetting->SetLevel(static_cast<SettingLevel>(level));

    if (csetting->GetLevel() < SettingLevelBasic || csetting->GetLevel() > SettingLevelInternal)
      csetting->SetLevel(SettingLevelStandard);

    // deserialize dependencies
    const TiXmlNode *dependenciesNode = element->FirstChild(AttributeDependencies);
    if (dependenciesNode != nullptr)
    {
      auto dependencies = csetting->GetDependencies();
      const TiXmlNode *dependencyNode = dependenciesNode->FirstChild(AttributeDependency);
      while (dependencyNode != nullptr)
      {
        CSettingDependency dependency(settingsManager);
        if (dependency.Deserialize(dependencyNode))
          dependencies.push_back(dependency);
        else
          CLog::Log(LOGWARNING, "CSettingDefinitionXmlDeserializer: error reading <dependency> tag of \"%s\"", setting->GetId().c_str());

        dependencyNode = dependencyNode->NextSibling(AttributeDependency);
      }

      csetting->SetDependencies(dependencies);
    }

    // deserialize control
    const TiXmlElement *control = element->FirstChildElement(SETTING_XML_ELM_CONTROL);
    if (control != nullptr)
    {
      const char *controlType = control->Attribute(SETTING_XML_ATTR_TYPE);
      if (controlType == nullptr)
      {
        CLog::Log(LOGERROR, "CSettingDefinitionXmlDeserializer: error reading \"type\" attribute of <control> tag of \"%s\"", setting->GetId().c_str());
        return false;
      }

      if (csetting->GetControl() != nullptr)
        delete csetting->GetControl();
      csetting->SetControl(settingsManager->CreateControl(controlType));
      if (csetting->GetControl() == nullptr || !csetting->GetControl()->Deserialize(control, update))
      {
        CLog::Log(LOGERROR, "CSettingDefinitionXmlDeserializer: error reading <control> tag of \"%s\"", setting->GetId().c_str());
        return false;
      }
    }
    else if (!update && csetting->GetLevel() < SettingLevelInternal)
    {
      CLog::Log(LOGERROR, "CSettingDefinitionXmlDeserializer: missing <control> tag of \"%s\"", setting->GetId().c_str());
      return false;
    }

    // deserialize updates
    const TiXmlNode *updatesNode = element->FirstChild(SETTING_XML_ELM_UPDATES);
    if (updatesNode != nullptr)
    {
      auto updates = csetting->GetUpdates();
      const TiXmlElement *updateElem = updatesNode->FirstChildElement(SETTING_XML_ELM_UPDATE);
      while (updateElem != nullptr)
      {
        CSettingUpdate settingUpdate;
        if (settingUpdate.Deserialize(updateElem))
        {
          if (!updates.insert(settingUpdate).second)
            CLog::Log(LOGWARNING, "CSettingDefinitionXmlDeserializer: duplicate <update> definition for \"%s\"", setting->GetId().c_str());
        }
        else
          CLog::Log(LOGWARNING, "CSettingDefinitionXmlDeserializer: error reading <update> tag of \"%s\"", setting->GetId().c_str());

        updateElem = updateElem->NextSiblingElement(SETTING_XML_ELM_UPDATE);
      }

      csetting->SetUpdates(updates);
    }
  }

  return true;
}

bool CSettingDefinitionXmlDeserializer::DeserializeIdentification(const TiXmlElement* element, std::string& identification) const
{
  if (element == nullptr)
    return false;

  const char *idAttribute = element->Attribute(AttributeId);
  if (idAttribute == nullptr || strlen(idAttribute) <= 0)
    return false;

  identification = idAttribute;
  return true;
}
