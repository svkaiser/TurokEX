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
// DESCRIPTION: Javascript API
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "zone.h"
#include "kernel.h"

CVAR_EXTERNAL(kf_basepath);

#define JS_RUNTIME_HEAP_SIZE 64L * 1024L * 1024L
#define JS_STACK_CHUNK_SIZE  8192

typedef struct js_scrobj_s
{
    char name[MAX_FILEPATH];
    JSScript *script;
    JSObject *obj;
    struct js_scrobj_s *next;
} js_scrobj_t;

static js_scrobj_t *js_scrobj_list[MAX_HASH];

static JSRuntime    *js_runtime;
static JSContext    *js_context;
static JSObject     *js_gobject;
static js_scrobj_t  *js_rootscript;

//
// J_GlobalEnumerate
//
// Lazy enumeration for the ECMA standard classes.
// Doing this is said to lower memory usage.
//

static JSBool J_GlobalEnumerate(JSContext *cx, JSObject *obj)
{
    return JS_EnumerateStandardClasses(cx, obj);
}

//
// J_GlobalResolve
//
// Lazy resolution for the ECMA standard classes.
//

static JSBool J_GlobalResolve(JSContext *cx, JSObject *obj, jsval id,
                              uintN flags, JSObject **objp)
{
    if((flags & JSRESOLVE_ASSIGNING) == 0)
    {
        JSBool resolved;

        if(!JS_ResolveStandardClass(cx, obj, id, &resolved))
        {
            return JS_FALSE;
        }

        if(resolved)
        {
            *objp = obj;
            return JS_TRUE;
        }
    }

    return JS_TRUE;
}

