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
// DESCRIPTION: Javascript Client Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "kernel.h"
#include "client.h"

JSObject *js_objClient;

enum client_enum
{
    CL_CMD,
    CL_ORIGIN,
    CL_VELOCITY,
    CL_YAW,
    CL_PITCH,
    CL_MOVETYPE
};

//
// client_getProperty
//

static JSBool client_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    JSObject *nobj;

    switch(JSVAL_TO_INT(id))
    {
    case CL_CMD:
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(js_objCmd));
        return JS_TRUE;

    case CL_ORIGIN:
        if(!(nobj = JS_NewObject(cx, &Vector_class, NULL, NULL)))
            return JS_FALSE;

        if(!(JS_SetPrivate(cx, nobj, &client.moveframe.origin)))
            return JS_FALSE;

        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(nobj));
        return JS_TRUE;

    case CL_VELOCITY:
        if(!(nobj = JS_NewObject(cx, &Vector_class, NULL, NULL)))
            return JS_FALSE;

        if(!(JS_SetPrivate(cx, nobj, &client.moveframe.velocity)))
            return JS_FALSE;

        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(nobj));
        return JS_TRUE;

    case CL_YAW:
        return JS_NewNumberValue(cx, client.moveframe.yaw, vp);

    case CL_PITCH:
        return JS_NewNumberValue(cx, client.moveframe.pitch, vp);

    case CL_MOVETYPE:
        return JS_NewNumberValue(cx, client.pmove.terraintype, vp);
    }

    return JS_FALSE;
}

//
// system_class
//

JSClass Client_class =
{
    "Client",                                   // name
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    client_getProperty,                         // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// Client_props
//

JSPropertySpec Client_props[] =
{
    { "Cmd",        CL_CMD,         JSPROP_ENUMERATE,   NULL, NULL },
    { "origin",     CL_ORIGIN,      JSPROP_ENUMERATE,   NULL, NULL },
    { "velocity",   CL_VELOCITY,    JSPROP_ENUMERATE,   NULL, NULL },
    { "yaw",        CL_YAW,         JSPROP_ENUMERATE,   NULL, NULL },
    { "pitch",      CL_PITCH,       JSPROP_ENUMERATE,   NULL, NULL },
    { "movetype",   CL_MOVETYPE,    JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// Client_functions
//

JSFunctionSpec Client_functions[] =
{
    JS_FS_END
};
