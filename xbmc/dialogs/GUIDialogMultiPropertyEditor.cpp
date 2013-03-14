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

#include "GUIDialogMultiPropertyEditor.h"
#include "FileItem.h"
#include "guilib/LocalizeStrings.h"
#include "utils/Variant.h"

#define CONTROL_HEADING                   1
#define CONTROL_LIST                      2
#define CONTROL_LIST_ASSIGNED             3
#define CONTROL_BUTTON                    4

static CFileItemList EmtpyItemList;

CGUIDialogMultiPropertyEditor::CGUIDialogMultiPropertyEditor()
    : CGUIDialogBoxBase(WINDOW_DIALOG_MULTIPROPERTY_EDITOR, "DialogMultiPropertyEditor.xml")
{
  m_aborted = false;
  m_sorting.sortBy = SortByLabel;
  m_items = new CFileItemList;
  m_assignedItems = new CFileItemList;
}

CGUIDialogMultiPropertyEditor::~CGUIDialogMultiPropertyEditor()
{
  delete m_items;
  delete m_assignedItems;
}

bool CGUIDialogMultiPropertyEditor::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_WINDOW_INIT:
      m_aborted = true;
      CGUIDialog::OnMessage(message);
      return true;

    case GUI_MSG_WINDOW_DEINIT:
      CGUIDialog::OnMessage(message);
      m_viewControlItems.Clear();
      m_viewControlAssignedItems.Clear();

      m_items->Clear();
      SET_CONTROL_LABEL(CONTROL_BUTTON, "");
      return true;

    case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (iControl == CONTROL_BUTTON)
      {
        m_aborted = false;
        Close();
        break;
      }

      if (iControl == CONTROL_LIST || iControl == CONTROL_LIST_ASSIGNED)
      {
        int iAction = message.GetParam1();
        if (iAction == ACTION_SELECT_ITEM || ACTION_MOUSE_LEFT_CLICK == iAction)
        {
          CFileItemList *itemsFrom = iControl == CONTROL_LIST ? m_items : m_assignedItems;
          CFileItemList *itemsTo   = iControl == CONTROL_LIST ? m_assignedItems : m_items;
          CGUIViewControl *viewControlFrom = iControl == CONTROL_LIST ? &m_viewControlItems : &m_viewControlAssignedItems;
          CGUIViewControl *viewControlTo = iControl == CONTROL_LIST ? &m_viewControlAssignedItems : &m_viewControlItems;
          int iSelected = viewControlFrom->GetSelectedItem();
          if (iSelected >= 0 && iSelected < (int)itemsFrom->Size())
          {
            // get the selected item
            CFileItemPtr item(itemsFrom->Get(iSelected));
            // add it to the other list
            itemsTo->Add(item);
            // remove it from this list
            itemsFrom->Remove(iSelected);

            // resort the lists
            m_items->Sort(m_sorting);
            m_assignedItems->Sort(m_sorting);

            // update the view controls with the new items
            m_viewControlItems.SetItems(*m_items);
            m_viewControlAssignedItems.SetItems(*m_assignedItems);
            
            // try to select the item based on its path
            if (!item->GetPath().empty() && itemsTo->Get(item->GetPath()) != NULL)
              viewControlTo->SetSelectedItem(item->GetPath());
            else
            {
              // try to select the item based on its label
              iSelected = -1;
              for (int index = 0; index < itemsTo->Size(); index++)
              {
                if (itemsTo->Get(index)->GetLabel().Equals(item->GetLabel(), false))
                {
                  iSelected = index;
                  break;
                }
              }

              if (iSelected >= 0)
                viewControlTo->SetSelectedItem(iSelected);
            }
          }
        }
      }
    }
    break;

    case GUI_MSG_SETFOCUS:
    {
      int iControl = message.GetControlId();
      if (m_viewControlItems.HasControl(iControl) && m_viewControlItems.GetCurrentControl() != iControl)
      {
        m_viewControlItems.SetFocused();
        return true;
      }
      else if (m_viewControlAssignedItems.HasControl(iControl) && m_viewControlAssignedItems.GetCurrentControl() != iControl)
      {
        m_viewControlAssignedItems.SetFocused();
        return true;
      }
    }
    break;
  }

  return CGUIDialog::OnMessage(message);
}

