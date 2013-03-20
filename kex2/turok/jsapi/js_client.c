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
// DESCRIPTION: Javascript Client Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "kernel.h"
#include "client.h"
#include "mathlib.h"

JS_CLASSOBJECT(NClient);

enum client_enum
{
    CL_HOST,
    CL_PEER,
    CL_STATE,
    CL_NETEVENT,
    CL_ID,
    CL_SUBCLASS,
    CL_PLAYERACTOR
};

JS_PROP_FUNC_GET(NClient)
{
    switch(JSVAL_TO_INT(id))
    {
    case CL_STATE:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(client.state));
        return JS_TRUE;

    case CL_SUBCLASS:
        return JS_TRUE;

    case CL_HOST:
        JS_NEWOBJECTPOOL(client.host, Host);
        return JS_TRUE;

    case CL_PEER:
        JS_NEWOBJECTPOOL(client.peer, Peer);
        return JS_TRUE;

    case CL_NETEVENT:
        JS_NEWOBJECTPOOL(&client.netEvent, NetEvent);
        return JS_TRUE;

    case CL_ID:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(client.client_id));
        return JS_TRUE;

    case CL_PLAYERACTOR:
        JS_NEWOBJECTPOOL(client.playerActor, GameActor);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_PROP_FUNC_SET(NClient)
{
    jsdouble val;

    switch(JSVAL_TO_INT(id))
    {
    case CL_STATE:
        JS_GETNUMBER(val, vp, 0);
        client.state = (float)val;
        return JS_TRUE;

    case CL_ID:
        JS_GETNUMBER(val, vp, 0);
        client.client_id = (int)val;
        return JS_TRUE;

    case CL_SUBCLASS:
        {
            JSObject *obj2;
            JS_GETOBJECT(obj2, vp, 0);
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj2));
            return JS_TRUE;
        }

    case CL_PLAYERACTOR:
        {
            JSObject *object;
            gActor_t *actor;
            JS_GETOBJECT(object, vp, 0);
            if(!(actor = (gActor_t*)JS_GetInstancePrivate(cx, object, &GameActor_class, NULL)))
                return JS_TRUE;

            client.playerActor = actor;
            return JS_TRUE;
        }

    default:
        JS_ReportError(cx, "Unknown property");
        break;
    }

    return JS_FALSE;
}

JS_FASTNATIVE_BEGIN(NClient, inLevel)
{
    kbool b = gLevel.loaded;

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(b));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NClient, getEvent)
{
    event_t *ev = CL_GetEvent();

    if(ev == NULL)
    {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }

    JS_NEWOBJECTPOOL(ev, InputEvent);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NClient, spawnPlayer)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(NClient)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    NClient_getProperty,                        // getProperty
    NClient_setProperty,                        // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(NClient)
{
    { "host",       CL_HOST,        JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "peer",       CL_PEER,        JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "netEvent",   CL_NETEVENT,    JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "state",      CL_STATE,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "id",         CL_ID,          JSPROP_ENUMERATE,                   NULL, NULL },
    { "subclass",   CL_SUBCLASS,    JSPROP_ENUMERATE,                   NULL, NULL },
    { "playerActor",CL_PLAYERACTOR, JSPROP_ENUMERATE,                   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(NClient)
{
    { CL_STATE_UNINITIALIZED,   "STATE_UNINITIALIZED",  0, { 0, 0, 0 } },
    { CL_STATE_CONNECTING,      "STATE_CONNECTING",     0, { 0, 0, 0 } },
    { CL_STATE_CONNECTED,       "STATE_CONNECTED",      0, { 0, 0, 0 } },
    { CL_STATE_DISCONNECTED,    "STATE_DISCONNECTED",   0, { 0, 0, 0 } },
    { CL_STATE_READY,           "STATE_READY",          0, { 0, 0, 0 } },
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NClient)
{
    JS_FASTNATIVE(NClient, inLevel, 0),
    JS_FASTNATIVE(NClient, getEvent, 0),
    JS_FASTNATIVE(NClient, spawnPlayer, 4),
    JS_FS_END
};
