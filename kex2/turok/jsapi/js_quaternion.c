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

JSObject *js_objQuaternion;

enum vec4_enum
{
    VEC4_X,
    VEC4_Y,
    VEC4_Z,
    VEC4_W
};

//
// quaternion_getProperty
//

static JSBool quaternion_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    vec4_t *vector = NULL;

    if(!(vector = (vec4_t*)JS_GetInstancePrivate(cx, obj, &Quaternion_class, NULL)))
        return JS_FALSE;

    switch(JSVAL_TO_INT(id))
    {
    case VEC4_X:
        return JS_NewNumberValue(cx, (*vector)[0], vp);
        break;
    case VEC4_Y:
        return JS_NewNumberValue(cx, (*vector)[1], vp);
        break;
    case VEC4_Z:
        return JS_NewNumberValue(cx, (*vector)[2], vp);
        break;
    case VEC4_W:
        return JS_NewNumberValue(cx, (*vector)[3], vp);
        break;
    }

    return JS_FALSE;
}

//
// quaternion_setProperty
//

static JSBool quaternion_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    vec4_t *vector = NULL;
    jsdouble val;

    if(!(vector = (vec4_t*)JS_GetInstancePrivate(cx, obj, &Quaternion_class, NULL)))
        return JS_FALSE;

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
        (*vector)[3] = (float)val;
        break;
    }

    return JS_TRUE;
}

//
// quaternion_finalize
//

static void quaternion_finalize(JSContext *cx, JSObject *obj)
{
    vec4_t *rot;

    if(rot = (vec4_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, rot);

    return;
}

//
// quaternion_toString
//

static JSBool quaternion_toString(JSContext *cx, uintN argc, jsval *vp)
{
    vec4_t *vector;
    char buffer[256];

    JS_THISQUATERNION(vector, vp);
    sprintf(buffer, "%f, %f, %f, %f",
        (*vector)[0],
        (*vector)[1],
        (*vector)[2],
        (*vector)[3]);

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, buffer)));
    return JS_TRUE;
}

//
// quaternion_normalize
//

static JSBool quaternion_normalize(JSContext *cx, uintN argc, jsval *vp)
{
    vec4_t *vector;

    JS_THISQUATERNION(vector, vp);

    Vec_Normalize4(*vector);
    return JS_TRUE;
}

//
// quaternion_multiply
//

static JSBool quaternion_multiply(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    JSObject *nobj;
    jsval *v;
    vec4_t *outrot = NULL;
    vec4_t *rot1 = NULL;
    vec4_t *rot2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    if(!(nobj = JS_NewObject(cx, &Quaternion_class, NULL, NULL)))
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    JS_GETQUATERNION(rot1, v, -2);
    JS_GETQUATERNION(rot2, v, -1);

    outrot = (vec4_t*)JS_malloc(cx, sizeof(vec4_t));
    Vec_MultQuaternion(*outrot, *rot1, *rot2);

    if(!(JS_SetPrivate(cx, nobj, outrot)))
        return JS_FALSE;

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(nobj));
    return JS_TRUE;
}

//
// quaternion_pointTo
//

static JSBool quaternion_pointTo(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    JSObject *nobj;
    jsval *v;
    vec4_t *outvec = NULL;
    vec3_t *vec1 = NULL;
    vec3_t *vec2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    if(!(nobj = JS_NewObject(cx, &Quaternion_class, NULL, NULL)))
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    JS_GETVECTOR(vec1, v, -2);
    JS_GETVECTOR(vec2, v, -1);

    outvec = (vec4_t*)JS_malloc(cx, sizeof(vec4_t));
    Vec_PointToAngle(*outvec, *vec1, *vec2);

    if(!(JS_SetPrivate(cx, nobj, outvec)))
        return JS_FALSE;

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(nobj));
    return JS_TRUE;
}

//
// Quaternion_class
//

JSClass Quaternion_class =
{
    "Quaternion",                               // name
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    quaternion_getProperty,                     // getProperty
    quaternion_setProperty,                     // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    quaternion_finalize,                        // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// Quaternion_props
//

JSPropertySpec Quaternion_props[] =
{
    { "x",  VEC4_X, JSPROP_ENUMERATE,   NULL, NULL },
    { "y",  VEC4_Y, JSPROP_ENUMERATE,   NULL, NULL },
    { "z",  VEC4_Z, JSPROP_ENUMERATE,   NULL, NULL },
    { "w",  VEC4_W, JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

//
// Quaternion_functions
//

JSFunctionSpec Quaternion_functions[] =
{
    JS_FN("toString",   quaternion_toString,    0, 0, 0),
    JS_FN("normalize",  quaternion_normalize,   0, 0, 0),
    JS_FS_END
};

//
// Quaternion_functions_static
//

JSFunctionSpec Quaternion_functions_static[] =
{
    JS_FS("multiply",   quaternion_multiply,    2, 0, 0),
    JS_FS("pointTo",    quaternion_pointTo,     2, 0, 0),
    JS_FS_END
};

//
// Quaternion_construct
//

JSBool Quaternion_construct(JSContext *cx, JSObject *obj, uintN argc,
                        jsval *argv, jsval *rval)
{
    JSObject *vobj;
    jsval *v;
    vec4_t *rot;
    jsdouble x;
    jsdouble y;
    jsdouble z;
    jsdouble angle;

    if(!(vobj = JS_NewObject(cx, &Quaternion_class, NULL, NULL)))
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    rot = (vec4_t*)JS_malloc(cx, sizeof(vec4_t));

    if(!(JS_SetPrivate(cx, vobj, rot)))
        return JS_FALSE;

    JS_GETNUMBER(angle, v, -2);
    JS_GETNUMBER(x, v, -1);
    JS_GETNUMBER(y, v, 0);
    JS_GETNUMBER(z, v, 1);

    Vec_SetQuaternion(*rot, (float)angle, (float)x, (float)y, (float)z);

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(vobj));

    return JS_TRUE;
}
