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

#include "AndroidKeyboard.h"
#include "android/jni/Context.h"
#include "android/jni/InputMethodManager.h"

bool CAndroidKeyboard::ShowAndGetInput(char_callback_t pCallback, const std::string &initialString, std::string &typedString, const std::string &heading, bool bHiddenInput /* = false */)
{
  bool confirmed = false;
  m_pCharCallback = pCallback;

  CJNIInputMethodManager imm(CJNIContext::getSystemService("input_method"));

  /* TODO
  InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
  imm.showSoftInput(mUserNameEdit, InputMethodManager.SHOW_IMPLICIT);

  JNIEnv *env = NULL;
  AttachCurrentThread(&env);

  android_printf("CXBMCApp::ToggleSoftKeyboard(%s)", show ? "true" : "false");
  jobject oActivity = m_activity->clazz;

  // String oContextINPUT_METHOD_SERVICE = android.content.Context.INPUT_METHOD_SERVICE;
  jclass cContext = env->FindClass("android/content/Context");
  jfieldID fidContextINPUT_METHOD_SERVICE = env->GetStaticFieldID(cContext, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
  jobject oContextINPUT_METHOD_SERVICE = env->GetStaticObjectField(cContext, fidContextINPUT_METHOD_SERVICE);
  env->DeleteLocalRef(cContext);

  // android.view.inputmethod.InputMethodManager oInputMethodManager = activity.getSystemService(oContextINPUT_METHOD_SERVICE);
  jclass cActivity = env->GetObjectClass(oActivity);
  jmethodID midActivityGetSystemService = env->GetMethodID(cActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
  jobject oInputMethodManager = env->CallObjectMethod(oActivity, midActivityGetSystemService, oContextINPUT_METHOD_SERVICE);
  env->DeleteLocalRef(oContextINPUT_METHOD_SERVICE);

  // android.view.Window oWindow = activity.getWindow();
  jmethodID midActivityGetWindow = env->GetMethodID(cActivity, "getWindow", "()Landroid/view/Window;");
  jobject oWindow = env->CallObjectMethod(oActivity, midActivityGetWindow);
  env->DeleteLocalRef(cActivity);

  // android.view.View oDecorView = oWindow.getDecorView();
  jclass cWindow = env->GetObjectClass(oWindow);
  jmethodID midWindowGetDecorView = env->GetMethodID(cWindow, "getDecorView", "()Landroid/view/View;");
  jobject oDecorView = env->CallObjectMethod(oWindow, midWindowGetDecorView);
  env->DeleteLocalRef(oWindow);
  env->DeleteLocalRef(cWindow);

  jclass cInputMethodManager = env->GetObjectClass(oInputMethodManager);

  if (show)
  {
    // oInputMethodManager.showSoftInput(oDecorView, 0);
    jmethodID midInputMethodManagerShowSoftInput = env->GetMethodID(cInputMethodManager, "showSoftInput", "(Landroid/view/View;I)Z");
    env->CallBooleanMethod(oInputMethodManager, midInputMethodManagerShowSoftInput, oDecorView, (jint)0);
  }
  else
  {
    // android.os.IBinder oBinder = oDecorView.getWindowToken();
    jclass cView = env->GetObjectClass(oDecorView);
    jmethodID midViewGetWindowToken = env->GetMethodID(cView, "getWindowToken", "()Landroid/os/IBinder;");
    jobject oBinder = env->CallObjectMethod(oDecorView, midViewGetWindowToken);

    // oInputMethodManager.hideSoftInputFromWindow(oBinder, 0);
    jmethodID midInputMethodManagerHideSoftInputFromWindow = env->GetMethodID(cInputMethodManager, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
    env->CallBooleanMethod(oInputMethodManager, midInputMethodManagerHideSoftInputFromWindow, oBinder, (jint)0);
  }

  env->DeleteLocalRef(cInputMethodManager);
  env->DeleteLocalRef(oDecorView);
  env->DeleteLocalRef(oInputMethodManager);

  DetachCurrentThread();
  */

  return confirmed;
}

void CAndroidKeyboard::Cancel()
{
  // TODO
  m_bCanceled = true;
}

void CAndroidKeyboard::fireCallback(const std::string &str)
{
  if (m_pCharCallback != NULL)
    m_pCharCallback(this, str);
}
