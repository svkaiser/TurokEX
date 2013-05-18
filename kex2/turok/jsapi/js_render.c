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
#include "render.h"

kbool R_FrustumTestBox(bbox_t box);

// -----------------------------------------------
//
// RENDER CLASS
//
// -----------------------------------------------

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

JS_FASTNATIVE_BEGIN(NRender, drawModel)
{
    JSObject *object;
    kmodel_t *model;
    animstate_t *animstate;

    JS_CHECKARGS(2);
    JS_GETOBJECT(object, v, 0);
    JS_GET_PRIVATE_DATA(object, &Model_class, kmodel_t, model);

    animstate = NULL;

    if(!JSVAL_IS_NULL(v[1]))
    {
        JS_GETOBJECT(object, v, 1);
        JS_GET_PRIVATE_DATA(object, &AnimState_class, animstate_t, animstate);
    }

    R_TraverseDrawNode(model, &model->nodes[0], NULL, 0, animstate);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(NRender, testBoxInFrustum)
{
    bbox_t bbox;
    jsdouble box[6];

    JS_CHECKARGS(6);
    JS_GETNUMBER(box[0], v, 0);
    JS_GETNUMBER(box[1], v, 1);
    JS_GETNUMBER(box[2], v, 2);
    JS_GETNUMBER(box[3], v, 3);
    JS_GETNUMBER(box[4], v, 4);
    JS_GETNUMBER(box[5], v, 5);

    bbox.min[0] = (float)box[0];
    bbox.min[1] = (float)box[1];
    bbox.min[2] = (float)box[2];
    bbox.max[0] = (float)box[3];
    bbox.max[1] = (float)box[4];
    bbox.max[2] = (float)box[5];

    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(R_FrustumTestBox(bbox)));
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
    JS_DEFINE_CONST(SCREEN_WIDTH,       FIXED_WIDTH),
    JS_DEFINE_CONST(SCREEN_HEIGHT,      FIXED_HEIGHT),
    JS_DEFINE_CONST(ANIM_BLEND,         ANF_BLEND),
    JS_DEFINE_CONST(ANIM_LOOP,          ANF_LOOP),
    JS_DEFINE_CONST(ANIM_STOPPED,       ANF_STOPPED),
    JS_DEFINE_CONST(ANIM_NOINTERRUPT,   ANF_NOINTERRUPT),
    JS_DEFINE_CONST(ANIM_ROOTMOTION,    ANF_ROOTMOTION),
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NRender)
{
    JS_FASTNATIVE(NRender, clearViewPort, 3),
    JS_FASTNATIVE(NRender, setOrtho, 0),
    JS_FASTNATIVE(NRender, drawModel, 2),
    JS_FASTNATIVE(NRender, testBoxInFrustum, 6),
    JS_FS_END
};

// -----------------------------------------------
//
// MODEL CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Model);

JS_FASTNATIVE_BEGIN(Model, setNodeRotation)
{
    JSObject *thisObj;
    JSObject *obj;
    vec4_t rot;
    int node;
    kmodel_t *model;

    JS_CHECKARGS(2);
    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_CHECKINTEGER(0);
    node = JSVAL_TO_INT(JS_ARG(0));
    JS_GET_PRIVATE_DATA(thisObj, &Model_class, kmodel_t, model);
    JS_GETOBJECT(obj, v, 1);
    JS_GETQUATERNION2(obj, rot);

    if(node >= 0 && node < (int)model->numnodes)
        Vec_Copy4(model->nodes[node].offset_r, rot);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(Model)
    JSCLASS_HAS_PRIVATE,                        // flags
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

JS_BEGINPROPS(Model)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Model)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Model)
{
    JS_FASTNATIVE(Model, setNodeRotation, 2),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Model)
{
    JS_FS_END
};

// -----------------------------------------------
//
// TEXTURE CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Texture);

JS_BEGINCLASS(Texture)
    JSCLASS_HAS_PRIVATE,                        // flags
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

JS_BEGINPROPS(Texture)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Texture)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Texture)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Texture)
{
    JS_FS_END
};

