#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include <set>

#include <boost/shared_ptr.hpp>

#include "media/MediaType.h"
#include "media/import/IMediaImporter.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportChangesetTypes.h"

class CFileItem;
class CFileItemList;
class IMediaImportTask;

/*!
 * \brief Interface of a handler capable of handling imported media items of a
 * specific media type.
 */
class IMediaImportHandler
{
public:
  virtual ~IMediaImportHandler() { }

  virtual IMediaImportHandler* Create() const = 0;

  /*!
   * \brief Returns the media type the implementation is capable of handling.
   */
  virtual MediaType GetMediaType() const = 0;
  /*!
   * \brief Returns a list of media types which need to be handled before
   * using this implementation.
   */
  virtual std::set<MediaType> GetDependencies() const { return std::set<MediaType>(); }
  /*!
   * \brief Returns a list of media types which must be importable for
   * this implementation to be usable.
   */
  virtual std::set<MediaType> GetRequiredMediaTypes() const { return std::set<MediaType>(); }
  /*!
   * \brief Returns a list of media types which can be grouped together
   * with the media type of this implementation.
   */
  virtual std::vector<MediaType> GetGroupedMediaTypes() const { return std::vector<MediaType>(); }

  /*!
  * \brief Get the translated label representing the given item
  *
  * \param item Item to get the label for
  * \return Translated label representing the given item
  */
  virtual std::string GetItemLabel(const CFileItem* item) const = 0;

  /*!
  * \brief Gets a list of previously imported items from the given media import.
  *
  * \param import place from where the given items were imported
  * \param items List of previously imported items from the given media import
  */
  virtual bool GetLocalItems(const CMediaImport &import, CFileItemList& items) = 0;

  /*!
   * \brief TODO
   *
   * \param import TODO
   * \return TODO
   */
  virtual bool StartChangeset(const CMediaImport &import) = 0;

  /*!
  * \brief TODO
  *
  * \param import TODO
  * \return TODO
  */
  virtual bool FinishChangeset(const CMediaImport &import) = 0;

  /*!
  * \brief TODO
  *
  * \param import TODO
  * \param item TODO
  * \param localItems TODO
  * \return TODO
  */
  virtual MediaImportChangesetType DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemList& localItems) = 0;

  /*!
  * \brief Starts the synchronisation process
  *
  * \param import TODO
  * \return TODO
  */
  virtual bool StartSynchronisation(const CMediaImport &import) = 0;

  /*!
  * \brief Finishs the synchronisation process
  *
  * \param import TODO
  * \return TODO
  */
  virtual bool FinishSynchronisation(const CMediaImport &import) = 0;

  /*!
   * \brief Adds the given item from the given import to the library
   *
   * \param import TODO
   * \param item TODO
   * \return TODO
   */
  virtual bool AddImportedItem(const CMediaImport &import, CFileItem* item) = 0;

  /*!
   * \brief Updates the given item from the given import in the library
   *
   * \param import TODO
   * \param item TODO
   * \return TODO
   */
  virtual bool UpdateImportedItem(const CMediaImport &import, CFileItem* item) = 0;

  /*!
   * \brief Removes the given item from the given import from the library
   *
   * \param import TODO
   * \param item TODO
   * \return TODO
   */
  virtual bool RemoveImportedItem(const CMediaImport &import, const CFileItem* item) = 0;

  /*!
  * \brief Cleans up the imported items in the library
  *
  * \param import TODO
  * \param item TODO
  * \return TODO
  */
  virtual bool CleanupImportedItems(const CMediaImport &import) = 0;

  /*!
  * \brief Removes all imported items from the library
  *
  * \param import TODO
  * \param item TODO
  * \return TODO
  */
  virtual bool RemoveImportedItems(const CMediaImport &import) = 0;

  /*!
   * \brief Enable/disable imported items
   *
   * \param import TODO
   * \param enable Whether to enable or disable imported items
   */
  virtual void SetImportedItemsEnabled(const CMediaImport &import, bool enable) = 0;
};

typedef boost::shared_ptr<IMediaImportHandler> MediaImportHandlerPtr;
