// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------

#ifndef _JS_SHARED_H_
#define _JS_SHARED_H_

#include "jsapi.h"

#define JS_DEFINEOBJECT(name)                                           \
    js_obj ##name = J_AddObject(&name ## _class, name ## _functions,    \
    name ## _props, # name, js_context, js_gobject)

#define JS_INITCLASS(name, args)                                        \
    js_obj ##name = JS_InitClass(js_context, js_gobject, NULL,          \
    &name ## _class, name ## _construct, args, name ## _props,          \
    name ## _functions, NULL, NULL)

#define JS_EXTERNOBJECT(name)                                           \
    extern JSObject *js_obj ##name;                                     \
    extern JSClass name ## _class;                                      \
    extern JSFunctionSpec name ## _functions[];                         \
    extern JSPropertySpec name ## _props[]

#define JS_EXTERNCLASS(name)                                            \
    extern JSObject *js_obj ##name;                                     \
    extern JSClass name ## _class;                                      \
    extern JSFunctionSpec name ## _functions[];                         \
    extern JSPropertySpec name ## _props[];                             \
    JSBool name ## _construct(JSContext *cx, JSObject *obj, uintN argc, \
                        jsval *argv, jsval *rval)

JS_EXTERNOBJECT(sys);
JS_EXTERNCLASS(vector3);

#endif