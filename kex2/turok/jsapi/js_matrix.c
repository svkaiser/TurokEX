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
// DESCRIPTION: Javascript Matrix Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"
#include "gl.h"
#include "zone.h"
#include "kernel.h"

JSObject *js_objMatrix;

//
// matrix_finalize
//

static void matrix_finalize(JSContext *cx, JSObject *obj)
{
    mtx_t *mtx;

    if(mtx = (mtx_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, mtx);

    return;
}

//
// matrix_toString
//

static JSBool matrix_toString(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    char *buffer;

    JS_THISMATRIX(mtx, vp);

    buffer = Z_Alloca(512);

    strcat(buffer, kva("%f %f %f %f\n", (*mtx)[ 0], (*mtx)[ 4], (*mtx)[ 8], (*mtx)[12]));
    strcat(buffer, kva("%f %f %f %f\n", (*mtx)[ 1], (*mtx)[ 5], (*mtx)[ 9], (*mtx)[13]));
    strcat(buffer, kva("%f %f %f %f\n", (*mtx)[ 2], (*mtx)[ 6], (*mtx)[10], (*mtx)[14]));
    strcat(buffer, kva("%f %f %f %f",   (*mtx)[ 3], (*mtx)[ 7], (*mtx)[11], (*mtx)[15]));

    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, buffer)));
    return JS_TRUE;
}

//
// matrix_addTranslation
//

static JSBool matrix_addTranslation(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble x;
    jsdouble y;
    jsdouble z;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(x, vp, 2);
    JS_GETNUMBER(y, vp, 1);
    JS_GETNUMBER(z, vp, 0);

    Mtx_AddTranslation(*mtx, (float)x, (float)y, (float)z);
    return JS_TRUE;
}

//
// matrix_scale
//

static JSBool matrix_scale(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble x;
    jsdouble y;
    jsdouble z;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(x, vp, 2);
    JS_GETNUMBER(y, vp, 1);
    JS_GETNUMBER(z, vp, 0);

    Mtx_Scale(*mtx, (float)x, (float)y, (float)z);
    return JS_TRUE;
}

//
// matrix_setTranslation
//

static JSBool matrix_setTranslation(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble x;
    jsdouble y;
    jsdouble z;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(x, vp, 2);
    JS_GETNUMBER(y, vp, 1);
    JS_GETNUMBER(z, vp, 0);

    Mtx_SetTranslation(*mtx, (float)x, (float)y, (float)z);
    return JS_TRUE;
}

//
// matrix_transpose
//

static JSBool matrix_transpose(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;

    JS_THISMATRIX(mtx, vp);
    Mtx_Transpose(*mtx);

    return JS_TRUE;
}

//
// matrix_identity
//

static JSBool matrix_identity(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;

    JS_THISMATRIX(mtx, vp);
    Mtx_Identity(*mtx);

    return JS_TRUE;
}

//
// matrix_identityX
//

static JSBool matrix_identityX(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble angle;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(angle, vp, 2);

    Mtx_IdentityX(*mtx, (float)angle);

    return JS_TRUE;
}

//
// matrix_identityY
//

static JSBool matrix_identityY(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble angle;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(angle, vp, 2);

    Mtx_IdentityY(*mtx, (float)angle);

    return JS_TRUE;
}

//
// matrix_identityZ
//

static JSBool matrix_identityZ(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble angle;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(angle, vp, 2);

    Mtx_IdentityZ(*mtx, (float)angle);

    return JS_TRUE;
}

//
// matrix_applyVector
//

static JSBool matrix_applyVector(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    vec3_t *vector;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETVECTOR(vector, vp, 2);

    Mtx_ApplyVector(*mtx, *vector);

    return JS_TRUE;
}

//
// matrix_rotateX
//

static JSBool matrix_rotateX(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble angle;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(angle, vp, 2);

    Mtx_RotateX(*mtx, (float)angle);

    return JS_TRUE;
}

//
// matrix_rotateY
//

static JSBool matrix_rotateY(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble angle;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(angle, vp, 2);

    Mtx_RotateY(*mtx, (float)angle);

    return JS_TRUE;
}

//
// matrix_rotateZ
//

static JSBool matrix_rotateZ(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;
    jsdouble angle;

    if(argc <= 0)
        return JS_FALSE;

    JS_THISMATRIX(mtx, vp);
    JS_GETNUMBER(angle, vp, 2);

    Mtx_RotateZ(*mtx, (float)angle);

    return JS_TRUE;
}

//
// matrix_load
//

static JSBool matrix_load(JSContext *cx, uintN argc, jsval *vp)
{
    mtx_t *mtx;

    JS_THISMATRIX(mtx, vp);
    dglLoadMatrixf(*mtx);

    return JS_TRUE;
}

//
// matrix_modelview
//

