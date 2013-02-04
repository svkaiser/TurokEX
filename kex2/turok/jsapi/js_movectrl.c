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
// DESCRIPTION: Javascript MoveController Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "client.h"
#include "pred.h"
#include "zone.h"

JSObject *js_objMoveController;

enum movectrl_enum
{
    MC_ORIGIN,
    MC_VELOCITY,
    MC_FORWARD,
    MC_RIGHT,
    MC_UP,
    MC_ACCEL,
    MC_MOVETIME,
    MC_WIDTH,
    MC_HEIGHT,
    MC_CENTERY,
    MC_VIEWY,
    MC_YAW,
    MC_PITCH,
    MC_ROLL,
    MC_DELTATIME,
    MC_PLANE,
    MC_CMD,
    MC_STATE,
    MC_LOCAL
};

//
// movectrl_getProperty
//

static JSBool movectrl_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch(JSVAL_TO_INT(id))
    {
    case MC_ORIGIN:
        JS_INSTVECTOR(js_objMoveController, vp, movecontroller.origin);
        return JS_TRUE;

    case MC_VELOCITY:
        JS_INSTVECTOR(js_objMoveController, vp, movecontroller.velocity);
        return JS_TRUE;

    case MC_FORWARD:
        JS_INSTVECTOR(js_objMoveController, vp, movecontroller.forward);
        return JS_TRUE;

    case MC_RIGHT:
        JS_INSTVECTOR(js_objMoveController, vp, movecontroller.right);
        return JS_TRUE;

    case MC_UP:
        JS_INSTVECTOR(js_objMoveController, vp, movecontroller.up);
        return JS_TRUE;

    case MC_ACCEL:
        JS_INSTVECTOR(js_objMoveController, vp, movecontroller.accel);
        return JS_TRUE;

    case MC_MOVETIME:
        return JS_NewNumberValue(cx, movecontroller.movetime, vp);

    case MC_CENTERY:
        return JS_NewNumberValue(cx, movecontroller.center_y, vp);

    case MC_VIEWY:
        return JS_NewNumberValue(cx, movecontroller.view_y, vp);

    case MC_CMD:
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(js_objCmd));
        return JS_TRUE;

    case MC_PLANE:
        JS_INSTPLANE(vp, movecontroller.plane);
        return JS_TRUE;

    case MC_YAW:
        return JS_NewNumberValue(cx, movecontroller.yaw, vp);

    case MC_PITCH:
        return JS_NewNumberValue(cx, movecontroller.pitch, vp);

    case MC_ROLL:
        return JS_NewNumberValue(cx, movecontroller.roll, vp);

    case MC_DELTATIME:
        return JS_NewNumberValue(cx, movecontroller.deltatime, vp);

    case MC_STATE:
        return JS_NewNumberValue(cx, movecontroller.movetype, vp);

    case MC_LOCAL:
        JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(movecontroller.local));
        return JS_TRUE;

    default:
        JS_ReportError(cx, "Unknown property");
        break;
    }

    return JS_FALSE;
}

//
// movectrl_setProperty
//

