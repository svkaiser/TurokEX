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
// DESCRIPTION: Javascript AI Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "actor.h"
#include "ai.h"

JS_CLASSOBJECT(AI);

JS_PROP_FUNC_GET(AI)
{
    ai_t *ai;

    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, obj, &AI_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        return JS_NewDoubleValue(cx, ai->turnSpeed, vp);

    case 1:
        return JS_NewDoubleValue(cx, ai->activeDistance, vp);

    case 2:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ai->flags));
        return JS_TRUE;

    case 3:
        JS_NEWVECTORPOOL(ai->goalOrigin);
        return JS_TRUE;

    case 4:
        return JS_NewDoubleValue(cx, ai->thinkTime, vp);

    case 5:
        return JS_NewDoubleValue(cx, ai->nextThinkTime, vp);

    case 6:
        if(ai->owner)
        {
            //JS_NEWOBJECT_SETPRIVATE(ai->owner, &GameActor_class);
            JS_NEWOBJECTPOOL(ai->owner, GameActor);
        }
        else
            JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;

    case 7:
        if(ai->target)
        {
            //JS_NEWOBJECT_SETPRIVATE(ai->target, &GameActor_class);
            JS_NEWOBJECTPOOL(ai->target, GameActor);
        }
        else
            JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;

    case 8:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_TURNING) != 0);
        return JS_TRUE;

    case 9:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_DORMANT) != 0);
        return JS_TRUE;

    case 10:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_SEETARGET) != 0);
        return JS_TRUE;

    case 11:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_SEEPLAYER) != 0);
        return JS_TRUE;

    case 12:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_FINDACTOR) != 0);
        return JS_TRUE;

    case 13:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_FINDPLAYERS) != 0);
        return JS_TRUE;

    case 14:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_AVOIDWALLS) != 0);
        return JS_TRUE;

    case 15:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_AVOIDACTORS) != 0);
        return JS_TRUE;

    case 16:
        return JS_NewDoubleValue(cx, ai->idealYaw, vp);

    case 17:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_DISABLED) != 0);
        return JS_TRUE;

    case 18:
        JS_RETURNBOOLEAN(vp, (ai->flags & AIF_LOOKATTARGET) != 0);
        return JS_TRUE;

    case 19:
        return JS_NewDoubleValue(cx, ai->maxHeadAngle, vp);

    case 20:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ai->nodeHead));
        return JS_TRUE;

    case 21:
        JS_NEWVECTOR2(ai->headYawAxis);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(AI)
{
    ai_t *ai;
    JSObject *object;
    jsdouble dval;
    JSBool ok;

    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, obj, &AI_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        JS_GETNUMBER(dval, vp, 0);
        ai->turnSpeed = (float)dval;
        return JS_TRUE;

    case 1:
        JS_GETNUMBER(dval, vp, 0);
        ai->activeDistance = (float)dval;
        return JS_TRUE;

    case 3:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ai->goalOrigin);
        return JS_TRUE;

    case 4:
        JS_GETNUMBER(dval, vp, 0);
        ai->thinkTime = (float)dval;
        return JS_TRUE;

    case 7:
        if(!JSVAL_IS_NULL(*vp))
        {
            gActor_t *pActor;

            JS_GETOBJECT(object, vp, 0);
            if(!(pActor = (gActor_t*)JS_GetInstancePrivate(cx, object,
                &GameActor_class, NULL)))
            {
                return JS_FALSE;
            }
            Actor_SetTarget(&ai->target, pActor);
            return JS_TRUE;
        }

        Actor_SetTarget(&ai->target, NULL);
        return JS_TRUE;

    case 8:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_TURNING;
        else if(ai->flags & AIF_TURNING)
            ai->flags &= ~AIF_TURNING;
        return JS_TRUE;

    case 9:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_DORMANT;
        else if(ai->flags & AIF_DORMANT)
            ai->flags &= ~AIF_DORMANT;
        return JS_TRUE;

    case 10:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_SEETARGET;
        else if(ai->flags & AIF_SEETARGET)
            ai->flags &= ~AIF_SEETARGET;
        return JS_TRUE;

    case 11:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_SEEPLAYER;
        else if(ai->flags & AIF_SEEPLAYER)
            ai->flags &= ~AIF_SEEPLAYER;
        return JS_TRUE;

    case 12:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_FINDACTOR;
        else if(ai->flags & AIF_FINDACTOR)
            ai->flags &= ~AIF_FINDACTOR;
        return JS_TRUE;

    case 13:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_FINDPLAYERS;
        else if(ai->flags & AIF_FINDPLAYERS)
            ai->flags &= ~AIF_FINDPLAYERS;
        return JS_TRUE;

    case 14:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_AVOIDWALLS;
        else if(ai->flags & AIF_AVOIDWALLS)
            ai->flags &= ~AIF_AVOIDWALLS;
        return JS_TRUE;

    case 15:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_AVOIDACTORS;
        else if(ai->flags & AIF_AVOIDACTORS)
            ai->flags &= ~AIF_AVOIDACTORS;
        return JS_TRUE;

    case 17:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_DISABLED;
        else if(ai->flags & AIF_DISABLED)
            ai->flags &= ~AIF_DISABLED;
        return JS_TRUE;

    case 18:
        JS_GETBOOL(ok, vp, 0);
        if(ok) ai->flags |= AIF_LOOKATTARGET;
        else if(ai->flags & AIF_LOOKATTARGET)
            ai->flags &= ~AIF_LOOKATTARGET;
        return JS_TRUE;

    case 19:
        JS_GETNUMBER(dval, vp, 0);
        ai->maxHeadAngle = (float)dval;
        return JS_TRUE;

    case 20:
        ai->nodeHead = JSVAL_TO_INT(*vp);
        return JS_TRUE;

    case 21:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ai->headYawAxis);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AI, yawToTarget)
{
    jsval *v = JS_ARGV(cx, vp);
    JSObject *thisObj;
    float angle;
    ai_t *ai;
    gActor_t *target;

    if(argc > 1)
        JS_WARNING();

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    if(argc == 1)
    {
        JSObject *obj;
        JS_GETOBJECT(obj, v, 0);
        JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, target);
    }
    else
        target = ai->target;

    angle = 0.0f;

    if(target != NULL)
        angle = AI_GetYawToTarget(ai, target);

    return JS_NewDoubleValue(cx, angle, vp);
}

