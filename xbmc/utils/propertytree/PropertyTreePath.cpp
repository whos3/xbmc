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

#include "PropertyTreePath.h"
#include "utils/StringUtils.h"

const std::string CPropertyTreePath::Separator = ".";
const std::string CPropertyTreePath::AttributePrefix = "@";

CPropertyTreePath::CPropertyTreePath(const std::string& path)
{
  if (path.empty())
    return;

  const auto elements = StringUtils::Split(path, Separator);
  for (auto element : elements)
  {
    PathElement pathElement = { element, StringUtils::StartsWith(element, AttributePrefix) };
    if (pathElement.isAttribute)
    {
      pathElement.name = StringUtils::Mid(pathElement.name, AttributePrefix.size());
      m_elements.push_back(pathElement);
      break;
    }
    else
    {
      // check if the element contains an attribute
      size_t pos = pathElement.name.find(AttributePrefix);
      if (pos != std::string::npos)
      {
        pathElement.name = StringUtils::Left(pathElement.name, pos);
        m_elements.push_back(pathElement);

        // now extract the attribute
        pathElement.name = StringUtils::Mid(pathElement.name, pos + 1);
        if (!pathElement.name.empty())
        {
          pathElement.isAttribute = true;
          m_elements.push_back(pathElement);
        }
      }
      else
        m_elements.push_back(pathElement);
    }
  }
}

CPropertyTreePath::CPropertyTreePath(const PathElements& elements)
  : m_elements(elements)
{ }

bool CPropertyTreePath::IsElement() const
{
  if (IsEmpty())
    return true;

  return !m_elements.front().isAttribute;
}

std::string CPropertyTreePath::GetCurrentElement() const
{
  if (IsEmpty())
    return "";

  return m_elements.front().name;
}

CPropertyTreePath CPropertyTreePath::Next() const
{
  if (m_elements.size() < 2)
    return CPropertyTreePath("");

  return CPropertyTreePath(PathElements(m_elements.cbegin() + 1, m_elements.end()));
}

std::string CPropertyTreePath::ToString() const
{
  std::string path;
  for (const auto& element : m_elements)
  {
    if (element.isAttribute)
      path += AttributePrefix;
    else if (!path.empty())
      path += Separator;

    path += element.name;
  }

  return path;
}
