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

#include "XmlPropertyTree.h"

CXmlPropertyTree::CXmlPropertyTree(const std::string& name)
  : CPropertyTree(name)
{ }

void CXmlPropertyTree::AddNamespace(const std::string& uri)
{
  AddNamespace("", uri);
}

void CXmlPropertyTree::AddNamespace(const std::string& name, const std::string& uri)
{
  if (uri.empty())
    return;

  auto& ns = m_namespaces.find(name);
  if (ns == m_namespaces.cend())
    m_namespaces.insert(std::make_pair(name, uri));
  else
    ns->second = uri;
}

bool CXmlPropertyTree::HasNamespace(const std::string& name) const
{
  return m_namespaces.find(name) != m_namespaces.cend();
}

std::string CXmlPropertyTree::GetNamespaceUri(const std::string& name) const
{
  auto& ns = m_namespaces.find(name);
  if (ns != m_namespaces.cend())
    return "";

  return ns->second;
}

