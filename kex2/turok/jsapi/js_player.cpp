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
// DESCRIPTION: Javascript Player Classes
//
//-----------------------------------------------------------------------------

#include "enet/enet.h"
#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "client.h"
#include "actor_old.h"
#include "player.h"

// -----------------------------------------------
//
// CLIENT PLAYER CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(ClientPlayer);

JS_PROP_FUNC_GET(ClientPlayer)
{
    switch(JSVAL_TO_INT(id))
    {
    case 0:
        JS_NEWOBJECT_SETPRIVATE(&client.LocalPlayer().worldState, &WorldState_class);
        return JS_TRUE;

    case 1:
        JS_NEWOBJECTPOOL(client.LocalPlayer().actor, GameActor);
        //JS_NEWOBJECT_SETPRIVATE(client.player->actor, &GameActor_class);
        return JS_TRUE;

    case 2:
        JS_NEWOBJECTPOOL(client.LocalPlayer().camera, GameActor);
        return JS_TRUE;

    case 3:
        //JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(client.LocalPlayer().GetScriptObject()));
        return JS_TRUE;

    case 4:
        JS_NEWOBJECT_SETPRIVATE(client.LocalPlayer().Cmd(), &Command_class);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(ClientPlayer)
{
    return JS_TRUE;
}

JS_BEGINCLASS(ClientPlayer)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    ClientPlayer_getProperty,                   // getProperty
    ClientPlayer_setProperty,                   // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(ClientPlayer)
{
    { "worldState", 0,  JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "actor",      1,  JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "camera",     2,  JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "component",  3,  JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "command",    4,  JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(ClientPlayer)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(ClientPlayer)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(ClientPlayer)
{
    JS_FS_END
};

// -----------------------------------------------
//
// COMMAND CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Command);

JS_PROP_FUNC_GET(Command)
{
    ticcmd_t *cmd;

    if(!(cmd = (ticcmd_t*)JS_GetInstancePrivate(cx, obj, &Command_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        return J_NewDoubleEx(cx, cmd->mouse[0].f, vp);

    case 1:
        return J_NewDoubleEx(cx, cmd->mouse[1].f, vp);

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(Command)
{
    jsdouble dval;
    ticcmd_t *cmd;

    if(!(cmd = (ticcmd_t*)JS_GetInstancePrivate(cx, obj, &Command_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        JS_GETNUMBER(dval, vp, 0);
        cmd->mouse[0].f = (float)dval;
        return JS_TRUE;

    case 1:
        JS_GETNUMBER(dval, vp, 0);
        cmd->mouse[1].f = (float)dval;
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Command, getAction)
{
    JSObject *thisObj;
    ticcmd_t *cmd;
    JSString *str;
    char *bytes;
    int action;

    JS_CHECKARGS(1);
    JS_GETSTRING(str, bytes, v, 0);

    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GET_PRIVATE_DATA(thisObj, &Command_class, ticcmd_t, cmd);

    action = inputKey.FindAction(bytes);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(action != -1 &&
        cmd->buttons[action]));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Command, getActionHeldTime)
{
    JSObject *thisObj;
    ticcmd_t *cmd;
    JSString *str;
    char *bytes;
    int action;
    int heldtime;

    JS_CHECKARGS(1);
    JS_GETSTRING(str, bytes, v, 0);

    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GET_PRIVATE_DATA(thisObj, &Command_class, ticcmd_t, cmd);

    action = inputKey.FindAction(bytes);
    JS_free(cx, bytes);

    heldtime = 0;

    if(action != -1)
    {
        heldtime = (cmd->heldtime[action] - 1);
        if(heldtime < 0)
            heldtime = 0;
    }

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(heldtime));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Command, clearButtons)
{
    JSObject *thisObj;
    ticcmd_t *cmd;
    int i;

    JS_CHECKARGS(0);
    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GET_PRIVATE_DATA(thisObj, &Command_class, ticcmd_t, cmd);

    for(i = 0; i < MAXACTIONS; i++)
        cmd->buttons[i] = false;

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(Command)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Command_getProperty,                        // getProperty
    Command_setProperty,                        // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Command)
{
    { "mouse_x",    0, JSPROP_ENUMERATE,    NULL, NULL },
    { "mouse_y",    1, JSPROP_ENUMERATE,    NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Command)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Command)
{
    JS_FASTNATIVE(Command, getAction, 1),
    JS_FASTNATIVE(Command, getActionHeldTime, 1),
    JS_FASTNATIVE(Command, clearButtons, 0),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Command)
{
    JS_FS_END
};

