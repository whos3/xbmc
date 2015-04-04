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

#include "network/upnp/openHome/didllite/objects/UPnPObject.h"
#include "network/upnp/openHome/didllite/objects/properties/UPnPClass.h"

class CUPnPContainer : public CUPnPObject
{
public:
  class CUPnPClassDefinition : public CUPnPClass
  {
  public:
    CUPnPClassDefinition(const std::string& classType = "", const std::string& className = "", bool includeDerived = false);
    CUPnPClassDefinition(const CUPnPClassDefinition& classDefinition);
    virtual ~CUPnPClassDefinition();

    // implementations of IDidlLiteElement
    virtual CUPnPClassDefinition* Create() const override { return new CUPnPClassDefinition(); }
    virtual CUPnPClassDefinition* Clone() const override { return new CUPnPClassDefinition(*this); }
    virtual std::string GetIdentifier() const { return "ClassDefinition"; }
    virtual std::set<std::string> Extends() const { return { "Class" }; }

    bool IncludeDerived() const { return m_includeDerived; }

  protected:
    // implementation of IDidlLiteElement
    void initializeProperties();

  private:
    bool m_includeDerived;
  };

public:
  CUPnPContainer();
  CUPnPContainer(const std::string& classType, const std::string& className = "");
  CUPnPContainer(const CUPnPContainer& container);
  virtual ~CUPnPContainer();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const override { return Clone(); }
  virtual IDidlLiteElement* Clone() const override { return new CUPnPContainer(*this); }
  virtual std::string GetIdentifier() const { return "Container"; }
  virtual std::set<std::string> Extends() const { return { "Object" }; }

  // specializations of IFileItemElement
  virtual bool CanHandleFileItem(const CFileItem& item) const override;
  virtual bool ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const override;
  virtual bool FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context) override;

protected:
  unsigned int GetContainerUpdateID() const { return m_containerUpdateID; }
  unsigned int GetTotalDeletedChildCount() const { return m_totalDeletedChildCount; }
  unsigned int GetChildCount() const { return m_childCount; }
  unsigned int GetChildContainerCount() const { return m_childContainerCount; }
  const std::vector<CUPnPClassDefinition>& GetCreateClasses() const { return m_createClass; }
  const std::vector<CUPnPClassDefinition>& GetSearchClasses() const { return m_searchClass; }
  bool IsSearchable() const { return m_searchable; }
  int64_t GetStorageTotal() const { return m_storageTotal; }
  int64_t GetStorageUsed() const { return m_storageUsed; }
  int64_t GetStorageFree() const { return m_storageFree; }
  int64_t GetStorageMaxPartition() const { return m_storageMaxPartition; }

private:
  void initializeProperties();

  uint32_t m_containerUpdateID;
  uint32_t m_totalDeletedChildCount;
  uint32_t m_childCount;
  uint32_t m_childContainerCount;
  std::vector<CUPnPClassDefinition> m_createClass;
  std::vector<CUPnPClassDefinition> m_searchClass;
  bool m_searchable;
  int64_t m_storageTotal;
  int64_t m_storageUsed;
  int64_t m_storageFree;
  int64_t m_storageMaxPartition;
};
