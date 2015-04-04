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

#include <string>

#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

class CFileItem;

class IFileItemElement : public IDidlLiteElement
{
public:
  virtual ~IFileItemElement() { }

  /*!
   * \brief TODO
   */
  virtual std::string GetType() const = 0;

  /*!
   * \brief TODO
   *
   * \param[in] item TODO
   * \return TODO
   */
  virtual bool CanHandleFileItem(const CFileItem& item) const = 0;

  /*!
   * \brief TODO
   *
   * \param[out] item TODO
   * \param[in] context TODO
   * \return TODO
   */
  virtual bool ToFileItem(CFileItem& item, const OhUPnPControlPointContext& context) const = 0;

  /*!
   * \brief TODO
   *
   * \param[out] item TODO
   * \param[in] context TODO
   * \return TODO
   */
  virtual bool FromFileItem(const CFileItem& item, const OhUPnPRootDeviceContext& context) = 0;

protected:
  IFileItemElement() { }
  IFileItemElement(const std::string& name)
    : IDidlLiteElement(name)
  { }
  IFileItemElement(const std::string& ns, const std::string& name)
    : IDidlLiteElement(ns, name)
  { }
  IFileItemElement(const IFileItemElement& fileItemElement)
    : IDidlLiteElement(fileItemElement)
  { }
};
