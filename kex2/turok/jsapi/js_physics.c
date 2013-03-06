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
// DESCRIPTION: Javascript Physics Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "game.h"

JS_CLASSOBJECT(Physics);

JS_FASTNATIVE_BEGIN(Physics, move)
{
    JSObject *objPlane;
    vec3_t origin;
    vec3_t velocity;
    plane_t *plane;
    gActor_t *actor;
    jsdouble yaw;
    JSObject *objOrig;
    JSObject *objVel;
    JSObject *objActor;

    JS_CHECKARGS(5);

    plane = NULL;

    JS_GETOBJECT(objOrig, v, 0);
    JS_GETOBJECT(objVel, v, 1);
    JS_GETVECTOR2(objOrig, origin);
    JS_GETVECTOR2(objVel, velocity);
    JS_GETOBJECT(objPlane, v, 2);
    //JS_GETNUMBER(center_y, v, 3);
    JS_GETOBJECT(objActor, v, 3);
    JS_GETNUMBER(yaw, v, 4);
    //JS_GETNUMBER(width, v, 5);

    if(objPlane)
        JS_GET_PRIVATE_DATA(objPlane, &Plane_class, plane_t, plane);

    if(plane == NULL)
    {
        if(!(plane = Map_FindClosestPlane(origin)))
            return JS_TRUE;
    }

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, objActor, &GameActor_class, NULL)))
        return JS_FALSE;

    G_ClipMovement(origin, velocity, &plane,
        actor, (float)yaw, NULL);

    Vec_Add(origin, origin, velocity);

    JS_SETVECTOR(objOrig, origin);
    JS_SETVECTOR(objVel, velocity);

    JS_SetPrivate(cx, objPlane, plane);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(Physics)
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
JS_ENDCLASS();

JS_BEGINPROPS(Physics)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Physics)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Physics)
{
    JS_FASTNATIVE(Physics, move,  5),
    JS_FS_END
};
