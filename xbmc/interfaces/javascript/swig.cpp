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

#include "swig.h"

using namespace v8;

namespace JavaScriptBindings
{
  JavaScriptToCppException::JavaScriptToCppException(const v8::TryCatch &exception) : XbmcCommons::UncheckedException("")
  {
    setClassname("JavaScriptToCppException");

    HandleScope handle_scope;

    CStdString msg;
    msg = "-->JavaScript callback/script returned the following error<--\n";
    msg += " - NOTE: IGNORING THIS CAN LEAD TO MEMORY LEAKS!\n";

    if (exception.Exception().IsEmpty())
      msg = "<unknown exception type>";
    else
    {
      msg.AppendFormat("Error Type: %s\n", *String::Utf8Value(exception.Exception()));

      Handle<Message> message = exception.Message();
      if (!message.IsEmpty())
      {
        // provide the filename of the script that caused the error
        msg.AppendFormat("Script    : %s\n", *String::Utf8Value(message->GetScriptResourceName()));
        // provide the line number in the script that caused the error
        msg.AppendFormat("Line      : %d\n", message->GetLineNumber());
        // provide the actual code line that caused the error
        msg.AppendFormat("Code      : %s\n", *String::Utf8Value(message->GetSourceLine()));
        // provide an indication on where in the code line the error was caused
        msg +="            ";
        int column;
        for (column = 0; column < message->GetStartColumn(); column++)
          msg += " ";
        for (; column < message->GetEndColumn(); column++)
          msg += "^";
        msg += "\n";
        // provide a stack trace if available
        Handle<StackTrace> stackTrace = message->GetStackTrace();
        if (!stackTrace.IsEmpty())
        {
          msg += "StackTrace:\n";
          for (int i = 0; i < stackTrace->GetFrameCount(); i++)
          {
            Local<StackFrame> frame = stackTrace->GetFrame(i);
            if (frame.IsEmpty())
            {
              msg += "     at unknown location\n";
              continue;
            }

            if (frame->IsEval())
              msg += "eval ";
            else
              msg += "     ";

            msg += "at ";

            if (frame->IsConstructor())
              msg += "new ";

            msg.AppendFormat("%s (%s:%d:%d)\n", *String::Utf8Value(frame->GetFunctionName()), *String::Utf8Value(frame->GetScriptName()), frame->GetLineNumber(), frame->GetColumn());
          }
        }
      }
    }
    msg += "-->End of JavaScript script error report<--\n";

    SetMessage("%s", msg.c_str());
  }
}
