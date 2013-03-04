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
// DESCRIPTION: Javascript Angle Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"

JSObject *js_objAngle;

//
// angle_clamp
//

static JSBool angle_clamp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    jsdouble x;
    float an;

    if(argc <= 0)
        return JS_FALSE;

    JS_GETNUMBER(x, argv, 0);
    an = (float)x;
    Ang_Clamp(&an);

    return JS_NewDoubleValue(cx, an, rval);
}

//
// angle_invert
//

static JSBool angle_invertClamp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    jsdouble x;

    if(argc <= 0)
        return JS_FALSE;

    JS_GETNUMBER(x, argv, 0);

    return JS_NewDoubleValue(cx, Ang_ClampInvert((float)x), rval);
}

//
// angle_invertSums
//

static JSBool angle_invertClampSum(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    jsdouble x1;
    jsdouble x2;

    if(argc <= 0)
        return JS_FALSE;

    JS_GETNUMBER(x1, argv, 0);
    JS_GETNUMBER(x2, argv, 1);

    return JS_NewDoubleValue(cx, Ang_ClampInvertSums((float)x1, (float)x2), rval);
}

//
// angle_diff
//

static JSBool angle_diff(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    jsdouble x1;
    jsdouble x2;

    if(argc <= 0)
        return JS_FALSE;

    JS_GETNUMBER(x1, argv, 0);
    JS_GETNUMBER(x2, argv, 1);

    return JS_NewDoubleValue(cx, Ang_Diff((float)x1, (float)x2), rval);
}

//
// angle_radToDeg
//

static JSBool angle_radToDeg(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    jsdouble x;

    if(argc <= 0)
        return JS_FALSE;

    JS_GETNUMBER(x, argv, 0);

    return JS_NewDoubleValue(cx, RAD2DEG((float)x), rval);
}

//
// angle_degToRad
//

static JSBool angle_degToRad(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    jsdouble x;

    if(argc <= 0)
        return JS_FALSE;

    JS_GETNUMBER(x, argv, 0);

    return JS_NewDoubleValue(cx, DEG2RAD((float)x), rval);
}

//
// system_class
//

JSClass Angle_class =
{
    "Angle",                                    // name
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
};

//
// Angle_props
//

JSPropertySpec Angle_props[] =
{
    { NULL, 0, 0, NULL, NULL }
};

//
// Angle_const
//

JSConstDoubleSpec Angle_const[] =
{
    { 0, 0, 0, { 0, 0, 0 } }
};

//
// Angle_functions
//

JSFunctionSpec Angle_functions[] =
{
    JS_FS("clamp",          angle_clamp,            1, 0, 0),
    JS_FS("invertClamp",    angle_invertClamp,      1, 0, 0),
    JS_FS("invertClampSum", angle_invertClampSum,   2, 0, 0),
    JS_FS("diff",           angle_diff,             2, 0, 0),
    JS_FS("radToDeg",       angle_radToDeg,         1, 0, 0),
    JS_FS("degToRad",       angle_degToRad,         1, 0, 0),
    JS_FS_END
};
