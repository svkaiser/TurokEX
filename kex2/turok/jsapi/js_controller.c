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
// DESCRIPTION: Javascript Controller Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "controller.h"

JS_CLASSOBJECT(AController);

JS_PROP_FUNC_GET(AController)
{
    aController_t *ctrl;

    if(!(ctrl = (aController_t*)JS_GetInstancePrivate(cx, obj, &AController_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 9:
        return JS_NewDoubleValue(cx, ctrl->timeStamp, vp);

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(AController)
{
    aController_t *ctrl;

    if(!(ctrl = (aController_t*)JS_GetInstancePrivate(cx, obj, &AController_class, NULL)))
        return JS_TRUE;

    return JS_TRUE;
}

JS_FINALIZE_FUNC(AController)
{
    aController_t *ctrl;

    if(ctrl = (aController_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, ctrl);
}

JS_FASTNATIVE_BEGIN(AController, move)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AController, setMoveDirection)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AController, applyFriction)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AController, applyVerticalFriction)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AController, accelerate)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(AController, applyGravity)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(AController)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    AController_getProperty,                    // getProperty
    AController_setProperty,                    // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    AController_finalize,                       // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(AController)
{
    { "owner",      0,  JSPROP_ENUMERATE, NULL, NULL },
    { "velocity",   1,  JSPROP_ENUMERATE, NULL, NULL },
    { "forward",    2,  JSPROP_ENUMERATE, NULL, NULL },
    { "right",      3,  JSPROP_ENUMERATE, NULL, NULL },
    { "up",         4,  JSPROP_ENUMERATE, NULL, NULL },
    { "accel",      5,  JSPROP_ENUMERATE, NULL, NULL },
    { "angles",     6,  JSPROP_ENUMERATE, NULL, NULL },
    { "moveTime",   7,  JSPROP_ENUMERATE, NULL, NULL },
    { "frameTime",  8,  JSPROP_ENUMERATE, NULL, NULL },
    { "timeStamp",  9,  JSPROP_ENUMERATE, NULL, NULL },
    { "plane",      10, JSPROP_ENUMERATE, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(AController)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(AController)
{
    JS_FASTNATIVE(AController, move, 0),
    JS_FASTNATIVE(AController, setMoveDirection, 3),
    JS_FASTNATIVE(AController, applyFriction, 1),
    JS_FASTNATIVE(AController, applyVerticalFriction, 1),
    JS_FASTNATIVE(AController, accelerate, 3),
    JS_FASTNATIVE(AController, applyGravity, 1),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(AController)
{
    JS_FS_END
};
