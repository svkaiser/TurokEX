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
#include "mathlib.h"

JSObject *js_objClient;

enum client_enum
{
    CL_CMD,
    CL_ORIGIN,
    CL_VELOCITY,
    CL_YAW,
    CL_PITCH,
    CL_CENTER_Y,
    CL_VIEW_Y,
    CL_WIDTH,
    CL_HEIGHT,
    CL_MOVETYPE,
    CL_PLANE
};

//
// client_getProperty
//

static JSBool client_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch(JSVAL_TO_INT(id))
    {
    case CL_CMD:
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(js_objCmd));
        return JS_TRUE;

    case CL_ORIGIN:
        JS_NEWVECTOR(vp, client.moveframe.origin);
        return JS_TRUE;

    case CL_VELOCITY:
        JS_NEWVECTOR(vp, client.moveframe.velocity);
        return JS_TRUE;

    case CL_YAW:
        return JS_NewNumberValue(cx, client.moveframe.yaw, vp);

    case CL_PITCH:
        return JS_NewNumberValue(cx, client.moveframe.pitch, vp);

    case CL_CENTER_Y:
        return JS_NewNumberValue(cx, client.pmove.centerheight.f, vp);

    case CL_VIEW_Y:
        return JS_NewNumberValue(cx, client.pmove.viewheight.f, vp);

    case CL_WIDTH:
        return JS_NewNumberValue(cx, client.pmove.radius.f, vp);

    case CL_HEIGHT:
        return JS_NewNumberValue(cx, client.pmove.height.f, vp);

    case CL_MOVETYPE:
        return JS_NewNumberValue(cx, client.pmove.terraintype, vp);

    case CL_PLANE:
        JS_INSTPLANE(vp, client.moveframe.plane);
        return JS_TRUE;
    }

    return JS_FALSE;
}

//
// client_setProperty
//

static JSBool client_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch(JSVAL_TO_INT(id))
    {
    case CL_CMD:
        break;

    case CL_ORIGIN:
        {
            vec3_t *vector;
            JS_GETVECTOR(vector, vp, 0);
            client.pmove.origin[0].f = (*vector)[0];
            client.pmove.origin[1].f = (*vector)[1];
            client.pmove.origin[2].f = (*vector)[2];
            return JS_TRUE;
        }
        break;

    case CL_VELOCITY:
        {
            vec3_t *vector;
            JS_GETVECTOR(vector, vp, 0);
            client.pmove.velocity[0].f = (*vector)[0];
            client.pmove.velocity[1].f = (*vector)[1];
            client.pmove.velocity[2].f = (*vector)[2];
            return JS_TRUE;
        }
        break;

    case CL_YAW:
        break;

    case CL_PITCH:
        break;

    case CL_MOVETYPE:
        break;
    }

    return JS_FALSE;
}

//
// client_inLevel
//

static JSBool client_inLevel(JSContext *cx, uintN argc, jsval *rval)
{
    kbool b = (g_currentmap != NULL);

    JS_SET_RVAL(cx, rval, BOOLEAN_TO_JSVAL(b));
    return JS_TRUE;
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
    client_setProperty,                         // setProperty
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
    { "Cmd",        CL_CMD,         JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "origin",     CL_ORIGIN,      JSPROP_ENUMERATE,                   NULL, NULL },
    { "velocity",   CL_VELOCITY,    JSPROP_ENUMERATE,                   NULL, NULL },
    { "yaw",        CL_YAW,         JSPROP_ENUMERATE,                   NULL, NULL },
    { "pitch",      CL_PITCH,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "center_y",   CL_CENTER_Y,    JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "view_y",     CL_VIEW_Y,      JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "width",      CL_WIDTH,       JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "height",     CL_HEIGHT,      JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "movetype",   CL_MOVETYPE,    JSPROP_ENUMERATE,                   NULL, NULL },
    { "plane",      CL_PLANE,       JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// Client_functions
//

JSFunctionSpec Client_functions[] =
{
    JS_FN("inLevel",    client_inLevel,     0, 0, 0),
    JS_FS_END
};
