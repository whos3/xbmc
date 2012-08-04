/*
 *      Copyright (C) 2011 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "GUIOperations.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "GUIInfoManager.h"
#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "addons/AddonManager.h"
#include "settings/GUISettings.h"
#include "utils/Variant.h"

using namespace std;
using namespace API;
using namespace ADDON;

APIStatus CGUIOperations::GetProperties(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CVariant properties = CVariant(CVariant::VariantTypeObject);
  for (unsigned int index = 0; index < parameterObject["properties"].size(); index++)
  {
    CStdString propertyName = parameterObject["properties"][index].asString();
    CVariant property;
    APIStatus ret;
    if ((ret = GetPropertyValue(propertyName, property)) != APIStatusOK)
      return ret;

    properties[propertyName] = property;
  }

  result = properties;

  return APIStatusOK;
}

APIStatus CGUIOperations::ShowNotification(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  string image = parameterObject["image"].asString();
  string title = parameterObject["title"].asString();
  string message = parameterObject["message"].asString();
  unsigned int displaytime = (unsigned int)parameterObject["displaytime"].asUnsignedInteger();

  if (image.compare("info") == 0)
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, title, message, displaytime);
  else if (image.compare("warning") == 0)
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Warning, title, message, displaytime);
  else if (image.compare("error") == 0)
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, title, message, displaytime);
  else
    CGUIDialogKaiToast::QueueNotification(image, title, message, displaytime);

  return APIStatusOK;
}

APIStatus CGUIOperations::SetFullscreen(const std::string &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  if ((parameterObject["fullscreen"].isString() &&
       parameterObject["fullscreen"].asString().compare("toggle") == 0) ||
      (parameterObject["fullscreen"].isBoolean() &&
       parameterObject["fullscreen"].asBoolean() != g_application.IsFullScreen()))
    CApplicationMessenger::Get().SendAction(CAction(ACTION_SHOW_GUI));
  else if (!parameterObject["fullscreen"].isBoolean() && !parameterObject["fullscreen"].isString())
    return APIStatusInvalidParameters;

  return GetPropertyValue("fullscreen", result);
}

APIStatus CGUIOperations::GetPropertyValue(const std::string &property, CVariant &result)
{
  std::string prop;
  std::transform(property.begin(), property.end(), prop.begin(), ::tolower);

  if (prop.compare("currentwindow") == 0)
  {
    result["label"] = g_infoManager.GetLabel(g_infoManager.TranslateString("System.CurrentWindow"));
    result["id"] = g_windowManager.GetFocusedWindow();
  }
  else if (prop.compare("currentcontrol") == 0)
    result["label"] = g_infoManager.GetLabel(g_infoManager.TranslateString("System.CurrentControl"));
  else if (prop.compare("skin") == 0)
  {
    CStdString skinId = g_guiSettings.GetString("lookandfeel.skin");
    AddonPtr addon;
    CAddonMgr::Get().GetAddon(skinId, addon, ADDON_SKIN);

    result["id"] = skinId;
    if (addon.get())
      result["name"] = addon->Name();
  }
  else if (prop.compare("fullscreen") == 0)
    result = g_application.IsFullScreen();
  else
    return APIStatusInvalidParameters;

  return APIStatusOK;
}
