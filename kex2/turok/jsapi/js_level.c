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
// DESCRIPTION: Javascript Level Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "level.h"

JS_CLASSOBJECT(Level);

JS_PROP_FUNC_GET(Level)
{
    switch(JSVAL_TO_INT(id))
    {
    case 0:
        //return JS_NewNumberValue(cx, gLevel.mapID, vp);
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(gLevel.mapID));
        return JS_TRUE;

    case 1:
        JS_RETURNSTRING(vp, gLevel.name);
        return JS_TRUE;

    case 2:
        //return JS_NewNumberValue(cx, gLevel.tics, vp);
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(gLevel.tics));
        return JS_TRUE;

    case 3:
        return JS_NewDoubleValue(cx, gLevel.time, vp);

    case 4:
        JS_RETURNBOOLEAN(vp, gLevel.loaded);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(Level)
{
    switch(JSVAL_TO_INT(id))
    {
    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Level, findPlane)
{
    vec3_t vector;
    JSObject *obj;
    plane_t *plane;

    JS_CHECKARGS(1);
    JS_GETOBJECT(obj, v, 0);
    JS_GETVECTOR2(obj, vector);

    plane = Map_FindClosestPlane(vector);

    if(plane != NULL)
    {
        JS_NEWOBJECTPOOL(plane, Plane);
        //JS_INSTPLANE(vp, plane);
    }
    else
    {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Level, getActor)
{
    unsigned int index;

    JS_CHECKARGS(1);
    JS_GETINTEGER(index, 0);

    if(index >= gLevel.numActors)
    {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }

    JS_NEWOBJECTPOOL(&gLevel.gActors[index], GameActor);
    //JS_NEWOBJECT_SETPRIVATE(&gLevel.gActors[index], &GameActor_class);
    return JS_TRUE;
}

JS_BEGINCLASS(Level)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Level_getProperty,                          // getProperty
    Level_setProperty,                          // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Level)
{
    { "mapID",  0,  JSPROP_ENUMERATE, NULL, NULL },
    { "name",   1,  JSPROP_ENUMERATE, NULL, NULL },
    { "tics",   2,  JSPROP_ENUMERATE, NULL, NULL },
    { "time",   3,  JSPROP_ENUMERATE, NULL, NULL },
    { "loaded", 4,  JSPROP_ENUMERATE, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Level)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Level)
{
    JS_FASTNATIVE(Level, findPlane, 1),
    JS_FASTNATIVE(Level, getActor, 1),
    JS_FS_END
};