static JSBool movectrl_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    jsdouble val;
    JSObject *vobj;
    vec3_t *vector = NULL;

    switch(JSVAL_TO_INT(id))
    {
    case MC_ORIGIN:
        if(!(JS_ValueToObject(cx, *vp, &vobj)))
            return JS_FALSE;
        if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, vobj, &Vector_class, NULL)))
            return JS_FALSE;
        Vec_Copy3(movecontroller.origin, *vector);
        return JS_TRUE;

    case MC_VELOCITY:
        if(!(JS_ValueToObject(cx, *vp, &vobj)))
            return JS_FALSE;
        if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, vobj, &Vector_class, NULL)))
            return JS_FALSE;
        Vec_Copy3(movecontroller.velocity, *vector);
        return JS_TRUE;

    case MC_FORWARD:
        if(!(JS_ValueToObject(cx, *vp, &vobj)))
            return JS_FALSE;
        if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, vobj, &Vector_class, NULL)))
            return JS_FALSE;
        Vec_Copy3(movecontroller.forward, *vector);
        return JS_TRUE;

    case MC_RIGHT:
        if(!(JS_ValueToObject(cx, *vp, &vobj)))
            return JS_FALSE;
        if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, vobj, &Vector_class, NULL)))
            return JS_FALSE;
        Vec_Copy3(movecontroller.right, *vector);
        return JS_TRUE;

    case MC_UP:
        if(!(JS_ValueToObject(cx, *vp, &vobj)))
            return JS_FALSE;
        if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, vobj, &Vector_class, NULL)))
            return JS_FALSE;
        Vec_Copy3(movecontroller.up, *vector);
        return JS_TRUE;

    case MC_ACCEL:
        if(!(JS_ValueToObject(cx, *vp, &vobj)))
            return JS_FALSE;
        if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, vobj, &Vector_class, NULL)))
            return JS_FALSE;
        Vec_Copy3(movecontroller.accel, *vector);
        return JS_TRUE;

    case MC_YAW:
        JS_GETNUMBER(val, vp, 0);
        movecontroller.yaw = (float)val;
        return JS_TRUE;

    case MC_ROLL:
        JS_GETNUMBER(val, vp, 0);
        movecontroller.roll = (float)val;
        return JS_TRUE;

    case MC_MOVETIME:
        JS_GETNUMBER(val, vp, 0);
        movecontroller.movetime = (float)val;
        return JS_TRUE;

    case MC_STATE:
        JS_GETNUMBER(val, vp, 0);
        movecontroller.movetype = (int)val;
        return JS_TRUE;

    default:
        break;
    }

    return JS_FALSE;
}

//
// moveCtrl_setDirection
//

static JSBool moveCtrl_setDirection(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    jsdouble yaw;
    jsdouble pitch;
    jsdouble roll;

    if(argc != 3)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(pitch, v, 0);
    JS_GETNUMBER(yaw, v, 1);
    JS_GETNUMBER(roll, v, 2);

    Pred_SetDirection(&movecontroller,
        (float)yaw, (float)pitch, (float)roll);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// moveCtrl_move
//

static JSBool moveCtrl_move(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    jsdouble friction;
    jsdouble gravity;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(friction, v, 0);
    JS_GETNUMBER(gravity, v, 1);

    Pred_ProcessMove(&movecontroller,
        (float)friction, (float)gravity);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// MoveController_class
//

JSClass MoveController_class =
{
    "MoveController",                           // name
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    movectrl_getProperty,                       // getProperty
    movectrl_setProperty,                       // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// MoveController_props
//

JSPropertySpec MoveController_props[] =
{
    { "origin",     MC_ORIGIN,      JSPROP_ENUMERATE,                   NULL, NULL },
    { "velocity",   MC_VELOCITY,    JSPROP_ENUMERATE,                   NULL, NULL },
    { "forward",    MC_FORWARD,     JSPROP_ENUMERATE,                   NULL, NULL },
    { "right",      MC_RIGHT,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "up",         MC_UP,          JSPROP_ENUMERATE,                   NULL, NULL },
    { "accel",      MC_ACCEL,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "movetime",   MC_MOVETIME,    JSPROP_ENUMERATE,                   NULL, NULL },
    { "width",      MC_WIDTH,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "height",     MC_HEIGHT,      JSPROP_ENUMERATE,                   NULL, NULL },
    { "center_y",   MC_CENTERY,     JSPROP_ENUMERATE,                   NULL, NULL },
    { "view_y",     MC_VIEWY,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "yaw",        MC_YAW,         JSPROP_ENUMERATE,                   NULL, NULL },
    { "pitch",      MC_PITCH,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "roll",       MC_ROLL,        JSPROP_ENUMERATE,                   NULL, NULL },
    { "deltatime",  MC_DELTATIME,   JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "plane",      MC_PLANE,       JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "cmd",        MC_CMD,         JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "state",      MC_STATE,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "local",      MC_LOCAL,       JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// MoveController_const
//

JSConstDoubleSpec MoveController_const[] =
{
    { 0, 0, 0, { 0, 0, 0 } }
};

//
// MoveController_functions
//

JSFunctionSpec MoveController_functions[] =
{
    JS_FN("setDirection",   moveCtrl_setDirection,  3, 0, 0),
    JS_FN("move",           moveCtrl_move,          2, 0, 0),
    JS_FS_END
};
