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
// DESCRIPTION: Javascript MapProperty Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "zone.h"

JSObject *js_objMapProperty;
JSObject *mapProperties[MAXMAPS];

//
// mapProp_add
//

static JSBool mapProp_add(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSObject *obj;
    jsdouble mapid;
    int id;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETOBJECT(obj, v, 0);
    JS_GETNUMBER(mapid, v, 1);

    if(!JS_IsArrayObject(cx, obj))
        return JS_FALSE;

    id = (int)mapid;

    if(id < 0 || id >= MAXMAPS)
        return JS_FALSE;

    mapProperties[id] = obj;
    JS_AddRoot(cx, &mapProperties[id]);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// MapProperty_class
//

JSClass MapProperty_class =
{
    "MapProperty",                              // name
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    JS_PropertyStub,                            // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// MapProperty_props
//

JSPropertySpec MapProperty_props[] =
{
    { NULL, 0, 0, NULL, NULL }
};

//
// MapProperty_const
//

JSConstDoubleSpec MapProperty_const[] =
{
    { 0, 0, 0, { 0, 0, 0 } }
};

//
// MapProperty_functions
//

JSFunctionSpec MapProperty_functions[] =
{
    JS_FN("add",    mapProp_add,   2, 0, 0),
    JS_FS_END
};
