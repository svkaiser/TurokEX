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
// DESCRIPTION: Javascript Input Class
//
//-----------------------------------------------------------------------------

#include "SDL.h"

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "client.h"

JS_CLASSOBJECT(NInput);

JS_FASTNATIVE_BEGIN(NInput, add)
{
    jsval *v;
    jsdouble n;
    JSString *str;
    char *bytes;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(n, v, 0);

    if(!(str = JS_ValueToString(cx, v[1])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    Key_AddAction((byte)n, bytes);

    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NInput, process)
{
    jsval *v;

    v = JS_ARGV(cx, vp);

    IN_PollInput();

    // TEMP
    CL_ProcessEvents();

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NInput, keyPress)
{
    jsval *v;
    jsdouble key;
    JSBool keyup;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(key, v, 0);
    JS_GETBOOL(keyup, v, 1);

    Key_ExecCmd((char)key, keyup);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NInput, getActions)
{
    jsval *v;
    jsval eval;
    int i;
    JSObject *arr;

    v = JS_ARGV(cx, vp);

    if(!(arr = JS_NewArrayObject(cx, MAXACTIONS, NULL)))
        return JS_FALSE;

    JS_AddRoot(cx, &arr);

    for(i = 0; i < MAXACTIONS; i++)
    {
        eval = INT_TO_JSVAL(control.actions[i]);
        JS_SetElement(cx, arr, i, &eval);
    }

    JS_RemoveRoot(cx, &arr);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(arr));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NInput, getActionID)
{
    jsval *v;
    JSString *str;
    char *bytes;
    int action;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    if(!(str = JS_ValueToString(cx, v[0])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    action = Key_FindAction(bytes);
    JS_free(cx, bytes);

    //return JS_NewNumberValue(cx, action, vp);
    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(action));
    return JS_TRUE;
}

JS_BEGINCLASS(NInput)
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

JS_BEGINPROPS(NInput)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(NInput)
{
    JS_DEFINE_CONST(MAXKEYS, MAX_KEYS),
    JS_DEFINE_CONST(K_UNKNOWN          , SDLK_UNKNOWN),
    JS_DEFINE_CONST(K_FIRST                , SDLK_FIRST),
    JS_DEFINE_CONST(K_BACKSPACE            , SDLK_BACKSPACE),
    JS_DEFINE_CONST(K_TAB                  , SDLK_TAB),
    JS_DEFINE_CONST(K_CLEAR                , SDLK_CLEAR),
    JS_DEFINE_CONST(K_RETURN               , SDLK_RETURN),
    JS_DEFINE_CONST(K_PAUSE                , SDLK_PAUSE),
    JS_DEFINE_CONST(K_ESCAPE               , SDLK_ESCAPE),
    JS_DEFINE_CONST(K_SPACE                , SDLK_SPACE),
    JS_DEFINE_CONST(K_EXCLAIM          , SDLK_EXCLAIM),
    JS_DEFINE_CONST(K_QUOTEDBL         , SDLK_QUOTEDBL),
    JS_DEFINE_CONST(K_HASH             , SDLK_HASH),
    JS_DEFINE_CONST(K_DOLLAR               , SDLK_DOLLAR),
    JS_DEFINE_CONST(K_AMPERSAND            , SDLK_AMPERSAND),
    JS_DEFINE_CONST(K_QUOTE                , SDLK_QUOTE),
    JS_DEFINE_CONST(K_LEFTPAREN            , SDLK_LEFTPAREN),
    JS_DEFINE_CONST(K_RIGHTPAREN       , SDLK_RIGHTPAREN),
    JS_DEFINE_CONST(K_ASTERISK         , SDLK_ASTERISK),
    JS_DEFINE_CONST(K_PLUS             , SDLK_PLUS),
    JS_DEFINE_CONST(K_COMMA                , SDLK_COMMA),
    JS_DEFINE_CONST(K_MINUS                , SDLK_MINUS),
    JS_DEFINE_CONST(K_PERIOD               , SDLK_PERIOD),
    JS_DEFINE_CONST(K_SLASH                , SDLK_SLASH),
    JS_DEFINE_CONST(K_0                    , SDLK_0),
    JS_DEFINE_CONST(K_1                    , SDLK_1),
    JS_DEFINE_CONST(K_2                    , SDLK_2),
    JS_DEFINE_CONST(K_3                    , SDLK_3),
    JS_DEFINE_CONST(K_4                    , SDLK_4),
    JS_DEFINE_CONST(K_5                    , SDLK_5),
    JS_DEFINE_CONST(K_6                    , SDLK_6),
    JS_DEFINE_CONST(K_7                    , SDLK_7),
    JS_DEFINE_CONST(K_8                    , SDLK_8),
    JS_DEFINE_CONST(K_9                    , SDLK_9),
    JS_DEFINE_CONST(K_COLON                , SDLK_COLON),
    JS_DEFINE_CONST(K_SEMICOLON            , SDLK_SEMICOLON),
    JS_DEFINE_CONST(K_LESS             , SDLK_LESS),
    JS_DEFINE_CONST(K_EQUALS               , SDLK_EQUALS),
    JS_DEFINE_CONST(K_GREATER          , SDLK_GREATER),
    JS_DEFINE_CONST(K_QUESTION         , SDLK_QUESTION),
    JS_DEFINE_CONST(K_AT                   , SDLK_AT),
    JS_DEFINE_CONST(K_LEFTBRACKET      , SDLK_LEFTBRACKET),
    JS_DEFINE_CONST(K_BACKSLASH            , SDLK_BACKSLASH),
    JS_DEFINE_CONST(K_RIGHTBRACKET     , SDLK_RIGHTBRACKET),
    JS_DEFINE_CONST(K_CARET                , SDLK_CARET),
    JS_DEFINE_CONST(K_UNDERSCORE       , SDLK_UNDERSCORE),
    JS_DEFINE_CONST(K_BACKQUOTE            , SDLK_BACKQUOTE),
    JS_DEFINE_CONST(K_a                    , SDLK_a),
    JS_DEFINE_CONST(K_b                    , SDLK_b),
    JS_DEFINE_CONST(K_c                    , SDLK_c),
    JS_DEFINE_CONST(K_d                    , SDLK_d),
    JS_DEFINE_CONST(K_e                    , SDLK_e),
    JS_DEFINE_CONST(K_f                    , SDLK_f),
    JS_DEFINE_CONST(K_g                    , SDLK_g),
    JS_DEFINE_CONST(K_h                    , SDLK_h),
    JS_DEFINE_CONST(K_i                    , SDLK_i),
    JS_DEFINE_CONST(K_j                    , SDLK_j),
    JS_DEFINE_CONST(K_k                    , SDLK_k),
    JS_DEFINE_CONST(K_l                    , SDLK_l),
    JS_DEFINE_CONST(K_m                    , SDLK_m),
    JS_DEFINE_CONST(K_n                    , SDLK_n),
    JS_DEFINE_CONST(K_o                    , SDLK_o),
    JS_DEFINE_CONST(K_p                    , SDLK_p),
    JS_DEFINE_CONST(K_q                    , SDLK_q),
    JS_DEFINE_CONST(K_r                    , SDLK_r),
    JS_DEFINE_CONST(K_s                    , SDLK_s),
    JS_DEFINE_CONST(K_t                    , SDLK_t),
    JS_DEFINE_CONST(K_u                    , SDLK_u),
    JS_DEFINE_CONST(K_v                    , SDLK_v),
    JS_DEFINE_CONST(K_w                    , SDLK_w),
    JS_DEFINE_CONST(K_x                    , SDLK_x),
    JS_DEFINE_CONST(K_y                    , SDLK_y),
    JS_DEFINE_CONST(K_z                    , SDLK_z),
    JS_DEFINE_CONST(K_DELETE               , SDLK_DELETE),
    JS_DEFINE_CONST(K_KP0                  , SDLK_KP0),
    JS_DEFINE_CONST(K_KP1                  , SDLK_KP1),
    JS_DEFINE_CONST(K_KP2                  , SDLK_KP2),
    JS_DEFINE_CONST(K_KP3                  , SDLK_KP3),
    JS_DEFINE_CONST(K_KP4                  , SDLK_KP4),
    JS_DEFINE_CONST(K_KP5                  , SDLK_KP5),
    JS_DEFINE_CONST(K_KP6                  , SDLK_KP6),
    JS_DEFINE_CONST(K_KP7                  , SDLK_KP7),
    JS_DEFINE_CONST(K_KP8                  , SDLK_KP8),
    JS_DEFINE_CONST(K_KP9                  , SDLK_KP9),
    JS_DEFINE_CONST(K_KP_PERIOD            , SDLK_KP_PERIOD),
    JS_DEFINE_CONST(K_KP_DIVIDE            , SDLK_KP_DIVIDE),
    JS_DEFINE_CONST(K_KP_MULTIPLY      , SDLK_KP_MULTIPLY),
    JS_DEFINE_CONST(K_KP_MINUS         , SDLK_KP_MINUS),
    JS_DEFINE_CONST(K_KP_PLUS          , SDLK_KP_PLUS),
    JS_DEFINE_CONST(K_KP_ENTER         , SDLK_KP_ENTER),
    JS_DEFINE_CONST(K_KP_EQUALS            , SDLK_KP_EQUALS),
    JS_DEFINE_CONST(K_UP                   , SDLK_UP),
    JS_DEFINE_CONST(K_DOWN             , SDLK_DOWN),
    JS_DEFINE_CONST(K_RIGHT                , SDLK_RIGHT),
    JS_DEFINE_CONST(K_LEFT             , SDLK_LEFT),
    JS_DEFINE_CONST(K_INSERT               , SDLK_INSERT),
    JS_DEFINE_CONST(K_HOME             , SDLK_HOME),
    JS_DEFINE_CONST(K_END                  , SDLK_END),
    JS_DEFINE_CONST(K_PAGEUP               , SDLK_PAGEUP),
    JS_DEFINE_CONST(K_PAGEDOWN         , SDLK_PAGEDOWN),
    JS_DEFINE_CONST(K_F1                   , SDLK_F1),
    JS_DEFINE_CONST(K_F2                   , SDLK_F2),
    JS_DEFINE_CONST(K_F3                   , SDLK_F3),
    JS_DEFINE_CONST(K_F4                   , SDLK_F4),
    JS_DEFINE_CONST(K_F5                   , SDLK_F5),
    JS_DEFINE_CONST(K_F6                   , SDLK_F6),
    JS_DEFINE_CONST(K_F7                   , SDLK_F7),
    JS_DEFINE_CONST(K_F8                   , SDLK_F8),
    JS_DEFINE_CONST(K_F9                   , SDLK_F9),
    JS_DEFINE_CONST(K_F10                  , SDLK_F10),
    JS_DEFINE_CONST(K_F11                  , SDLK_F11),
    JS_DEFINE_CONST(K_F12                  , SDLK_F12),
    JS_DEFINE_CONST(K_F13                  , SDLK_F13),
    JS_DEFINE_CONST(K_F14                  , SDLK_F14),
    JS_DEFINE_CONST(K_F15                  , SDLK_F15),
    JS_DEFINE_CONST(K_NUMLOCK          , SDLK_NUMLOCK),
    JS_DEFINE_CONST(K_CAPSLOCK         , SDLK_CAPSLOCK),
    JS_DEFINE_CONST(K_SCROLLOCK            , SDLK_SCROLLOCK),
    JS_DEFINE_CONST(K_RSHIFT               , SDLK_RSHIFT),
    JS_DEFINE_CONST(K_LSHIFT               , SDLK_LSHIFT),
    JS_DEFINE_CONST(K_RCTRL                , SDLK_RCTRL),
    JS_DEFINE_CONST(K_LCTRL                , SDLK_LCTRL),
    JS_DEFINE_CONST(K_RALT             , SDLK_RALT),
    JS_DEFINE_CONST(K_LALT             , SDLK_LALT),
    JS_DEFINE_CONST(K_RMETA                , SDLK_RMETA),
    JS_DEFINE_CONST(K_LMETA                , SDLK_LMETA),
    JS_DEFINE_CONST(K_LSUPER               , SDLK_LSUPER),
    JS_DEFINE_CONST(K_RSUPER               , SDLK_RSUPER),
    JS_DEFINE_CONST(K_MODE             , SDLK_MODE),
    JS_DEFINE_CONST(K_COMPOSE          , SDLK_COMPOSE),
    JS_DEFINE_CONST(K_HELP             , SDLK_HELP),
    JS_DEFINE_CONST(K_PRINT                , SDLK_PRINT),
    JS_DEFINE_CONST(K_SYSREQ               , SDLK_SYSREQ),
    JS_DEFINE_CONST(K_BREAK                , SDLK_BREAK),
    JS_DEFINE_CONST(K_MENU             , SDLK_MENU),
    JS_DEFINE_CONST(K_POWER                , SDLK_POWER),
    JS_DEFINE_CONST(K_EURO             , SDLK_EURO),
    JS_DEFINE_CONST(K_UNDO             , SDLK_UNDO),
    JS_DEFINE_CONST(BUTTON_LEFT      ,SDL_BUTTON_LEFT),
    JS_DEFINE_CONST(BUTTON_MIDDLE    ,SDL_BUTTON_MIDDLE),
    JS_DEFINE_CONST(BUTTON_RIGHT     ,SDL_BUTTON_RIGHT),
    JS_DEFINE_CONST(BUTTON_WHEELUP   ,SDL_BUTTON_WHEELUP),
    JS_DEFINE_CONST(BUTTON_WHEELDOWN ,SDL_BUTTON_WHEELDOWN),
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NInput)
{
    JS_FASTNATIVE(NInput,   add,            2),
    JS_FASTNATIVE(NInput,   process,        0),
    JS_FASTNATIVE(NInput,   keyPress,       2),
    JS_FASTNATIVE(NInput,   getActions,     0),
    JS_FASTNATIVE(NInput,   getActionID,    1),
    JS_FS_END
};

JS_CLASSOBJECT(InputEvent);

JS_PROP_FUNC_GET(InputEvent)
{
    event_t *ev;

    if(!(ev = (event_t*)JS_GetInstancePrivate(cx, obj, &InputEvent_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ev->type));
        return JS_TRUE;
    case 1:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ev->data1));
        return JS_TRUE;
    case 2:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ev->data2));
        return JS_TRUE;
    case 3:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ev->data3));
        return JS_TRUE;
    case 4:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ev->data4));
        return JS_TRUE;
    default:
        return JS_FALSE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(InputEvent)
{
    event_t *ev;
    int val;

    if(!(ev = (event_t*)JS_GetInstancePrivate(cx, obj, &InputEvent_class, NULL)))
        return JS_TRUE;

    val = JSVAL_TO_INT(*vp);

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        ev->type = val;
        return JS_TRUE;
    case 1:
        ev->data1 = val;
        return JS_TRUE;
    case 2:
        ev->data2 = val;
        return JS_TRUE;
    case 3:
        ev->data3 = val;
        return JS_TRUE;
    case 4:
        ev->data4 = val;
        return JS_TRUE;
    default:
        return JS_FALSE;
    }

    return JS_TRUE;
}

JS_BEGINCLASS(InputEvent)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    InputEvent_getProperty,                     // getProperty
    InputEvent_setProperty,                     // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(InputEvent)
{
    { "type",   0,  JSPROP_ENUMERATE,   NULL, NULL },
    { "data1",  1,  JSPROP_ENUMERATE,   NULL, NULL },
    { "data2",  2,  JSPROP_ENUMERATE,   NULL, NULL },
    { "data3",  3,  JSPROP_ENUMERATE,   NULL, NULL },
    { "data4",  4,  JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(InputEvent)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(InputEvent)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(InputEvent)
{
    JS_FS_END
};
