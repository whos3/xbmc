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

#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

class CUPnPClass : public IDidlLiteElement
{
public:
  CUPnPClass(const std::string& classType = "", const std::string& className = "");
  CUPnPClass(const CUPnPClass& upnpClass);
  virtual ~CUPnPClass();

  // implementations of IDidlLiteElement
  virtual CUPnPClass* Create() const override { return new CUPnPClass(); }
  virtual CUPnPClass* Clone() const override { return new CUPnPClass(*this); }
  virtual std::string GetIdentifier() const { return "Class"; }
  virtual std::set<std::string> Extends() const { return { }; }

  const std::string& GetType() const { return m_classType; }
  void SetType(const std::string& classType) { m_classType = classType; }
  const std::string& GetName() const { return m_className; }
  void SetName(const std::string& className) { m_className = className; }

protected:
  // implementations of IDidlLiteElement
  virtual bool deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context) override;

private:
  void initializeProperties();

  std::string m_classType;
  std::string m_className;
};
