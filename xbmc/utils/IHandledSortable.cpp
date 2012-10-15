/*
 *      Copyright (C) 2012 Team XBMC
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

#include "IHandledSortable.h"
#include "Variant.h"

void IHandledSortable::ToSortable(SortItem& sortable) const
{
  if (m_propertyHandler == NULL)
    return;
  
  std::set<Field> fields = m_propertyHandler->GetHandledPropertyFields();
  for (std::set<Field>::const_iterator field = fields.begin(); field != fields.end(); field++)
  {
    CVariant var;
    if (m_propertyHandler->GetHandledPropertyValue(*field, var))
      sortable[*field] = var;
  }
}
