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

#define JS_MAX_GL_PARAMS    16

JS_CLASSOBJECT(GL);

JS_FASTNATIVE_BEGIN(GL, isEnabled)
{
    JS_CHECKARGS(1);
    JS_CHECKINTEGER(0);

    JS_RVAL(cx, vp) = BOOLEAN_TO_JSVAL(glIsEnabled(JSVAL_TO_INT(JS_ARG(0))));  
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, getBoolean)
{
    GLboolean params;

    JS_CHECKARGS(1);
    JS_CHECKINTEGER(0);

    dglGetBooleanv(JSVAL_TO_INT(JS_ARG(0)), &params);  
    JS_RVAL(cx, vp) = BOOLEAN_TO_JSVAL(params);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, drawBuffer)
{
    JS_CHECKARGS(1);
    JS_CHECKINTEGER(0);

    dglDrawBuffer(JSVAL_TO_INT(JS_ARG(0)));  

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, readBuffer)
{
    JS_CHECKARGS(1);
    JS_CHECKINTEGER(0);

    dglReadBuffer(JSVAL_TO_INT(JS_ARG(0)));  

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, accum)
{
    jsdouble val;

    JS_CHECKARGS(2);
    JS_CHECKINTEGER(0);
    JS_GETNUMBER(val, v, 1);

    dglAccum(JSVAL_TO_INT(JS_ARG(0)), (float)val);  

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, stencilFunc)
{
    GLuint mask;

    JS_CHECKARGS(3);
    JS_CHECKINTEGER(0);
    JS_CHECKINTEGER(1);
    JS_CHECKNUMBER(2);

    if(JS_ARG(2) == INT_TO_JSVAL(-1))
        mask = 0xffffffff;
    else
        mask = JSVAL_TO_INT(JS_ARG(2));

    dglStencilFunc(JSVAL_TO_INT(JS_ARG(0)), JSVAL_TO_INT(JS_ARG(1)), mask);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;   
}

