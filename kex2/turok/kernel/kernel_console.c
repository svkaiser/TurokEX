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
// DESCRIPTION: Native-to-script console functions
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "kernel.h"

CVAR_EXTERNAL(developer);

char con_lastOutputBuffer[512];

//
// Con_GetLastBuffer
//

char *Con_GetLastBuffer(void)
{
    return con_lastOutputBuffer;
}

//
// Con_ProcessConsoleInput
//

kbool Con_ProcessConsoleInput(event_t *ev)
{
    gObject_t *cObject;
    gObject_t *function;
    gObject_t *evObject;
    JSContext *cx;
    jsval val;
    jsval argv;
    jsval rval;

    cx = js_context;

    if(!JS_GetProperty(cx, js_gobject, "Console", &val))
        return false;
    if(!JS_ValueToObject(cx, val, &cObject))
        return false;
    if(!JS_GetProperty(cx, cObject, "processInput", &val))
        return false;
    if(!JS_ValueToObject(cx, val, &function))
        return false;
    if(!(evObject = JPool_GetFree(&objPoolInputEvent, &InputEvent_class)))
        return false;
    if(!JS_SetPrivate(js_context, evObject, ev))
        return false;

    argv = OBJECT_TO_JSVAL(evObject);

    if(!JS_CallFunctionValue(cx, cObject, OBJECT_TO_JSVAL(function), 1, &argv, &rval))
        return false;

    return JSVAL_TO_BOOLEAN(rval);
}

//
// Con_Ticker
//

void Con_Ticker(void)
{
    gObject_t *cObject;
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval rval;

    cx = js_context;

    if(!JS_GetProperty(cx, js_gobject, "Console", &val))
        return;
    if(!JS_ValueToObject(cx, val, &cObject))
        return;
    if(!JS_GetProperty(cx, cObject, "tick", &val))
        return;
    if(!JS_ValueToObject(cx, val, &function))
        return;

    JS_CallFunctionValue(cx, cObject, OBJECT_TO_JSVAL(function), 0, NULL, &rval);
}

//
// Con_Drawer
//

void Con_Drawer(void)
{
    gObject_t *cObject;
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval rval;

    cx = js_context;

    if(!JS_GetProperty(cx, js_gobject, "Console", &val))
        return;
    if(!JS_ValueToObject(cx, val, &cObject))
        return;
    if(!JS_GetProperty(cx, cObject, "draw", &val))
        return;
    if(!JS_ValueToObject(cx, val, &function))
        return;

    JS_CallFunctionValue(cx, cObject, OBJECT_TO_JSVAL(function), 0, NULL, &rval);
}

//
// Con_Printf
//

void Con_Printf(rcolor clr, const char *s)
{
    char *buf[2];
    gObject_t *cObject;
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval *argv;
    jsval rval;
    int i;
    int nargs;

    if(js_gobject == NULL)
        return;

    if(developer.value)
    {
        memset(con_lastOutputBuffer, 0, 512);
        strcpy(con_lastOutputBuffer, kva("%f : %s",
            (Sys_GetMilliseconds() / 1000.0f), s));
    }
    
    nargs = 1;
    buf[0] = (char*)s;
    buf[1] = NULL;

    if(clr != COLOR_WHITE)
    {
        buf[1] = kva("%i,%i,%i",
            clr & 0xff,
            (clr >> 8) & 0xff,
            (clr >> 16) & 0xff);

        nargs = 2;
    }

    cx = js_context;

    if(!JS_GetProperty(cx, js_gobject, "Console", &val))
        return;
    if(!JS_ValueToObject(cx, val, &cObject))
        return;
    if(!JS_GetProperty(cx, cObject, "print", &val))
        return;
    if(!JS_ValueToObject(cx, val, &function))
        return;

    argv = (jsval*)JS_malloc(cx, sizeof(jsval) * nargs);
    for(i = 0; i < nargs; i++)
        argv[i] = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, buf[i]));

    JS_CallFunctionValue(cx, cObject, OBJECT_TO_JSVAL(function), nargs, argv, &rval);
    JS_free(cx, argv);
}
