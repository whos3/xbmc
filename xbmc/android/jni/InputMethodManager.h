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

#include "JNIBase.h"

class CJNIInputMethodManager : public CJNIBase
{
public:
  CJNIInputMethodManager(const jni::jhobject &object) : CJNIBase(object) { };
  ~CJNIInputMethodManager() { };

  
  
  static void PopulateStaticFields();

private:
  CJNIInputMethodManager();

  static int HIDE_IMPLICIT_ONLY;
  static int HIDE_NOT_ALWAYS;
  static int RESULT_HIDDEN;
  static int RESULT_SHOWN;
  static int RESULT_UNCHANGED_HIDDEN;
  static int RESULT_UNCHANGED_SHOWN;
  static int SHOW_FORCED;
  static int SHOW_IMPLICIT;
};
