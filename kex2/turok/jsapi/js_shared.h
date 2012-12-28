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

#ifndef _JS_SHARED_H_
#define _JS_SHARED_H_

#include "jsapi.h"
#include "shared.h"

extern JSContext    *js_context;
extern JSObject     *js_gobject;

typedef struct js_scrobj_s
{
    char name[MAX_FILEPATH];
    JSScript *script;
    JSObject *obj;
    struct js_scrobj_s *next;
} js_scrobj_t;

jsval J_CallFunctionOnObject(JSContext *cx, JSObject *object, const char *function);
jsval J_GetObjectElement(JSContext *cx, JSObject *object, jsint index);

js_scrobj_t *J_LoadScript(const char *name);
void J_ExecScriptObj(js_scrobj_t *scobj);

#define JS_DEFINEOBJECT(name)                                                       \
    js_obj ##name = J_AddObject(&name ## _class, name ## _functions,                \
    name ## _props, name ## _const, # name, js_context, js_gobject)

#define JS_INITCLASS(name, args)                                                    \
    js_obj ##name = JS_InitClass(js_context, js_gobject, NULL,                      \
    &name ## _class, name ## _construct, args, name ## _props,                      \
    name ## _functions, NULL, name ## _functions_static)

#define JS_INITCLASS_NOSTATIC(name, args)                                           \
    js_obj ##name = JS_InitClass(js_context, js_gobject, NULL,                      \
    &name ## _class, NULL, args, name ## _props,                                    \
    name ## _functions, NULL, NULL)

#define JS_EXTERNOBJECT(name)                                                       \
    extern JSObject *js_obj ##name;                                                 \
    extern JSClass name ## _class;                                                  \
    extern JSFunctionSpec name ## _functions[];                                     \
    extern JSPropertySpec name ## _props[];                                         \
    extern JSConstDoubleSpec name ## _const[]

#define JS_EXTERNCLASS(name)                                                        \
    extern JSObject *js_obj ##name;                                                 \
    extern JSClass name ## _class;                                                  \
    extern JSFunctionSpec name ## _functions[];                                     \
    extern JSFunctionSpec name ## _functions_static[];                              \
    extern JSPropertySpec name ## _props[];                                         \
    JSBool name ## _construct(JSContext *cx, JSObject *obj, uintN argc,             \
                        jsval *argv, jsval *rval)

#define JS_EXTERNCLASS_NOSTATIC(name)                                               \
    extern JSObject *js_obj ##name;                                                 \
    extern JSClass name ## _class;                                                  \
    extern JSFunctionSpec name ## _functions[];                                     \
    extern JSPropertySpec name ## _props[];                                         

#define JS_GETVECTOR(vec, v, a)                                                     \
{                                                                                   \
    JSObject *vobj; if(!JS_ValueToObject(cx, v[a], &vobj)) return JS_FALSE;         \
    if(JSVAL_IS_NULL(v[a])) return JS_FALSE;                                        \
    if(!(JS_InstanceOf(cx, vobj, &Vector_class, NULL))) return JS_FALSE;            \
    if(!(vec = (vec3_t*)JS_GetInstancePrivate(cx, vobj, &Vector_class, NULL)))      \
        return JS_FALSE;                                                            \
}

#define JS_GETQUATERNION(vec, v, a)                                                 \
{                                                                                   \
    JSObject *vobj; if(!JS_ValueToObject(cx, v[a], &vobj)) return JS_FALSE;         \
    if(JSVAL_IS_NULL(v[a])) return JS_FALSE;                                        \
    if(!(JS_InstanceOf(cx, vobj, &Quaternion_class, NULL))) return JS_FALSE;        \
    if(!(vec = (vec4_t*)JS_GetInstancePrivate(cx, vobj, &Quaternion_class, NULL)))  \
        return JS_FALSE;                                                            \
}