JS_FASTNATIVE_BEGIN(GL, stencilOp)
{
    JS_CHECKARGS(3);
    JS_CHECKINTEGER(0);
    JS_CHECKINTEGER(1);
    JS_CHECKINTEGER(2);

    dglStencilOp(JSVAL_TO_INT(JS_ARG(0)), JSVAL_TO_INT(JS_ARG(1)), JSVAL_TO_INT(JS_ARG(2)));

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, stencilMask)
{
    GLuint mask;

    JS_CHECKARGS(1);
    JS_CHECKNUMBER(0);

    if(JS_ARG(0) == INT_TO_JSVAL(-1) )
        mask = 0xffffffff;
    else
        mask = JSVAL_TO_INT(JS_ARG(0));

    dglStencilMask(mask);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, alphaFunc)
{
    jsdouble val;

    JS_CHECKARGS(2);
    JS_CHECKINTEGER(0);
    JS_GETNUMBER(val, v, 1);

    dglAlphaFunc(JSVAL_TO_INT(JS_ARG(0)), (float)val);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, flush)
{
    dglFlush();

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, finish)
{
    dglFinish();

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, fog)
{
    jsval *v = JS_ARGV(cx, vp);

    JS_CHECKINTEGER(0);

    JS_RVAL(cx, vp) = JSVAL_VOID;

    if(JSVAL_IS_INT(JS_ARG(1)))
    {
        dglFogi(JSVAL_TO_INT(JS_ARG(0)), JSVAL_TO_INT(JS_ARG(1)));
        return JS_TRUE;
    }

    if(JSVAL_IS_DOUBLE(JS_ARG(1)))
    {
        jsdouble param;
        
        JS_GETNUMBER(param, v, 1);
        dglFogf(JSVAL_TO_INT(JS_ARG(0)), (float)param);
        return JS_TRUE;
    }

    if(JSVAL_IS_OBJECT(JS_ARG(1)))
    {
        JSObject *obj;
        float params[JS_MAX_GL_PARAMS];
        float *p;

        JS_GETOBJECT(obj, v, 1);

        p = (float*)params;

        if(J_AllocFloatArray(cx, obj, &p, JS_TRUE) == 0)
            return JS_FALSE;

        dglFogfv(JSVAL_TO_INT(JS_ARG(0)), params);
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_FASTNATIVE_BEGIN(GL, hint)
{
    JS_CHECKARGS(2);
    JS_CHECKINTEGER(0);
    JS_CHECKINTEGER(1);

    dglHint(JSVAL_TO_INT(JS_ARG(0)), JSVAL_TO_INT(JS_ARG(1)));

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, texParameter)
{
    JS_CHECKARGS(2);
    JS_CHECKINTEGER(0);
    JS_CHECKINTEGER(1);

    dglTexParameteri(GL_TEXTURE_2D, JSVAL_TO_INT(JS_ARG(0)), JSVAL_TO_INT(JS_ARG(1)));

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, enable)
{
    JS_CHECKARGS(1);
    JS_CHECKINTEGER(0);

    dglEnable(JSVAL_TO_INT(JS_ARG(0)));

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, disable)
{
    JS_CHECKARGS(1);
    JS_CHECKINTEGER(0);

    dglDisable(JSVAL_TO_INT(JS_ARG(0)));

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

#define JS_GL_STATEFUNC(name, state)                        \
JS_FASTNATIVE_BEGIN(GL, name)                               \
{                                                           \
    JS_CHECKARGS(1);                                        \
    JS_CHECKINTEGER(0);                                     \
    GL_SetState(state, JSVAL_TO_INT(JS_ARG(0)));            \
    JS_RVAL(cx, vp) = JSVAL_VOID;                           \
    return JS_TRUE;                                         \
}

JS_GL_STATEFUNC(setBlend, GLSTATE_BLEND);
JS_GL_STATEFUNC(setCulling, GLSTATE_CULL);
JS_GL_STATEFUNC(enableTexture, GLSTATE_TEXTURE0);
JS_GL_STATEFUNC(setAlphaTest, GLSTATE_ALPHATEST);
JS_GL_STATEFUNC(setTexgen_S, GLSTATE_TEXGEN_S);
JS_GL_STATEFUNC(setTexgen_T, GLSTATE_TEXGEN_T);

JS_FASTNATIVE_BEGIN(GL, enableClientState)
{
    JSBool toggle;
    unsigned int state;

    JS_CHECKARGS(2);
    JS_GETBOOL(toggle, v, 0);
    JS_GETINTEGER(state, 1);

    if(toggle)
        dglEnableClientState(state);
    else
        dglDisableClientState(state);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, bindTexture)
{
    JSObject *obj;
    JSString *str;
    char *bytes;
    texture_t *texture;

    JS_CHECKARGS(1);

    if(JSVAL_IS_OBJECT(JS_ARG(0)))
    {
        JS_GETOBJECT(obj, v, 0);
        JS_GET_PRIVATE_DATA(obj, &Texture_class, texture_t, texture);

        GL_BindTexture(texture);
    }
    else if(JSVAL_IS_STRING(JS_ARG(0)))
    {
        JS_GETSTRING(str, bytes, v, 0);

        GL_BindTextureName(bytes);
        JS_free(cx, bytes);
    }
    else
    {
        // bad argument type
        return JS_FALSE;
    }

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, vertex)
{
    jsdouble x, y, z;
    jsdouble tu, tv;
    jsdouble nx, ny, nz;
    jsdouble r, g, b, a;

    JS_CHECKARGS(12);

    JS_GETNUMBER(x, v, 0);
    JS_GETNUMBER(y, v, 1);
    JS_GETNUMBER(z, v, 2);
    JS_GETNUMBER(tu, v, 3);
    JS_GETNUMBER(tv, v, 4);
    JS_GETNUMBER(nx, v, 5);
    JS_GETNUMBER(ny, v, 6);
    JS_GETNUMBER(nz, v, 7);
    JS_GETNUMBER(r, v, 8);
    JS_GETNUMBER(g, v, 9);
    JS_GETNUMBER(b, v, 10);
    JS_GETNUMBER(a, v, 11);

    GL_Vertex((float)x, (float)y, (float)z,
        (float)tu, (float)tv,
        (float)nx, (float)ny, (float)nz,
        (byte)r, (byte)g, (byte)b, (byte)a);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, triangle)
{
    JS_CHECKARGS(3);

    JS_CHECKINTEGER(0);
    JS_CHECKINTEGER(1);
    JS_CHECKINTEGER(2);

    GL_Triangle(
        JSVAL_TO_INT(JS_ARG(0)),
        JSVAL_TO_INT(JS_ARG(1)),
        JSVAL_TO_INT(JS_ARG(2)));

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, drawElements)
{
    JS_CHECKARGS(0);

    GL_DrawElements2();

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, pushMatrix)
{
    dglPushMatrix();

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, popMatrix)
{
    dglPopMatrix();

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, multMatrix)
{
    mtx_t *mtx = NULL;

    JS_CHECKARGS(1);
    JS_GETMATRIX(mtx, v, 0);

    dglMultMatrixf(*mtx);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, begin)
{
    JS_CHECKARGS(1);
    JS_CHECKINTEGER(0);

    dglBegin(JSVAL_TO_INT(JS_ARG(0)));

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, end)
{
    JS_CHECKARGS(0);

    dglEnd();

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, vertex2f)
{
    jsdouble x, y;

    JS_CHECKARGS(2);
    JS_GETNUMBER(x, v, 0);
    JS_GETNUMBER(y, v, 1);

    dglVertex2d(x, y);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, vertex3f)
{
    jsdouble x, y, z;

    JS_CHECKARGS(3);
    JS_GETNUMBER(x, v, 0);
    JS_GETNUMBER(y, v, 1);
    JS_GETNUMBER(z, v, 2);

    dglVertex3d(x, y, z);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, color4)
{
    int r, g, b, a;

    JS_CHECKARGS(4);
    JS_GETINTEGER(r, 0);
    JS_GETINTEGER(g, 1);
    JS_GETINTEGER(b, 2);
    JS_GETINTEGER(a, 3);

    dglColor4ub(r, g, b, a);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, translate)
{
    jsdouble x, y, z;

    JS_CHECKARGS(3);
    JS_GETNUMBER(x, v, 0);
    JS_GETNUMBER(y, v, 1);
    JS_GETNUMBER(z, v, 2);

    dglTranslated(x, y, z);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, scale)
{
    jsdouble x, y, z;

    JS_CHECKARGS(3);
    JS_GETNUMBER(x, v, 0);
    JS_GETNUMBER(y, v, 1);
    JS_GETNUMBER(z, v, 2);

    dglScaled(x, y, z);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(GL, rotate)
{
    jsdouble a, x, y, z;

    JS_CHECKARGS(4);
    JS_GETNUMBER(a, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);
    JS_GETNUMBER(z, v, 3);

    dglRotated(a, x, y, z);

    JS_RVAL(cx, vp) = JSVAL_VOID;
    return JS_TRUE;
}

JS_BEGINCLASS(GL)
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

JS_BEGINPROPS(GL)
{
    { NULL, 0, 0, NULL, NULL }
};

#define GL_CONST(name)          JS_DEFINE_CONST(name, GL_##name)
#define GL_CONST_EX(name, ext)  JS_DEFINE_CONST(name, GL_##name##_##ext)

JS_BEGINCONST(GL)
{
    GL_CONST(ACCUM),
    GL_CONST(LOAD),
    GL_CONST(RETURN),
    GL_CONST(MULT),
    GL_CONST(ADD),

    GL_CONST(NEVER),
    GL_CONST(LESS),
    GL_CONST(EQUAL),
    GL_CONST(LEQUAL),
    GL_CONST(GREATER),
    GL_CONST(NOTEQUAL),
    GL_CONST(GEQUAL),
    GL_CONST(ALWAYS),

    GL_CONST(CURRENT_BIT),
    GL_CONST(POINT_BIT),
    GL_CONST(LINE_BIT),
    GL_CONST(POLYGON_BIT),
    GL_CONST(POLYGON_STIPPLE_BIT),
    GL_CONST(PIXEL_MODE_BIT),
    GL_CONST(LIGHTING_BIT),
    GL_CONST(FOG_BIT),
    GL_CONST(DEPTH_BUFFER_BIT),
    GL_CONST(ACCUM_BUFFER_BIT),
    GL_CONST(STENCIL_BUFFER_BIT),
    GL_CONST(VIEWPORT_BIT),
    GL_CONST(TRANSFORM_BIT),
    GL_CONST(ENABLE_BIT),
    GL_CONST(COLOR_BUFFER_BIT),
    GL_CONST(HINT_BIT),
    GL_CONST(EVAL_BIT),
    GL_CONST(LIST_BIT),
    GL_CONST(TEXTURE_BIT),
    GL_CONST(SCISSOR_BIT),
    GL_CONST(ALL_ATTRIB_BITS),

    GL_CONST(POINTS),
    GL_CONST(LINES),
    GL_CONST(LINE_LOOP),
    GL_CONST(LINE_STRIP),
    GL_CONST(TRIANGLES),
    GL_CONST(TRIANGLE_STRIP),
    GL_CONST(TRIANGLE_FAN),
    GL_CONST(QUADS),
    GL_CONST(QUAD_STRIP),
    GL_CONST(POLYGON),

    GL_CONST(ZERO),
    GL_CONST(ONE),
    GL_CONST(SRC_COLOR),
    GL_CONST(ONE_MINUS_SRC_COLOR),
    GL_CONST(SRC_ALPHA),
    GL_CONST(ONE_MINUS_SRC_ALPHA),
    GL_CONST(DST_ALPHA),
    GL_CONST(ONE_MINUS_DST_ALPHA),

    GL_CONST(DST_COLOR),
    GL_CONST(ONE_MINUS_DST_COLOR),
    GL_CONST(SRC_ALPHA_SATURATE),

    GL_CONST(TRUE),
    GL_CONST(FALSE),

    GL_CONST(CLIP_PLANE0),
    GL_CONST(CLIP_PLANE1),
    GL_CONST(CLIP_PLANE2),
    GL_CONST(CLIP_PLANE3),
    GL_CONST(CLIP_PLANE4),
    GL_CONST(CLIP_PLANE5),

    GL_CONST(BYTE),
    GL_CONST(UNSIGNED_BYTE),
    GL_CONST(SHORT),
    GL_CONST(UNSIGNED_SHORT),
    GL_CONST(INT),
    GL_CONST(UNSIGNED_INT),
    GL_CONST(FLOAT),
    GL_CONST(2_BYTES),
    GL_CONST(3_BYTES),
    GL_CONST(4_BYTES),
    GL_CONST(DOUBLE),

    GL_CONST(NONE),
    GL_CONST(FRONT_LEFT),
    GL_CONST(FRONT_RIGHT),
    GL_CONST(BACK_LEFT),
    GL_CONST(BACK_RIGHT),
    GL_CONST(FRONT),
    GL_CONST(BACK),
    GL_CONST(LEFT),
    GL_CONST(RIGHT),
    GL_CONST(FRONT_AND_BACK),
    GL_CONST(AUX0),
    GL_CONST(AUX1),
    GL_CONST(AUX2),
    GL_CONST(AUX3),

    GL_CONST(NO_ERROR),
    GL_CONST(INVALID_ENUM),
    GL_CONST(INVALID_VALUE),
    GL_CONST(INVALID_OPERATION),
    GL_CONST(STACK_OVERFLOW),
    GL_CONST(STACK_UNDERFLOW),
    GL_CONST(OUT_OF_MEMORY),

    GL_CONST(2D),
    GL_CONST(3D),
    GL_CONST(3D_COLOR),
    GL_CONST(3D_COLOR_TEXTURE),
    GL_CONST(4D_COLOR_TEXTURE),

    GL_CONST(PASS_THROUGH_TOKEN),
    GL_CONST(POINT_TOKEN),
    GL_CONST(LINE_TOKEN),
    GL_CONST(POLYGON_TOKEN),
    GL_CONST(BITMAP_TOKEN),
    GL_CONST(DRAW_PIXEL_TOKEN),
    GL_CONST(COPY_PIXEL_TOKEN),
    GL_CONST(LINE_RESET_TOKEN),

    GL_CONST(EXP),
    GL_CONST(EXP2),

    GL_CONST(CW),
    GL_CONST(CCW),

    GL_CONST(COEFF),
    GL_CONST(ORDER),
    GL_CONST(DOMAIN),

    GL_CONST(CURRENT_COLOR),
    GL_CONST(CURRENT_INDEX),
    GL_CONST(CURRENT_NORMAL),
    GL_CONST(CURRENT_TEXTURE_COORDS),
    GL_CONST(CURRENT_RASTER_COLOR),
    GL_CONST(CURRENT_RASTER_INDEX),
    GL_CONST(CURRENT_RASTER_TEXTURE_COORDS),
    GL_CONST(CURRENT_RASTER_POSITION),
    GL_CONST(CURRENT_RASTER_POSITION_VALID),
    GL_CONST(CURRENT_RASTER_DISTANCE),
    GL_CONST(POINT_SMOOTH),
    GL_CONST(POINT_SIZE),
    GL_CONST(POINT_SIZE_RANGE),
    GL_CONST(POINT_SIZE_GRANULARITY),
    GL_CONST(LINE_SMOOTH),
    GL_CONST(LINE_WIDTH),
    GL_CONST(LINE_WIDTH_RANGE),
    GL_CONST(LINE_WIDTH_GRANULARITY),
    GL_CONST(LINE_STIPPLE),
    GL_CONST(LINE_STIPPLE_PATTERN),
    GL_CONST(LINE_STIPPLE_REPEAT),
    GL_CONST(LIST_MODE),
    GL_CONST(MAX_LIST_NESTING),
    GL_CONST(LIST_BASE),
    GL_CONST(LIST_INDEX),
    GL_CONST(POLYGON_MODE),
    GL_CONST(POLYGON_SMOOTH),
    GL_CONST(POLYGON_STIPPLE),
    GL_CONST(EDGE_FLAG),
    GL_CONST(CULL_FACE),
    GL_CONST(CULL_FACE_MODE),
    GL_CONST(FRONT_FACE),
    GL_CONST(LIGHTING),
    GL_CONST(LIGHT_MODEL_LOCAL_VIEWER),
    GL_CONST(LIGHT_MODEL_TWO_SIDE),
    GL_CONST(LIGHT_MODEL_AMBIENT),
    GL_CONST(SHADE_MODEL),
    GL_CONST(COLOR_MATERIAL_FACE),
    GL_CONST(COLOR_MATERIAL_PARAMETER),
    GL_CONST(COLOR_MATERIAL),
    GL_CONST(FOG),
    GL_CONST(FOG_INDEX),
    GL_CONST(FOG_DENSITY),
    GL_CONST(FOG_START),
    GL_CONST(FOG_END),
    GL_CONST(FOG_MODE),
    GL_CONST(FOG_COLOR),
    GL_CONST(DEPTH_RANGE),
    GL_CONST(DEPTH_TEST),
    GL_CONST(DEPTH_WRITEMASK),
    GL_CONST(DEPTH_CLEAR_VALUE),
    GL_CONST(DEPTH_FUNC),
    GL_CONST(ACCUM_CLEAR_VALUE),
    GL_CONST(STENCIL_TEST),
    GL_CONST(STENCIL_CLEAR_VALUE),
    GL_CONST(STENCIL_FUNC),
    GL_CONST(STENCIL_VALUE_MASK),
    GL_CONST(STENCIL_FAIL),
    GL_CONST(STENCIL_PASS_DEPTH_FAIL),
    GL_CONST(STENCIL_PASS_DEPTH_PASS),
    GL_CONST(STENCIL_REF),
    GL_CONST(STENCIL_WRITEMASK),
    GL_CONST(MATRIX_MODE),
    GL_CONST(NORMALIZE),
    GL_CONST(VIEWPORT),
    GL_CONST(MODELVIEW_STACK_DEPTH),
    GL_CONST(PROJECTION_STACK_DEPTH),
    GL_CONST(TEXTURE_STACK_DEPTH),
    GL_CONST(MODELVIEW_MATRIX),
    GL_CONST(PROJECTION_MATRIX),
    GL_CONST(TEXTURE_MATRIX),
    GL_CONST(ATTRIB_STACK_DEPTH),
    GL_CONST(CLIENT_ATTRIB_STACK_DEPTH),
    GL_CONST(ALPHA_TEST),
    GL_CONST(ALPHA_TEST_FUNC),
    GL_CONST(ALPHA_TEST_REF),
    GL_CONST(DITHER),
    GL_CONST(BLEND_DST),
    GL_CONST(BLEND_SRC),
    GL_CONST(BLEND),
    GL_CONST(LOGIC_OP_MODE),
    GL_CONST(INDEX_LOGIC_OP),
    GL_CONST(COLOR_LOGIC_OP),
    GL_CONST(AUX_BUFFERS),
    GL_CONST(DRAW_BUFFER),
    GL_CONST(READ_BUFFER),
    GL_CONST(SCISSOR_BOX),
    GL_CONST(SCISSOR_TEST),
    GL_CONST(INDEX_CLEAR_VALUE),
    GL_CONST(INDEX_WRITEMASK),
    GL_CONST(COLOR_CLEAR_VALUE),
    GL_CONST(COLOR_WRITEMASK),
    GL_CONST(INDEX_MODE),
    GL_CONST(RGBA_MODE),
    GL_CONST(DOUBLEBUFFER),
    GL_CONST(STEREO),
    GL_CONST(RENDER_MODE),
    GL_CONST(PERSPECTIVE_CORRECTION_HINT),
    GL_CONST(POINT_SMOOTH_HINT),
    GL_CONST(LINE_SMOOTH_HINT),
    GL_CONST(POLYGON_SMOOTH_HINT),
    GL_CONST(FOG_HINT),
    GL_CONST(TEXTURE_GEN_S),
    GL_CONST(TEXTURE_GEN_T),
    GL_CONST(TEXTURE_GEN_R),
    GL_CONST(TEXTURE_GEN_Q),
    GL_CONST(PIXEL_MAP_I_TO_I),
    GL_CONST(PIXEL_MAP_S_TO_S),
    GL_CONST(PIXEL_MAP_I_TO_R),
    GL_CONST(PIXEL_MAP_I_TO_G),
    GL_CONST(PIXEL_MAP_I_TO_B),
    GL_CONST(PIXEL_MAP_I_TO_A),
    GL_CONST(PIXEL_MAP_R_TO_R),
    GL_CONST(PIXEL_MAP_G_TO_G),
    GL_CONST(PIXEL_MAP_B_TO_B),
    GL_CONST(PIXEL_MAP_A_TO_A),
    GL_CONST(PIXEL_MAP_I_TO_I_SIZE),
    GL_CONST(PIXEL_MAP_S_TO_S_SIZE),
    GL_CONST(PIXEL_MAP_I_TO_R_SIZE),
    GL_CONST(PIXEL_MAP_I_TO_G_SIZE),
    GL_CONST(PIXEL_MAP_I_TO_B_SIZE),
    GL_CONST(PIXEL_MAP_I_TO_A_SIZE),
    GL_CONST(PIXEL_MAP_R_TO_R_SIZE),
    GL_CONST(PIXEL_MAP_G_TO_G_SIZE),
    GL_CONST(PIXEL_MAP_B_TO_B_SIZE),
    GL_CONST(PIXEL_MAP_A_TO_A_SIZE),
    GL_CONST(UNPACK_SWAP_BYTES),
    GL_CONST(UNPACK_LSB_FIRST),
    GL_CONST(UNPACK_ROW_LENGTH),
    GL_CONST(UNPACK_SKIP_ROWS),
    GL_CONST(UNPACK_SKIP_PIXELS),
    GL_CONST(UNPACK_ALIGNMENT),
    GL_CONST(PACK_SWAP_BYTES),
    GL_CONST(PACK_LSB_FIRST),
    GL_CONST(PACK_ROW_LENGTH),
    GL_CONST(PACK_SKIP_ROWS),
    GL_CONST(PACK_SKIP_PIXELS),
    GL_CONST(PACK_ALIGNMENT),
    GL_CONST(MAP_COLOR),
    GL_CONST(MAP_STENCIL),
    GL_CONST(INDEX_SHIFT),
    GL_CONST(INDEX_OFFSET),
    GL_CONST(RED_SCALE),
    GL_CONST(RED_BIAS),
    GL_CONST(ZOOM_X),
    GL_CONST(ZOOM_Y),
    GL_CONST(GREEN_SCALE),
    GL_CONST(GREEN_BIAS),
    GL_CONST(BLUE_SCALE),
    GL_CONST(BLUE_BIAS),
    GL_CONST(ALPHA_SCALE),
    GL_CONST(ALPHA_BIAS),
    GL_CONST(DEPTH_SCALE),
    GL_CONST(DEPTH_BIAS),
    GL_CONST(MAX_EVAL_ORDER),
    GL_CONST(MAX_LIGHTS),
    GL_CONST(MAX_CLIP_PLANES),
    GL_CONST(MAX_TEXTURE_SIZE),
    GL_CONST(MAX_PIXEL_MAP_TABLE),
    GL_CONST(MAX_ATTRIB_STACK_DEPTH),
    GL_CONST(MAX_MODELVIEW_STACK_DEPTH),
    GL_CONST(MAX_NAME_STACK_DEPTH),
    GL_CONST(MAX_PROJECTION_STACK_DEPTH),
    GL_CONST(MAX_TEXTURE_STACK_DEPTH),
    GL_CONST(MAX_VIEWPORT_DIMS),
    GL_CONST(MAX_CLIENT_ATTRIB_STACK_DEPTH),
    GL_CONST(SUBPIXEL_BITS),
    GL_CONST(INDEX_BITS),
    GL_CONST(RED_BITS),
    GL_CONST(GREEN_BITS),
    GL_CONST(BLUE_BITS),
    GL_CONST(ALPHA_BITS),
    GL_CONST(DEPTH_BITS),
    GL_CONST(STENCIL_BITS),
    GL_CONST(ACCUM_RED_BITS),
    GL_CONST(ACCUM_GREEN_BITS),
    GL_CONST(ACCUM_BLUE_BITS),
    GL_CONST(ACCUM_ALPHA_BITS),
    GL_CONST(NAME_STACK_DEPTH),
    GL_CONST(AUTO_NORMAL),
    GL_CONST(MAP1_COLOR_4),
    GL_CONST(MAP1_INDEX),
    GL_CONST(MAP1_NORMAL),
    GL_CONST(MAP1_TEXTURE_COORD_1),
    GL_CONST(MAP1_TEXTURE_COORD_2),
    GL_CONST(MAP1_TEXTURE_COORD_3),
    GL_CONST(MAP1_TEXTURE_COORD_4),
    GL_CONST(MAP1_VERTEX_3),
    GL_CONST(MAP1_VERTEX_4),
    GL_CONST(MAP2_COLOR_4),
    GL_CONST(MAP2_INDEX),
    GL_CONST(MAP2_NORMAL),
    GL_CONST(MAP2_TEXTURE_COORD_1),
    GL_CONST(MAP2_TEXTURE_COORD_2),
    GL_CONST(MAP2_TEXTURE_COORD_3),
    GL_CONST(MAP2_TEXTURE_COORD_4),
    GL_CONST(MAP2_VERTEX_3),
    GL_CONST(MAP2_VERTEX_4),
    GL_CONST(MAP1_GRID_DOMAIN),
    GL_CONST(MAP1_GRID_SEGMENTS),
    GL_CONST(MAP2_GRID_DOMAIN),
    GL_CONST(MAP2_GRID_SEGMENTS),
    GL_CONST(TEXTURE_1D),
    GL_CONST(TEXTURE_2D),
    GL_CONST(FEEDBACK_BUFFER_POINTER),
    GL_CONST(FEEDBACK_BUFFER_SIZE),
    GL_CONST(FEEDBACK_BUFFER_TYPE),
    GL_CONST(SELECTION_BUFFER_POINTER),
    GL_CONST(SELECTION_BUFFER_SIZE),

    GL_CONST(FOG_COORD_SRC),
    GL_CONST(FRAGMENT_DEPTH),

    GL_CONST(TEXTURE_WIDTH),
    GL_CONST(TEXTURE_HEIGHT),
    GL_CONST(TEXTURE_INTERNAL_FORMAT),
    GL_CONST(TEXTURE_BORDER_COLOR),
    GL_CONST(TEXTURE_BORDER),

    GL_CONST(DONT_CARE),
    GL_CONST(FASTEST),
    GL_CONST(NICEST),

    GL_CONST(LIGHT0),
    GL_CONST(LIGHT1),
    GL_CONST(LIGHT2),
    GL_CONST(LIGHT3),
    GL_CONST(LIGHT4),
    GL_CONST(LIGHT5),
    GL_CONST(LIGHT6),
    GL_CONST(LIGHT7),

    GL_CONST(AMBIENT),
    GL_CONST(DIFFUSE),
    GL_CONST(SPECULAR),
    GL_CONST(POSITION),
    GL_CONST(SPOT_DIRECTION),
    GL_CONST(SPOT_EXPONENT),
    GL_CONST(SPOT_CUTOFF),
    GL_CONST(CONSTANT_ATTENUATION),
    GL_CONST(LINEAR_ATTENUATION),
    GL_CONST(QUADRATIC_ATTENUATION),

    GL_CONST(COMPILE),
    GL_CONST(COMPILE_AND_EXECUTE),

    GL_CONST(CLEAR),
    GL_CONST(AND),
    GL_CONST(AND_REVERSE),
    GL_CONST(COPY),
    GL_CONST(AND_INVERTED),
    GL_CONST(NOOP),
    GL_CONST(XOR),
    GL_CONST(OR),
    GL_CONST(NOR),
    GL_CONST(EQUIV),
    GL_CONST(INVERT),
    GL_CONST(OR_REVERSE),
    GL_CONST(COPY_INVERTED),
    GL_CONST(OR_INVERTED),
    GL_CONST(NAND),
    GL_CONST(SET),

    GL_CONST(EMISSION),
    GL_CONST(SHININESS),
    GL_CONST(AMBIENT_AND_DIFFUSE),
    GL_CONST(COLOR_INDEXES),

    GL_CONST(MODELVIEW),
    GL_CONST(PROJECTION),
    GL_CONST(TEXTURE),

    GL_CONST(COLOR),
    GL_CONST(DEPTH),
    GL_CONST(STENCIL),

    GL_CONST(COLOR_INDEX),
    GL_CONST(STENCIL_INDEX),
    GL_CONST(DEPTH_COMPONENT),
    GL_CONST(RED),
    GL_CONST(GREEN),
    GL_CONST(BLUE),
    GL_CONST(ALPHA),
    GL_CONST(RGB),
    GL_CONST(RGBA),
    GL_CONST(LUMINANCE),
    GL_CONST(LUMINANCE_ALPHA),

    GL_CONST(BITMAP),

    GL_CONST(POINT),
    GL_CONST(LINE),
    GL_CONST(FILL),

    GL_CONST(RENDER),
    GL_CONST(FEEDBACK),
    GL_CONST(SELECT),

    GL_CONST(FLAT),
    GL_CONST(SMOOTH),

    GL_CONST(KEEP),
    GL_CONST(REPLACE),
    GL_CONST(INCR),
    GL_CONST(DECR),

    GL_CONST(VENDOR),
    GL_CONST(RENDERER),
    GL_CONST(VERSION),
    GL_CONST(EXTENSIONS),

    GL_CONST(S),
    GL_CONST(T),
    GL_CONST(R),
    GL_CONST(Q),

    GL_CONST(MODULATE),
    GL_CONST(DECAL),

    GL_CONST(TEXTURE_ENV_MODE),
    GL_CONST(TEXTURE_ENV_COLOR),

    GL_CONST(TEXTURE_ENV),

    GL_CONST(EYE_LINEAR),
    GL_CONST(OBJECT_LINEAR),
    GL_CONST(SPHERE_MAP),

    GL_CONST(TEXTURE_GEN_MODE),
    GL_CONST(OBJECT_PLANE),
    GL_CONST(EYE_PLANE),

    GL_CONST(NEAREST),
    GL_CONST(LINEAR),

    GL_CONST(NEAREST_MIPMAP_NEAREST),
    GL_CONST(LINEAR_MIPMAP_NEAREST),
    GL_CONST(NEAREST_MIPMAP_LINEAR),
    GL_CONST(LINEAR_MIPMAP_LINEAR),

    GL_CONST(TEXTURE_MAG_FILTER),
    GL_CONST(TEXTURE_MIN_FILTER),
    GL_CONST(TEXTURE_WRAP_S),
    GL_CONST(TEXTURE_WRAP_T),

    GL_CONST(CLAMP),
    GL_CONST(REPEAT),

    GL_CONST(CLIENT_PIXEL_STORE_BIT),
    GL_CONST(CLIENT_VERTEX_ARRAY_BIT),
    GL_CONST(CLIENT_ALL_ATTRIB_BITS),

    GL_CONST(POLYGON_OFFSET_FACTOR),
    GL_CONST(POLYGON_OFFSET_UNITS),
    GL_CONST(POLYGON_OFFSET_POINT),
    GL_CONST(POLYGON_OFFSET_LINE),
    GL_CONST(POLYGON_OFFSET_FILL),

    GL_CONST(ALPHA4),
    GL_CONST(ALPHA8),
    GL_CONST(ALPHA12),
    GL_CONST(ALPHA16),
    GL_CONST(LUMINANCE4),
    GL_CONST(LUMINANCE8),
    GL_CONST(LUMINANCE12),
    GL_CONST(LUMINANCE16),
    GL_CONST(LUMINANCE4_ALPHA4),
    GL_CONST(LUMINANCE6_ALPHA2),
    GL_CONST(LUMINANCE8_ALPHA8),
    GL_CONST(LUMINANCE12_ALPHA4),
    GL_CONST(LUMINANCE12_ALPHA12),
    GL_CONST(LUMINANCE16_ALPHA16),
    GL_CONST(INTENSITY),
    GL_CONST(INTENSITY4),
    GL_CONST(INTENSITY8),
    GL_CONST(INTENSITY12),
    GL_CONST(INTENSITY16),
    GL_CONST(R3_G3_B2),
    GL_CONST(RGB4),
    GL_CONST(RGB5),
    GL_CONST(RGB8),
    GL_CONST(RGB10),
    GL_CONST(RGB12),
    GL_CONST(RGB16),
    GL_CONST(RGBA2),
    GL_CONST(RGBA4),
    GL_CONST(RGB5_A1),
    GL_CONST(RGBA8),
    GL_CONST(RGB10_A2),
    GL_CONST(RGBA12),
    GL_CONST(RGBA16),
    GL_CONST(TEXTURE_RED_SIZE),
    GL_CONST(TEXTURE_GREEN_SIZE),
    GL_CONST(TEXTURE_BLUE_SIZE),
    GL_CONST(TEXTURE_ALPHA_SIZE),
    GL_CONST(TEXTURE_LUMINANCE_SIZE),
    GL_CONST(TEXTURE_INTENSITY_SIZE),
    GL_CONST(PROXY_TEXTURE_1D),
    GL_CONST(PROXY_TEXTURE_2D),

    GL_CONST(TEXTURE_PRIORITY),
    GL_CONST(TEXTURE_RESIDENT),
    GL_CONST(TEXTURE_BINDING_1D),
    GL_CONST(TEXTURE_BINDING_2D),

    GL_CONST(V2F),
    GL_CONST(V3F),
    GL_CONST(C4UB_V2F),
    GL_CONST(C4UB_V3F),
    GL_CONST(C3F_V3F),
    GL_CONST(N3F_V3F),
    GL_CONST(C4F_N3F_V3F),
    GL_CONST(T2F_V3F),
    GL_CONST(T4F_V4F),
    GL_CONST(T2F_C4UB_V3F),
    GL_CONST(T2F_C3F_V3F),
    GL_CONST(T2F_N3F_V3F),
    GL_CONST(T2F_C4F_N3F_V3F),
    GL_CONST(T4F_C4F_N3F_V4F),


    GL_CONST_EX(VERTEX_ARRAY, EXT),
    GL_CONST_EX(NORMAL_ARRAY, EXT),
    GL_CONST_EX(COLOR_ARRAY, EXT),
    GL_CONST_EX(INDEX_ARRAY, EXT),
    GL_CONST_EX(TEXTURE_COORD_ARRAY, EXT),
    GL_CONST_EX(EDGE_FLAG_ARRAY, EXT),
    GL_CONST_EX(VERTEX_ARRAY_SIZE, EXT),
    GL_CONST_EX(VERTEX_ARRAY_TYPE, EXT),
    GL_CONST_EX(VERTEX_ARRAY_STRIDE, EXT),
    GL_CONST_EX(VERTEX_ARRAY_COUNT, EXT),
    GL_CONST_EX(NORMAL_ARRAY_TYPE, EXT),
    GL_CONST_EX(NORMAL_ARRAY_STRIDE, EXT),
    GL_CONST_EX(NORMAL_ARRAY_COUNT, EXT),
    GL_CONST_EX(COLOR_ARRAY_SIZE, EXT),
    GL_CONST_EX(COLOR_ARRAY_TYPE, EXT),
    GL_CONST_EX(COLOR_ARRAY_STRIDE, EXT),
    GL_CONST_EX(COLOR_ARRAY_COUNT, EXT),
    GL_CONST_EX(INDEX_ARRAY_TYPE, EXT),
    GL_CONST_EX(INDEX_ARRAY_STRIDE, EXT),
    GL_CONST_EX(INDEX_ARRAY_COUNT, EXT),
    GL_CONST_EX(TEXTURE_COORD_ARRAY_SIZE, EXT),
    GL_CONST_EX(TEXTURE_COORD_ARRAY_TYPE, EXT),
    GL_CONST_EX(TEXTURE_COORD_ARRAY_STRIDE, EXT),
    GL_CONST_EX(TEXTURE_COORD_ARRAY_COUNT, EXT),
    GL_CONST_EX(EDGE_FLAG_ARRAY_STRIDE, EXT),
    GL_CONST_EX(EDGE_FLAG_ARRAY_COUNT, EXT),
    GL_CONST_EX(VERTEX_ARRAY_POINTER, EXT),
    GL_CONST_EX(NORMAL_ARRAY_POINTER, EXT),
    GL_CONST_EX(COLOR_ARRAY_POINTER, EXT),
    GL_CONST_EX(INDEX_ARRAY_POINTER, EXT),
    GL_CONST_EX(TEXTURE_COORD_ARRAY_POINTER, EXT),
    GL_CONST_EX(EDGE_FLAG_ARRAY_POINTER, EXT),

    GL_CONST_EX(TEXTURE0, ARB),
    GL_CONST_EX(TEXTURE1, ARB),
    GL_CONST_EX(TEXTURE2, ARB),
    GL_CONST_EX(TEXTURE3, ARB),
    GL_CONST_EX(TEXTURE4, ARB),
    GL_CONST_EX(TEXTURE5, ARB),
    GL_CONST_EX(TEXTURE6, ARB),
    GL_CONST_EX(TEXTURE7, ARB),
    GL_CONST_EX(TEXTURE8, ARB),
    GL_CONST_EX(TEXTURE9, ARB),
    GL_CONST_EX(TEXTURE10, ARB),
    GL_CONST_EX(TEXTURE11, ARB),
    GL_CONST_EX(TEXTURE12, ARB),
    GL_CONST_EX(TEXTURE13, ARB),
    GL_CONST_EX(TEXTURE14, ARB),
    GL_CONST_EX(TEXTURE15, ARB),
    GL_CONST_EX(TEXTURE16, ARB),
    GL_CONST_EX(TEXTURE17, ARB),
    GL_CONST_EX(TEXTURE18, ARB),
    GL_CONST_EX(TEXTURE19, ARB),
    GL_CONST_EX(TEXTURE20, ARB),
    GL_CONST_EX(TEXTURE21, ARB),
    GL_CONST_EX(TEXTURE22, ARB),
    GL_CONST_EX(TEXTURE23, ARB),
    GL_CONST_EX(TEXTURE24, ARB),
    GL_CONST_EX(TEXTURE25, ARB),
    GL_CONST_EX(TEXTURE26, ARB),
    GL_CONST_EX(TEXTURE27, ARB),
    GL_CONST_EX(TEXTURE28, ARB),
    GL_CONST_EX(TEXTURE29, ARB),
    GL_CONST_EX(TEXTURE30, ARB),
    GL_CONST_EX(TEXTURE31, ARB),
    GL_CONST_EX(ACTIVE_TEXTURE, ARB),

    GL_CONST_EX(BUFFER_SIZE, ARB),
    GL_CONST_EX(BUFFER_USAGE, ARB),
    GL_CONST_EX(ARRAY_BUFFER, ARB),
    GL_CONST_EX(ELEMENT_ARRAY_BUFFER, ARB),
    GL_CONST_EX(ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(ELEMENT_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(VERTEX_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(NORMAL_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(COLOR_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(INDEX_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(TEXTURE_COORD_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(EDGE_FLAG_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(SECONDARY_COLOR_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(FOG_COORDINATE_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(WEIGHT_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, ARB),
    GL_CONST_EX(READ_ONLY, ARB),
    GL_CONST_EX(WRITE_ONLY, ARB),
    GL_CONST_EX(READ_WRITE, ARB),
    GL_CONST_EX(BUFFER_ACCESS, ARB),
    GL_CONST_EX(BUFFER_MAPPED, ARB),
    GL_CONST_EX(BUFFER_MAP_POINTER, ARB),
    GL_CONST_EX(STREAM_DRAW, ARB),
    GL_CONST_EX(STREAM_READ, ARB),
    GL_CONST_EX(STREAM_COPY, ARB),
    GL_CONST_EX(STATIC_DRAW, ARB),
    GL_CONST_EX(STATIC_READ, ARB),
    GL_CONST_EX(STATIC_COPY, ARB),
    GL_CONST_EX(DYNAMIC_DRAW, ARB),
    GL_CONST_EX(DYNAMIC_READ, ARB),
    GL_CONST_EX(DYNAMIC_COPY, ARB),

    GL_CONST_EX(FRAGMENT_PROGRAM, ARB),
    GL_CONST_EX(PROGRAM_ALU_INSTRUCTIONS, ARB),
    GL_CONST_EX(PROGRAM_TEX_INSTRUCTIONS, ARB),
    GL_CONST_EX(PROGRAM_TEX_INDIRECTIONS, ARB),
    GL_CONST_EX(PROGRAM_NATIVE_ALU_INSTRUCTIONS, ARB),
    GL_CONST_EX(PROGRAM_NATIVE_TEX_INSTRUCTIONS, ARB),
    GL_CONST_EX(PROGRAM_NATIVE_TEX_INDIRECTIONS, ARB),
    GL_CONST_EX(MAX_PROGRAM_ALU_INSTRUCTIONS, ARB),
    GL_CONST_EX(MAX_PROGRAM_TEX_INSTRUCTIONS, ARB),
    GL_CONST_EX(MAX_PROGRAM_TEX_INDIRECTIONS, ARB),
    GL_CONST_EX(MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS, ARB),
    GL_CONST_EX(MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS, ARB),
    GL_CONST_EX(MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS, ARB),
    GL_CONST_EX(MAX_TEXTURE_COORDS, ARB),
    GL_CONST_EX(MAX_TEXTURE_IMAGE_UNITS, ARB),

    GL_CONST_EX(PROGRAM_OBJECT, ARB),
    GL_CONST_EX(SHADER_OBJECT, ARB),
    GL_CONST_EX(OBJECT_TYPE, ARB),
    GL_CONST_EX(OBJECT_SUBTYPE, ARB),
    GL_CONST_EX(FLOAT_VEC2, ARB),
    GL_CONST_EX(FLOAT_VEC3, ARB),
    GL_CONST_EX(FLOAT_VEC4, ARB),
    GL_CONST_EX(INT_VEC2, ARB),
    GL_CONST_EX(INT_VEC3, ARB),
    GL_CONST_EX(INT_VEC4, ARB),
    GL_CONST_EX(BOOL, ARB),
    GL_CONST_EX(BOOL_VEC2, ARB),
    GL_CONST_EX(BOOL_VEC3, ARB),
    GL_CONST_EX(BOOL_VEC4, ARB),
    GL_CONST_EX(FLOAT_MAT2, ARB),
    GL_CONST_EX(FLOAT_MAT3, ARB),
    GL_CONST_EX(FLOAT_MAT4, ARB),
    GL_CONST_EX(SAMPLER_1D, ARB),
    GL_CONST_EX(SAMPLER_2D, ARB),
    GL_CONST_EX(SAMPLER_3D, ARB),
    GL_CONST_EX(SAMPLER_CUBE, ARB),
    GL_CONST_EX(SAMPLER_1D_SHADOW, ARB),
    GL_CONST_EX(SAMPLER_2D_SHADOW, ARB),
    GL_CONST_EX(SAMPLER_2D_RECT, ARB),
    GL_CONST_EX(SAMPLER_2D_RECT_SHADOW, ARB),
    GL_CONST_EX(OBJECT_DELETE_STATUS, ARB),
    GL_CONST_EX(OBJECT_COMPILE_STATUS, ARB),
    GL_CONST_EX(OBJECT_LINK_STATUS, ARB),
    GL_CONST_EX(OBJECT_VALIDATE_STATUS, ARB),
    GL_CONST_EX(OBJECT_INFO_LOG_LENGTH, ARB),
    GL_CONST_EX(OBJECT_ATTACHED_OBJECTS, ARB),
    GL_CONST_EX(OBJECT_ACTIVE_UNIFORMS, ARB),
    GL_CONST_EX(OBJECT_ACTIVE_UNIFORM_MAX_LENGTH, ARB),
    GL_CONST_EX(OBJECT_SHADER_SOURCE_LENGTH, ARB),

    GL_CONST_EX(VERTEX_SHADER, ARB),
    GL_CONST_EX(MAX_VERTEX_UNIFORM_COMPONENTS, ARB),
    GL_CONST_EX(MAX_VARYING_FLOATS, ARB),
    GL_CONST_EX(MAX_VERTEX_TEXTURE_IMAGE_UNITS, ARB),
    GL_CONST_EX(MAX_COMBINED_TEXTURE_IMAGE_UNITS, ARB),
    GL_CONST_EX(OBJECT_ACTIVE_ATTRIBUTES, ARB),
    GL_CONST_EX(OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH, ARB),

    GL_CONST_EX(FRAGMENT_SHADER, ARB),
    GL_CONST_EX(MAX_FRAGMENT_UNIFORM_COMPONENTS, ARB),
    GL_CONST_EX(FRAGMENT_SHADER_DERIVATIVE_HINT, ARB),

    GL_CONST_EX(STENCIL_TEST_TWO_SIDE, EXT),
    GL_CONST_EX(ACTIVE_STENCIL_FACE, EXT),

    GL_CONST_EX(FUNC_SUBTRACT, EXT),
    GL_CONST_EX(FUNC_REVERSE_SUBTRACT, EXT),

    GL_CONST_EX(COMBINE, ARB),
    GL_CONST_EX(COMBINE_RGB, ARB),
    GL_CONST_EX(COMBINE_ALPHA, ARB),
    GL_CONST_EX(SOURCE0_RGB, ARB),
    GL_CONST_EX(SOURCE1_RGB, ARB),
    GL_CONST_EX(SOURCE2_RGB, ARB),
    GL_CONST_EX(SOURCE0_ALPHA, ARB),
    GL_CONST_EX(SOURCE1_ALPHA, ARB),
    GL_CONST_EX(SOURCE2_ALPHA, ARB),
    GL_CONST_EX(OPERAND0_RGB, ARB),
    GL_CONST_EX(OPERAND1_RGB, ARB),
    GL_CONST_EX(OPERAND2_RGB, ARB),
    GL_CONST_EX(OPERAND0_ALPHA, ARB),
    GL_CONST_EX(OPERAND1_ALPHA, ARB),
    GL_CONST_EX(OPERAND2_ALPHA, ARB),
    GL_CONST_EX(RGB_SCALE, ARB),
    GL_CONST_EX(ADD_SIGNED, ARB),
    GL_CONST_EX(INTERPOLATE, ARB),
    GL_CONST_EX(SUBTRACT, ARB),
    GL_CONST_EX(CONSTANT, ARB),
    GL_CONST_EX(PRIMARY_COLOR, ARB),
    GL_CONST_EX(PREVIOUS, ARB),
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(GL)
{
    JS_FASTNATIVE(GL, isEnabled, 1), // cap
    //JS_FASTNATIVE(GL, get, 1), // pname
    JS_FASTNATIVE(GL, getBoolean, 1), // pname [,count]
    //JS_FASTNATIVE(GL, getInteger, 2), // pname [,count]
    //JS_FASTNATIVE(GL, getDouble, 2), // pname [,count]
    //JS_FASTNATIVE(GL, getString, 1),
    JS_FASTNATIVE(GL, drawBuffer, 1), // mode
    JS_FASTNATIVE(GL, readBuffer, 1), // mode
    JS_FASTNATIVE(GL, accum, 2), // op, value
    JS_FASTNATIVE(GL, stencilFunc, 3), // func, ref, mask
    JS_FASTNATIVE(GL, stencilOp, 3), // fail, zfail, zpass
    JS_FASTNATIVE(GL, stencilMask, 1), // mask
    JS_FASTNATIVE(GL, alphaFunc, 2), // func, ref
    JS_FASTNATIVE(GL, flush, 0),
    JS_FASTNATIVE(GL, finish, 0),
    JS_FASTNATIVE(GL, fog, 2), // pname, param | array of params
    JS_FASTNATIVE(GL, hint, 2), // target, mode
    JS_FASTNATIVE(GL, vertex2f, 2),
    JS_FASTNATIVE(GL, vertex3f, 3),
    /*JS_FASTNATIVE(GL, vertex, 4), // x, y [, z [, w]]
    JS_FASTNATIVE(GL, edgeFlag, 1), // flag*/
    JS_FASTNATIVE(GL, color4, 4), // r, g, b [, a]
    /*JS_FASTNATIVE(GL, normal, 3), // nx, ny, nz
    JS_FASTNATIVE(GL, texCoord, 3), // s [, t [,r ]]
    */JS_FASTNATIVE(GL, texParameter, 2), // pname, param | array of params
    /*JS_FASTNATIVE(GL, texEnv, 3), // target, pname, param | array of params
    JS_FASTNATIVE(GL, texGen, 3), // coord, pname, params
    JS_FASTNATIVE(GL, texImage2D, 9), // target, level, internalFormat, width, height, border, format, type, data
    JS_FASTNATIVE(GL, copyTexSubImage2D, 8), // target, level, xoffset, yoffset, x, y, width, height
    JS_FASTNATIVE(GL, texSubImage2D, 9), // target, level, xoffset, yoffset, width, height, format, type, data
    JS_FASTNATIVE(GL, lightModel, 2), // pname, param
    JS_FASTNATIVE(GL, light, 3), // light, pname, param
    JS_FASTNATIVE(GL, getLight, 3), // light, pname, count
    JS_FASTNATIVE(GL, colorMaterial, 2), // face, mode
    JS_FASTNATIVE(GL, material, 3), // face, pname, param*/
    JS_FASTNATIVE(GL, enable, 1), // cap
    JS_FASTNATIVE(GL, disable, 1), // cap

    JS_FASTNATIVE(GL, setBlend, 1),
    JS_FASTNATIVE(GL, setCulling, 1),
    JS_FASTNATIVE(GL, enableTexture, 1),
    JS_FASTNATIVE(GL, setAlphaTest, 1),
    JS_FASTNATIVE(GL, setTexgen_S, 1),
    JS_FASTNATIVE(GL, setTexgen_T, 1),

    JS_FASTNATIVE(GL, enableClientState, 2),
    JS_FASTNATIVE(GL, bindTexture, 1),
    JS_FASTNATIVE(GL, vertex, 12),
    JS_FASTNATIVE(GL, triangle, 3),
    JS_FASTNATIVE(GL, drawElements, 0),

    /*JS_FASTNATIVE(GL, pointSize, 1), // size
    JS_FASTNATIVE(GL, lineWidth, 1), // width
    JS_FASTNATIVE(GL, shadeModel, 1), // mode
    JS_FASTNATIVE(GL, blendFunc, 2), // sfactor, dfactor
    JS_FASTNATIVE(GL, depthFunc, 1), // func
    JS_FASTNATIVE(GL, depthMask, 1), // mask
    JS_FASTNATIVE(GL, depthRange, 2), // zNear, zFar
    JS_FASTNATIVE(GL, polygonOffset, 2), // factor, units

    JS_FASTNATIVE(GL, cullFace, 1), // mode
    JS_FASTNATIVE(GL, frontFace, 1), // mode
    JS_FASTNATIVE(GL, clearStencil, 1), // s
    JS_FASTNATIVE(GL, clearDepth, 1), // depth
    JS_FASTNATIVE(GL, clearColor, 4), // r, g, b, alpha
    JS_FASTNATIVE(GL, clearAccum, 4), // r, g, b, alpha
    JS_FASTNATIVE(GL, clear, 1), // mask
    JS_FASTNATIVE(GL, colorMask, 4), // r,g,b,a
    JS_FASTNATIVE(GL, clipPlane, 2), // plane, equation
    JS_FASTNATIVE(GL, viewport, 4), // x, y, width, height
    JS_FASTNATIVE(GL, frustum, 6), // left, right, bottom, top, zNear, zFar
    JS_FASTNATIVE(GL, perspective, 4), // fovY, aspectRatio, zNear, zFar (non-OpenGL API),
    JS_FASTNATIVE(GL, ortho, 6), // left, right, bottom, top, zNear, zFar
    JS_FASTNATIVE(GL, matrixMode, 1), // mode
    JS_FASTNATIVE(GL, loadIdentity, 0),
    */JS_FASTNATIVE(GL, pushMatrix, 0),
    JS_FASTNATIVE(GL, popMatrix, 0),
    /*JS_FASTNATIVE(GL, loadMatrix, 1), // matrix
    */JS_FASTNATIVE(GL, multMatrix, 1), // matrix
    JS_FASTNATIVE(GL, rotate, 4), // angle, x, y, z
    JS_FASTNATIVE(GL, translate, 3), // x, y, z
    JS_FASTNATIVE(GL, scale, 3), // x, y, z
    /*JS_FASTNATIVE(GL, newList, 0),
    JS_FASTNATIVE(GL, deleteList, 1), // listId
    JS_FASTNATIVE(GL, endList, 0),
    JS_FASTNATIVE(GL, callList, 1), // listId | array of listId
    JS_FASTNATIVE(GL, polygonMode, 2), // face, mode*/
    JS_FASTNATIVE(GL, begin, 1), // mode
    JS_FASTNATIVE(GL, end, 0),
    /*JS_FASTNATIVE(GL, pushAttrib, 1), // mask
    JS_FASTNATIVE(GL, popAttrib, 0),
    JS_FASTNATIVE(GL, genTexture, 0),
    JS_FASTNATIVE(GL, deleteTexture, 1), // textureId
    JS_FASTNATIVE(GL, copyTexImage2D, 8), // target, level, internalFormat, x, y, width, height, border
    JS_FASTNATIVE(GL, pixelTransfer, 2), // pname, param
    JS_FASTNATIVE(GL, pixelStore, 2), // pname, param
    JS_FASTNATIVE(GL, rasterPos, 4), // x,y,z,w
    JS_FASTNATIVE(GL, pixelZoom, 2), // x,y
    JS_FASTNATIVE(GL, pixelMap, 2), // map,<array>

    JS_FASTNATIVE(GL, createTextureBuffer, 0),
    JS_FASTNATIVE(GL, defineTextureImage, 3), // target, format, image (non-OpenGL API),


// OpenGL extensions
    JS_FASTNATIVE(GL, hasExtensionProc, 1), // procName
    JS_FASTNATIVE(GL, hasExtensionName, 1), // name

    JS_FASTNATIVE(GL, blendEquation, 1), // mode
    JS_FASTNATIVE(GL, stencilFuncSeparate, 4), // func, ref, mask
    JS_FASTNATIVE(GL, stencilOpSeparate, 4), // fail, zfail, zpass
    JS_FASTNATIVE(GL, activeStencilFaceEXT, 1), // face

    JS_FASTNATIVE(GL, bindRenderbuffer, 2), // target, renderbuffer
    JS_FASTNATIVE(GL, genRenderbuffer, 0),
    JS_FASTNATIVE(GL, deleteRenderbuffer, 1), // renderbuffer
    JS_FASTNATIVE(GL, renderbufferStorage, 4), // target, internalformat, width, height
    JS_FASTNATIVE(GL, getRenderbufferParameter, 3), // target, pname [, count]
    JS_FASTNATIVE(GL, bindFramebuffer, 2), // target, renderbuffer
    JS_FASTNATIVE(GL, genFramebuffer, 0),
    JS_FASTNATIVE(GL, deleteFramebuffer, 1), // framebuffer
    JS_FASTNATIVE(GL, checkFramebufferStatus, 1), // target
    JS_FASTNATIVE(GL, framebufferTexture1D, 5), // target, attachment, textarget, texture, level
    JS_FASTNATIVE(GL, framebufferTexture2D, 5), // target, attachment, textarget, texture, level
    JS_FASTNATIVE(GL, framebufferTexture3D, 6), // target, attachment, textarget, texture, level, zoffset
    JS_FASTNATIVE(GL, framebufferRenderbuffer, 4), // target, attachment, renderbuffertarget, renderbuffer
    JS_FASTNATIVE(GL, getFramebufferAttachmentParameter, 4), // target, attachment, pname [, count]

    JS_FASTNATIVE(GL, createShaderObject, 1),
    JS_FASTNATIVE(GL, deleteObject, 1),
    JS_FASTNATIVE(GL, getInfoLog, 1),
    JS_FASTNATIVE(GL, createProgramObject, 0),
    JS_FASTNATIVE(GL, shaderSource, 2),
    JS_FASTNATIVE(GL, compileShader, 1),
    JS_FASTNATIVE(GL, attachObject, 2),
    JS_FASTNATIVE(GL, linkProgram, 1),
    JS_FASTNATIVE(GL, useProgramObject, 1),
    JS_FASTNATIVE(GL, getUniformLocation, 2),
    JS_FASTNATIVE(GL, uniform, 5),
    JS_FASTNATIVE(GL, uniformMatrix, 2),
    JS_FASTNATIVE(GL, uniformFloatVector, 3),
    JS_FASTNATIVE(GL, uniformFloat, 5),
    JS_FASTNATIVE(GL, uniformInteger, 5),
    JS_FASTNATIVE(GL, getObjectParameter, 2),
    JS_FASTNATIVE(GL, bindAttribLocation, 3),
    JS_FASTNATIVE(GL, getAttribLocation, 2),
    JS_FASTNATIVE(GL, vertexAttrib, 2),
    JS_FASTNATIVE(GL, genBuffer, 0),
    JS_FASTNATIVE(GL, bindBuffer, 2), // target, buffer

    JS_FASTNATIVE(GL, pointParameter, 2), // pname, param | Array of param
    JS_FASTNATIVE(GL, activeTexture, 1), // texture
    JS_FASTNATIVE(GL, clientActiveTexture, 1), // texture
    JS_FASTNATIVE(GL, multiTexCoord, 4), // target, s, t, r

    JS_FASTNATIVE(GL, genQueries, 0),
    JS_FASTNATIVE(GL, deleteQueries, 1), // query id
    JS_FASTNATIVE(GL, beginQuery, 2), // target, query id
    JS_FASTNATIVE(GL, endQuery, 1), // query id
    JS_FASTNATIVE(GL, getQuery, 2), // target, pname
    JS_FASTNATIVE(GL, getQueryObject, 3), // id, pname, length*/
    JS_FS_END
};
