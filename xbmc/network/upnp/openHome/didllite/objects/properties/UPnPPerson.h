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

class CUPnPPerson : public IDidlLiteElement
{
public:
  virtual ~CUPnPPerson();

  const std::string& GetPerson() const { return m_personName; }
  const std::string& GetPersonRole() const { return m_personRole; }
  virtual std::string GetIdentifier() const { return "Person"; }
  virtual std::set<std::string> Extends() const { return { }; }

protected:
  explicit CUPnPPerson(const std::string& elementName);
  CUPnPPerson(const std::string& elementName, const std::string& person, const std::string& role = "");
  CUPnPPerson(const CUPnPPerson& upnpPerson);

  // implementations of IDidlLiteElement
  virtual bool deserialize(const TiXmlNode* node, const OhUPnPControlPointContext& context) override;

  void initializeProperties();

private:
  std::string m_personName;
  std::string m_personRole;
};

class CUPnPActor : public CUPnPPerson
{
public:
  CUPnPActor();
  CUPnPActor(const std::string& person, const std::string& role = "");
  CUPnPActor(const CUPnPActor& upnpActor);
  virtual ~CUPnPActor();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const override { return new CUPnPActor(); }
  virtual IDidlLiteElement* Clone() const override { return new CUPnPActor(*this); }
  virtual std::string GetIdentifier() const { return "Actor"; }
  virtual std::set<std::string> Extends() const { return { "Person" }; }
};

class CUPnPArtist : public CUPnPPerson
{
public:
  CUPnPArtist();
  CUPnPArtist(const std::string& person, const std::string& role = "");
  CUPnPArtist(const CUPnPArtist& upnpArtist);
  virtual ~CUPnPArtist();

  // implementations of IDidlLiteElement
  virtual CUPnPArtist* Create() const override { return new CUPnPArtist(); }
  virtual CUPnPArtist* Clone() const override { return new CUPnPArtist(*this); }
  virtual std::string GetIdentifier() const { return "Artist"; }
  virtual std::set<std::string> Extends() const { return { "Person" }; }
};

class CUPnPAuthor : public CUPnPPerson
{
public:
  CUPnPAuthor();
  CUPnPAuthor(const std::string& person, const std::string& role = "");
  CUPnPAuthor(const CUPnPAuthor& upnpAuthor);
  virtual ~CUPnPAuthor();

  // implementations of IDidlLiteElement
  virtual CUPnPAuthor* Create() const override { return new CUPnPAuthor(); }
  virtual CUPnPAuthor* Clone() const override { return new CUPnPAuthor(*this); }
  virtual std::string GetIdentifier() const { return "Author"; }
  virtual std::set<std::string> Extends() const { return { "Person" }; }
};
