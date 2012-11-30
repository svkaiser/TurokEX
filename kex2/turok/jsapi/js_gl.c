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
// DESCRIPTION: Javascript OpenGL Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "gl.h"

JSObject *js_objGL;

//
// js_glTexCoord2f
//

static JSBool js_glTexCoord2f(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    jsdouble tu, tv;

    if(argc != 2) return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(tu, v, 0);
    JS_GETNUMBER(tv, v, 1);

    dglTexCoord2f((float)tu, (float)tv);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// js_glVertex3f
//

static JSBool js_glVertex3f(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    jsdouble x, y, z;

    if(argc != 3) return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(x, v, 0);
    JS_GETNUMBER(y, v, 1);
    JS_GETNUMBER(z, v, 2);

    dglVertex3f((float)x, (float)y, (float)z);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// js_glLoadTexture
//

static JSBool js_glLoadTexture(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    jsdouble clamp;
    JSBool masked;
    JSString *str;
    char *bytes;

    if(argc != 3)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    if(!(str = JS_ValueToString(cx, v[0])) ||
            !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    JS_GETNUMBER(clamp, v, 1);
    JS_GETBOOL(masked, v, 2);

    Tex_CacheTextureFile(bytes, (int)clamp, masked);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// js_glBindTexture
//

static JSBool js_glBindTexture(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    JSString *str;
    char *bytes;

    if(argc != 1)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    if(!(str = JS_ValueToString(cx, v[0])) ||
            !(bytes = JS_EncodeString(cx, str)))
    {
        return JS_FALSE;
    }

    GL_BindTextureName(bytes);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// js_glSetTextureUnit
//

static JSBool js_glSetTextureUnit(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    jsdouble unit;
    JSBool enable;

    if(argc != 2) return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(unit, v, 0);
    JS_GETBOOL(enable, v, 1);

    GL_SetTextureUnit((int)unit, enable);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// js_glClearView
//

static JSBool js_glClearView(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    jsdouble rgba[4];

    if(argc != 4) return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETNUMBER(rgba[0], v, 0);
    JS_GETNUMBER(rgba[1], v, 1);
    JS_GETNUMBER(rgba[2], v, 2);
    JS_GETNUMBER(rgba[3], v, 3);

    GL_ClearView((float*)rgba);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// gl_class
//

JSClass GL_class =
{
    "GL",
    0,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

//
// GL_props
//

JSPropertySpec GL_props[] =
{
    { NULL, 0, 0, NULL, NULL }
};

//
// GL_constants
//

JSConstDoubleSpec GL_constants[] =
{
    { GLSTATE_BLEND,        "GLSTATE_BLEND",        0, { 0, 0, 0 } },
    { GLSTATE_CULL,         "GLSTATE_CULL",         0, { 0, 0, 0 } },
    { GLSTATE_TEXTURE0,     "GLSTATE_TEXTURE0",     0, { 0, 0, 0 } },
    { GLSTATE_TEXTURE1,     "GLSTATE_TEXTURE1",     0, { 0, 0, 0 } },
    { GLSTATE_TEXTURE2,     "GLSTATE_TEXTURE2",     0, { 0, 0, 0 } },
    { GLSTATE_TEXTURE3,     "GLSTATE_TEXTURE3",     0, { 0, 0, 0 } },
    { GLSTATE_ALPHATEST,    "GLSTATE_ALPHATEST",    0, { 0, 0, 0 } },
    { GLSTATE_TEXGEN_S,     "GLSTATE_TEXGEN_S",     0, { 0, 0, 0 } },
    { GLSTATE_TEXGEN_T,     "GLSTATE_TEXGEN_T",     0, { 0, 0, 0 } },
    { GL_CLAMP,             "GL_CLAMP",             0, { 0, 0, 0 } },
    { GL_REPEAT,            "GL_REPEAT",            0, { 0, 0, 0 } },
    { MAX_COORD,            "MAX_COORD",            0, { 0, 0, 0 } },
    { 0, 0, 0, { 0, 0, 0 } }
};

//
// GL_functions
//

JSFunctionSpec GL_functions[] =
{
    JS_FN("TexCoord2f",     js_glTexCoord2f,        2, 0, 0),
    JS_FN("Vertex3f",       js_glVertex3f,          3, 0, 0),
    JS_FN("LoadTexture",    js_glLoadTexture,       3, 0, 0),
    JS_FN("BindTexture",    js_glBindTexture,       1, 0, 0),
    JS_FN("SetTextureUnit", js_glSetTextureUnit,    2, 0, 0),
    JS_FN("ClearView",      js_glClearView,         4, 0, 0),
    JS_FS_END
};



