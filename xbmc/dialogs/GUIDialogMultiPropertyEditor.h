#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://www.xbmc.org
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

#include "GUIDialogBoxBase.h"
#include "view/GUIViewControl.h"

class CFileItem;
class CFileItemList;
class CVariant;

class CGUIDialogMultiPropertyEditor : public CGUIDialogBoxBase
{
public:
  CGUIDialogMultiPropertyEditor();
  virtual ~CGUIDialogMultiPropertyEditor();

  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnBack(int actionID);

  void Reset();
  void SetItems(const CFileItemList &items);
  void SetAssignedItems(const CFileItemList &items);
  void AddItem(const std::string &item);
  void AddItem(const CFileItem *item);
  void AddAssignedItem(const std::string &item);
  void AddAssignedItem(const CFileItem *item);
  void SetSorting(const SortDescription &sorting);
  void SetLimit(unsigned int limit);

  const CFileItemList& GetAssignedItems() const;

protected:
  virtual CGUIControl *GetFirstFocusableControl(int id);
  virtual void OnWindowLoaded();
  virtual void OnInitWindow();
  virtual void OnWindowUnload();

  void SetupButton();
  void UpdateControls();

  bool m_aborted;
  SortDescription m_sorting;
  unsigned int m_limit;
  
  CFileItemList* m_items;
  CFileItemList* m_assignedItems;
  CGUIViewControl m_viewControlItems;
  CGUIViewControl m_viewControlAssignedItems;
};
