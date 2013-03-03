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

JS_CLASSOBJECT(Vector);

enum vec3_enum
{
    VEC3_X,
    VEC3_Y,
    VEC3_Z
};

JS_PROP_FUNC_GET(Vector)
{
    vec3_t *vector = NULL;

    if(vector = (vec3_t*)JS_GetInstancePrivate(cx, obj, &Vector_class, NULL))
    {
        switch(JSVAL_TO_INT(id))
        {
        case VEC3_X:
            return JS_NewNumberValue(cx, (*vector)[0], vp);
        case VEC3_Y:
            return JS_NewNumberValue(cx, (*vector)[1], vp);
        case VEC3_Z:
            return JS_NewNumberValue(cx, (*vector)[2], vp);
        }
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(Vector)
{
    vec3_t *vector = NULL;

    if(vector = (vec3_t*)JS_GetInstancePrivate(cx, obj, &Vector_class, NULL))
    {
        jsdouble val;

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
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, copy)
{
    JSObject *thisObj;
    JSObject *obj;
    vec3_t thisvec;
    vec3_t vec;

    JS_CHECKARGS(1);

    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GETVECTOR2(thisObj, thisvec);
    JS_GETOBJECT(obj, v, 0);
    JS_GETVECTOR2(obj, vec);

    Vec_Copy3(thisvec, vec);
    
    JS_SETVECTOR(thisObj, thisvec);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, clear)
{
    JSObject *obj;
    vec3_t vector;

    JS_CHECKARGS(0);

    obj = JS_THIS_OBJECT(cx, vp);
    JS_GETVECTOR2(obj, vector);

    Vec_Set3(vector, 0, 0, 0);

    JS_SETVECTOR(obj, vector);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, add)
{
    JSObject *thisObj;
    JSObject *obj;
    vec3_t thisvec;
    vec3_t vec;

    JS_CHECKARGS(1);

    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GETVECTOR2(thisObj, thisvec);
    JS_GETOBJECT(obj, v, 0);
    JS_GETVECTOR2(obj, vec);

    Vec_Add(thisvec, thisvec, vec);
    
    JS_SETVECTOR(thisObj, thisvec);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, sub)
{
    JSObject *thisObj;
    JSObject *obj;
    vec3_t thisvec;
    vec3_t vec;

    JS_CHECKARGS(1);

    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GETVECTOR2(thisObj, thisvec);
    JS_GETOBJECT(obj, v, 0);
    JS_GETVECTOR2(obj, vec);

    Vec_Sub(thisvec, thisvec, vec);
    
    JS_SETVECTOR(thisObj, thisvec);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, multiply)
{
    JSObject *thisObj;
    JSObject *obj;
    vec3_t thisvec;
    vec3_t vec;

    JS_CHECKARGS(1);

    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GETVECTOR2(thisObj, thisvec);
    JS_GETOBJECT(obj, v, 0);
    JS_GETVECTOR2(obj, vec);

    Vec_Mult(thisvec, thisvec, vec);

    JS_SETVECTOR(thisObj, thisvec);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, normalize)
{
    JSObject *obj;
    vec3_t vector;

    JS_CHECKARGS(0);

    obj = JS_THIS_OBJECT(cx, vp);
    JS_GETVECTOR2(obj, vector);

    Vec_Normalize3(vector);

    JS_SETVECTOR(obj, vector);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, lerp)
{
    JSObject *thisObj;
    JSObject *obj;
    vec3_t thisvec;
    vec3_t nextvec;
    jsdouble x;

    JS_CHECKARGS(2);

    thisObj = JS_THIS_OBJECT(cx, vp);
    JS_GETVECTOR2(thisObj, thisvec);
    JS_GETOBJECT(obj, v, 0);
    JS_GETVECTOR2(obj, nextvec);
    JS_GETNUMBER(x, v, 1);

    Vec_Lerp3(thisvec, (float)x, thisvec, nextvec);

    JS_SETVECTOR(thisObj, thisvec);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, scale)
{
    JSObject *object;
    vec3_t thisvec;
    jsdouble x;

    JS_CHECKARGS(1);

    object = JS_THIS_OBJECT(cx, vp);
    JS_GETNUMBER(x, v, 0);
    JS_GETVECTOR2(object, thisvec);

    Vec_Scale(thisvec, thisvec, (float)x);

    JS_SETVECTOR(object, thisvec);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, toString)
{
    vec3_t vector;
    char buffer[256];

    JS_CHECKARGS(0);

    JS_THISVECTOR(vector);
    sprintf(buffer, "%f, %f, %f",
        vector[0],
        vector[1],
        vector[2]);

    JS_RETURNSTRING(vp, buffer);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, unit2)
{
    vec3_t vector;

    JS_CHECKARGS(0);
    JS_THISVECTOR(vector);

    return JS_NewDoubleValue(cx, Vec_Unit2(vector), vp);
}

JS_FASTNATIVE_BEGIN(Vector, unit3)
{
    vec3_t vector;

    JS_CHECKARGS(0);
    JS_THISVECTOR(vector);

    return JS_NewDoubleValue(cx, Vec_Unit3(vector), vp);
}

JS_FASTNATIVE_BEGIN(Vector, toYaw)
{
    vec3_t vector;

    JS_CHECKARGS(0);
    JS_THISVECTOR(vector);

    return JS_NewDoubleValue(cx, Ang_VectorToAngle(vector), vp);
}

JS_FASTNATIVE_BEGIN(Vector, cross)
{
    vec3_t outvec;
    JSObject *obj1;
    JSObject *obj2;
    vec3_t vec1;
    vec3_t vec2;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj1, v, 0);
    JS_GETOBJECT(obj2, v, 1);
    JS_GETVECTOR2(obj1, vec1);
    JS_GETVECTOR2(obj2, vec2);

    Vec_Cross(outvec, vec1, vec2);
    
    JS_NEWVECTORPOOL(outvec);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, dot)
{
    JSObject *obj1;
    JSObject *obj2;
    vec3_t vec1;
    vec3_t vec2;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj1, v, 0);
    JS_GETOBJECT(obj2, v, 1);
    JS_GETVECTOR2(obj1, vec1);
    JS_GETVECTOR2(obj2, vec2);

    return JS_NewDoubleValue(cx, Vec_Dot(vec1, vec2), vp);
}

JS_FASTNATIVE_BEGIN(Vector, gScale)
{
    vec3_t outvec;
    vec3_t vec;
    jsdouble x;
    JSObject *obj;

    JS_CHECKARGS(2);

    JS_GETOBJECT(obj, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETVECTOR2(obj, vec);
    
    Vec_Scale(outvec, vec, (float)x);
    
    JS_NEWVECTORPOOL(outvec);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, length2)
{
    JSObject *obj1;
    JSObject *obj2;
    vec3_t vec1;
    vec3_t vec2;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj1, v, 0);
    JS_GETOBJECT(obj2, v, 1);
    JS_GETVECTOR2(obj1, vec1);
    JS_GETVECTOR2(obj2, vec2);

    return JS_NewDoubleValue(cx, Vec_Length2(vec1, vec2), vp);
}

JS_FASTNATIVE_BEGIN(Vector, length3)
{
    JSObject *obj1;
    JSObject *obj2;
    vec3_t vec1;
    vec3_t vec2;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj1, v, 0);
    JS_GETOBJECT(obj2, v, 1);
    JS_GETVECTOR2(obj1, vec1);
    JS_GETVECTOR2(obj2, vec2);

    return JS_NewDoubleValue(cx, Vec_Length3(vec1, vec2), vp);
}