//
// global_class
//
static JSClass global_class =
{
    "global",                                   // name
    JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS, // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    JS_PropertyStub,                            // getProperty
    JS_PropertyStub,                            // setProperty
    J_GlobalEnumerate,                          // enumerate
    (JSResolveOp)J_GlobalResolve,               // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// J_Error
//

static void J_Error(JSContext *cx, const char *message, JSErrorReport *report)
{
    char *buf;
    char *f = NULL;
    char *l = NULL;
    char *m = NULL;
    int len = 0;

    if(!report)
    {
        Com_CPrintf(COLOR_RED, "%s\n", message);
        return;
    }

    if(report->filename)
    {
        f = Z_Strdupa(kva("%s: ", report->filename));
        len += strlen(f);
    }

    if(report->lineno)
    {
        l = Z_Strdupa(kva("%i ", report->lineno));
        len += strlen(l);
    }

    m = Z_Strdupa(message);
    len += strlen(m);

    buf = Z_Alloca(len+1);
    if(f) strcat(buf, f);
    if(l) strcat(buf, l);
    if(m) strcat(buf, m);

    if(JSREPORT_IS_WARNING(report->flags))
    {
        Com_Warning(buf);
        return;
    }
    else
    {
        Com_CPrintf(COLOR_RED, "%s\n", buf);
    }
}

//
// J_ContextCallback
//

static JSBool J_ContextCallback(JSContext *cx, uintN contextOp)
{
    if(contextOp == JSCONTEXT_NEW)
    {
        JS_SetErrorReporter(cx, J_Error);
        JS_SetVersion(cx, JSVERSION_LATEST);
    }
    
    return JS_TRUE;
}

//
// J_AddObject
//

JSObject *J_AddObject(JSClass *class, JSFunctionSpec *func, JSPropertySpec *prop,
                     const char *name, JSContext *cx, JSObject *obj)
{
    JSObject *newobj;

    if(!(newobj = JS_DefineObject(cx, obj, name, class, NULL, 0)))
        Com_Error("J_AddObject: Failed to create a new class for %s", name);

    if(prop)
    {
        if(!JS_DefineProperties(cx, newobj, prop))
             Com_Error("J_AddObject: Failed to define properties for class %s", name);
    }

    if(func)
    {
        if(!JS_DefineFunctions(cx, newobj, func))
            Com_Error("J_AddObject: Failed to define functions for class %s", name);
    }

    return newobj;
}

//
// J_LoadScriptObject
//

js_scrobj_t *J_LoadScriptObject(const char *name, char *buffer, int size)
{
    char scrname[MAX_FILEPATH];
    unsigned int hash;
    js_scrobj_t *scrobj;
    JSContext *cx;
    JSObject *obj;

    if(strlen(name) >= MAX_FILEPATH)
    {
        Com_Error("J_LoadScriptObject: \"%s\" is too long", name);
    }

    cx = js_context;
    obj = js_gobject;

    if(!JS_BufferIsCompilableUnit(cx, obj, buffer, size))
    {
        return NULL;
    }

    scrobj = Z_Calloc(sizeof(js_scrobj_t), PU_JSOBJ, 0);
    strcpy(scrobj->name, name);
    strcpy(scrname, name);
    Com_StripPath(scrname);
    Com_StripExt(scrname);

    if(!(scrobj->script = JS_CompileScript(cx, obj, buffer, size, scrname, 1)))
    {
        Z_Free(scrobj);
        return NULL;
    }

    if(!(scrobj->obj = JS_NewScriptObject(cx, scrobj->script)))
    {
        JS_DestroyScript(cx, scrobj->script);
        Z_Free(scrobj);
        return NULL;
    }

    if(!JS_AddNamedRoot(cx, &scrobj->obj, scrname))
    {
        JS_DestroyScript(cx, scrobj->script);
        Z_Free(scrobj);
        return NULL;
    }

    hash = Com_HashFileName(name);
    scrobj->next = js_scrobj_list[hash];
    js_scrobj_list[hash] = scrobj;

    return scrobj;
}

//
// J_FindScript
//

js_scrobj_t *J_FindScript(const char *name)
{
    js_scrobj_t *scrobj;
    unsigned int hash;

    if(name[0] == 0)
    {
        return NULL;
    }

    hash = Com_HashFileName(name);

    for(scrobj = js_scrobj_list[hash]; scrobj; scrobj = scrobj->next)
    {
        if(!strcmp(name, scrobj->name))
        {
            return scrobj;
        }
    }

    return NULL;
}

//
// J_LoadScript
//

js_scrobj_t *J_LoadScript(const char *name)
{
    js_scrobj_t *scrobj;
    int size;

    if(name[0] == 0)
    {
        return NULL;
    }

    scrobj = J_FindScript(name);

    if(scrobj == NULL)
    {
        char *file;

        if((size = KF_OpenFileCache(name, &file, PU_STATIC)) == 0)
        {
            if((size = KF_ReadTextFile(name, &file)) == -1)
            {
                return NULL;
            }
        }

        scrobj = J_LoadScriptObject(name, file, size);
        Z_Free(file);
    }

    return scrobj;
}

//
// J_ExecBuffer
//

void J_ExecBuffer(char *buffer)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    jsval result;

    if(JS_BufferIsCompilableUnit(cx, obj, buffer, strlen(buffer)))
    {
        JSScript *script;

        JS_ClearPendingException(cx);
        if(script = JS_CompileScript(cx, obj, buffer,
            strlen(buffer), "execBuffer", 1))
        {
            JS_ExecuteScript(cx, obj, script, &result);
            JS_MaybeGC(cx);
            JS_DestroyScript(cx, script);
        }
    }
}

//
// J_Shutdown
//

void J_Shutdown(void)
{
    JS_DestroyContext(js_context);
    JS_DestroyRuntime(js_runtime);
    JS_ShutDown();

    Z_FreeTags(PU_JSOBJ, PU_JSOBJ);
}

//
// FCmd_JS
//

