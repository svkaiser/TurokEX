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
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "js_class.h"

classObject_t classObj;

//
// JClass_Find
//

static gObject_t *JClass_Find(const char *classname)
{
    jsval argv;
    jsval rval;
    gObject_t *object;

    argv = STRING_TO_JSVAL(JS_NewStringCopyZ(js_context, classname));

    if(!JS_CallFunctionValue(js_context, classObj.object,
        OBJECT_TO_JSVAL(classObj.findObj), 1, &argv, &rval))
        return NULL;
    if(!JSVAL_IS_OBJECT(rval))
        return NULL;
    if(!JS_ValueToObject(js_context, rval, &object))
        return NULL;

    return object;
}

//
// JClass_Create
//

static gObject_t *JClass_Create(gObject_t *proto)
{
    jsval argv;
    jsval rval;
    gObject_t *object;

    argv = OBJECT_TO_JSVAL(proto);

    // construct class object
    if(!JS_CallFunctionValue(js_context, classObj.object,
        OBJECT_TO_JSVAL(classObj.createObj), 1, &argv, &rval))
        return NULL;
    if(!JSVAL_IS_OBJECT(rval))
        return NULL;
    if(!JS_ValueToObject(js_context, rval, &object))
        return NULL;

    return object;
}

//
// JClass_InitObject
//

void JClass_InitObject(void)
{
    jsval val;

    if(!JS_GetProperty(js_context, js_gobject, "class", &val))
        common.Error("JS_InitClassObject: javascript object 'class' not found");
    if(!JS_ValueToObject(js_context, val, &classObj.object))
        common.Error("JS_InitClassObject: unable to get class object");
    if(!JS_GetProperty(js_context, classObj.object, "find", &val))
        common.Error("JS_InitClassObject: unable to get 'find' property");
    if(!JS_ValueToObject(js_context, val, &classObj.findObj))
        common.Error("JS_InitClassObject: failed to setup 'find' object");
    if(!JS_ObjectIsFunction(js_context, classObj.findObj))
        common.Error("JS_InitClassObject: 'find' object is not a function");
    if(!JS_GetProperty(js_context, classObj.object, "create", &val))
        common.Error("JS_InitClassObject: unable to get 'create' property");
    if(!JS_ValueToObject(js_context, val, &classObj.createObj))
        common.Error("JS_InitClassObject: failed to setup 'create' object");
    if(!JS_ObjectIsFunction(js_context, classObj.createObj))
        common.Error("JS_InitClassObject: 'create' object is not a function");

    classObj.find   = JClass_Find;
    classObj.create = JClass_Create;
}
