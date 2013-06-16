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
#include "js_class.h"
#include "debug.h"

CVAR_EXTERNAL(kf_basepath);

#define JS_RUNTIME_HEAP_SIZE 128L * 1024L * 1024L
#define JS_STACK_CHUNK_SIZE  8192

static js_scrobj_t  *js_scrobj_list[MAX_HASH];
static JSRuntime    *js_runtime     = NULL;
static js_scrobj_t  *js_rootscript  = NULL;

static unsigned int js_GCTime = 0;

JSContext   *js_context = NULL;
JSObject    *js_gobject = NULL;

jsObjectPool_t objPoolVector;
jsObjectPool_t objPoolQuaternion;
jsObjectPool_t objPoolPacket;
jsObjectPool_t objPoolAnimState;
jsObjectPool_t objPoolHost;
jsObjectPool_t objPoolPeer;
jsObjectPool_t objPoolNetEvent;
jsObjectPool_t objPoolGameActor;
jsObjectPool_t objPoolPlane;
jsObjectPool_t objPoolInputEvent;

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
        Com_Error("%s\n", buf);
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

JSObject *J_AddObject(JSClass *class,
                      JSFunctionSpec *func,
                      JSPropertySpec *prop,
                      JSConstDoubleSpec *constant,
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

    if(constant)
    {
        if(!JS_DefineConstDoubles(cx, newobj, constant))
            Com_Error("J_AddObject: Failed to define constants for class %s", name);
    }

    return newobj;
}


//
// J_NewObjectLog
//
#ifdef JS_LOGNEWOBJECTS
JSObject *J_NewObjectLog(JSContext *cx, JSClass *clasp, JSObject *proto, JSObject *parent,
                        char *file, int line)
{
    const char *name = clasp ? clasp->name : "Object";

    if(!JS_ReportWarning(cx, "New Object(%s) called at:\n%s : %i\n", name, file, line))
        Com_Printf("New Object(%s): %s : %i\n", name, file, line);

    return JS_NewObject(cx, clasp, proto, parent);
}
#endif

//
// J_CallObject
//

void J_CallObject(void *object, char **args, int nargs)
{
    JSContext *cx;
    JSObject *obj;

    cx = js_context;
    obj = (JSObject*)object;

    if(JS_ObjectIsFunction(cx, obj))
    {
        jsval rval;
        jsval *argv;

        if(nargs == 0)
        {
            argv = (jsval*)JS_malloc(cx, sizeof(jsval) * 1);
            argv[0] = JSVAL_VOID;
        }
        else
        {
            int i;

            argv = (jsval*)JS_malloc(cx, sizeof(jsval) * nargs);
            for(i = 0; i < nargs; i++)
                argv[i] = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, args[i]));
        }

        JS_CallFunctionValue(cx, js_gobject, OBJECT_TO_JSVAL(obj), nargs, argv, &rval);
        JS_free(cx, argv);
    }
}

//
// J_GetObjectElement
//

jsval J_GetObjectElement(JSContext *cx, JSObject *object, jsint index)
{
    jsuint length;
    jsval val;

    if(object != NULL &&
        JS_IsArrayObject(cx, object) &&
        JS_GetArrayLength(cx, object, &length) &&
        length > 0 &&
        JS_GetElement(cx, object, index, &val) &&
        !JSVAL_IS_NULL(val))
    {
        return val;
    }

    JS_SET_RVAL(cx, &val, JSVAL_NULL);
    return val;
}

//
// J_AllocFloatArray
//

jsuint J_AllocFloatArray(JSContext *cx, JSObject *object, float **arr, JSBool fixed)
{
    float *ptr;
    jsuint length;
    jsuint i;

    if(!JS_IsArrayObject(cx, object))
        return 0;

    if(!JS_GetArrayLength(cx, object, &length))
        return 0;

    if(!fixed)
        ptr = (float*)JS_malloc(cx, sizeof(float) * length);
    else
        ptr = *arr;

    for(i = 0; i < length; i++)
    {
        jsval val;
        jsdouble n;

        val = J_GetObjectElement(cx, object, i);

        if(JSVAL_IS_NULL(val))
        {
            ptr[i] = 0;
            continue;
        }

        if(!JS_ValueToNumber(cx, val, &n))
            return 0;

        ptr[i] = (float)n;
    }

    if(!fixed)
        *arr = ptr;

    return length;
}

//
// J_AllocWordArray
//

