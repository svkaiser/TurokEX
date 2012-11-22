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
// DESCRIPTION: Javascript Vector Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"

JSObject *js_objvector3;

enum vec3_enum
{
    VEC3_X,
    VEC3_Y,
    VEC3_Z
};

//
// vector3_getProperty
//

static JSBool vector3_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    vec3_t *vector = NULL;

    if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, obj, &vector3_class, NULL)))
        return JS_FALSE;

    switch(JSVAL_TO_INT(id))
    {
    case VEC3_X:
        return JS_NewNumberValue(cx, (*vector)[0], vp);
        break;
    case VEC3_Y:
        return JS_NewNumberValue(cx, (*vector)[1], vp);
        break;
    case VEC3_Z:
        return JS_NewNumberValue(cx, (*vector)[2], vp);
        break;
    }

    return JS_FALSE;
}

//
// vector3_setProperty
//

static JSBool vector3_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    vec3_t *vector = NULL;
    jsdouble val;

    if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, obj, &vector3_class, NULL)))
        return JS_FALSE;

    if(!(JS_ValueToNumber(cx, *vp, &val)))
        return JS_FALSE;

    switch(JSVAL_TO_INT(id))
    {
    case VEC3_X:
        (*vector)[0] = (float)val;
        break;
    case VEC3_Y:
        (*vector)[1] = (float)val;
        break;
    case VEC3_Z:
        (*vector)[2] = (float)val;
        break;
    }

    return JS_TRUE;
}

//
// vector3_finalize
//

static void vector3_finalize(JSContext *cx, JSObject *obj)
{
    vec3_t *vector;

    if(vector = (vec3_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, vector);

    return;
}

//
// vector_add
//

static JSBool vector_add(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISVECTOR(vec1, vp);
    JS_GETVECTOR(vec2, vp, 2);

    Vec_Add(*vec1, *vec1, *vec2);
    return JS_TRUE;
}

//
// vector_sub
//

static JSBool vector_sub(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISVECTOR(vec1, vp);
    JS_GETVECTOR(vec2, vp, 2);

    Vec_Sub(*vec1, *vec1, *vec2);
    return JS_TRUE;
}

//
// vector_mult
//

static JSBool vector_mult(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISVECTOR(vec1, vp);
    JS_GETVECTOR(vec2, vp, 2);

    Vec_Mult(*vec1, *vec1, *vec2);
    return JS_TRUE;
}

//
// vector_normalize
//

static JSBool vector_normalize(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vector;

    JS_THISVECTOR(vector, vp);

    Vec_Normalize3(*vector);
    return JS_TRUE;
}

//
// vector_lerp
//

static JSBool vector_lerp(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *thisvec;
    vec3_t *nextvec;
    jsval *v;
    jsdouble x;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_THISVECTOR(thisvec, vp);
    JS_GETVECTOR(nextvec, v, 2);
    JS_GETNUMBER(x, v, 1);

    Vec_Lerp3(*thisvec, (float)x, *thisvec, *nextvec);
    return JS_TRUE;
}

//
// vector_toString
//

static JSBool vector_toString(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vector;
    char buffer[256];

    JS_THISVECTOR(vector, vp);
    sprintf(buffer, "%f, %f, %f",
        (*vector)[0],
        (*vector)[1],
        (*vector)[2]);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, buffer)));
    return JS_TRUE;
}

//
// vector_cross
//

static JSBool vector_cross(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    JSObject *nobj;
    jsval *v;
    vec3_t *outvec = NULL;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    if(!(nobj = JS_NewObject(cx, &vector3_class, NULL, NULL)))
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    JS_GETVECTOR(vec1, v, -2);
    JS_GETVECTOR(vec2, v, -1);

    outvec = (vec3_t*)JS_malloc(cx, sizeof(vec3_t));
    Vec_Cross(*outvec, *vec1, *vec2);

    if(!(JS_SetPrivate(cx, nobj, outvec)))
        return JS_FALSE;

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(nobj));
    return JS_TRUE;
}

//
// vector_dot
//

static JSBool vector_dot(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    jsval *v;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    JS_GETVECTOR(vec1, v, -2);
    JS_GETVECTOR(vec2, v, -1);

    return JS_NewDoubleValue(cx, Vec_Dot(*vec1, *vec2), rval);
}

//
// vector3_class
//

JSClass vector3_class =
{
    "vector3",                                  // name
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    vector3_getProperty,                        // getProperty
    vector3_setProperty,                        // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    vector3_finalize,                           // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// vector3_props
//

JSPropertySpec vector3_props[] =
{
    { "x",  VEC3_X, JSPROP_ENUMERATE,   NULL, NULL },
    { "y",  VEC3_Y, JSPROP_ENUMERATE,   NULL, NULL },
    { "z",  VEC3_Z, JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// vector3_functions
//

JSFunctionSpec vector3_functions[] =
{
    JS_FN("add",        vector_add,         1, 0, 0),
    JS_FN("sub",        vector_sub,         1, 0, 0),
    JS_FN("mult",       vector_mult,        1, 0, 0),
    JS_FN("normalize",  vector_normalize,   0, 0, 0),
    JS_FN("lerp",       vector_lerp,        2, 0, 0),
    JS_FN("toString",   vector_toString,    0, 0, 0),
    JS_FS_END
};

//
// vector3_functions_static
//

JSFunctionSpec vector3_functions_static[] =
{
    JS_FS("cross",  vector_cross,   2, 0, 0),
    JS_FS("dot",    vector_dot,     2, 0, 0),
    JS_FS_END
};

//
// vector3_construct
//

JSBool vector3_construct(JSContext *cx, JSObject *obj, uintN argc,
                        jsval *argv, jsval *rval)
{
    JSObject *vobj;
    jsval *v;
    vec3_t *vector;

    if(!(vobj = JS_NewObject(cx, &vector3_class, NULL, NULL)))
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    vector = (vec3_t*)JS_malloc(cx, sizeof(vec3_t));

    if(!(JS_SetPrivate(cx, vobj, vector)))
        return JS_FALSE;

    JS_SetProperty(cx, vobj, "x", &v[-2]);
    JS_SetProperty(cx, vobj, "y", &v[-1]);
    JS_SetProperty(cx, vobj, "z", &v[ 0]);

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(vobj));

    return JS_TRUE;
}
