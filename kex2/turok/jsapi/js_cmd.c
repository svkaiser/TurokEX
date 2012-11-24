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
// DESCRIPTION: Javascript Ticcmd Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "kernel.h"
#include "client.h"

JSObject *js_objCmd;

enum cmd_enum
{
    CMD_ANGLE_X,
    CMD_ANGLE_Y,
    CMD_MOUSE_X,
    CMD_MOUSE_Y,
    CMD_BUTTONS
};

//
// cmd_getProperty
//

static JSBool cmd_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch(JSVAL_TO_INT(id))
    {
    case CMD_ANGLE_X:
        return JS_NewNumberValue(cx, client.cmd.angle[0].f, vp);

    case CMD_ANGLE_Y:
        return JS_NewNumberValue(cx, client.cmd.angle[1].f, vp);

    case CMD_MOUSE_X:
        return JS_NewNumberValue(cx, client.cmd.mouse[0].f, vp);

    case CMD_MOUSE_Y:
        return JS_NewNumberValue(cx, client.cmd.mouse[1].f, vp);

    case CMD_BUTTONS:
        return JS_NewNumberValue(cx, client.cmd.buttons, vp);
    }

    return JS_FALSE;
}

//
// system_class
//

JSClass Cmd_class =
{
    "Cmd",                                      // name
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    cmd_getProperty,                            // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// Cmd_props
//

JSPropertySpec Cmd_props[] =
{
    { "angle_x",    CMD_ANGLE_X,    JSPROP_ENUMERATE,   NULL, NULL },
    { "angle_y",    CMD_ANGLE_Y,    JSPROP_ENUMERATE,   NULL, NULL },
    { "mouse_x",    CMD_MOUSE_X,    JSPROP_ENUMERATE,   NULL, NULL },
    { "mouse_y",    CMD_MOUSE_Y,    JSPROP_ENUMERATE,   NULL, NULL },
    { "buttons",    CMD_BUTTONS,    JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// Cmd_functions
//

JSFunctionSpec Cmd_functions[] =
{
    JS_FS_END
};
