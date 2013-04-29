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

JS_CLASSOBJECT(Angle);

JS_FASTNATIVE_BEGIN(Angle, clamp)
{
    jsdouble x;
    float an;

    JS_CHECKARGS(1);
    JS_GETNUMBER(x, v, 0);
    an = (float)x;
    Ang_Clamp(&an);
    return JS_NewDoubleValue(cx, an, vp);
}

JS_FASTNATIVE_BEGIN(Angle, invertClamp)
{
    jsdouble x;

    JS_CHECKARGS(1);
    JS_GETNUMBER(x, v, 0);
    return JS_NewDoubleValue(cx, Ang_ClampInvert((float)x), vp);
}

JS_FASTNATIVE_BEGIN(Angle, invertClampSum)
{
    jsdouble x1;
    jsdouble x2;

    JS_CHECKARGS(2);
    JS_GETNUMBER(x1, v, 0);
    JS_GETNUMBER(x2, v, 1);
    return JS_NewDoubleValue(cx, Ang_ClampInvertSums((float)x1, (float)x2), vp);
}

JS_FASTNATIVE_BEGIN(Angle, diff)
{
    jsdouble x1;
    jsdouble x2;

    JS_CHECKARGS(2);
    JS_GETNUMBER(x1, v, 0);
    JS_GETNUMBER(x2, v, 1);
    return JS_NewDoubleValue(cx, Ang_Diff((float)x1, (float)x2), vp);
}

JS_FASTNATIVE_BEGIN(Angle, radToDeg)
{
    jsdouble x;

    JS_CHECKARGS(1);
    JS_GETNUMBER(x, v, 0);
    return JS_NewDoubleValue(cx, RAD2DEG((float)x), vp);
}

JS_FASTNATIVE_BEGIN(Angle, degToRad)
{
    jsdouble x;

    JS_CHECKARGS(1);
    JS_GETNUMBER(x, v, 0);
    return JS_NewDoubleValue(cx, DEG2RAD((float)x), vp);
}

JS_FASTNATIVE_BEGIN(Angle, alignYawToDirection)
{
    jsdouble an;
    float yaw;
    vec3_t vec1;
    vec3_t vec2;
    JSObject *obj;

    JS_CHECKARGS(3);
    JS_GETNUMBER(an, v, 0);
    JS_GETOBJECT(obj, v, 1);
    JS_GETVECTOR2(obj, vec1);
    JS_GETOBJECT(obj, v, 2);
    JS_GETVECTOR2(obj, vec2);

    yaw = Ang_AlignYawToVector((float)an, vec1, vec2);
    Ang_Clamp(&yaw);

    return JS_NewDoubleValue(cx, yaw, vp);
}

JS_BEGINCLASS(Angle)
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

JS_BEGINPROPS(Angle)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Angle)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Angle)
{
    JS_FASTNATIVE(Angle, clamp, 1),
    JS_FASTNATIVE(Angle, invertClamp, 1),
    JS_FASTNATIVE(Angle, invertClampSum, 2),
    JS_FASTNATIVE(Angle, diff, 2),
    JS_FASTNATIVE(Angle, radToDeg, 1),
    JS_FASTNATIVE(Angle, degToRad, 1),
    JS_FASTNATIVE(Angle, alignYawToDirection, 3),
    JS_FS_END
};
