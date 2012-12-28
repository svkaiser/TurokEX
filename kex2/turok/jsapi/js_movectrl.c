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

typedef struct
{
    JSObject *object;
    int priority;
} mctrltypes_t;

static mctrltypes_t *mctrltypes;
static int num_mctrltypes = 0;

//
// J_GetMoveState
//

static JSObject *J_GetMoveState(void)
{
    if(movecontroller.movetype >= num_mctrltypes)
        return NULL;

    return mctrltypes[movecontroller.movetype].object;
}

//
// J_RunMoveState
//

void J_RunMoveState(void)
{
    int i;

    for(i = 0; i < num_mctrltypes; i++)
    {
        mctrltypes_t *mctrl = &mctrltypes[i];
        JSObject *object = mctrl->object;
        jsval rval;
        JSBool ret;

        rval = J_CallFunctionOnObject(js_context, object, "OnCheck");

        if(JSVAL_IS_NULL(rval) || !JSVAL_IS_BOOLEAN(rval))
            continue;

        if(!JS_ValueToBoolean(js_context, rval, &ret))
            continue;

        if(ret)
        {
            movecontroller.movetype = i;
            J_CallFunctionOnObject(js_context, object, "OnMove");
            return;
        }
    }
}

enum movectrl_enum
{
    MC_ORIGIN,
    MC_VELOCITY,
    MC_FORWARD,
    MC_RIGHT,
    MC_UP,
    MC_ACCEL,
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
    MC_STATE
};

//
// movectrl_getProperty
//

static JSBool movectrl_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch(JSVAL_TO_INT(id))
    {
    case MC_ORIGIN:
        JS_INSTVECTOR(vp, movecontroller.origin);
        return JS_TRUE;

    case MC_VELOCITY:
        JS_INSTVECTOR(vp, movecontroller.velocity);
        return JS_TRUE;

    case MC_FORWARD:
        JS_INSTVECTOR(vp, movecontroller.forward);
        return JS_TRUE;

    case MC_RIGHT:
        JS_INSTVECTOR(vp, movecontroller.right);
        return JS_TRUE;

    case MC_UP:
        JS_INSTVECTOR(vp, movecontroller.up);
        return JS_TRUE;

    case MC_ACCEL:
        JS_INSTVECTOR(vp, movecontroller.accel);
        return JS_TRUE;

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

    case MC_STATE:
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(J_GetMoveState()));
        return JS_TRUE;

    default:
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

    switch(JSVAL_TO_INT(id))
    {
    case MC_YAW:
        JS_GETNUMBER(val, vp, 0);
        movecontroller.yaw = (float)val;
        return JS_TRUE;

    default:
        break;
    }

    return JS_FALSE;
}

//
// moveCtrl_addMovetype
//

static JSBool moveCtrl_addMovetype(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSObject *obj;
    jsdouble priority;
    int i;
    mctrltypes_t *mctrl;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETOBJECT(obj, v, 0);
    JS_GETNUMBER(priority, v, 1);

    for(i = 0; i < num_mctrltypes; i++)
        JS_RemoveRoot(cx, &mctrltypes[i].object);

    mctrltypes = Z_Realloc(mctrltypes, sizeof(mctrltypes_t) *
        ++num_mctrltypes, PU_STATIC, 0);

    mctrl = &mctrltypes[num_mctrltypes - 1];
    mctrl->object = obj;
    mctrl->priority = (int)priority;

    for(i = 0; i < num_mctrltypes; i++)
        JS_AddRoot(cx, &mctrltypes[i].object);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
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
    { "state",      MC_STATE,       JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
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
    JS_FN("addMovetype",    moveCtrl_addMovetype,   2, 0, 0),
    JS_FN("setDirection",   moveCtrl_setDirection,  3, 0, 0),
    JS_FN("move",           moveCtrl_move,          2, 0, 0),
    JS_FS_END
};
