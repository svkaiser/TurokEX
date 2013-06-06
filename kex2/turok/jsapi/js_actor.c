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

            bbox = JS_NewObject(cx, NULL, NULL, NULL);
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

            angles = JS_NewObject(cx, NULL, NULL, NULL);
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
        return JS_NewDoubleValue(cx, actor->radius, vp);

    case 13:
        return JS_NewDoubleValue(cx, actor->height, vp);

    case 14:
        return JS_NewDoubleValue(cx, actor->centerHeight, vp);

    case 15:
        return JS_NewDoubleValue(cx, actor->viewHeight, vp);

    case 16:
        return JS_TRUE;

    case 17:
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(actor->components));
        return JS_TRUE;

    case 18:
        JS_RETURNBOOLEAN(vp, actor->bHidden);
        return JS_TRUE;

    case 19:
        return JS_NewDoubleValue(cx, actor->angles[0], vp);

    case 20:
        return JS_NewDoubleValue(cx, actor->angles[1], vp);

    case 21:
        return JS_NewDoubleValue(cx, actor->angles[2], vp);

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
        return JS_NewDoubleValue(cx, actor->timestamp, vp);

    case 24:
        JS_NEWOBJECT_SETPRIVATE(actor->model, &Model_class);
        return JS_TRUE;

    case 25:
        if(actor->owner)
        {
            JS_NEWOBJECT_SETPRIVATE(actor->owner, &GameActor_class);
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
            actor->owner = pActor;
            return JS_TRUE;
        }

        actor->owner = NULL;

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

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, addKey)
{
    JSObject *thisObj;
    gActor_t *actor;
    int id;
    JSString *str;
    char *bytes;

    JS_CHECKARGS(3);
    thisObj = JS_THIS_OBJECT(cx, vp);

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_GETSTRING(str, bytes, v, 0);
    JS_GETINTEGER(id, 1);

    if(JSVAL_IS_INT(v[2]))
    {
        int value;
        JS_GETINTEGER(value, 2);
        Actor_AddIntegerProperty(actor, bytes, id, value);
    }
    else if(JSVAL_IS_DOUBLE(v[2]))
    {
        jsdouble value;
        JS_GETNUMBER(value, v, 2);
        Actor_AddFloatProperty(actor, bytes, id, (float)value);
    }
    else if(JSVAL_IS_STRING(v[2]))
    {
        JSString *s;
        char *sb;
        JS_GETSTRING(s, sb, v, 2);
        Actor_AddStringProperty(actor, bytes, id, sb);
    }

    JS_free(cx, bytes);
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, getKey)
{
    JSObject *thisObj;
    gActor_t *actor;
    propKey_t *key;

    JS_CHECKARGS(1);
    thisObj = JS_THIS_OBJECT(cx, vp);

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    key = NULL;

    if(JSVAL_IS_INT(v[0]))
    {
        int value;
        JS_GETINTEGER(value, 0);
        key = &actor->properties[value];
    }
    else if(JSVAL_IS_STRING(v[0]))
    {
        JSString *str;
        char *bytes;
        JS_GETSTRING(str, bytes, v, 0);
        // TODO
        JS_free(cx, bytes);
    }

    if(key != NULL)
    {
        switch(key->type)
        {
        case 0:
            JS_SET_RVAL(cx, vp, INT_TO_JSVAL(key->val.i));
            return JS_TRUE;
        case 1:
            JS_NewDoubleValue(cx, key->val.f, vp);
            return JS_TRUE;
        case 2:
            JS_RETURNSTRING(vp, key->val.c);
            return JS_TRUE;
        }
    }

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, setKey)
{
    JSObject *thisObj;
    gActor_t *actor;
    propKey_t *key;

    JS_CHECKARGS(2);
    thisObj = JS_THIS_OBJECT(cx, vp);

    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    key = NULL;

    if(JSVAL_IS_INT(v[0]))
    {
        int value;
        JS_GETINTEGER(value, 0);
        key = &actor->properties[value];
    }
    else if(JSVAL_IS_STRING(v[0]))
    {
        JSString *str;
        char *bytes;
        JS_GETSTRING(str, bytes, v, 0);
        // TODO
        JS_free(cx, bytes);
    }

    if(key != NULL)
    {
        int ival;
        jsdouble fval;

        switch(key->type)
        {
        case 0:
            JS_GETINTEGER(ival, 1);
            key->val.i = ival;
            break;
        case 1:
            JS_GETNUMBER(fval, v, 1);
            key->val.f = (float)fval;
            break;
        }
    }

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
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

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, setAnim)
{
    JSObject *thisObj;
    gActor_t *actor;
    JSString *str;
    char *bytes;
    jsdouble speed;

    JS_CHECKARGS(3);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(speed, v, 1);
    JS_CHECKINTEGER(2);

    Mdl_SetAnimState(&actor->animState, Mdl_GetAnim(actor->model, bytes),
        (float)speed, JSVAL_TO_INT(JS_ARG(2)));

    JS_free(cx, bytes);
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, blendAnim)
{
    JSObject *thisObj;
    gActor_t *actor;
    JSString *str;
    char *bytes;
    jsdouble speed;
    jsdouble blendtime;

    JS_CHECKARGS(4);

    thisObj = JS_THIS_OBJECT(cx, vp);
    if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, thisObj, &GameActor_class, NULL)))
        return JS_FALSE;

    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(speed, v, 1);
    JS_GETNUMBER(blendtime, v, 2);
    JS_CHECKINTEGER(3);

    Mdl_BlendAnimStates(&actor->animState, Mdl_GetAnim(actor->model, bytes),
        (float)speed, (float)blendtime, JSVAL_TO_INT(JS_ARG(3)));

    JS_free(cx, bytes);
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
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

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GameActor, compare)
{
    JSObject *obj;
    gActor_t *actor1;
    gActor_t *actor2;
    JSBool ok;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj, v, 0);
    JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, actor1);
    JS_GETOBJECT(obj, v, 1);
    JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, actor2);

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
    { "timeStamp",      23, JSPROP_ENUMERATE, NULL, NULL },
    { "model",          24, JSPROP_ENUMERATE, NULL, NULL },
    { "owner",          25, JSPROP_ENUMERATE, NULL, NULL },
    { "targetID",       26, JSPROP_ENUMERATE, NULL, NULL },
    { "modelVariant",   27, JSPROP_ENUMERATE, NULL, NULL },
    { "bNoDropOff",     28, JSPROP_ENUMERATE, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(GameActor)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(GameActor)
{
    JS_FASTNATIVE(GameActor, updateTransform, 0),
    JS_FASTNATIVE(GameActor, addKey, 3),
    JS_FASTNATIVE(GameActor, getKey, 1),
    JS_FASTNATIVE(GameActor, setKey, 2),
    JS_FASTNATIVE(GameActor, setBounds, 6),
    JS_FASTNATIVE(GameActor, setAnim, 3),
    JS_FASTNATIVE(GameActor, blendAnim, 4),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(GameActor)
{
    JS_FASTNATIVE(GameActor, spawn, 7),
    JS_FASTNATIVE(GameActor, remove, 1),
    JS_FASTNATIVE(GameActor, compare, 2),
    JS_FS_END
};