JS_FASTNATIVE_BEGIN(AI, getTargetDistance)
{
    jsval *v = JS_ARGV(cx, vp);
    JSObject *thisObj;
    float dist;
    ai_t *ai;
    gActor_t *target;

    if(argc > 1)
        JS_WARNING();

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    if(argc == 1)
    {
        JSObject *obj;
        JS_GETOBJECT(obj, v, 0);
        JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, target);
    }
    else
        target = ai->target;

    dist = 0.0f;

    if(target != NULL)
        dist = AI_GetTargetDistance(ai, target);

    return JS_NewDoubleValue(cx, dist, vp);
}

JS_FASTNATIVE_BEGIN(AI, checkPosition)
{
    jsval *v = JS_ARGV(cx, vp);
    JSObject *thisObj;
    ai_t *ai;
    jsdouble radius;
    jsdouble angle;
    vec3_t origin;

    if(argc > 3)
        JS_WARNING();

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    JS_GETNUMBER(radius, v, 0);
    JS_GETNUMBER(angle, v, 1);

    if(argc == 3)
    {
        JSObject *obj;

        JS_GETOBJECT(obj, vp, 2);
        JS_GETVECTOR2(obj, origin);
    }
    else
        Vec_Copy3(origin, ai->owner->origin);

    JS_RETURNBOOLEAN(vp, AI_CheckPosition(ai, origin,
        (float)radius, (float)angle));

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AI, setIdealYaw)
{
    jsval *v = JS_ARGV(cx, vp);
    JSObject *thisObj;
    ai_t *ai;
    jsdouble yaw;
    jsdouble speed;
    
    if(argc > 2)
        JS_WARNING();

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    JS_GETNUMBER(yaw, v, 0);

    speed = ai->turnSpeed;

    if(argc == 2)
    {
        JS_GETNUMBER(speed, v, 1);
    }

    AI_SetIdealYaw(ai, (float)yaw, (float)speed);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AI, canSeeTarget)
{
    jsval *v = JS_ARGV(cx, vp);
    JSObject *thisObj;
    ai_t *ai;
    gActor_t *target;

    if(argc > 1)
        JS_WARNING();

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    if(argc == 1)
    {
        JSObject *obj;
        JS_GETOBJECT(obj, v, 0);
        JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, target);
    }
    else
        target = ai->target;

    JS_RETURNBOOLEAN(vp, AI_CanSeeTarget(ai, target));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AI, getBestAngleToTarget)
{
    jsval *v = JS_ARGV(cx, vp);
    JSObject *thisObj;
    float angle;
    jsdouble radius;
    ai_t *ai;
    gActor_t *target;

    if(argc > 2)
        JS_WARNING();

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    JS_GETNUMBER(radius, v, 0);

    if(argc == 2)
    {
        JSObject *obj;
        JS_GETOBJECT(obj, v, 1);
        JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, target);
    }
    else
        target = ai->target;

    angle = 0.0f;

    if(target != NULL)
        angle = AI_FindBestAngleToTarget(ai, target, (float)radius);

    return JS_NewDoubleValue(cx, angle, vp);
}

