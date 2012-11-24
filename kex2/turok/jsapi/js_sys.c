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
// DESCRIPTION: Javascript System Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "kernel.h"
#include "client.h"

JSObject *js_objSys;

//
// sys_print
//

static JSBool sys_print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    uintN i;
    JSString *str;
    char *bytes;

    for (i = 0; i < argc; i++)
    {
        if(!(str = JS_ValueToString(cx, argv[i])) ||
            !(bytes = JS_EncodeString(cx, str)))
        {
            return JS_FALSE;
        }

        Com_Printf("%s\n", bytes);
        JS_free(cx, bytes);
    }

    return JS_TRUE;
}

//
// sys_getms
//

static JSBool sys_getms(JSContext *cx, uintN argc, jsval *rval)
{
    return JS_NewNumberValue(cx, Sys_GetMilliseconds(), rval);
}

//
// sys_getTime
//

static JSBool sys_getTime(JSContext *cx, uintN argc, jsval *rval)
{
    return JS_NewNumberValue(cx, client.time, rval);
}

//
// sys_getDeltaTime
//

static JSBool sys_getDeltaTime(JSContext *cx, uintN argc, jsval *rval)
{
    return JS_NewDoubleValue(cx, client.runtime, rval);
}

//
// sys_getTicks
//

static JSBool sys_getTicks(JSContext *cx, uintN argc, jsval *rval)
{
    return JS_NewNumberValue(cx, client.tics, rval);
}

//
// sys_getCvar
//

static JSBool sys_getCvar(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSString *str;
    char *bytes;
    cvar_t *cvar;

    if(argc <= 0)
        return JS_FALSE;

    if(!(str = JS_ValueToString(cx, argv[0])) ||
            !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    if(!(cvar = Cvar_Get(bytes)))
        return JS_FALSE;

    JS_free(cx, bytes);

    return JS_NewNumberValue(cx, cvar->value, rval);
}


//
// sys_GC
//

static JSBool sys_GC(JSContext *cx, uintN argc, jsval *vp)
{
   JS_GC(cx);

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

//
// sys_maybeGC
//

static JSBool sys_maybeGC(JSContext *cx, uintN argc, jsval *vp)
{
   JS_MaybeGC(cx);

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

//
// system_class
//

JSClass Sys_class =
{
    "Sys",                                      // name
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
};

//
// Sys_props
//

JSPropertySpec Sys_props[] =
{
    { NULL, 0, 0, NULL, NULL }
};

//
// Sys_functions
//

JSFunctionSpec Sys_functions[] =
{
    JS_FS("print",      sys_print,          0, 0, 0),
    JS_FN("ms",         sys_getms,          0, 0, 0),
    JS_FN("time",       sys_getTime,        0, 0, 0),
    JS_FN("deltatime",  sys_getDeltaTime,   0, 0, 0),
    JS_FN("ticks",      sys_getTicks,       0, 0, 0),
    JS_FS("getCvar",    sys_getCvar,        1, 0, 0),
    JS_FN("GC",         sys_GC,             0, 0, 0),
    JS_FN("maybeGC",    sys_maybeGC,        0, 0, 0),
    JS_FS_END
};
