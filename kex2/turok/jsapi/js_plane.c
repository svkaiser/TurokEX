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
// DESCRIPTION: Javascript Plane Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"

extern JSObject *mapProperties[MAXMAPS];

JSObject *js_objPlane;

enum plane_enum
{
    PL_PT1,
    PL_PT2,
    PL_PT3,
    PL_HEIGHT1,
    PL_HEIGHT2,
    PL_HEIGHT3,
    PL_NORMAL,
    PL_LINK1,
    PL_LINK2,
    PL_LINK3,
    PL_FLAGS,
    PL_AREA
};

//
// plane_getProperty
//

static JSBool plane_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    plane_t *plane = NULL;

    if(!(plane = (plane_t*)JS_GetInstancePrivate(cx, obj, &Plane_class, NULL)))
        return JS_FALSE;

    switch(JSVAL_TO_INT(id))
    {
    case PL_PT1:
        JS_NEWVECTOR(vp, plane->points[0]);
        return JS_TRUE;

    case PL_PT2:
        JS_NEWVECTOR(vp, plane->points[1]);
        return JS_TRUE;

    case PL_PT3:
        JS_NEWVECTOR(vp, plane->points[2]);
        return JS_TRUE;

    case PL_HEIGHT1:
        return JS_NewNumberValue(cx, plane->height[0], vp);

    case PL_HEIGHT2:
        return JS_NewNumberValue(cx, plane->height[1], vp);

    case PL_HEIGHT3:
        return JS_NewNumberValue(cx, plane->height[2], vp);

    case PL_NORMAL:
        JS_NEWVECTOR(vp, plane->normal);
        return JS_TRUE;

    case PL_LINK1:
        JS_INSTPLANE(vp, plane->link[0]);
        return JS_TRUE;

    case PL_LINK2:
        JS_INSTPLANE(vp, plane->link[1]);
        return JS_TRUE;

    case PL_LINK3:
        JS_INSTPLANE(vp, plane->link[2]);
        return JS_TRUE;

    case PL_FLAGS:
        return JS_NewNumberValue(cx, plane->flags, vp);

    case PL_AREA:
        {
            int map;
            jsval val;

            if(g_currentmap == NULL)
                return JS_FALSE;

            map = (g_currentmap - kmaps);

            if(map < 0 || map >= MAXMAPS)
                return JS_FALSE;

            val = J_GetObjectElement(cx, mapProperties[map], plane->area_id);

            if(!JSVAL_IS_OBJECT(val))
                return JS_FALSE;

            *vp = val;
        }
        return JS_TRUE;
    }

    return JS_FALSE;
}

//
// plane_distance
//

static JSBool plane_distance(JSContext *cx, uintN argc, jsval *vp)
{
    plane_t *plane = NULL;
    jsval *v;
    vec3_t *vector;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_THISPLANE(plane, vp);
    JS_GETVECTOR(vector, v, 0);

    return JS_NewDoubleValue(cx, Plane_GetDistance(plane, *vector), vp);
}

//
// plane_getNormal
//

static JSBool plane_getNormal(JSContext *cx, uintN argc, jsval *vp)
{
    plane_t *plane = NULL;
    vec3_t normal;

    JS_THISPLANE(plane, vp);
    Plane_GetNormal(normal, plane);
    JS_NEWVECTOR(vp, normal);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// plane_isAWall
//

static JSBool plane_isAWall(JSContext *cx, uintN argc, jsval *vp)
{
    plane_t *plane = NULL;

    JS_THISPLANE(plane, vp);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(Plane_IsAWall(plane)));
    return JS_TRUE;
}

//
// plane_getYaw
//

static JSBool plane_getYaw(JSContext *cx, uintN argc, jsval *vp)
{
    plane_t *plane = NULL;

    JS_THISPLANE(plane, vp);
    return JS_NewDoubleValue(cx, Plane_GetYaw(plane), vp);
}

//
// Plane_class
//

JSClass Plane_class =
{
    "Plane",                                    // name
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    plane_getProperty,                          // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// Plane_props
//

JSPropertySpec Plane_props[] =
{
    { "pt1",        PL_PT1,     JSPROP_ENUMERATE,                   NULL, NULL },
    { "pt2",        PL_PT2,     JSPROP_ENUMERATE,                   NULL, NULL },
    { "pt3",        PL_PT3,     JSPROP_ENUMERATE,                   NULL, NULL },
    { "height1",    PL_HEIGHT1, JSPROP_ENUMERATE,                   NULL, NULL },
    { "height2",    PL_HEIGHT2, JSPROP_ENUMERATE,                   NULL, NULL },
    { "height3",    PL_HEIGHT3, JSPROP_ENUMERATE,                   NULL, NULL },
    { "normal",     PL_NORMAL,  JSPROP_ENUMERATE,                   NULL, NULL },
    { "link1",      PL_LINK1,   JSPROP_ENUMERATE,                   NULL, NULL },
    { "link2",      PL_LINK2,   JSPROP_ENUMERATE,                   NULL, NULL },
    { "link3",      PL_LINK3,   JSPROP_ENUMERATE,                   NULL, NULL },
    { "flags",      PL_FLAGS,   JSPROP_ENUMERATE,                   NULL, NULL },
    { "area",       PL_AREA,    JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// Plane_const
//

JSConstDoubleSpec Plane_const[] =
{
    { 0, 0, 0, { 0, 0, 0 } }
};

//
// Plane_functions
//

JSFunctionSpec Plane_functions[] =
{
    JS_FN("distance",   plane_distance,         1, 0, 0),
    JS_FN("getNormal",  plane_getNormal,        0, 0, 0),
    JS_FN("isAWall",    plane_isAWall,          0, 0, 0),
    JS_FN("getYaw",     plane_getYaw,           0, 0, 0),
    JS_FS_END
};
