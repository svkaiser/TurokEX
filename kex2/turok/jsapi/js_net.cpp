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
// DESCRIPTION: Javascript Network Classes
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "packet.h"
#include "client.h"

// -----------------------------------------------
//
// NET CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Net);

JS_FASTNATIVE_BEGIN(Net, newPacket)
{
    ENetPacket *packet;

    if(packet = packetManager.Create())
    {
        JSObject *object;

        if(!(object = JPool_GetFree(&objPoolPacket, &Packet_class)) ||
            !(JS_SetPrivate(cx, object, packet)))
        {
            enet_packet_destroy(packet);
            return JS_FALSE;
        }

        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(object));
    }
    else
    {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Net, hostService)
{
    jsval *v;
    JSObject *obj;
    ENetEvent ev;
    ENetHost *host;
    int ret;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETOBJECT(obj, v, 0);
    JS_GETHOST(obj);

    ret = enet_host_service(host, &ev, 0);

    if(ret > 0)
    {
        //JS_NEWOBJECT_SETPRIVATE(&ev, &NetEvent_class);
        JS_NEWOBJECTPOOL(&ev, NetEvent);
    }
    else
    {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
    }

    return JS_TRUE;
}


JS_BEGINCLASS(Net)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    JS_PropertyStub,                            // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Net)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Net)
{
    { ENET_EVENT_TYPE_CONNECT,      "connect",      0, { 0, 0, 0 } },
    { ENET_EVENT_TYPE_DISCONNECT,   "disconnect",   0, { 0, 0, 0 } },
    { ENET_EVENT_TYPE_RECEIVE,      "packet",       0, { 0, 0, 0 } },
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Net)
{
    JS_FASTNATIVE(Net, newPacket,  0),
    JS_FASTNATIVE(Net, hostService, 1),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Net)
{
    JS_FS_END
};

// -----------------------------------------------
//
// NETEVENT CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(NetEvent);

enum netevent_enum
{
    NE_TYPE,
    NE_PEER,
    NE_CHANNEL,
    NE_PACKET
};

JS_PROP_FUNC_GET(NetEvent)
{
    ENetEvent *ev;

    JS_GETNETEVENT(obj);

    switch(JSVAL_TO_INT(id))
    {
    case NE_TYPE:
        //return JS_NewNumberValue(cx, ev->type, vp);
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ev->type));
        return JS_TRUE;

    case NE_PEER:
        JS_NEWOBJECTPOOL(ev->peer, Peer);
        //JS_NEWOBJECT_SETPRIVATE(ev->peer, &Peer_class);
        return JS_TRUE;

    case NE_CHANNEL:
        //return JS_NewNumberValue(cx, ev->channelID, vp);
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ev->channelID));
        return JS_TRUE;

    case NE_PACKET:
        JS_NEWOBJECTPOOL(ev->packet, Packet);
        //JS_NEWOBJECT_SETPRIVATE(ev->packet, &Packet_class);
        return JS_TRUE;

    default:
        JS_ReportError(cx, "Unknown property");
        break;
    }

    return JS_FALSE;
}

JS_BEGINCLASS(NetEvent)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    NetEvent_getProperty,                       // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(NetEvent)
{
    { "type",       NE_TYPE,    JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "peer",       NE_PEER,    JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "channel",    NE_CHANNEL, JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "packet",     NE_PACKET,  JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(NetEvent)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NetEvent)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(NetEvent)
{
    JS_FS_END
};

// -----------------------------------------------
//
// PACKET CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Packet);

JS_FINALIZE_FUNC(Packet)
{
    ENetPacket *packet;

    if(packet = (ENetPacket*)JS_GetPrivate(cx, obj))
    {
        common.DPrintf("Finalized packet object still has private data\n");
    }
}

#define JS_WRITE_PACKET_FUNC(name, func)                \
    JS_FASTNATIVE_BEGIN(Packet, name)                   \
    {                                                   \
        jsval *v;                                       \
        jsdouble n;                                     \
        unsigned int value;                             \
        ENetPacket *packet;                             \
        if(argc != 1)                                   \
            return JS_FALSE;                            \
        v = JS_ARGV(cx, vp);                            \
        JS_GETPACKET(JS_THIS_OBJECT(cx, vp));           \
        JS_GETNUMBER(n, v, 0);                          \
        value = (unsigned int)n;                        \
        func(packet, value);                            \
        JS_SET_RVAL(cx, vp, JSVAL_VOID);                \
        return JS_TRUE;                                 \
    }

#define JS_READINT_PACKET_FUNC(name, func)              \
    JS_FASTNATIVE_BEGIN(Packet, name)                   \
    {                                                   \
        jsval *v;                                       \
        ENetPacket *packet;                             \
        unsigned int n;                                 \
        v = JS_ARGV(cx, vp);                            \
        JS_GETPACKET(JS_THIS_OBJECT(cx, vp));           \
        func(packet, &n);                               \
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(n));           \
        return JS_TRUE;                                 \
    }

JS_WRITE_PACKET_FUNC(write8, packetManager.Write8);
JS_WRITE_PACKET_FUNC(write16, packetManager.Write16);
JS_WRITE_PACKET_FUNC(write32, packetManager.Write32);

