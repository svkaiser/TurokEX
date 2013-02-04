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
// DESCRIPTION: Javascript Render Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "gl.h"

JS_CLASSOBJECT(NRender);

enum render_enum
{
    RE_VIEWPORT,
    RE_SUBCLASS
};

JS_PROP_FUNC_GET(NRender)
{
    switch(JSVAL_TO_INT(id))
    {
    case RE_VIEWPORT:
        {
            JSObject *obj;

            obj = JS_NewObject(cx, NULL, NULL, NULL);

            JS_AddRoot(cx, &obj);
            JS_DefineProperty(cx, obj, "width",  INT_TO_JSVAL(video_width),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_DefineProperty(cx, obj, "height", INT_TO_JSVAL(video_height),
                NULL, NULL, JSPROP_ENUMERATE);
            JS_RemoveRoot(cx, &obj);

            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
            return JS_TRUE;
        }

    case RE_SUBCLASS:
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_FASTNATIVE_BEGIN(NRender, clearViewPort)
{
    float f[4];
    jsdouble n1;
    jsdouble n2;
    jsdouble n3;

    JS_CHECKARGS(3);
    JS_GETNUMBER(n1, v, 0);
    JS_GETNUMBER(n2, v, 1);
    JS_GETNUMBER(n3, v, 2);

    f[0] = (float)n1;
    f[1] = (float)n2;
    f[2] = (float)n3;
    f[3] = 1;

    GL_ClearView(f);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NRender, setOrtho)
{
    JS_CHECKARGS(0);

    GL_SetOrtho();

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NRender, drawTile)
{
    JSString *str;
    char *bytes;
    jsdouble x, y;
    jsdouble tx1, ty1, tx2, ty2;
    jsdouble r, g, b, a;
    jsdouble width, height;
    jsval *v = JS_ARGV(cx, vp);

    if(argc < 1)
        return JS_FALSE;

    x = y = 0;
    tx1 = ty1 = 0;
    tx2 = ty2 = 1;
    r = g = b = a = 255;
    width = height = 8;

    if(argc >= 2)  JS_GETNUMBER(x, v, 1);
    if(argc >= 3)  JS_GETNUMBER(y, v, 2);
    if(argc >= 4)  JS_GETNUMBER(width, v, 3);
    if(argc >= 5)  JS_GETNUMBER(height, v, 4);
    if(argc >= 6)  JS_GETNUMBER(tx1, v, 5);
    if(argc >= 7)  JS_GETNUMBER(ty1, v, 6);
    if(argc >= 8)  JS_GETNUMBER(tx2, v, 7);
    if(argc >= 9)  JS_GETNUMBER(ty2, v, 8);
    if(argc >= 10) JS_GETNUMBER(r, v, 9);
    if(argc >= 11) JS_GETNUMBER(g, v, 10);
    if(argc >= 12) JS_GETNUMBER(b, v, 11);
    if(argc == 13) JS_GETNUMBER(a, v, 12);

    if(!(str = JS_ValueToString(cx, v[0])) ||
        !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    Draw_Tile(bytes, (float)x, (float)y,
        (float)tx1, (float)ty1, (float)tx2, (float)ty2,
        (float)width, (float)height,
        (byte)r, (byte)g, (byte)b, (byte)a);

    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(NRender)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    NRender_getProperty,                        // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(NRender)
{
    { "viewport",   RE_VIEWPORT,    JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "subclass",   RE_SUBCLASS,    JSPROP_ENUMERATE,                   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(NRender)
{
    { FIXED_WIDTH,  "SCREEN_WIDTH",     0, { 0, 0, 0 } },
    { FIXED_HEIGHT, "SCREEN_HEIGHT",    0, { 0, 0, 0 } },
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NRender)
{
    JS_FASTNATIVE(NRender, clearViewPort, 3),
    JS_FASTNATIVE(NRender, setOrtho, 0),
    JS_FASTNATIVE(NRender, drawTile, 13),
    JS_FS_END
};
