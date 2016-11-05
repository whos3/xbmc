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

#include <string>

class ISetting;
class CSettingsManager;
class TiXmlElement;

class ISettingDefinitionXmlDeserializer
{
protected:
  ISettingDefinitionXmlDeserializer() = default;

public:
  virtual ~ISettingDefinitionXmlDeserializer() = default;

  /*!
  \brief TODO

  TODO
  */
  virtual bool CanHandle(const TiXmlElement* element) const = 0;

  /*!
   \brief TODO

   TODO
  */
  virtual ISetting* Create(const std::string& id) const = 0;

  /*!
   \brief Deserializes the given XML node into the properties of the setting
   object.
   
   If the update parameter is true, the checks for mandatory properties are
   skipped and values are only updated.
   
   \param element XML element containing the properties of the setting object
   \param setting Setting object
   \param update Whether to perform checks for mandatory properties or not
   \return True if deserialization was successful, false otherwise
  */
  virtual bool Deserialize(const TiXmlElement* element, ISetting* setting, CSettingsManager* settingsManager, bool update = false) const = 0;
};
