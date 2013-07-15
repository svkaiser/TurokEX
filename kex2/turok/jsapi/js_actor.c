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
// DESCRIPTION: Javascript Actor Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "actor.h"
#include "ai.h"
#include "zone.h"

JS_CLASSOBJECT(GameActor);

JS_PROP_FUNC_GET(GameActor)
{
    gActor_t *actor;

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, obj, &GameActor_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        JS_RETURNBOOLEAN(vp, actor->bOrientOnSlope);
        return JS_TRUE;

    case 1:
        JS_RETURNBOOLEAN(vp, actor->bStatic);
        return JS_TRUE;

    case 2:
        JS_RETURNBOOLEAN(vp, actor->bCollision);
        return JS_TRUE;

    case 3:
        JS_RETURNBOOLEAN(vp, actor->bTouch);
        return JS_TRUE;

    case 4:
        JS_RETURNBOOLEAN(vp, actor->bClientOnly);
        return JS_TRUE;

    case 5:
        JS_NEWVECTOR2(actor->origin);
        return JS_TRUE;

    case 6:
        JS_NEWVECTOR2(actor->scale);
        return JS_TRUE;

    case 7:
        JS_NEWQUATERNION(actor->rotation);
        return JS_TRUE;

    case 8:
        {
            JSObject *bbox;

            bbox = J_NewObjectEx(cx, NULL, NULL, NULL);
            JS_AddRoot(cx, &bbox);
            JS_DefineProperty(cx, bbox, "min_x",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->bbox.min[0])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, bbox, "min_y",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->bbox.min[1])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, bbox, "min_z",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->bbox.min[2])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, bbox, "max_x",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->bbox.max[0])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, bbox, "max_y",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->bbox.max[1])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, bbox, "max_z",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->bbox.max[2])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_RemoveRoot(cx, &bbox);

            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(bbox));
        }
        return JS_TRUE;

    case 9:
        {
            JSObject *angles;

            angles = J_NewObjectEx(cx, NULL, NULL, NULL);
            JS_AddRoot(cx, &angles);
            JS_DefineProperty(cx, angles, "yaw",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->angles[0])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, angles, "pitch",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->angles[1])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, angles, "roll",
                DOUBLE_TO_JSVAL(JS_NewDouble(cx, actor->angles[2])),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_RemoveRoot(cx, &angles);

            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(angles));
        }
        return JS_TRUE;

    case 10:
        JS_RETURNSTRING(vp, actor->name);
        return JS_TRUE;

    case 11:
        //return JS_NewNumberValue(cx, actor->plane, vp);
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(actor->plane));
        return JS_TRUE;

    case 12:
        return J_NewDoubleEx(cx, actor->radius, vp);

    case 13:
        return J_NewDoubleEx(cx, actor->height, vp);

    case 14:
        return J_NewDoubleEx(cx, actor->centerHeight, vp);

    case 15:
        return J_NewDoubleEx(cx, actor->viewHeight, vp);

    case 16:
        return JS_TRUE;

    case 17:
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(actor->components));
        return JS_TRUE;

    case 18:
        JS_RETURNBOOLEAN(vp, actor->bHidden);
        return JS_TRUE;

    case 19:
        return J_NewDoubleEx(cx, actor->angles[0], vp);

    case 20:
        return J_NewDoubleEx(cx, actor->angles[1], vp);

    case 21:
        return J_NewDoubleEx(cx, actor->angles[2], vp);

    case 22:
        {
            JSObject *vobj;
            
            if(!(vobj = JPool_GetFree(&objPoolAnimState, &AnimState_class)) ||
                !(JS_SetPrivate(cx, vobj, &actor->animState)))
                return JS_FALSE;
            
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(vobj));
            return JS_TRUE;
        }

    case 23:
        return J_NewDoubleEx(cx, actor->timestamp, vp);

    case 24:
        JS_NEWOBJECT_SETPRIVATE(actor->model, &Model_class);
        return JS_TRUE;

    case 25:
        if(actor->owner)
        {
            //JS_NEWOBJECT_SETPRIVATE(actor->owner, &GameActor_class);
            JS_NEWOBJECTPOOL(actor->owner, GameActor);
        }
        else
            JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;

    case 26:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(actor->targetID));
        return JS_TRUE;

    case 27:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(actor->variant));
        return JS_TRUE;

    case 28:
        JS_RETURNBOOLEAN(vp, actor->bNoDropOff);
        return JS_TRUE;

    case 29:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(actor->physics));
        return JS_TRUE;

    case 30:
        JS_NEWVECTORPOOL(actor->velocity);
        return JS_TRUE;

    case 31:
        if(actor->ai == NULL)
        {
            JS_SET_RVAL(cx, vp, JSVAL_NULL);
            return JS_TRUE;
        }
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(actor->ai->object));
        return JS_TRUE;

    case 32:
        return J_NewDoubleEx(cx, actor->mass, vp);

    case 33:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(actor->classFlags));
        return JS_TRUE;

    case 34:
        return J_NewDoubleEx(cx, actor->friction, vp);

    case 35:
        return J_NewDoubleEx(cx, actor->bounceDamp, vp);

    case 36:
        JS_RETURNBOOLEAN(vp, actor->bRotor);
        return JS_TRUE;

    case 37:
        return J_NewDoubleEx(cx, actor->rotorSpeed, vp);

    case 38:
        JS_NEWVECTORPOOL(actor->rotorVector);
        return JS_TRUE;

    case 39:
        return J_NewDoubleEx(cx, actor->rotorFriction, vp);

    case 40:
        return J_NewDoubleEx(cx, actor->baseHeight, vp);

    case 41:
        return J_NewDoubleEx(cx, actor->tickDistance, vp);

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(GameActor)
{
    gActor_t *actor;
    gActor_t *pActor;
    JSObject *object;
    jsdouble dval;

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, obj, &GameActor_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 2:
        JS_GETBOOL(actor->bCollision, vp, 0);
        return JS_TRUE;

    case 3:
        JS_GETBOOL(actor->bTouch, vp, 0);
        return JS_TRUE;

    case 5:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, actor->origin);
        return JS_TRUE;

    case 6:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, actor->scale);
        return JS_TRUE;

    case 7:
        JS_GETOBJECT(object, vp, 0);
        JS_GETQUATERNION2(object, actor->rotation);
        return JS_TRUE;

    case 11:
        actor->plane = JSVAL_TO_INT(*vp);
        return JS_TRUE;

    case 14:
        JS_GETNUMBER(dval, vp, 0);
        actor->centerHeight = (float)dval;
        return JS_TRUE;

    case 15:
        JS_GETNUMBER(dval, vp, 0);
        actor->viewHeight = (float)dval;
        return JS_TRUE;

    case 18:
        JS_GETBOOL(actor->bHidden, vp, 0);
        return JS_TRUE;

    case 19:
        JS_GETNUMBER(dval, vp, 0);
        actor->angles[0] = (float)dval;
        return JS_TRUE;

    case 20:
        JS_GETNUMBER(dval, vp, 0);
        actor->angles[1] = (float)dval;
        return JS_TRUE;

    case 21:
        JS_GETNUMBER(dval, vp, 0);
        actor->angles[2] = (float)dval;
        return JS_TRUE;

    case 25:
        if(!JSVAL_IS_NULL(*vp))
        {
            JS_GETOBJECT(object, vp, 0);
            if(!(pActor = (gActor_t*)JS_GetInstancePrivate(cx, object,
                &GameActor_class, NULL)))
            {
                return JS_FALSE;
            }
            Actor_SetTarget(&actor->owner, pActor);
            return JS_TRUE;
        }

        Actor_SetTarget(&actor->owner, NULL);
        return JS_TRUE;

    case 26:
        actor->targetID = JSVAL_TO_INT(*vp);
        return JS_TRUE;

    case 27:
        actor->variant = JSVAL_TO_INT(*vp);
        return JS_TRUE;

    case 28:
        JS_GETBOOL(actor->bNoDropOff, vp, 0);
        return JS_TRUE;

    case 29:
        actor->physics = JSVAL_TO_INT(*vp);
        return JS_TRUE;

    case 30:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, actor->velocity);
        return JS_TRUE;

    case 32:
        JS_GETNUMBER(dval, vp, 0);
        actor->mass = (float)dval;
        return JS_TRUE;

    case 34:
        JS_GETNUMBER(dval, vp, 0);
        actor->friction = (float)dval;
        return JS_TRUE;

    case 35:
        JS_GETNUMBER(dval, vp, 0);
        actor->bounceDamp = (float)dval;
        return JS_TRUE;

    case 36:
        JS_GETBOOL(actor->bRotor, vp, 0);
        return JS_TRUE;

    case 37:
        JS_GETNUMBER(dval, vp, 0);
        actor->rotorSpeed = (float)dval;
        return JS_TRUE;

    case 38:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, actor->rotorVector);
        return JS_TRUE;

    case 39:
        JS_GETNUMBER(dval, vp, 0);
        actor->rotorFriction = (float)dval;
        return JS_TRUE;

    case 40:
        JS_GETNUMBER(dval, vp, 0);
        actor->baseHeight = (float)dval;
        return JS_TRUE;

    case 41:
        JS_GETNUMBER(dval, vp, 0);
        actor->tickDistance = (float)dval;
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, updateTransform)
{
    JSObject *thisObj;
    gActor_t *actor;

    JS_CHECKARGS(0);
    thisObj = JS_THIS_OBJECT(cx, vp);

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    Actor_UpdateTransform(actor);

    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, setBounds)
{
    JSObject *thisObj;
    gActor_t *actor;
    jsdouble b[6];

    JS_CHECKARGS(6);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_GETNUMBER(b[0], v, 0);
    JS_GETNUMBER(b[1], v, 1);
    JS_GETNUMBER(b[2], v, 2);
    JS_GETNUMBER(b[3], v, 3);
    JS_GETNUMBER(b[4], v, 4);
    JS_GETNUMBER(b[5], v, 5);

    Vec_Set3(actor->bbox.min, (float)b[0], (float)b[1], (float)b[2]);
    Vec_Set3(actor->bbox.max, (float)b[3], (float)b[4], (float)b[5]);

    Vec_Copy3(actor->bbox.omin, actor->bbox.min);
    Vec_Copy3(actor->bbox.omax, actor->bbox.max);

    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, setAnim)
{
    JSObject *thisObj;
    gActor_t *actor;
    jsdouble speed;
    anim_t *anim;

    JS_CHECKARGS(3);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    anim = NULL;

    if(!JSVAL_IS_STRING(v[0]))
    {
        int id;

        JS_GETINTEGER(id, 0);
        anim = Mdl_GetAnimFromID(actor->model, id);
    }
    else
    {
        JSString *str;
        char *bytes;

        JS_GETSTRING(str, bytes, v, 0);
        anim = Mdl_GetAnim(actor->model, bytes);
        JS_free(cx, bytes);
    }

    if(anim == NULL)
        JS_SAFERETURN();

    JS_GETNUMBER(speed, v, 1);
    JS_CHECKINTEGER(2);

    Mdl_SetAnimState(&actor->animState, anim,
        (float)speed, JSVAL_TO_INT(JS_ARG(2)));

    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, blendAnim)
{
    JSObject *thisObj;
    gActor_t *actor;
    jsdouble speed;
    jsdouble blendtime;
    anim_t *anim;

    JS_CHECKARGS(4);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    anim = NULL;

    if(!JSVAL_IS_STRING(v[0]))
    {
        int id;

        JS_GETINTEGER(id, 0);
        anim = Mdl_GetAnimFromID(actor->model, id);
    }
    else
    {
        JSString *str;
        char *bytes;

        JS_GETSTRING(str, bytes, v, 0);
        anim = Mdl_GetAnim(actor->model, bytes);
        JS_free(cx, bytes);
    }

    if(anim == NULL)
        JS_SAFERETURN();

    JS_GETNUMBER(speed, v, 1);
    JS_GETNUMBER(blendtime, v, 2);
    JS_CHECKINTEGER(3);

    Mdl_BlendAnimStates(&actor->animState, anim,
        (float)speed, (float)blendtime, JSVAL_TO_INT(JS_ARG(3)));

    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, checkAnimID)
{
    JSObject *thisObj;
    gActor_t *actor;

    JS_CHECKARGS(1);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_RETURNBOOLEAN(vp, Mdl_CheckAnimID(actor->model, JSVAL_TO_INT(JS_ARG(0))));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, spawnFX)
{
    JSObject *thisObj;
    gActor_t *actor;
    jsdouble x, y, z;
    JSString *str;
    char *bytes;

    JS_CHECKARGS(4);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);
    JS_GETNUMBER(z, v, 3);

    Actor_SpawnBodyFX(actor, bytes, (float)x, (float)y, (float)z);
    JS_free(cx, bytes);

    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, getLocalVector)
{
    JSObject *thisObj;
    gActor_t *actor;
    jsdouble x, y, z;
    vec3_t org;

    JS_CHECKARGS(3);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_GETNUMBER(x, v, 0);
    JS_GETNUMBER(y, v, 1);
    JS_GETNUMBER(z, v, 2);

    Actor_GetLocalVectors(org, actor, (float)x, (float)y, (float)z);

    JS_NEWVECTORPOOL(org);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, setNodeRotation)
{
    JSObject *thisObj;
    JSObject *obj;
    vec4_t rot;
    int node;
    gActor_t *actor;
    kmodel_t *model;

    JS_CHECKARGS(2);
    
    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    model = actor->model;

    JS_GETINTEGER(node, 0);
    JS_GETOBJECT(obj, v, 1);
    JS_GETQUATERNION2(obj, rot);

    if(node >= 0 && node < (int)model->numnodes)
        Vec_Copy4(actor->nodeOffsets_r[node], rot);

    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, onGround)
{
    JSObject *thisObj;
    gActor_t *actor;

    JS_CHECKARGS(0);
    
    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(Actor_OnGround(actor)));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, setTexture)
{
    JSObject *thisObj;
    gActor_t *actor;
    unsigned int node;
    unsigned int mesh;
    unsigned int section;
    kmodel_t *model;
    char **meshTexture;
    JSString *str;
    char *bytes;

    JS_CHECKARGS(4);
    
    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    if(!actor->model || !actor->textureSwaps)
        JS_SAFERETURN();

    JS_GETINTEGER(node, 0);
    JS_GETINTEGER(mesh, 1);
    JS_GETINTEGER(section, 2);

    model = actor->model;

    if(node >= model->numnodes)
        JS_SAFERETURN();
    if(mesh >= model->nodes[node].nummeshes)
        JS_SAFERETURN();
    if(section >= model->nodes[node].meshes[mesh].numsections)
        JS_SAFERETURN();

    JS_GETSTRING(str, bytes, v, 3);

    meshTexture = &actor->textureSwaps[node][mesh][section];

    if(*meshTexture)
        Z_Free(*meshTexture);

    *meshTexture = Z_Strdup(bytes, PU_ACTOR, NULL);

    JS_free(cx, bytes);
    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, spawn)
{
    JSString *str;
    char *bytes;
    jsdouble x;
    jsdouble y;
    jsdouble z;
    jsdouble yaw;
    jsdouble pitch;
    plane_t *plane;
    gActor_t *actor;

    JS_CHECKARGS(7);
    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);
    JS_GETNUMBER(z, v, 3);
    JS_GETNUMBER(yaw, v, 4);
    JS_GETNUMBER(pitch, v, 5);
    
    if(!JSVAL_IS_NULL(v[6]))
    {
        JSObject *plObj;

        JS_GETOBJECT(plObj, v, 6);
        JS_GET_PRIVATE_DATA(plObj, &Plane_class, plane_t, plane);
    }
    else
        plane = NULL;

    actor = Actor_Spawn(bytes,
        (float)x,
        (float)y,
        (float)z,
        (float)yaw,
        (float)pitch,
        plane == NULL ? -1 : plane - gLevel.planes);

    JS_free(cx, bytes);

    JS_NEWOBJECT_SETPRIVATE(actor, &GameActor_class);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, remove)
{
    JSObject *obj;
    gActor_t *actor;

    JS_CHECKARGS(1);
    JS_GETOBJECT(obj, v, 0);
    JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, actor);

    actor->bStale = true;

    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(GameActor, compare)
{
    JSObject *obj1;
    JSObject *obj2;
    gActor_t *actor1;
    gActor_t *actor2;
    JSBool ok;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj1, v, 0);
    JS_GET_PRIVATE_DATA(obj1, &GameActor_class, gActor_t, actor1);
    JS_GETOBJECT(obj2, v, 1);
    JS_GET_PRIVATE_DATA(obj2, &GameActor_class, gActor_t, actor2);

    ok = (actor1 == actor2);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(ok));
    return JS_TRUE;
}