// -----------------------------------------------
//
// CANVAS CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Canvas);

JS_PROP_FUNC_GET(Canvas)
{
    canvas_t *canvas;

    if(!(canvas = (canvas_t*)JS_GetInstancePrivate(cx, obj, &Canvas_class, NULL)))
        return JS_FALSE;

    switch(JSVAL_TO_INT(id))
    {
    case 0:
        return JS_NewDoubleValue(cx, canvas->scale, vp);
    default:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_FINALIZE_FUNC(Canvas)
{
    canvas_t *canvas;

    if(canvas = (canvas_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, canvas);
}

#define JS_THISCANVAS() \
    JS_GET_PRIVATE_DATA(JS_THIS_OBJECT(cx, vp), &Canvas_class, \
        canvas_t, canvas)

JS_FASTNATIVE_BEGIN(Canvas, setDrawColor)
{
    canvas_t *canvas;

    JS_CHECKARGS(3);
    JS_THISCANVAS();
    JS_CHECKINTEGER(0);
    JS_CHECKINTEGER(1);
    JS_CHECKINTEGER(2);

    Canvas_SetDrawColor(canvas,
        (byte)JSVAL_TO_INT(JS_ARG(0)),
        (byte)JSVAL_TO_INT(JS_ARG(1)),
        (byte)JSVAL_TO_INT(JS_ARG(2)));

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, setVertexColor)
{
    canvas_t *canvas;
    int i;

    JS_CHECKARGS(4);
    JS_THISCANVAS();
    JS_CHECKINTEGER(0);
    JS_CHECKINTEGER(1);
    JS_CHECKINTEGER(2);
    JS_CHECKINTEGER(3);

    i = JSVAL_TO_INT(JS_ARG(0));

    if(i >= 0 && i < 4)
    {
        canvas->drawColor[i][0] = (byte)JSVAL_TO_INT(JS_ARG(1));
        canvas->drawColor[i][1] = (byte)JSVAL_TO_INT(JS_ARG(2));
        canvas->drawColor[i][2] = (byte)JSVAL_TO_INT(JS_ARG(3));
    }

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, setDrawAlpha)
{
    canvas_t *canvas;

    JS_CHECKARGS(1);
    JS_THISCANVAS();
    JS_CHECKINTEGER(0);

    Canvas_SetDrawAlpha(canvas, (byte)JSVAL_TO_INT(JS_ARG(0)));

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, setTextureTile)
{
    canvas_t *canvas;
    jsdouble u1, u2, t1, t2;

    JS_CHECKARGS(4);
    JS_THISCANVAS();
    JS_GETNUMBER(u1, v, 0);
    JS_GETNUMBER(u2, v, 1);
    JS_GETNUMBER(t1, v, 2);
    JS_GETNUMBER(t2, v, 3);

    Canvas_SetTextureTile(canvas,
        (float)u1,
        (float)u2,
        (float)t1,
        (float)t2);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, setDrawScale)
{
    canvas_t *canvas;
    jsdouble scale;

    JS_CHECKARGS(1);
    JS_THISCANVAS();
    JS_GETNUMBER(scale, v, 0);

    Canvas_SetDrawScale(canvas, (float)scale);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, setFont)
{
    JSObject *obj;
    canvas_t *canvas;
    font_t *font;

    JS_CHECKARGS(1);
    JS_THISCANVAS();
    JS_GETOBJECT(obj, v, 0);
    JS_GET_PRIVATE_DATA(obj, &Font_class, font_t, font);

    Canvas_SetFont(canvas, font);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, drawTile)
{
    canvas_t *canvas;
    texture_t *texture;
    jsdouble x, y, w, h;

    JS_CHECKARGS(5);
    JS_THISCANVAS();

    if(JSVAL_IS_STRING(v[0]))
    {
        JSString *str;
        char *bytes;

        JS_GETSTRING(str, bytes, v, 0);
        texture = Tex_CacheTextureFile(bytes, DGL_CLAMP, true);
        JS_free(cx, bytes);
    }
    else
    {
        JSObject *obj;

        JS_GETOBJECT(obj, v, 0);
        JS_GET_PRIVATE_DATA(obj, &Texture_class, texture_t, texture);
    }

    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);
    JS_GETNUMBER(w, v, 3);
    JS_GETNUMBER(h, v, 4);

    Canvas_DrawTile(canvas, texture,
        (float)x, (float)y, (float)w, (float)h);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, drawFixedTile)
{
    canvas_t *canvas;
    texture_t *texture;
    jsdouble x, y;

    JS_CHECKARGS(3);
    JS_THISCANVAS();

    if(JSVAL_IS_STRING(v[0]))
    {
        JSString *str;
        char *bytes;

        JS_GETSTRING(str, bytes, v, 0);
        texture = Tex_CacheTextureFile(bytes, DGL_CLAMP, true);
        JS_free(cx, bytes);
    }
    else
    {
        JSObject *obj;

        JS_GETOBJECT(obj, v, 0);
        JS_GET_PRIVATE_DATA(obj, &Texture_class, texture_t, texture);
    }

    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);

    Canvas_DrawFixedTile(canvas, texture, (float)x, (float)y);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, drawString)
{
    canvas_t *canvas;
    JSString *str;
    char *bytes;
    JSBool center;
    jsdouble x, y;
    jsval *v = JS_ARGV(cx, vp);

    if(argc < 3)
        return JS_FALSE;

    center = JS_FALSE;

    JS_THISCANVAS();
    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);

    if(argc >= 3)
        JS_GETBOOL(center, v, 3);

    Canvas_DrawString(canvas, bytes, (float)x, (float)y, center);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Canvas, drawFixedString)
{
    canvas_t *canvas;
    JSString *str;
    char *bytes;
    JSBool center;
    jsdouble x, y;
    jsval *v = JS_ARGV(cx, vp);

    if(argc < 3)
        return JS_FALSE;

    center = JS_FALSE;

    JS_THISCANVAS();
    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);

    if(argc >= 3)
        JS_GETBOOL(center, v, 3);

    Canvas_DrawFixedString(canvas, bytes, (float)x, (float)y, center);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

