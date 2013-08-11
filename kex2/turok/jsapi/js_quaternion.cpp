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
// DESCRIPTION: Javascript Quaternion Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"

JS_CLASSOBJECT(Quaternion);

enum vec4_enum
{
    VEC4_X,
    VEC4_Y,
    VEC4_Z,
    VEC4_W
};

JS_PROP_FUNC_GET(Quaternion)
{
    vec4_t *vector = NULL;

    if(vector = (vec4_t*)JS_GetInstancePrivate(cx, obj, &Quaternion_class, NULL))
    {
        switch(JSVAL_TO_INT(id))
        {
        case VEC4_X:
            return J_NewDoubleEx(cx, (*vector)[0], vp);
        case VEC4_Y:
            return J_NewDoubleEx(cx, (*vector)[1], vp);
        case VEC4_Z:
            return J_NewDoubleEx(cx, (*vector)[2], vp);
        case VEC4_W:
            return J_NewDoubleEx(cx, (*vector)[2], vp);
        }
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(Quaternion)
{
    vec4_t *vector = NULL;

    if(vector = (vec4_t*)JS_GetInstancePrivate(cx, obj, &Quaternion_class, NULL))
    {
        jsdouble val;

        if(!(JS_ValueToNumber(cx, *vp, &val)))
            return JS_FALSE;

        switch(JSVAL_TO_INT(id))
        {
        case VEC4_X:
            (*vector)[0] = (float)val;
            break;
        case VEC4_Y:
            (*vector)[1] = (float)val;
            break;
        case VEC4_Z:
            (*vector)[2] = (float)val;
            break;
        case VEC4_W:
            (*vector)[2] = (float)val;
            break;
        }
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Quaternion, toString)
{
    vec4_t rot;
    JSObject *object;
    char buffer[256];

    object = JS_THIS_OBJECT(cx, vp);

    JS_GETQUATERNION2(object, rot);

    sprintf(buffer, "%f, %f, %f, %f",
        rot[0], rot[1], rot[2], rot[3]);

    JS_RETURNSTRING(vp, buffer);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Quaternion, normalize)
{
    vec4_t rot;
    JSObject *object;

    object = JS_THIS_OBJECT(cx, vp);

    JS_GETQUATERNION2(object, rot);

    Vec_Normalize4(rot);

    JS_SETQUATERNION(object, rot);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Quaternion, setRotation)
{
    vec4_t rot;
    JSObject *object;
    jsdouble angle;
    jsdouble x, y, z;

    JS_CHECKARGS(4);

    object = JS_THIS_OBJECT(cx, vp);

    JS_GETQUATERNION2(object, rot);

    JS_GETNUMBER(angle, v, 0);
    JS_GETNUMBER(x, v, 1);
    JS_GETNUMBER(y, v, 2);
    JS_GETNUMBER(z, v, 3);

    Vec_SetQuaternion(rot, (float)angle, (float)x, (float)y, (float)z);

    JS_SETQUATERNION(object, rot);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Quaternion, multiply)
{
    vec4_t rot1;
    vec4_t rot2;
    vec4_t out;
    JSObject *obj1;
    JSObject *obj2;

    JS_CHECKARGS(2);

    JS_GETOBJECT(obj1, v, 0);
    JS_GETOBJECT(obj2, v, 1);
    JS_GETQUATERNION2(obj1, rot1);
    JS_GETQUATERNION2(obj2, rot2);

    Vec_MultQuaternion(out, rot1, rot2);

    JS_NEWQUATERNIONPOOL(out);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Quaternion, pointTo)
{
    // TODO
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(JS_THIS_OBJECT(cx, vp)));
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Quaternion, getAxis)
{
    vec4_t rot;
    vec3_t out;
    JSObject *obj;
    float an;

    JS_CHECKARGS(1);
    JS_GETOBJECT(obj, v, 0);
    JS_GETQUATERNION2(obj, rot);

    Vec_QuaternionToAxis(&an, out, rot);

    JS_NEWVECTORPOOL(out);
    return JS_TRUE;
}

JS_BEGINCLASS(Quaternion)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Quaternion_getProperty,                     // getProperty
    Quaternion_setProperty,                     // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Quaternion)
{
    { "x",  VEC4_X, JSPROP_ENUMERATE,   NULL, NULL },
    { "y",  VEC4_Y, JSPROP_ENUMERATE,   NULL, NULL },
    { "z",  VEC4_Z, JSPROP_ENUMERATE,   NULL, NULL },
    { "w",  VEC4_W, JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINFUNCS(Quaternion)
{
    JS_FASTNATIVE(Quaternion, toString, 0),
    JS_FASTNATIVE(Quaternion, normalize, 0),
    JS_FASTNATIVE(Quaternion, setRotation, 4),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Quaternion)
{
    JS_FASTNATIVE(Quaternion, multiply, 2),
    JS_FASTNATIVE(Quaternion, pointTo, 2),
    JS_FASTNATIVE(Quaternion, getAxis, 1),
    JS_FS_END
};

JS_BEGINCONST(Quaternion)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_CONSTRUCTOR(Quaternion)
{
    jsval *v;
    vec4_t rot;
    jsdouble x;
    jsdouble y;
    jsdouble z;
    jsdouble angle;

    v = JS_ARGV(cx, argv);

    JS_GETNUMBER(angle, v, -2);
    JS_GETNUMBER(x, v, -1);
    JS_GETNUMBER(y, v, 0);
    JS_GETNUMBER(z, v, 1);

    Vec_SetQuaternion(rot, (float)angle, (float)x, (float)y, (float)z);

    JS_NEWQUATERNION(rot);
    return JS_TRUE;
}