JS_FASTNATIVE_BEGIN(Vector, pointToAxis)
{
    vec3_t outvec;
    JSObject *obj1;
    JSObject *obj2;
    vec3_t vec1;
    vec3_t vec2;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj1, v, 0);
    JS_GETOBJECT(obj2, v, 1);
    JS_GETVECTOR2(obj1, vec1);
    JS_GETVECTOR2(obj2, vec2);

    Vec_PointToAxis(outvec, vec1, vec2);
    
    JS_NEWVECTORPOOL(outvec);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Vector, toWorld)
{
    JSObject *obj;
    vec3_t outvec;
    vec3_t vec;
    mtx_t *mtx = NULL;

    JS_CHECKARGS(2);
    JS_GETOBJECT(obj, v, 0);
    JS_GETMATRIX(mtx, v, 1);
    JS_GETVECTOR2(obj, vec);

    Vec_TransformToWorld(*mtx, vec, outvec);
    
    JS_NEWVECTORPOOL(outvec);
    return JS_TRUE;
}

JS_BEGINCLASS(Vector)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Vector_getProperty,                         // getProperty
    Vector_setProperty,                         // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Vector)
{
    { "x",  VEC3_X, JSPROP_ENUMERATE,   NULL, NULL },
    { "y",  VEC3_Y, JSPROP_ENUMERATE,   NULL, NULL },
    { "z",  VEC3_Z, JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINFUNCS(Vector)
{
    JS_FASTNATIVE(Vector, copy,         1),
    JS_FASTNATIVE(Vector, clear,        0),
    JS_FASTNATIVE(Vector, add,          1),
    JS_FASTNATIVE(Vector, sub,          1),
    JS_FASTNATIVE(Vector, multiply,     1),
    JS_FASTNATIVE(Vector, normalize,    0),
    JS_FASTNATIVE(Vector, lerp,         2),
    JS_FASTNATIVE(Vector, scale,        1),
    JS_FASTNATIVE(Vector, toString,     0),
    JS_FASTNATIVE(Vector, unit2,        0),
    JS_FASTNATIVE(Vector, unit3,        0),
    JS_FASTNATIVE(Vector, toYaw,        0),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Vector)
{
    JS_FASTNATIVE(Vector, cross,        2),
    JS_FASTNATIVE(Vector, dot,          2),
    JS_FASTNATIVE(Vector, gScale,       2),
    JS_FASTNATIVE(Vector, length2,      2),
    JS_FASTNATIVE(Vector, length3,      2),
    JS_FASTNATIVE(Vector, pointToAxis,  2),
    JS_FASTNATIVE(Vector, toWorld,      2),
    JS_FS_END
};

JS_BEGINCONST(Vector)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_CONSTRUCTOR(Vector)
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

    JS_NEWVECTOR2(vector);
    return JS_TRUE;
}
