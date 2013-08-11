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
// DESCRIPTION: Javascript Animation Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "gl.h"
#include "render.h"

// -----------------------------------------------
//
// ANIMATION CLASS
//
// -----------------------------------------------

JS_CLASSOBJECT(Animation);

JS_PROP_FUNC_GET(Animation)
{
    return JS_TRUE;
}

JS_FINALIZE_FUNC(Animation)
{
    //anim_t *anim;

    //if(anim = (anim_t*)JS_GetPrivate(cx, obj))
        //JS_free(cx, anim);
}

JS_BEGINCLASS(Animation)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Animation_getProperty,                      // getProperty
    JS_PropertyStub,                            // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    Animation_finalize,                         // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Animation)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Animation)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Animation)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Animation)
{
    JS_FS_END
};

// -----------------------------------------------
//
// ANIMSTATE CLASS
//
// -----------------------------------------------

enum animstate_enum
{
    ANS_FLAGS,
    ANS_TIME,
    ANS_FRAMETIME,
    ANS_PLAYTIME,
    ANS_ROOTMOTION,
    ANS_RESTARTFRAME,
    ANS_ANIMID,
};

JS_CLASSOBJECT(AnimState);

JS_PROP_FUNC_GET(AnimState)
{
    animstate_t *animstate;

    JS_GET_PRIVATE_DATA(obj, &AnimState_class, animstate_t, animstate);

    switch(JSVAL_TO_INT(id))
    {
    case ANS_FLAGS:
        //return JS_NewNumberValue(cx, animstate->flags, vp);
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(animstate->flags));
        return JS_TRUE;

    case ANS_TIME:
        return J_NewDoubleEx(cx, animstate->time, vp);

    case ANS_FRAMETIME:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(animstate->track.frame));
        return JS_TRUE;

    case ANS_PLAYTIME:
        return J_NewDoubleEx(cx, animstate->playtime, vp);

    case ANS_ROOTMOTION:
        JS_NEWVECTORPOOL(animstate->rootMotion);
        return JS_TRUE;

    case ANS_RESTARTFRAME:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(animstate->restartframe));
        return JS_TRUE;

    case ANS_ANIMID:
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(animstate->track.anim->animID));
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_PROP_FUNC_SET(AnimState)
{
    animstate_t *animstate;
    jsval *v = vp;
    jsdouble val;

    JS_GET_PRIVATE_DATA(obj, &AnimState_class, animstate_t, animstate);

    switch(JSVAL_TO_INT(id))
    {
    case ANS_FLAGS:
        JS_CHECKINTEGER(0);
        animstate->flags = JSVAL_TO_INT(JS_ARG(0));
        break;

    case ANS_FRAMETIME:
        JS_GETNUMBER(val, vp, 0);
        animstate->frametime = (float)val;
        break;

    case ANS_PLAYTIME:
        JS_GETNUMBER(val, vp, 0);
        animstate->playtime = (float)val;
        break;

    case ANS_RESTARTFRAME:
        JS_CHECKINTEGER(0);
        animstate->restartframe = JSVAL_TO_INT(JS_ARG(0));
        if(animstate->restartframe >= animstate->track.anim->numframes)
            animstate->restartframe = animstate->track.anim->numframes-1;
        break;
    }

    return JS_TRUE;
}

JS_FINALIZE_FUNC(AnimState)
{
    /*animstate_t *animstate;

    if(animstate = (animstate_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, animstate);*/
}

JS_FASTNATIVE_BEGIN(AnimState, setAnim)
{
    JSObject *object;
    jsdouble speed;
    anim_t *anim;
    animstate_t *animstate;

    JS_CHECKARGS(3);
    JS_GET_PRIVATE_DATA(JS_THIS_OBJECT(cx, vp), &AnimState_class,
        animstate_t, animstate);
    JS_GETOBJECT(object, v, 0);
    JS_GET_PRIVATE_DATA(object, &Animation_class, anim_t, anim);
    JS_GETNUMBER(speed, v, 1);
    JS_CHECKINTEGER(2);

    Mdl_SetAnimState(animstate, anim, (float)speed, JSVAL_TO_INT(JS_ARG(2)));

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AnimState, blendAnim)
{
    JSObject *object;
    jsdouble speed;
    jsdouble blendtime;
    anim_t *anim;
    animstate_t *animstate;

    JS_CHECKARGS(4);
    JS_GET_PRIVATE_DATA(JS_THIS_OBJECT(cx, vp), &AnimState_class,
        animstate_t, animstate);
    JS_GETOBJECT(object, v, 0);
    JS_GET_PRIVATE_DATA(object, &Animation_class, anim_t, anim);
    JS_GETNUMBER(speed, v, 1);
    JS_GETNUMBER(blendtime, v, 2);
    JS_CHECKINTEGER(3);

    Mdl_BlendAnimStates(animstate, anim, (float)speed, (float)blendtime, JSVAL_TO_INT(JS_ARG(3)));

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AnimState, update)
{
    animstate_t *animstate;

    JS_CHECKARGS(0);
    JS_GET_PRIVATE_DATA(JS_THIS_OBJECT(cx, vp), &AnimState_class,
        animstate_t, animstate);

    Mdl_UpdateAnimState(animstate);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(AnimState)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    AnimState_getProperty,                      // getProperty
    AnimState_setProperty,                      // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    AnimState_finalize,                         // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(AnimState)
{
    { "flags",          ANS_FLAGS,          JSPROP_ENUMERATE,                   NULL, NULL },
    { "time",           ANS_TIME,           JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "frame",          ANS_FRAMETIME,      JSPROP_ENUMERATE,                   NULL, NULL },
    { "playTime",       ANS_PLAYTIME,       JSPROP_ENUMERATE,                   NULL, NULL },
    { "rootMotion",     ANS_ROOTMOTION,     JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { "restartFrame",   ANS_RESTARTFRAME,   JSPROP_ENUMERATE,                   NULL, NULL },
    { "animID",         ANS_ANIMID,         JSPROP_ENUMERATE|JSPROP_READONLY,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(AnimState)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(AnimState)
{
    JS_FASTNATIVE(AnimState, setAnim, 3),
    JS_FASTNATIVE(AnimState, blendAnim, 4),
    JS_FASTNATIVE(AnimState, update, 0),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(AnimState)
{
    JS_FS_END
};

JS_CONSTRUCTOR(AnimState)
{
    animstate_t *animstate;

    animstate = (animstate_t*)JS_malloc(cx, sizeof(animstate_t));
    if(animstate == NULL)
        return JS_FALSE;

    memset(animstate, 0, sizeof(animstate_t));

    JS_NEWOBJECT_SETPRIVATE(animstate, &AnimState_class);
    return JS_TRUE;
}