static JSBool matrix_modelview(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    dglMatrixMode(GL_MODELVIEW);
    return JS_TRUE;
}

//
// matrix_setProjection
//

static JSBool matrix_setProjection(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    jsval *v;
    jsdouble fov;
    jsdouble znear;

    if(argc <= 0)
        return JS_FALSE;

    v = JS_ARGV(cx, argv);
    JS_GETNUMBER(fov, v, -2);
    JS_GETNUMBER(znear, v, -1);

    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    Mtx_ViewFrustum(video_width, video_height,
        (float)fov, (float)znear);

    return JS_TRUE;
}

//
// matrix_multiply
//

static JSBool matrix_multiply(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    JSObject *nobj;
    jsval *v;
    mtx_t *outmtx = NULL;
    mtx_t *mtx1 = NULL;
    mtx_t *mtx2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    if(!(nobj = JS_NewObject(cx, &Matrix_class, NULL, NULL)))
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    JS_GETMATRIX(mtx1, v, -2);
    JS_GETMATRIX(mtx2, v, -1);

    outmtx = (mtx_t*)JS_malloc(cx, sizeof(mtx_t));
    Mtx_Multiply(*outmtx, *mtx1, *mtx2);

    if(!(JS_SetPrivate(cx, nobj, outmtx)))
        return JS_FALSE;

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(nobj));
    return JS_TRUE;
}

//
// matrix_multRotations
//

static JSBool matrix_multRotations(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
    JSObject *nobj;
    jsval *v;
    mtx_t *outmtx = NULL;
    mtx_t *mtx1 = NULL;
    mtx_t *mtx2 = NULL;

    if(argc <= 0)
        return JS_FALSE;

    if(!(nobj = JS_NewObject(cx, &Matrix_class, NULL, NULL)))
        return JS_FALSE;

    v = JS_ARGV(cx, argv);

    JS_GETMATRIX(mtx1, v, -2);
    JS_GETMATRIX(mtx2, v, -1);

    outmtx = (mtx_t*)JS_malloc(cx, sizeof(mtx_t));
    Mtx_MultiplyRotation(*outmtx, *mtx1, *mtx2);

    if(!(JS_SetPrivate(cx, nobj, outmtx)))
        return JS_FALSE;

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(nobj));
    return JS_TRUE;
}

//
// Matrix_class
//

JSClass Matrix_class =
{
    "Matrix",                                   // name
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    JS_PropertyStub,                            // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    matrix_finalize,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// Matrix_props
//

JSPropertySpec Matrix_props[] =
{
    { NULL, 0, 0, NULL, NULL }
};

//
// Matrix_functions
//

JSFunctionSpec Matrix_functions[] =
{
    JS_FN("toString",       matrix_toString,        0, 0, 0),
    JS_FN("addTranslation", matrix_addTranslation,  3, 0, 0),
    JS_FN("scale",          matrix_scale,           3, 0, 0),
    JS_FN("setTranslation", matrix_setTranslation,  3, 0, 0),
    JS_FN("transpose",      matrix_transpose,       0, 0, 0),
    JS_FN("identity",       matrix_identity,        0, 0, 0),
    JS_FN("identityX",      matrix_identityX,       1, 0, 0),
    JS_FN("identityY",      matrix_identityY,       1, 0, 0),
    JS_FN("identityZ",      matrix_identityZ,       1, 0, 0),
    JS_FN("applyVector",    matrix_applyVector,     1, 0, 0),
    JS_FN("rotateX",        matrix_rotateX,         1, 0, 0),
    JS_FN("rotateY",        matrix_rotateY,         1, 0, 0),
    JS_FN("rotateZ",        matrix_rotateZ,         1, 0, 0),
    JS_FN("load",           matrix_load,            0, 0, 0),
    JS_FS_END
};

//
// Matrix_functions_static
//

JSFunctionSpec Matrix_functions_static[] =
{
    JS_FS("setProjection",  matrix_setProjection,   1, 0, 0),
    JS_FS("setModelView",   matrix_modelview,       0, 0, 0),
    JS_FS("multiply",       matrix_multiply,        2, 0, 0),
    JS_FS("multRotations",  matrix_multRotations,   2, 0, 0),
    JS_FS_END
};

//
// Matrix_construct
//

JSBool Matrix_construct(JSContext *cx, JSObject *obj, uintN argc,
                        jsval *argv, jsval *rval)
{
    JSObject *vobj;
    mtx_t *mtx;

    if(!(vobj = JS_NewObject(cx, &Matrix_class, NULL, NULL)))
        return JS_FALSE;

    mtx = (mtx_t*)JS_malloc(cx, sizeof(mtx_t));

    if(!(JS_SetPrivate(cx, vobj, mtx)))
        return JS_FALSE;

    Mtx_Identity(*mtx);

    JS_SET_RVAL(cx, rval, OBJECT_TO_JSVAL(vobj));

    return JS_TRUE;
}
