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
#include "zone.h"

JSObject *js_objSys;

CVAR_EXTERNAL(cl_maxfps);

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

    JS_SET_RVAL(cx, rval, JSVAL_VOID);
    return JS_TRUE;
}

//
// sys_log
//

static JSBool sys_log(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
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

        Com_DPrintf("%s\n", bytes);
        JS_free(cx, bytes);
    }

    JS_SET_RVAL(cx, rval, JSVAL_VOID);
    return JS_TRUE;
}

//
// sys_error
//

static JSBool sys_error(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
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

        if(JS_IsExceptionPending(cx))
            JS_ReportPendingException(cx);

        JS_ReportError(cx, bytes);
        JS_free(cx, bytes);
    }

    JS_SET_RVAL(cx, rval, JSVAL_VOID);
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
// sys_getFixedTime
//

static JSBool sys_getFixedTime(JSContext *cx, uintN argc, jsval *rval)
{
    return JS_NewDoubleValue(cx, (1000.0f / cl_maxfps.value) / 1000.0f, rval);
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

    cvar = Cvar_Get(bytes);
    JS_free(cx, bytes);

    if(!cvar)
        return JS_FALSE;

    return JS_NewNumberValue(cx, cvar->value, rval);
}

//
// sys_execCommand
//

static JSBool sys_execCommand(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSString *str;
    char *bytes;

    if(argc <= 0)
        return JS_FALSE;

    if(!(str = JS_ValueToString(cx, argv[0])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    Cmd_ExecuteCommand(bytes);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, rval, JSVAL_VOID);
    return JS_TRUE;
}


//
// sys_addCommand
//

static JSBool sys_addCommand(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSObject *obj;
    JSString *str;
    char *bytes;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    if(!(str = JS_ValueToString(cx, v[0])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    JS_GETOBJECT(obj, v, 1);

    if(obj == NULL)
        return JS_FALSE;

    if(JS_ObjectIsFunction(cx, obj))
    {
        Cmd_AddCommandObject(bytes, obj);
    }
    
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// sys_addCvar
//

static JSBool sys_addCvar(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSString *str1;
    JSString *str2;
    char *bytes1;
    char *bytes2;
    cvar_t *cvar;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    if(!(str1 = JS_ValueToString(cx, v[0])) ||
        !(bytes1 = JS_EncodeString(cx, str1)))
    {
        return JS_FALSE;
    }

    if(!(str2 = JS_ValueToString(cx, v[1])) ||
        !(bytes2 = JS_EncodeString(cx, str2)))
    {
        JS_free(cx, bytes1);
        return JS_FALSE;
    }

    cvar = (cvar_t*)Z_Calloc(sizeof(cvar_t), PU_STATIC, 0);

    if(cvar == NULL)
    {
        JS_free(cx, bytes1);
        JS_free(cx, bytes2);

        return JS_FALSE;
    }

    cvar->name = bytes1;
    cvar->string = bytes2;
    cvar->defvalue = bytes2;
    cvar->nonclient = false;

    Cvar_Register(cvar);

    JS_SET_RVAL(cx, vp, v[1]);
    return JS_TRUE;
}

//
// sys_runScript
//

static JSBool sys_runScript(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSString *str;
    js_scrobj_t *jfile;
    char *bytes;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    if(!(str = JS_ValueToString(cx, v[0])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    if(!(jfile = J_LoadScript(bytes)))
    {
        JS_ReportPendingException(cx);
        JS_ReportError(cx, "Unable to load %s", bytes);
        JS_free(cx, bytes);

        return JS_FALSE;
    }

    J_ExecScriptObj(jfile);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// sys_dependsOn
//

static JSBool sys_dependsOn(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSString *str;
    char *bytes;
    js_scrobj_t *jfile;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    if(!(str = JS_ValueToString(cx, v[0])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    if(J_FindScript(bytes) == NULL)
    {
        if(!(jfile = J_LoadScript(bytes)))
        {
            JS_ReportPendingException(cx);
            JS_ReportError(cx, "Unable to load %s", bytes);
            JS_free(cx, bytes);

            return JS_FALSE;
        }

        J_ExecScriptObj(jfile);
    }

    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// sys_rootObject
//

static JSBool sys_rootObject(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSObject *obj;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETOBJECT(obj, v, 0);
    
    if(!JS_AddRoot(cx, &obj))
        return JS_FALSE;

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// sys_unrootObject
//

static JSBool sys_unrootObject(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSObject *obj;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETOBJECT(obj, v, 0);
    
    if(!JS_RemoveRoot(cx, &obj))
        return JS_FALSE;

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
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
// Sys_const
//

JSConstDoubleSpec Sys_const[] =
{
    { 0, 0, 0, { 0, 0, 0 } }
};

//
// Sys_functions
//

JSFunctionSpec Sys_functions[] =
{
    JS_FS("print",          sys_print,          0, 0, 0),
    JS_FS("log",            sys_log,            0, 0, 0),
    JS_FS("error",          sys_error,          0, 0, 0),
    JS_FN("ms",             sys_getms,          0, 0, 0),
    JS_FN("time",           sys_getTime,        0, 0, 0),
    JS_FN("deltatime",      sys_getDeltaTime,   0, 0, 0),
    JS_FN("fixedTime",      sys_getFixedTime,   0, 0, 0),
    JS_FN("ticks",          sys_getTicks,       0, 0, 0),
    JS_FS("getCvar",        sys_getCvar,        1, 0, 0),
    JS_FS("callCmd",        sys_execCommand,    1, 0, 0),
    JS_FN("addCommand",     sys_addCommand,     2, 0, 0),
    JS_FN("addCvar",        sys_addCvar,        2, 0, 0),
    JS_FN("runScript",      sys_runScript,      1, 0, 0),
    JS_FN("dependsOn",      sys_dependsOn,      1, 0, 0),
    JS_FN("rootObject",     sys_rootObject,     1, 0, 0),
    JS_FN("unrootObject",   sys_unrootObject,   1, 0, 0),
    JS_FN("GC",             sys_GC,             0, 0, 0),
    JS_FN("maybeGC",        sys_maybeGC,        0, 0, 0),
    JS_FS_END
};