JS_FASTNATIVE_BEGIN(AI, clearTarget)
{
    JSObject *thisObj;
    ai_t *ai;

    JS_CHECKARGS(0);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    AI_ClearTarget(ai);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AI, fireProjectile)
{
    JSObject *thisObj;
    ai_t *ai;
    jsdouble x, y, z;
    jsdouble maxangle;
    JSBool useLocal;
    JSString *str;
    char *bytes;

    JS_CHECKARGS(6);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(ai = (ai_t*)JS_GetInstancePrivate(cx, thisObj, &AI_class, NULL)))
        JS_WARNING();

    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);
    JS_GETNUMBER(z, v, 3);
    JS_GETNUMBER(maxangle, v, 4);
    JS_GETBOOL(useLocal, v, 5);

    AI_FireProjectile(ai, bytes, (float)x, (float)y, (float)z, (float)maxangle, useLocal);

    JS_free(cx, bytes);
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(AI)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    AI_getProperty,                             // getProperty
    AI_setProperty,                             // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(AI)
{
    { "turnSpeed",      0,  JSPROP_ENUMERATE, NULL, NULL },
    { "activeDistance", 1,  JSPROP_ENUMERATE, NULL, NULL },
    { "flags",          2,  JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { "goalOrigin",     3,  JSPROP_ENUMERATE, NULL, NULL },
    { "thinkTime",      4,  JSPROP_ENUMERATE, NULL, NULL },
    { "nextThinkTime",  5,  JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { "owner",          6,  JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { "target",         7,  JSPROP_ENUMERATE, NULL, NULL },
    { "bTurning",       8,  JSPROP_ENUMERATE, NULL, NULL },
    { "bDormant",       9,  JSPROP_ENUMERATE, NULL, NULL },
    { "bSeeTarget",     10, JSPROP_ENUMERATE, NULL, NULL },
    { "bSeePlayer",     11, JSPROP_ENUMERATE, NULL, NULL },
    { "bFindActor",     12, JSPROP_ENUMERATE, NULL, NULL },
    { "bFindPlayers",   13, JSPROP_ENUMERATE, NULL, NULL },
    { "bAvoidWalls",    14, JSPROP_ENUMERATE, NULL, NULL },
    { "bAvoidActors",   15, JSPROP_ENUMERATE, NULL, NULL },
    { "idealYaw",       16, JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { "bDisabled",      17, JSPROP_ENUMERATE, NULL, NULL },
    { "bLookAtTarget",  18, JSPROP_ENUMERATE, NULL, NULL },
    { "maxHeadAngle",   19, JSPROP_ENUMERATE, NULL, NULL },
    { "nodeHead",       20, JSPROP_ENUMERATE, NULL, NULL },
    { "headYawAxis",    21, JSPROP_ENUMERATE, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(AI)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(AI)
{
    JS_FASTNATIVE(AI, yawToTarget, 1),
    JS_FASTNATIVE(AI, getTargetDistance, 1),
    JS_FASTNATIVE(AI, checkPosition, 3),
    JS_FASTNATIVE(AI, setIdealYaw, 2),
    JS_FASTNATIVE(AI, canSeeTarget, 1),
    JS_FASTNATIVE(AI, getBestAngleToTarget, 2),
    JS_FASTNATIVE(AI, clearTarget, 0),
    JS_FASTNATIVE(AI, fireProjectile, 6),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(AI)
{
    JS_FS_END
};