#define JS_GETMATRIX(mtx, v, a)                                                     \
{                                                                                   \
    JSObject *vobj; if(!JS_ValueToObject(cx, v[a], &vobj)) return JS_FALSE;         \
    if(JSVAL_IS_NULL(v[a])) return JS_FALSE;                                        \
    if(!(JS_InstanceOf(cx, vobj, &Matrix_class, NULL))) return JS_FALSE;            \
    if(!(mtx = (mtx_t*)JS_GetInstancePrivate(cx, vobj, &Matrix_class, NULL)))       \
        return JS_FALSE;                                                            \
}

#define JS_GETNUMBER(val, v, a)                                                     \
    JS_ValueToNumber(cx, v[a], &val);                                               \
    if(JSVAL_IS_NULL(v[a]))                                                         \
        return JS_FALSE

#define JS_GETBOOL(val, v, a)                                                       \
    JS_ValueToBoolean(cx, v[a], &val);                                              \
    if(JSVAL_IS_NULL(v[a]))                                                         \
        return JS_FALSE

#define JS_GETOBJECT(val, v, a)                                                     \
    JS_ValueToObject(cx, v[a], &val);                                               \
    if(JSVAL_IS_NULL(v[a]))                                                         \
        return JS_FALSE

#define JS_THISVECTOR(vec, v)                                                       \
    if(!(vec = (vec3_t*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, v),            \
        &Vector_class, NULL)))                                                      \
        return JS_FALSE

#define JS_THISQUATERNION(vec, v)                                                   \
    if(!(vec = (vec4_t*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, v),            \
        &Quaternion_class, NULL)))                                                  \
        return JS_FALSE

#define JS_THISMATRIX(mtx, v)                                                       \
    if(!(mtx = (mtx_t*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, v),             \
        &Matrix_class, NULL)))                                                      \
        return JS_FALSE

#define JS_THISPLANE(pl, v)                                                         \
    if(!(pl = (plane_t*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, v),            \
        &Plane_class, NULL)))                                                       \
        return JS_FALSE

#define JS_INSTVECTOR(vp, vec)                                                      \
{                                                                                   \
    JSObject *nobj;                                                                 \
    if(!(nobj = JS_NewObject(cx, &Vector_class, NULL, NULL)))                       \
        return JS_FALSE;                                                            \
    if(!(JS_SetPrivate(cx, nobj, vec)))                                             \
        return JS_FALSE;                                                            \
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(nobj));                                     \
}

#define JS_NEWVECTOR(vp, vec)                                                       \
{                                                                                   \
    JSObject *nobj;                                                                 \
    vec3_t *out;                                                                    \
    if(!(nobj = JS_NewObject(cx, &Vector_class, NULL, NULL)))                       \
        return JS_FALSE;                                                            \
    out = (vec3_t*)JS_malloc(cx, sizeof(vec3_t));                                   \
    Vec_Copy3(*out, vec);                                                           \
    if(!(JS_SetPrivate(cx, nobj, out)))                                             \
    {                                                                               \
        JS_free(cx, out);                                                           \
        return JS_FALSE;                                                            \
    }                                                                               \
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(nobj));                                     \
}

#define JS_INSTPLANE(vp, pl)                                                        \
{                                                                                   \
    JSObject *nobj;                                                                 \
    if(!(nobj = JS_NewObject(cx, &Plane_class, NULL, NULL)))                        \
        return JS_FALSE;                                                            \
    if(!(JS_SetPrivate(cx, nobj, pl)))                                              \
        return JS_FALSE;                                                            \
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(nobj));                                     \
}

#define JS_RETURNOBJECT(vp)                                                         \
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(JS_THIS_OBJECT(cx, vp)))                    \

#define JS_RETURNSTRING(vp, string)                                                 \
    JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, string)))

JS_EXTERNOBJECT(Sys);
JS_EXTERNOBJECT(GL);
JS_EXTERNOBJECT(Client);
JS_EXTERNOBJECT(Cmd);
JS_EXTERNOBJECT(Angle);
JS_EXTERNOBJECT(MoveController);
JS_EXTERNOBJECT(MapProperty);
JS_EXTERNCLASS(Vector);
JS_EXTERNCLASS(Quaternion);
JS_EXTERNCLASS(Matrix);
JS_EXTERNCLASS_NOSTATIC(Plane);

#endif

