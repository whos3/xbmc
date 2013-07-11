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

#include "InputMethodManager.h"
#include "jutils/jutils-details.hpp"

using namespace jni;

int CJNIInputMethodManager::HIDE_IMPLICIT_ONLY(0);
int CJNIInputMethodManager::HIDE_NOT_ALWAYS(0);
int CJNIInputMethodManager::RESULT_HIDDEN(0);
int CJNIInputMethodManager::RESULT_SHOWN(0);
int CJNIInputMethodManager::RESULT_UNCHANGED_HIDDEN(0);
int CJNIInputMethodManager::RESULT_UNCHANGED_SHOWN(0);
int CJNIInputMethodManager::SHOW_FORCED(0);
int CJNIInputMethodManager::SHOW_IMPLICIT(0);

#define CLASSNAME_INPUTMETHODMANAGER    "android/view/inputmethod/InputMethodManager"

void CJNIInputMethodManager::PopulateStaticFields()
{
  jhclass clazz = find_class(CLASSNAME_INPUTMETHODMANAGER);
  HIDE_IMPLICIT_ONLY      = (get_static_field<int>(clazz, "HIDE_IMPLICIT_ONLY"));
  HIDE_NOT_ALWAYS         = (get_static_field<int>(clazz, "HIDE_NOT_ALWAYS"));
  RESULT_HIDDEN           = (get_static_field<int>(clazz, "RESULT_HIDDEN"));
  RESULT_SHOWN            = (get_static_field<int>(clazz, "RESULT_SHOWN"));
  RESULT_UNCHANGED_HIDDEN = (get_static_field<int>(clazz, "RESULT_UNCHANGED_HIDDEN"));
  RESULT_UNCHANGED_SHOWN  = (get_static_field<int>(clazz, "RESULT_UNCHANGED_SHOWN"));
  SHOW_FORCED             = (get_static_field<int>(clazz, "SHOW_FORCED"));
  SHOW_IMPLICIT           = (get_static_field<int>(clazz, "SHOW_IMPLICIT"));
}


