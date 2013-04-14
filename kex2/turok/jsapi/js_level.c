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
// DESCRIPTION: Javascript Level Classes
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "level.h"

// -----------------------------------------------
//
// LEVEL CLASS
//
// -----------------------------------------------

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
    unsigned int i = 0;

    JS_CHECKARGS(1);
    JS_GETINTEGER(index, 0);

    for(gLevel.actorRover = gLevel.actorRoot.next;
        gLevel.actorRover != &gLevel.actorRoot;
        gLevel.actorRover = gLevel.actorRover->next)
    {
        if(i == index)
        {
            JS_NEWOBJECT_SETPRIVATE(gLevel.actorRover, &GameActor_class);
            return JS_TRUE;
        }

        i++;
    }

    JS_SET_RVAL(cx, vp, JSVAL_NULL);
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

// -----------------------------------------------
//
// WORLDSTATE CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(WorldState);

JS_PROP_FUNC_GET(WorldState)
{
    worldState_t *ws;

    if(!(ws = (worldState_t*)JS_GetInstancePrivate(cx, obj, &WorldState_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        JS_NEWVECTOR2(ws->origin);
        return JS_TRUE;

    case 1:
        JS_NEWVECTOR2(ws->velocity);
        return JS_TRUE;

    case 2:
        JS_NEWVECTOR2(ws->forward);
        return JS_TRUE;

    case 3:
        JS_NEWVECTOR2(ws->right);
        return JS_TRUE;

    case 4:
        JS_NEWVECTOR2(ws->up);
        return JS_TRUE;

    case 5:
        JS_NEWVECTOR2(ws->accel);
        return JS_TRUE;

    case 6:
        return JS_NewDoubleValue(cx, ws->angles[0], vp);

    case 7:
        return JS_NewDoubleValue(cx, ws->angles[1], vp);

    case 8:
        return JS_NewDoubleValue(cx, ws->angles[2], vp);

    case 9:
        return JS_NewDoubleValue(cx, ws->moveTime, vp);

    case 10:
        return JS_NewDoubleValue(cx, ws->frameTime, vp);

    case 11:
        return JS_NewDoubleValue(cx, ws->timeStamp, vp);

    case 12:
        if(ws->plane == NULL)
            JS_SET_RVAL(cx, vp, JSVAL_NULL);
        else
            JS_NEWOBJECTPOOL(ws->plane, Plane);
        return JS_TRUE;

    case 13:
        JS_NEWOBJECTPOOL(ws->actor, GameActor);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(WorldState)
{
    worldState_t *ws;
    jsdouble dval;
    JSObject *object;

    if(!(ws = (worldState_t*)JS_GetInstancePrivate(cx, obj, &WorldState_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ws->origin);
        return JS_TRUE;

    case 1:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ws->velocity);
        return JS_TRUE;

    case 2:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ws->forward);
        return JS_TRUE;

    case 3:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ws->right);
        return JS_TRUE;

    case 4:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ws->up);
        return JS_TRUE;

    case 5:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ws->accel);
        return JS_TRUE;

    case 6:
        JS_GETNUMBER(dval, vp, 0);
        ws->angles[0] = (float)dval;
        return JS_TRUE;

    case 7:
        JS_GETNUMBER(dval, vp, 0);
        ws->angles[1] = (float)dval;
        return JS_TRUE;

    case 8:
        JS_GETNUMBER(dval, vp, 0);
        ws->angles[2] = (float)dval;
        return JS_TRUE;

    case 9:
        JS_GETNUMBER(dval, vp, 0);
        ws->moveTime = (float)dval;
        return JS_TRUE;

    case 10:
        JS_GETNUMBER(dval, vp, 0);
        ws->frameTime = (float)dval;
        return JS_TRUE;

    case 11:
        JS_GETNUMBER(dval, vp, 0);
        ws->timeStamp = (float)dval;
        return JS_TRUE;

    case 12:
        JS_GETOBJECT(object, vp, 0);
        JS_GET_PRIVATE_DATA(object, &Plane_class, plane_t, ws->plane);
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_BEGINCLASS(WorldState)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    WorldState_getProperty,                     // getProperty
    WorldState_setProperty,                     // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(WorldState)
{
    { "origin",     0,  JSPROP_ENUMERATE, NULL, NULL },
    { "velocity",   1,  JSPROP_ENUMERATE, NULL, NULL },
    { "forward",    2,  JSPROP_ENUMERATE, NULL, NULL },
    { "right",      3,  JSPROP_ENUMERATE, NULL, NULL },
    { "up",         4,  JSPROP_ENUMERATE, NULL, NULL },
    { "accel",      5,  JSPROP_ENUMERATE, NULL, NULL },
    { "yaw",        6,  JSPROP_ENUMERATE, NULL, NULL },
    { "pitch",      7,  JSPROP_ENUMERATE, NULL, NULL },
    { "roll",       8,  JSPROP_ENUMERATE, NULL, NULL },
    { "moveTime",   9,  JSPROP_ENUMERATE, NULL, NULL },
    { "frameTime",  10, JSPROP_ENUMERATE, NULL, NULL },
    { "timeStamp",  11, JSPROP_ENUMERATE, NULL, NULL },
    { "plane",      12, JSPROP_ENUMERATE, NULL, NULL },
    { "actor",      13, JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(WorldState)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(WorldState)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(WorldState)
{
    JS_FS_END
};
