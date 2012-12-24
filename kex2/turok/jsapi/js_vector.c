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

JSObject *js_objVector;

enum vec3_enum
{
    VEC3_X,
    VEC3_Y,
    VEC3_Z
};

//
// vector_getProperty
//

static JSBool vector_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    vec3_t *vector = NULL;

    if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, obj, &Vector_class, NULL)))
        return JS_FALSE;

    switch(JSVAL_TO_INT(id))
    {
    case VEC3_X:
        return JS_NewNumberValue(cx, (*vector)[0], vp);

    case VEC3_Y:
        return JS_NewNumberValue(cx, (*vector)[1], vp);

    case VEC3_Z:
        return JS_NewNumberValue(cx, (*vector)[2], vp);
    }

    return JS_FALSE;
}

//
// vector_setProperty
//

static JSBool vector_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    vec3_t *vector = NULL;
    jsdouble val;

    if(!(vector = (vec3_t*)JS_GetInstancePrivate(cx, obj, &Vector_class, NULL)))
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
// vector_finalize
//

static void vector_finalize(JSContext *cx, JSObject *obj)
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
    JS_RETURNOBJECT(vp);
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
    JS_RETURNOBJECT(vp);
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
    JS_RETURNOBJECT(vp);
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
    JS_RETURNOBJECT(vp);
    return JS_TRUE;
}

//
// vector_scale
//

static JSBool vector_scale(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *thisvec;
    jsval *v;
    jsdouble x;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_THISVECTOR(thisvec, vp);
    JS_GETNUMBER(x, v, 2);

    Vec_Scale(*thisvec, *thisvec, (float)x);
    JS_RETURNOBJECT(vp);
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

    JS_RETURNSTRING(vp, buffer);
    return JS_TRUE;
}

//
// vector_unit2
//

static JSBool vector_unit2(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vector;

    JS_THISVECTOR(vector, vp);

    return JS_NewDoubleValue(cx, Vec_Unit2(*vector), vp);
}

//
// vector_unit3
//

static JSBool vector_unit3(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vector;

    JS_THISVECTOR(vector, vp);

    return JS_NewDoubleValue(cx, Vec_Unit3(*vector), vp);
}

//
// vector_toYaw
//

static JSBool vector_toYaw(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t *vector;

    JS_THISVECTOR(vector, vp);

    return JS_NewDoubleValue(cx, Ang_VectorToAngle(*vector), vp);
}

//
// vector_cross
//

static JSBool vector_cross(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    vec3_t outvec;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETVECTOR(vec1, v, 0);
    JS_GETVECTOR(vec2, v, 1);

    Vec_Cross(outvec, *vec1, *vec2);
    JS_NEWVECTOR(vp, outvec);
    return JS_TRUE;
}

//
// vector_dot
//

static JSBool vector_dot(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETVECTOR(vec1, v, 0);
    JS_GETVECTOR(vec2, v, 1);

    return JS_NewDoubleValue(cx, Vec_Dot(*vec1, *vec2), vp);
}

//
// vector_gscale
//

static JSBool vector_gscale(JSContext *cx, uintN argc, jsval *vp)
{
    vec3_t outvec;
    vec3_t *vec = NULL;
    jsval *v;
    jsdouble x;

    if(argc != 2)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETVECTOR(vec, v, 0);
    JS_GETNUMBER(x, v, 1);

    Vec_Scale(outvec, *vec, (float)x);
    JS_NEWVECTOR(vp, outvec);
    return JS_TRUE;
}

//
// vector_length2
//

static JSBool vector_length2(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETVECTOR(vec1, v, 0);
    JS_GETVECTOR(vec2, v, 1);

    return JS_NewDoubleValue(cx, Vec_Length2(*vec1, *vec2), vp);
}

//
// vector_length3
//

static JSBool vector_length3(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETVECTOR(vec1, v, 0);
    JS_GETVECTOR(vec2, v, 1);

    return JS_NewDoubleValue(cx, Vec_Length3(*vec1, *vec2), vp);
}

//
// vector_pointToAxis
//

static JSBool vector_pointToAxis(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    vec3_t outvec;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETVECTOR(vec1, v, 0);
    JS_GETVECTOR(vec2, v, 1);

    Vec_PointToAxis(outvec, *vec1, *vec2);
    JS_NEWVECTOR(vp, outvec);
    return JS_TRUE;
}

//
// vector_toWorld
//

static JSBool vector_toWorld(JSContext *cx, uintN argc, jsval *vp)
{
    jsval *v;
    vec3_t outvec;
    vec3_t *vec = NULL;
    mtx_t *mtx = NULL;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, vp);

    JS_GETVECTOR(vec, v, 0);
    JS_GETMATRIX(mtx, v, 1);

    Vec_TransformToWorld(*mtx, *vec, outvec);
    JS_NEWVECTOR(vp, outvec);
    return JS_TRUE;
}

//
// Vector_class
//

JSClass Vector_class =
{
    "Vector",                                   // name
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    vector_getProperty,                         // getProperty
    vector_setProperty,                         // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    vector_finalize,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// Vector_props
//

JSPropertySpec Vector_props[] =
{
    { "x",  VEC3_X, JSPROP_ENUMERATE,   NULL, NULL },
    { "y",  VEC3_Y, JSPROP_ENUMERATE,   NULL, NULL },
    { "z",  VEC3_Z, JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// Vector_functions
//

JSFunctionSpec Vector_functions[] =
{
    JS_FN("add",        vector_add,         1, 0, 0),
    JS_FN("sub",        vector_sub,         1, 0, 0),
    JS_FN("multiply",   vector_mult,        1, 0, 0),
    JS_FN("normalize",  vector_normalize,   0, 0, 0),
    JS_FN("lerp",       vector_lerp,        2, 0, 0),
    JS_FN("scale",      vector_scale,       1, 0, 0),
    JS_FN("toString",   vector_toString,    0, 0, 0),
    JS_FN("unit2",      vector_unit2,       0, 0, 0),
    JS_FN("unit3",      vector_unit3,       0, 0, 0),
    JS_FN("toYaw",      vector_toYaw,       0, 0, 0),
    JS_FS_END
};

//
// Vector_functions_static
//

JSFunctionSpec Vector_functions_static[] =
{
    JS_FN("cross",      vector_cross,       2, 0, 0),
    JS_FN("dot",        vector_dot,         2, 0, 0),
    JS_FN("scale",      vector_gscale,      2, 0, 0),
    JS_FN("length2",    vector_length2,     2, 0, 0),
    JS_FN("length3",    vector_length3,     2, 0, 0),
    JS_FN("pointToAxis",vector_pointToAxis, 2, 0, 0),
    JS_FN("toWorld",    vector_toWorld,     2, 0, 0),
    JS_FS_END
};

//
// vector_construct
//

JSBool Vector_construct(JSContext *cx, JSObject *obj, uintN argc,
                        jsval *argv, jsval *rval)
{
    jsval *v;
    vec3_t vector;
    jsdouble x = 0;
    jsdouble y = 0;
    jsdouble z = 0;

    if(argc == 3)
    {
        v = JS_ARGV(cx, argv);

        JS_GETNUMBER(x, v, -2);
        JS_GETNUMBER(y, v, -1);
        JS_GETNUMBER(z, v,  0);
    }

    Vec_Set3(vector, (float)x, (float)y, (float)z);

    JS_NEWVECTOR(rval, vector);
    return JS_TRUE;
}
