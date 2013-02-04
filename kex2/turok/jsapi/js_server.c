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
// DESCRIPTION: Javascript Server Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "server.h"

JS_CLASSOBJECT(NServer);

enum nserver_enum
{
    NS_TOTALCLIENTS,
    NS_TIME,
    NS_RUNTIME,
    NS_TICS,
    NS_HOST,
    NS_NETEVENT,
    NS_SUBCLASS
};

JS_PROP_FUNC_GET(NServer)
{
    switch(JSVAL_TO_INT(id))
    {
    case NS_TOTALCLIENTS:
        return JS_NewNumberValue(cx, server.maxclients, vp);

    case NS_TIME:
        return JS_NewNumberValue(cx, server.time, vp);

    case NS_RUNTIME:
        return JS_NewNumberValue(cx, server.runtime, vp);

    case NS_TICS:
        return JS_NewNumberValue(cx, server.tics, vp);

    case NS_HOST:
        JS_NEWOBJECT_SETPRIVATE(server.host, NULL);
        return JS_TRUE;

    case NS_NETEVENT:
        JS_NEWOBJECT_SETPRIVATE(&server.netEvent, &NetEvent_class);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_PROP_FUNC_SET(NServer)
{
    jsdouble val;

    switch(JSVAL_TO_INT(id))
    {
    case NS_TIME:
        JS_GETNUMBER(val, vp, 0);
        server.time = (int)val;
        return JS_TRUE;

    case NS_RUNTIME:
        JS_GETNUMBER(val, vp, 0);
        server.runtime = (int)val;
        return JS_TRUE;

    case NS_TICS:
        JS_GETNUMBER(val, vp, 0);
        server.tics = (int)val;
        return JS_TRUE;

    case NS_SUBCLASS:
        {
            JSObject *obj2;
            JS_GETOBJECT(obj2, vp, 0);
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj2));
            return JS_TRUE;
        }

    default:
        JS_ReportError(cx, "Unknown property");
        break;
    }

    return JS_FALSE;
}

JS_FASTNATIVE_BEGIN(NServer, getPeerAddress)
{
    jsval *v;
    JSObject *obj;
    ENetEvent *ev;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETOBJECT(obj, v, 0);
    JS_GETNETEVENT(obj);
    
    JS_RETURNSTRING(vp, SV_GetPeerAddress(ev));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NServer, getClientID)
{
    jsval *v;
    JSObject *obj;
    ENetPeer *peer;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETOBJECT(obj, v, 0);
    JS_GETNETPEER(obj);

    return JS_NewNumberValue(cx, (jsdouble)SV_GetPlayerID(peer), vp);
}

JS_FASTNATIVE_BEGIN(NServer, spawnPlayer)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(NServer)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    NServer_getProperty,                        // getProperty
    NServer_setProperty,                        // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(NServer)
{
    { "totalClients",   NS_TOTALCLIENTS,    JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "time",           NS_TIME,            JSPROP_ENUMERATE,   NULL, NULL },
    { "runTime",        NS_RUNTIME,         JSPROP_ENUMERATE,   NULL, NULL },
    { "ticks",          NS_TICS,            JSPROP_ENUMERATE,   NULL, NULL },
    { "host",           NS_HOST,            JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "netEvent",       NS_NETEVENT,        JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "subclass",       NS_SUBCLASS,        JSPROP_ENUMERATE,                   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(NServer)
{
    { MAXCLIENTS,           "MAXCLIENTS",           0, { 0, 0, 0 } },
    { SV_STATE_UNAVAILABLE, "STATE_UNAVAILABLE",    0, { 0, 0, 0 } },
    { SV_STATE_BUSY,        "STATE_BUSY",           0, { 0, 0, 0 } },
    { SV_STATE_ACTIVE,      "STATE_ACTIVE",         0, { 0, 0, 0 } },
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NServer)
{
    JS_FASTNATIVE(NServer, getPeerAddress,  1),
    JS_FASTNATIVE(NServer, getClientID,     1),
    JS_FASTNATIVE(NServer, spawnPlayer,     4),
    JS_FS_END
};
