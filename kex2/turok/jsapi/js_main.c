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
#include "jsapi.h"
#include "common.h"

#define JS_RUNTIME_HEAP_SIZE 64L * 1024L * 1024L
#define JS_STACK_CHUNK_SIZE  8192

static JSRuntime    *js_runtime;
static JSContext    *js_context;

//
// J_Error
//

static void J_Error(JSContext *cx, const char *message, JSErrorReport *report)
{
    static char buf[1024];

    if(!report)
    {
        strcat(buf, message);
        Com_CPrintf(COLOR_RED, buf);
        return;
    }

    if(report->filename)
    {
        strcat(buf, kva("%s:", report->filename));
    }

    if(report->lineno)
    {
        strcat(buf, kva("%i", report->lineno));
    }

    strcat(buf, message);

    if(JSREPORT_IS_WARNING(report->flags))
    {
        Com_Warning(buf);
        return;
    }
    else
    {
        Com_Error(buf);
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
// J_Shutdown
//

void J_Shutdown(void)
{
    if(js_runtime)
    {
        JS_DestroyRuntime(js_runtime);
    }

    JS_ShutDown();
}

//
// J_Init
//

void J_Init(void)
{
    if(!(js_runtime = JS_NewRuntime(JS_RUNTIME_HEAP_SIZE)))
    {
        Com_Error("JS_Init: Failed to initialize JSAPI runtime");
    }

    JS_SetContextCallback(js_runtime, J_ContextCallback);

    if(!(js_context = JS_NewContext(js_runtime, JS_STACK_CHUNK_SIZE)))
    {
        Com_Error("JS_Init: Failed to initialize JSAPI context");
    }
}

