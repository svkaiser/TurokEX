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

static JSBool ReturnTraceResults(JSContext *cx, jsval *vp, trace_t *trace)
{
    JSObject *rObject;
    JSObject *hvObject;
    JSObject *hnObject;
    JSObject *haObject;
    JSObject *hplObject;
    jsval fracval;
    jsval haval;

    if(trace->type == TRT_NOHIT)
    {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }

    trace->frac = 1 + trace->frac;

    if(!(rObject = J_NewObjectEx(cx, NULL, NULL, NULL)))
        return JS_TRUE;
    if(!(hplObject = J_NewObjectEx(cx, &Plane_class, NULL, NULL)))
        return JS_FALSE;
    if(!(JS_SetPrivate(cx, hplObject, trace->hitpl)))
        return JS_FALSE;
    if(!JS_NewDoubleValue(cx, trace->frac, &fracval))
        return JS_FALSE;
    if(!(hvObject = JPool_GetFree(&objPoolVector, &Vector_class)))
        return JS_FALSE;
    if(!(hnObject = JPool_GetFree(&objPoolVector, &Vector_class)))
        return JS_FALSE;

    JS_SETVECTOR(hvObject, trace->hitvec);
    JS_SETVECTOR(hnObject, trace->normal);

    JS_AddRoot(cx, &rObject);
    JS_DefineProperty(cx, rObject, "hitPlane",
        OBJECT_TO_JSVAL(hplObject), NULL, NULL, JSPROP_ENUMERATE);
    JS_DefineProperty(cx, rObject, "fraction", fracval, NULL, NULL, JSPROP_ENUMERATE);
    JS_DefineProperty(cx, rObject, "hitVector",
        OBJECT_TO_JSVAL(hvObject), NULL, NULL, JSPROP_ENUMERATE);
    JS_DefineProperty(cx, rObject, "hitNormal",
        OBJECT_TO_JSVAL(hnObject), NULL, NULL, JSPROP_ENUMERATE);
    JS_DefineProperty(cx, rObject, "hitType",
        INT_TO_JSVAL(trace->type), NULL, NULL, JSPROP_ENUMERATE);

    haval = JSVAL_NULL;
    if(trace->hitActor != NULL)
    {
        if((haObject = JPool_GetFree(&objPoolGameActor, &GameActor_class)) &&
            JS_SetPrivate(cx, haObject, trace->hitActor))
        {
            haval = OBJECT_TO_JSVAL(haObject);
        }
    }
    JS_DefineProperty(cx, rObject, "hitActor", haval,
        NULL, NULL, JSPROP_ENUMERATE);
    JS_RemoveRoot(cx, &rObject);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(rObject));
    return JS_TRUE;
}

JS_CLASSOBJECT(Physics);

JS_FASTNATIVE_BEGIN(Physics, move)
{
    vec3_t origin;
    vec3_t velocity;
    plane_t *plane;
    gActor_t *actor;
    jsdouble frametime;
    JSObject *objPlane;
    JSObject *objOrig;
    JSObject *objVel;
    JSObject *objActor;
    JSObject *objAngles;
    JSObject *ctrlObject;
    trace_t traceResult;
    JSBool getResults;
    JSBool hitOk;
    jsval *v;

    v = JS_ARGV(cx, vp);

    if(argc > 2 || argc <= 0)
        return JS_FALSE;

    plane = NULL;
    hitOk = JS_FALSE;
    getResults = JS_FALSE;

    JS_GETOBJECT(ctrlObject, v, 0);

    if(argc == 2)
        JS_GETBOOL(getResults, v, 1);

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

    hitOk = G_ClipMovement(origin, velocity, (float)frametime, &plane, actor, &traceResult);

    JS_SETVECTOR(objOrig, origin);
    JS_SETVECTOR(objVel, velocity);

    if(objPlane != NULL)
        JS_SetPrivate(cx, objPlane, plane);

    if(getResults == JS_TRUE)
        return ReturnTraceResults(cx, vp, &traceResult);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(hitOk));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Physics, rayTrace)
{
    jsval *v;
    JSObject *obj1;
    JSObject *obj2;
    JSObject *plObj;
    int steps;
    int length;
    int dist;
    vec3_t start;
    vec3_t forward;
    vec3_t end;
    gActor_t *actor;
    plane_t *plane;
    plane_t *pl;
    trace_t trace;

    v = JS_ARGV(cx, vp);

    if(argc < 5 || argc > 6)
        return JS_FALSE;

    JS_GETOBJECT(obj1, v, 0);
    JS_GETVECTOR2(obj1, start);
    JS_GETOBJECT(obj2, v, 1);
    JS_GETVECTOR2(obj2, forward);
    JS_GETINTEGER(steps, 2);
    JS_GETINTEGER(length, 3);
    JS_GETOBJECT(plObj, v, 4);
    JS_GET_PRIVATE_DATA(plObj, &Plane_class, plane_t, plane);

    actor = NULL;

    if(argc == 6 && !JSVAL_IS_NULL(v[5]))
    {
        JSObject *obj3;
        JS_GETOBJECT(obj3, v, 5);

        if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, obj3,
            &GameActor_class, NULL)))
            return JS_FALSE;
    }

    dist = 0;
    pl = plane;

    while(dist < (steps * length))
    {
        vec3_t fwd;

        dist += length;

        Vec_Copy3(fwd, forward);
        Vec_Scale(fwd, fwd, (float)length);
        Vec_Add(end, start, fwd);
        trace = Trace(start, end, pl, NULL, actor, true);
        pl = trace.pl;

        if(trace.type != TRT_NOHIT)
            break;

        Vec_Copy3(start, end);
    }

    return ReturnTraceResults(cx, vp, &trace);
}

