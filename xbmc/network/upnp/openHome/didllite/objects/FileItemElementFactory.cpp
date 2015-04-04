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

#include "FileItemElementFactory.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "utils/SpecialSort.h"

CFileItemElementFactory::CFileItemElementFactory()
{ }

CFileItemElementFactory::~CFileItemElementFactory()
{ }

IFileItemElement* CFileItemElementFactory::GetElement(const CFileItem& item) const
{
  const auto& element = std::find_if(m_elements.cbegin(), m_elements.cend(),
    [&item](const IDidlLiteElementPair& element)
    {
      return std::static_pointer_cast<const IFileItemElement>(element.second)->CanHandleFileItem(item);
    });
  if (element == m_elements.cend())
    return nullptr;

  return static_cast<IFileItemElement*>(element->second->Clone());
}

bool CFileItemElementFactory::RegisterElement(const IFileItemElement* element)
{
  if (!CDidlLiteElementFactory::RegisterElement(element))
    return false;

  // sort all elements topologically
  std::vector<std::pair<std::string, std::string>> unsortedTopology;
  for (const auto& element : m_elements)
  {
    const auto& identifier = element.second->GetIdentifier();
    const auto& extends = element.second->Extends();

    for (const auto& extend : extends)
      unsortedTopology.push_back(std::make_pair(identifier, extend));
  }

  const auto& topology = SpecialSort::SortTopologically(unsortedTopology);

  // re-order m_elements
  IDidlLiteElementPairs elementsCopy(m_elements.cbegin(), m_elements.cend());
  m_elements.clear();
  for (const auto& node : topology)
  {
    // find the matching element
    const auto& element = FindElementByIdentifier(elementsCopy, node);
    if (element == elementsCopy.cend())
      continue;

    // add it to m_elements and remove it from the copy
    m_elements.push_back(*element);
    elementsCopy.erase(element);
  }

  // reverse the re-ordered m_elements to have the most specialized implementation at the beginning
  std::reverse(m_elements.begin(), m_elements.end());

  return true;
}

CDidlLiteElementFactory::IDidlLiteElementPairs::const_iterator CFileItemElementFactory::FindElementByIdentifier(const IDidlLiteElementPairs& elements, const std::string& identifier)
{
  return std::find_if(elements.cbegin(), elements.cend(),
    [&identifier](const IDidlLiteElementPair& pair)
  {
    return pair.second->GetIdentifier() == identifier;
  });
}