JS_BEGINCLASS(GameActor)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    GameActor_getProperty,                      // getProperty
    GameActor_setProperty,                      // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(GameActor)
{
    { "bOrientOnSlope", 0,  JSPROP_ENUMERATE, NULL, NULL },
    { "bStatic",        1,  JSPROP_ENUMERATE, NULL, NULL },
    { "bCollision",     2,  JSPROP_ENUMERATE, NULL, NULL },
    { "bTouch",         3,  JSPROP_ENUMERATE, NULL, NULL },
    { "bClientOnly",    4,  JSPROP_ENUMERATE, NULL, NULL },
    { "origin",         5,  JSPROP_ENUMERATE, NULL, NULL },
    { "scale",          6,  JSPROP_ENUMERATE, NULL, NULL },
    { "rotation",       7,  JSPROP_ENUMERATE, NULL, NULL },
    { "bbox",           8,  JSPROP_ENUMERATE, NULL, NULL },
    { "angles",         9,  JSPROP_ENUMERATE, NULL, NULL },
    { "name",           10, JSPROP_ENUMERATE, NULL, NULL },
    { "plane",          11, JSPROP_ENUMERATE, NULL, NULL },
    { "radius",         12, JSPROP_ENUMERATE, NULL, NULL },
    { "height",         13, JSPROP_ENUMERATE, NULL, NULL },
    { "centerHeight",   14, JSPROP_ENUMERATE, NULL, NULL },
    { "viewHeight",     15, JSPROP_ENUMERATE, NULL, NULL },
    { "matrix",         16, JSPROP_ENUMERATE, NULL, NULL },
    { "components",     17, JSPROP_ENUMERATE, NULL, NULL },
    { "bHidden",        18, JSPROP_ENUMERATE, NULL, NULL },
    { "yaw",            19, JSPROP_ENUMERATE, NULL, NULL },
    { "pitch",          20, JSPROP_ENUMERATE, NULL, NULL },
    { "roll",           21, JSPROP_ENUMERATE, NULL, NULL },
    { "animState",      22, JSPROP_ENUMERATE, NULL, NULL },
    { "timeStamp",      23, JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { "model",          24, JSPROP_ENUMERATE, NULL, NULL },
    { "owner",          25, JSPROP_ENUMERATE, NULL, NULL },
    { "targetID",       26, JSPROP_ENUMERATE, NULL, NULL },
    { "modelVariant",   27, JSPROP_ENUMERATE, NULL, NULL },
    { "bNoDropOff",     28, JSPROP_ENUMERATE, NULL, NULL },
    { "physics",        29, JSPROP_ENUMERATE, NULL, NULL },
    { "velocity",       30, JSPROP_ENUMERATE, NULL, NULL },
    { "ai",             31, JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { "mass",           32, JSPROP_ENUMERATE, NULL, NULL },
    { "classFlags",     33, JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { "friction",       34, JSPROP_ENUMERATE, NULL, NULL },
    { "bounceDamp",     35, JSPROP_ENUMERATE, NULL, NULL },
    { "bRotor",         36, JSPROP_ENUMERATE, NULL, NULL },
    { "rotorSpeed",     37, JSPROP_ENUMERATE, NULL, NULL },
    { "rotorVector",    38, JSPROP_ENUMERATE, NULL, NULL },
    { "rotorFriction",  39, JSPROP_ENUMERATE, NULL, NULL },
    { "baseHeight",     40, JSPROP_ENUMERATE, NULL, NULL },
    { "tickDistance",   41, JSPROP_ENUMERATE, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(GameActor)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(GameActor)
{
    JS_FASTNATIVE(GameActor, updateTransform, 0),
    JS_FASTNATIVE(GameActor, setBounds, 6),
    JS_FASTNATIVE(GameActor, setAnim, 3),
    JS_FASTNATIVE(GameActor, blendAnim, 4),
    JS_FASTNATIVE(GameActor, checkAnimID, 1),
    JS_FASTNATIVE(GameActor, spawnFX, 4),
    JS_FASTNATIVE(GameActor, getLocalVector, 3),
    JS_FASTNATIVE(GameActor, setNodeRotation, 2),
    JS_FASTNATIVE(GameActor, onGround, 0),
    JS_FASTNATIVE(GameActor, setTexture, 4),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(GameActor)
{
    JS_FASTNATIVE(GameActor, spawn, 7),
    JS_FASTNATIVE(GameActor, remove, 1),
    JS_FASTNATIVE(GameActor, compare, 2),
    JS_FS_END
};
