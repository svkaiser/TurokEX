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
    vec3_t origin;
    vec3_t velocity;
    vec3_t velocity2;
    plane_t *plane;
    gActor_t *actor;
    jsdouble yaw;
    jsdouble frametime;
    JSObject *objPlane;
    JSObject *objOrig;
    JSObject *objVel;
    JSObject *objActor;
    JSObject *objAngles;
    JSObject *ctrlObject;

    JS_CHECKARGS(1);

    plane = NULL;

    JS_GETOBJECT(ctrlObject, v, 0);

    JS_GET_PROPERTY_OBJECT(ctrlObject, "origin", objOrig);
    JS_GET_PROPERTY_OBJECT(ctrlObject, "velocity", objVel);
    JS_GET_PROPERTY_OBJECT(ctrlObject, "plane", objPlane);
    JS_GET_PROPERTY_OBJECT(ctrlObject, "owner", objActor);
    JS_GET_PROPERTY_OBJECT(ctrlObject, "angles", objAngles);
    JS_GET_PROPERTY_NUMBER(ctrlObject, "frametime", frametime);

    JS_GETVECTOR2(objOrig, origin);
    JS_GETVECTOR2(objVel, velocity);

    if(objPlane)
        JS_GET_PRIVATE_DATA(objPlane, &Plane_class, plane_t, plane);

    if(plane == NULL)
    {
        if(!(plane = Map_FindClosestPlane(origin)))
            return JS_TRUE;
    }

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, objActor, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_GET_PROPERTY_NUMBER(objAngles, "yaw", yaw);

    Vec_Copy3(velocity2, velocity);
    Vec_Scale(velocity2, velocity2, (float)frametime);

    G_ClipMovement(origin, velocity2, &plane,
        actor, (float)yaw, NULL);

    Vec_Add(origin, origin, velocity2);

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
    JS_FASTNATIVE(Physics, move,  1),
    JS_FS_END
};
