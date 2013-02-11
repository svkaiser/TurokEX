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
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NRender)
{
    JS_FASTNATIVE(NRender, clearViewPort, 3),
    JS_FASTNATIVE(NRender, setOrtho, 0),
    JS_FASTNATIVE(NRender, drawModel, 2),
    JS_FS_END
};

// -----------------------------------------------
//
// MODEL CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Model);

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