static void FCmd_JS(void)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    JSBool ok;
    JSString *str;
    jsval result;
    char *buffer;

    if(Cmd_GetArgc() < 2)
    {
        Com_Printf("Usage: js <code>\n");
        return;
    }

    buffer = Cmd_GetArgv(1);

    if(JS_BufferIsCompilableUnit(cx, obj, buffer, strlen(buffer)))
    {
        JSScript *script;

        JS_ClearPendingException(cx);
        if(script = JS_CompileScript(cx, obj, buffer,
            strlen(buffer), "console", 1))
        {
            ok = JS_ExecuteScript(cx, obj, script, &result);

            if(ok && result != JSVAL_VOID)
            {
                if(str = JS_ValueToString(cx, result))
                {
                    Com_Printf("%s\n", JS_GetStringBytes(str));
                }
            }

            JS_DestroyScript(cx, script);
        }
    }
}

//
// FCmd_JSFile
//

static void FCmd_JSFile(void)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    JSScript *script;
    jsval result;
    uint32 oldopts;

    if(Cmd_GetArgc() < 2)
    {
        Com_Printf("Usage: jsfile <filename>\n");
        return;
    }

    oldopts = JS_GetOptions(cx);
    JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
    if(script = JS_CompileFile(cx, obj, kva("%s\\%s",
        kf_basepath.string, Cmd_GetArgv(1))))
    {
        JS_ExecuteScript(cx, obj, script, &result);
        JS_DestroyScript(cx, script);
    }

    JS_SetOptions(cx, oldopts);
}

//
// FCmd_JSLoad
//

static void FCmd_JSLoad(void)
{
    if(Cmd_GetArgc() < 2)
    {
        Com_Printf("Usage: jsload <filename>\n");
        return;
    }

    if(J_LoadScript(Cmd_GetArgv(1)))
    {
        Com_Printf("Script loaded\n");
    }
}

//
// FCmd_JSExec
//

static void FCmd_JSExec(void)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    jsval result;
    js_scrobj_t *scrobj;

    if(Cmd_GetArgc() < 2)
    {
        Com_Printf("Usage: jsexec <name>\n");
        return;
    }

    if(!(scrobj = J_FindScript(Cmd_GetArgv(1))))
    {
        Com_Printf("\"%s\" is not loaded\n", Cmd_GetArgv(1));
        return;
    }

    JS_ExecuteScript(cx, obj, scrobj->script, &result);
    JS_MaybeGC(cx);
}

//
// J_Init
//

void J_Init(void)
{
    jsval result;

    if(!(js_runtime = JS_NewRuntime(JS_RUNTIME_HEAP_SIZE)))
        Com_Error("J_Init: Failed to initialize JSAPI runtime");

    JS_SetContextCallback(js_runtime, J_ContextCallback);

    if(!(js_context = JS_NewContext(js_runtime, JS_STACK_CHUNK_SIZE)))
        Com_Error("J_Init: Failed to create a JSAPI context");

    if(!(js_gobject = JS_NewObject(js_context, &global_class, NULL, NULL)))
        Com_Error("J_Init: Failed to create a global class object");

    JS_SetGlobalObject(js_context, js_gobject);

    JS_DEFINEOBJECT(Sys);
    JS_DEFINEOBJECT(Client);
    JS_DEFINEOBJECT(Cmd);
    JS_DEFINEOBJECT(Angle);
    JS_INITCLASS(Vector, 3);
    JS_INITCLASS(Quaternion, 4);
    JS_INITCLASS(Matrix, 0);
    JS_INITCLASS_NOSTATIC(Plane, 0);

    if(!(js_rootscript = J_LoadScript("scripts/main.js")))
        Com_Error("J_Init: Unable to load main.js");

    JS_ExecuteScript(js_context, js_gobject, js_rootscript->script, &result);

    Cmd_AddCommand("js", FCmd_JS);
    Cmd_AddCommand("jsfile", FCmd_JSFile);
    Cmd_AddCommand("jsload", FCmd_JSLoad);
    Cmd_AddCommand("jsexec", FCmd_JSExec);
}

