#pragma once
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

#include <v8.h>

#define V8_STR(str)           v8::String::New(str)
#define V8_GETTER(method)     v8::Handle<v8::Value> method(v8::Local<v8::String> property, const v8::AccessorInfo& info)
#define V8_SETTER(method)     void method(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
#define V8_FUNCTION(method)   v8::Handle<v8::Value> method(const v8::Arguments& args)