void Canvas_DrawActor(canvas_t *canvas, gActor_t *actor);

JS_FASTNATIVE_BEGIN(Canvas, drawActor)
{
    canvas_t *canvas;
    JSObject *obj;
    gActor_t *actor;

    JS_CHECKARGS(1);

    JS_THISCANVAS();
    JS_GETOBJECT(obj, v, 0);
    JS_GET_PRIVATE_DATA(obj, &GameActor_class, gActor_t, actor);

    Canvas_DrawActor(canvas, actor);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(Canvas)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Canvas_getProperty,                         // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    Canvas_finalize,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Canvas)
{
    { "scale", 0, JSPROP_ENUMERATE|JSPROP_READONLY, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Canvas)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Canvas)
{
    JS_FASTNATIVE(Canvas, setDrawColor, 3),
    JS_FASTNATIVE(Canvas, setVertexColor, 4),
    JS_FASTNATIVE(Canvas, setDrawAlpha, 1),
    JS_FASTNATIVE(Canvas, setTextureTile, 4),
	JS_FASTNATIVE(Canvas, setDrawScale, 1),
    JS_FASTNATIVE(Canvas, setFont, 1),
    JS_FASTNATIVE(Canvas, drawTile, 5),
    JS_FASTNATIVE(Canvas, drawFixedTile, 3),
    JS_FASTNATIVE(Canvas, drawString, 3),
    JS_FASTNATIVE(Canvas, drawFixedString, 3),
    JS_FASTNATIVE(Canvas, drawActor, 1),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Canvas)
{
    JS_FS_END
};

JS_CONSTRUCTOR(Canvas)
{
    canvas_t *canvas;

    canvas = (canvas_t*)JS_malloc(cx, sizeof(canvas_t));
    if(canvas == NULL)
        return JS_FALSE;

    memset(canvas, 0, sizeof(canvas_t));

    canvas->scale = 1.0f;
    Canvas_SetDrawAlpha(canvas, 255);
    canvas->drawCoord[2] = 1.0f;
    canvas->drawCoord[3] = 1.0f;

    JS_NEWOBJECT_SETPRIVATE(canvas, &Canvas_class);
    return JS_TRUE;
}

// -----------------------------------------------
//
// FONT CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Font);

enum jsfont_enum
{
    JSF_TEXTURE,
    JSF_WIDTH,
    JSF_HEIGHT
};

#define JS_THISFONT() \
    JS_GET_PRIVATE_DATA(JS_THIS_OBJECT(cx, vp), &Font_class, \
        font_t, font)

JS_PROP_FUNC_GET(Font)
{
    switch(JSVAL_TO_INT(id))
    {
    case JSF_TEXTURE:
        break;

    case JSF_WIDTH:
        break;

    case JSF_HEIGHT:
        break;

    default:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_PROP_FUNC_SET(Font)
{
    font_t *font;

    JS_GET_PRIVATE_DATA(obj, &Font_class, font_t, font);

    switch(JSVAL_TO_INT(id))
    {
    case JSF_TEXTURE:
        {
            JSString    *str;
            char        *bytes;

            JS_GETSTRING(str, bytes, vp, 0);
            font->texture = bytes;

            return JS_TRUE;
        }

    case JSF_WIDTH:
        font->width = JSVAL_TO_INT(*vp);
        return JS_TRUE;

    case JSF_HEIGHT:
        font->height = JSVAL_TO_INT(*vp);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_FASTNATIVE_BEGIN(Font, mapChar)
{
    font_t *font;
    byte ch;

    JS_CHECKARGS(5);
    JS_THISFONT();

    if(JSVAL_IS_STRING(v[0]))
    {
        JSString *str;
        char *bytes;

        JS_GETSTRING(str, bytes, v, 0);

        ch = *bytes;
        JS_free(cx, bytes);
    }
    else
    {
        JS_CHECKINTEGER(0);
        ch = (byte)JSVAL_TO_INT(JS_ARG(0));
    }

    JS_CHECKINTEGER(1);
    JS_CHECKINTEGER(2);
    JS_CHECKINTEGER(3);
    JS_CHECKINTEGER(4);

    Font_MapChar(font, ch,
        JSVAL_TO_INT(JS_ARG(1)),
        JSVAL_TO_INT(JS_ARG(2)),
        JSVAL_TO_INT(JS_ARG(3)),
        JSVAL_TO_INT(JS_ARG(4)));

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Font, stringWidth)
{
    font_t      *font;
    JSString    *str;
    char        *bytes;
    jsdouble    scale;
    int         fixedLen;
    float       width;
    jsval       *v = JS_ARGV(cx, vp);

    if(argc < 2)
        return JS_FALSE;

    JS_THISFONT();
    JS_GETSTRING(str, bytes, v, 0);
    JS_GETNUMBER(scale, v, 1);

    fixedLen = -1;

    if(argc >= 3)
    {
        JS_CHECKINTEGER(2);
        fixedLen = JSVAL_TO_INT(JS_ARG(2));
    }

    width = Font_StringWidth(font, bytes, (float)scale, fixedLen);
    JS_free(cx, bytes);

    return JS_NewDoubleValue(cx, width, vp);
}

JS_BEGINCLASS(Font)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Font_getProperty,                           // getProperty
    Font_setProperty,                           // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Font)
{
    { "texture",    JSF_TEXTURE,    JSPROP_ENUMERATE,   NULL, NULL },
    { "tex_width",  JSF_WIDTH,      JSPROP_ENUMERATE,   NULL, NULL },
    { "tex_height", JSF_HEIGHT,     JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Font)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Font)
{
    JS_FASTNATIVE(Font, mapChar, 5),
    JS_FASTNATIVE(Font, stringWidth, 3),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Font)
{
    JS_FS_END
};

JS_CONSTRUCTOR(Font)
{
    font_t *font;

    font = (font_t*)JS_malloc(cx, sizeof(font_t));
    if(font == NULL)
        return JS_FALSE;

    memset(font, 0, sizeof(font_t));

    JS_NEWOBJECT_SETPRIVATE(font, &Font_class);
    return JS_TRUE;
}