JS_READINT_PACKET_FUNC(read8, packetManager.Read8);
JS_READINT_PACKET_FUNC(read16, packetManager.Read8);
JS_READINT_PACKET_FUNC(read32, packetManager.Read32);

JS_FASTNATIVE_BEGIN(Packet, writeString)
{
    jsval *v;
    ENetPacket *packet;
    JSString *str;
    char *bytes;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETPACKET(JS_THIS_OBJECT(cx, vp));
    
    if(!(str = JS_ValueToString(cx, v[0])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    packetManager.WriteString(packet, bytes);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Packet, readString)
{
    jsval *v;
    ENetPacket *packet;

    v = JS_ARGV(cx, vp);

    JS_GETPACKET(JS_THIS_OBJECT(cx, vp));

    JS_RETURNSTRING(vp, packetManager.ReadString(packet));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Packet, writeVector)
{
    ENetPacket *packet;
    vec3_t vec;
    JSObject *object;

    JS_CHECKARGS(1);

    JS_GETPACKET(JS_THIS_OBJECT(cx, vp));
    JS_GETOBJECT(object, v, 0);
    JS_GETVECTOR2(object, vec);

    packetManager.WriteVector(packet, vec);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Packet, readVector)
{
    jsval *v;
    ENetPacket *packet;
    vec3_t vec;

    v = JS_ARGV(cx, vp);

    JS_GETPACKET(JS_THIS_OBJECT(cx, vp));
    packetManager.ReadVector(packet, &vec);

    JS_NEWVECTOR2(vec);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Packet, writeFloat)
{
    jsval *v;
    ENetPacket *packet;
    jsdouble val;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETPACKET(JS_THIS_OBJECT(cx, vp));
    JS_GETNUMBER(val, v, 0);

    packetManager.WriteFloat(packet, (float)val);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Packet, readFloat)
{
    jsval *v;
    ENetPacket *packet;
    float val;

    v = JS_ARGV(cx, vp);

    JS_GETPACKET(JS_THIS_OBJECT(cx, vp));
    packetManager.ReadFloat(packet, &val);

    J_NewDoubleEx(cx, (jsdouble)val, vp);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Packet, send)
{
    jsval *v;
    JSObject *obj;
    ENetPacket *packet;
    ENetPeer *peer;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETPACKET(JS_THIS_OBJECT(cx, vp));

    if(JSVAL_IS_NULL(v[0]))
        peer = client.GetPeer();
    else
    {
        JS_GETOBJECT(obj, v, 0);
        JS_GETNETPEER(obj);
    }

    packetManager.Send(packet, peer);
    JS_SetPrivate(cx, JS_THIS_OBJECT(cx, vp), NULL);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Packet, destroy)
{
    jsval *v;
    JSObject *obj;
    ENetPacket *packet;

    v = JS_ARGV(cx, vp);

    obj = JS_THIS_OBJECT(cx, vp);
    JS_GETPACKET(obj);

    enet_packet_destroy(packet);
    JS_SetPrivate(cx, obj, NULL);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(Packet)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    JS_PropertyStub,                            // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    Packet_finalize,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Packet)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Packet)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Packet)
{
    JS_FASTNATIVE(Packet, write8,       1),
    JS_FASTNATIVE(Packet, write16,      1),
    JS_FASTNATIVE(Packet, write32,      1),
    JS_FASTNATIVE(Packet, writeFloat,   1),
    JS_FASTNATIVE(Packet, writeString,  1),
    JS_FASTNATIVE(Packet, writeVector,  1),
    JS_FASTNATIVE(Packet, read8,        0),
    JS_FASTNATIVE(Packet, read16,       0),
    JS_FASTNATIVE(Packet, read32,       0),
    JS_FASTNATIVE(Packet, readFloat,    0),
    JS_FASTNATIVE(Packet, readString,   0),
    JS_FASTNATIVE(Packet, readVector,   0),
    JS_FASTNATIVE(Packet, send,         1),
    JS_FASTNATIVE(Packet, destroy,      0),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Packet)
{
    JS_FS_END
};

// -----------------------------------------------
//
// PEER CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Peer);

enum netpeer_enum
{
    NP_CONNECTID
};

JS_PROP_FUNC_GET(Peer)
{
    ENetPeer *peer;

    JS_GETNETPEER(obj);

    switch(JSVAL_TO_INT(id))
    {
    case NP_CONNECTID:
        //return JS_NewNumberValue(cx, peer->connectID, vp);
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(peer->connectID));
        return JS_TRUE;

    default:
        JS_ReportError(cx, "Unknown property");
        break;
    }

    return JS_FALSE;
}

JS_BEGINCLASS(Peer)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Peer_getProperty,                           // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Peer)
{
    { "connectID",  NP_CONNECTID,   JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Peer)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Peer)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Peer)
{
    JS_FS_END
};

// -----------------------------------------------
//
// HOST CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Host);

JS_BEGINCLASS(Host)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    JS_PropertyStub,                            // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Host)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Host)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Host)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Host)
{
    JS_FS_END
};