JS_FASTNATIVE_BEGIN(Physics, tryMove)
{
    vec3_t origin;
    vec3_t dest;
    plane_t *plane;
    gActor_t *actor;
    JSObject *objOrg;
    JSObject *objDst;
    JSObject *objAct;
    JSObject *objPln;
    kbool moveok;

    JS_CHECKARGS(4);
    JS_GETOBJECT(objAct, v, 0);
    JS_GET_PRIVATE_DATA(objAct, &GameActor_class, gActor_t, actor);
    JS_GETOBJECT(objOrg, v, 1);
    JS_GETVECTOR2(objOrg, origin);
    JS_GETOBJECT(objDst, v, 2);
    JS_GETVECTOR2(objDst, dest);

    if(JSVAL_IS_NULL(v[3]))
    {
        JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(JS_FALSE));
        return JS_TRUE;
    }

    JS_GETOBJECT(objPln, v, 3);
    JS_GET_PRIVATE_DATA(objPln, &Plane_class, plane_t, plane);

    moveok = G_TryMove(actor, origin, dest, &plane);

    JS_SetPrivate(cx, objPln, plane);
    JS_SETVECTOR(objDst, dest);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(moveok));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Physics, checkPosition)
{
    vec3_t origin;
    vec3_t dest;
    plane_t *plane;
    gActor_t *actor;
    JSObject *objOrg;
    JSObject *objDst;
    JSObject *objAct;
    JSObject *objPln;
    trace_t trace;

    JS_CHECKARGS(4);
    JS_GETOBJECT(objAct, v, 0);
    JS_GET_PRIVATE_DATA(objAct, &GameActor_class, gActor_t, actor);
    JS_GETOBJECT(objOrg, v, 1);
    JS_GETVECTOR2(objOrg, origin);
    JS_GETOBJECT(objDst, v, 2);
    JS_GETVECTOR2(objDst, dest);

    if(JSVAL_IS_NULL(v[3]))
    {
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(TRT_STUCK));
        return JS_TRUE;
    }

    JS_GETOBJECT(objPln, v, 3);
    JS_GET_PRIVATE_DATA(objPln, &Plane_class, plane_t, plane);

    trace = Trace(origin, dest, plane, NULL, actor, false);

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(trace.type));
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
    JS_DEFINE_CONST(TRT_NOHIT,          TRT_NOHIT),
    JS_DEFINE_CONST(TRT_SURFACE,        TRT_SLOPE),
    JS_DEFINE_CONST(TRT_WALLSURFACE,    TRT_WALL),
    JS_DEFINE_CONST(TRT_WALL,           TRT_EDGE),
    JS_DEFINE_CONST(TRT_ACTOR,          TRT_OBJECT),
    JS_DEFINE_CONST(TRT_STUCK,          TRT_STUCK),
    JS_DEFINE_CONST(PT_NONE,            PT_NONE),
    JS_DEFINE_CONST(PT_DEFAULT,         PT_DEFAULT),
    JS_DEFINE_CONST(PT_FX,              PT_FX),
    JS_DEFINE_CONST(PT_SWIMMING,        PT_SWIMMING),
    JS_DEFINE_CONST(PT_CLIMBING,        PT_CLIMBING),
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Physics)
{
    JS_FASTNATIVE(Physics, move,  2),
    JS_FASTNATIVE(Physics, rayTrace,  6),
    JS_FASTNATIVE(Physics, tryMove, 4),
    JS_FASTNATIVE(Physics, checkPosition, 4),
    JS_FS_END
};
