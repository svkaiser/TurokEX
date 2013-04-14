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

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(GameActor)
{
    gActor_t *actor;
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
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(GameActor)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(GameActor)
{
    JS_FASTNATIVE(GameActor, updateTransform, 0),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(GameActor)
{
    JS_FS_END
};