jsuint J_AllocWordArray(JSContext *cx, JSObject *object, word **arr, JSBool fixed)
{
    word *ptr;
    jsuint length;
    jsuint i;

    if(!JS_IsArrayObject(cx, object))
        return 0;

    if(!JS_GetArrayLength(cx, object, &length))
        return 0;

    if(!fixed)
        ptr = (word*)JS_malloc(cx, sizeof(word) * length);
    else
        ptr = *arr;

    for(i = 0; i < length; i++)
    {
        jsval val;
        jsdouble n;

        val = J_GetObjectElement(cx, object, i);

        if(JSVAL_IS_NULL(val))
        {
            ptr[i] = 0;
            continue;
        }

        if(!JS_ValueToNumber(cx, val, &n))
            return 0;

        ptr[i] = (word)n;
    }

    if(!fixed)
        *arr = ptr;

    return length;
}

//
// J_AllocByteArray
//

jsuint J_AllocByteArray(JSContext *cx, JSObject *object, byte **arr, JSBool fixed)
{
    byte *ptr;
    jsuint length;
    jsuint i;

    if(!JS_IsArrayObject(cx, object))
        return 0;

    if(!JS_GetArrayLength(cx, object, &length))
        return 0;

    if(!fixed)
        ptr = (byte*)JS_malloc(cx, sizeof(byte) * length);
    else
        ptr = *arr;

    for(i = 0; i < length; i++)
    {
        jsval val;
        jsdouble n;

        val = J_GetObjectElement(cx, object, i);

        if(JSVAL_IS_NULL(val))
        {
            ptr[i] = 0;
            continue;
        }

        if(!JS_ValueToNumber(cx, val, &n))
            return 0;

        ptr[i] = (byte)n;
    }

    if(!fixed)
        *arr = ptr;

    return length;
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
        JS_ReportPendingException(cx);
        Z_Free(scrobj);
        return NULL;
    }

    JS_ReportPendingException(cx);

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
        return NULL;

    scrobj = J_FindScript(name);

    if(scrobj == NULL)
    {
        char *file;

        if((size = KF_OpenFileCache(name, &file, PU_STATIC)) == 0)
        {
            if((size = KF_ReadTextFile(name, &file)) == -1)
                return NULL;
        }

        scrobj = J_LoadScriptObject(name, file, size);
        Z_Free(file);
    }

    return scrobj;
}

//
// J_CompileAndRunScript
//

void J_CompileAndRunScript(const char *name)
{
    J_ExecScriptObj(J_LoadScript(name));
}

//
// J_GetGCTime
//

unsigned int J_GetGCTime(void)
{
    return js_GCTime;
}

//
// J_GarbageCollect
//

CVAR_EXTERNAL(cl_maxfps);
CVAR_EXTERNAL(developer);

void J_GarbageCollect(void)
{
    JPool_ReleaseObjects(&objPoolVector);
    JPool_ReleaseObjects(&objPoolQuaternion);
    JPool_ReleaseObjects(&objPoolPlane);
    JPool_ReleaseObjects(&objPoolPacket);
    JPool_ReleaseObjects(&objPoolAnimState);
    JPool_ReleaseObjects(&objPoolHost);
    JPool_ReleaseObjects(&objPoolPeer);
    JPool_ReleaseObjects(&objPoolNetEvent);
    JPool_ReleaseObjects(&objPoolGameActor);
    JPool_ReleaseObjects(&objPoolInputEvent);

    if(developer.value)
        js_GCTime = Sys_GetMilliseconds();

    JS_MaybeGC(js_context);

#ifdef JS_LOGNEWOBJECTS
    Com_Printf("Garbage Collect...\n\n");
#endif

    if(developer.value)
    {
        js_GCTime = Sys_GetMilliseconds() - js_GCTime;
        if(js_GCTime > (unsigned int)(((1.0f / cl_maxfps.value) * 1000) / 4))
        {
            Com_DPrintf("GC hitch: %ims\n", js_GCTime);
        }
    }
}

//
// J_ExecBuffer
//

void J_ExecBuffer(char *buffer)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    jsval result;

    if(JS_BufferIsCompilableUnit(cx, obj, buffer, strlen(buffer)-1))
    {
        JSScript *script;

        JS_ClearPendingException(cx);
        if(script = JS_CompileScript(cx, obj, buffer,
            strlen(buffer)-1, "execBuffer", 1))
        {
            JS_ExecuteScript(cx, obj, script, &result);
            JS_MaybeGC(cx);
            JS_DestroyScript(cx, script);
        }
    }
}

//
// J_ExecScriptObj
//

void J_ExecScriptObj(js_scrobj_t *scobj)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    jsval result;

    if(scobj == NULL)
        return;

    if(!JS_ExecuteScript(cx, obj, scobj->script, &result))
    {
        JS_ReportPendingException(cx);
        Com_Warning("Unable to execute %s\n", scobj->name);
    }
}

#if 0
//
// J_LogScript
//

