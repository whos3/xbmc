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

#include "DlnaUtils.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

std::string CDlnaUtils::GetProfileID(const std::string& resourcePath)
{
  std::string extension = URIUtils::GetExtension(resourcePath);
  if (extension.empty())
    return "";

  // remove the leading dot if present
  if (StringUtils::StartsWith(extension, "."))
    extension = extension.substr(1);

  // make the extension all lower case for easier comparison
  StringUtils::ToLower(extension);

  if (extension == "jpg" || extension == "jpeg" || extension == "tbn")
    return "JPEG_TN";
  if (extension == "png")
    return "PNG_TN";
  if (extension == "gif")
    return "GIF_TN";

  return "";
}