bool CGUIDialogMultiPropertyEditor::OnBack(int actionID)
{
  m_aborted = true;
  return CGUIDialog::OnBack(actionID);
}

void CGUIDialogMultiPropertyEditor::Reset()
{
  m_aborted = false;
  m_sorting = SortDescription();
  m_sorting.sortBy = SortByLabel;

  m_items->Clear();
  m_assignedItems->Clear();
}

void CGUIDialogMultiPropertyEditor::SetItems(const CFileItemList &items)
{
  m_items->Clear();
  m_items->Copy(items);
}

void CGUIDialogMultiPropertyEditor::SetAssignedItems(const CFileItemList &items)
{
  m_assignedItems->Clear();
  m_assignedItems->Copy(items);
}

void CGUIDialogMultiPropertyEditor::AddItem(const std::string &item)
{
  CFileItemPtr pItem(new CFileItem(item));
  m_items->Add(pItem);
}

void CGUIDialogMultiPropertyEditor::AddItem(const CFileItem *item)
{
  if (item == NULL)
    return;

  CFileItemPtr pItem(new CFileItem(*item));
  m_items->Add(pItem);
}

void CGUIDialogMultiPropertyEditor::AddAssignedItem(const std::string &item)
{
  CFileItemPtr pItem(new CFileItem(item));
  m_assignedItems->Add(pItem);
}

void CGUIDialogMultiPropertyEditor::AddAssignedItem(const CFileItem *item)
{
  if (item == NULL)
    return;

  CFileItemPtr pItem(new CFileItem(*item));
  m_assignedItems->Add(pItem);
}

void CGUIDialogMultiPropertyEditor::SetSorting(const SortDescription &sorting)
{
  m_sorting = sorting;
}

const CFileItemList& CGUIDialogMultiPropertyEditor::GetAssignedItems() const
{
  if (m_aborted)
    return EmtpyItemList;

  return *m_assignedItems;
}

CGUIControl* CGUIDialogMultiPropertyEditor::GetFirstFocusableControl(int id)
{
  if (m_viewControlItems.HasControl(id))
    id = m_viewControlItems.GetCurrentControl();
  else if (m_viewControlAssignedItems.HasControl(id))
    id = m_viewControlAssignedItems.GetCurrentControl();

  return CGUIDialogBoxBase::GetFirstFocusableControl(id);
}

void CGUIDialogMultiPropertyEditor::OnWindowLoaded()
{
  CGUIDialogBoxBase::OnWindowLoaded();
  m_viewControlItems.Reset();
  m_viewControlItems.SetParentWindow(GetID());
  m_viewControlItems.AddView(GetControl(CONTROL_LIST));
  m_viewControlAssignedItems.Reset();
  m_viewControlAssignedItems.SetParentWindow(GetID());
  m_viewControlAssignedItems.AddView(GetControl(CONTROL_LIST_ASSIGNED));
}

void CGUIDialogMultiPropertyEditor::OnInitWindow()
{
  m_viewControlItems.SetItems(*m_items);
  m_viewControlItems.SetCurrentView(CONTROL_LIST);
  m_viewControlAssignedItems.SetItems(*m_assignedItems);
  m_viewControlAssignedItems.SetCurrentView(CONTROL_LIST_ASSIGNED);

  SetupButton();
  CGUIDialogBoxBase::OnInitWindow();

  m_viewControlItems.SetSelectedItem(0);
  m_viewControlAssignedItems.SetSelectedItem(0);
}

void CGUIDialogMultiPropertyEditor::OnWindowUnload()
{
  CGUIDialog::OnWindowUnload();
  m_viewControlItems.Reset();
  m_viewControlAssignedItems.Reset();
}

void CGUIDialogMultiPropertyEditor::SetupButton()
{
  SET_CONTROL_LABEL(CONTROL_BUTTON, g_localizeStrings.Get(186));
}