void J_LogScript(JSFunction *func, JSScript *scr, JSContext *const_cx, JSBool entering)
{
    JSContext *cx = const_cx;
    JSString *name = JS_GetFunctionId((JSFunction*)func);
    const char *entExit;
    const char *nameStr;
 
    /* build a C string for the function's name */
    if(!name) nameStr = "Unnamed function";
    else nameStr = JS_EncodeString(cx, name);
 
    /* build a string for whether we're entering or exiting */
    if(entering)  entExit = "Entering";
    else entExit = "Exiting";
 
    /* output information about the trace */
    Com_Printf("%s JavaScript function: %s at time: %i",
        entExit, nameStr, Sys_GetMilliseconds());
}
#endif

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

#if 0
//
// FCmd_LogJS
//

static void FCmd_LogJS(void)
{
    kbool enable;

    if(Cmd_GetArgc() != 2)
    {
        Com_Printf("Usage: jslog <1=enable, 0=disable>\n");
        return;
    }

    enable = atoi(Cmd_GetArgv(1));

    if(enable)
        JS_SetFunctionCallback(js_context, J_LogScript);
    else
        JS_SetFunctionCallback(js_context, NULL);
}
#endif

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

            JS_ReportPendingException(cx);
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
        JS_ReportPendingException(cx);
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
    if(!(js_runtime = JS_NewRuntime(JS_RUNTIME_HEAP_SIZE)))
        Com_Error("J_Init: Failed to initialize JSAPI runtime");

    JS_SetContextCallback(js_runtime, J_ContextCallback);

    if(!(js_context = JS_NewContext(js_runtime, JS_STACK_CHUNK_SIZE)))
        Com_Error("J_Init: Failed to create a JSAPI context");

    if(!(js_gobject = JS_NewObject(js_context, &global_class, NULL, NULL)))
        Com_Error("J_Init: Failed to create a global class object");

    JS_SetGlobalObject(js_context, js_gobject);

    JS_DEFINEOBJECT(Sys);
    JS_DEFINEOBJECT(NInput);
    JS_DEFINEOBJECT(GL);
    JS_DEFINEOBJECT(Net);
    JS_DEFINEOBJECT(NClient);
    JS_DEFINEOBJECT(NServer);
    JS_DEFINEOBJECT(NRender);
    JS_DEFINEOBJECT(NGame);
    JS_DEFINEOBJECT(Angle);
    JS_DEFINEOBJECT(Physics);
    JS_DEFINEOBJECT(Level);
    JS_DEFINEOBJECT(Snd);
    JS_DEFINEOBJECT(ClientPlayer);
    JS_INITCLASS(Vector, 3);
    JS_INITCLASS(Quaternion, 4);
    JS_INITCLASS(Matrix, 0);
    JS_INITCLASS(AnimState, 0);
    JS_INITCLASS(Canvas, 0);
    JS_INITCLASS(Font, 0);
    JS_INITCLASS_NOCONSTRUCTOR(WorldState, 0);
    JS_INITCLASS_NOCONSTRUCTOR(NetEvent, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Packet, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Peer, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Host, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Model, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Animation, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Texture, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Plane, 0);
    JS_INITCLASS_NOCONSTRUCTOR(GameActor, 0);
    JS_INITCLASS_NOCONSTRUCTOR(AI, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Component, 0);
    JS_INITCLASS_NOCONSTRUCTOR(InputEvent, 0);
    JS_INITCLASS_NOCONSTRUCTOR(Command, 0);

    if(!(js_rootscript = J_LoadScript("scripts/main.js")))
        Com_Error("J_Init: Unable to load main.js");

    JPool_Initialize(&objPoolVector, 128, &Vector_class);
    JPool_Initialize(&objPoolQuaternion, 128, &Quaternion_class);
    JPool_Initialize(&objPoolPlane, 64, &Plane_class);
    JPool_Initialize(&objPoolPacket, 64, &Packet_class);
    JPool_Initialize(&objPoolAnimState, 32, &AnimState_class);
    JPool_Initialize(&objPoolHost, 32, &Host_class);
    JPool_Initialize(&objPoolPeer, 32, &Peer_class);
    JPool_Initialize(&objPoolNetEvent, 32, &NetEvent_class);
    JPool_Initialize(&objPoolGameActor, 32, &GameActor_class);
    JPool_Initialize(&objPoolInputEvent, 8, &InputEvent_class);

    J_ExecScriptObj(js_rootscript);
    JClass_InitObject();

    Cmd_AddCommand("js", FCmd_JS);
    Cmd_AddCommand("jsfile", FCmd_JSFile);
    Cmd_AddCommand("jsload", FCmd_JSLoad);
    Cmd_AddCommand("jsexec", FCmd_JSExec);
#if 0
    Cmd_AddCommand("jslog", FCmd_LogJS);
#endif

    Debug_RegisterPerfStatVar((float*)&js_GCTime, "Script GC Time", false);
}

