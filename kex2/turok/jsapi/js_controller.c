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

JS_CLASSOBJECT(Controller);

JS_PROP_FUNC_GET(Controller)
{
    aController_t *ctrl;

    if(!(ctrl = (aController_t*)JS_GetInstancePrivate(cx, obj, &Controller_class, NULL)))
        return JS_TRUE;

    return JS_TRUE;
}

JS_PROP_FUNC_SET(Controller)
{
    aController_t *ctrl;

    if(!(ctrl = (aController_t*)JS_GetInstancePrivate(cx, obj, &Controller_class, NULL)))
        return JS_TRUE;

    return JS_TRUE;
}

JS_FINALIZE_FUNC(Controller)
{
    aController_t *ctrl;

    if(ctrl = (aController_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, ctrl);
}

JS_FASTNATIVE_BEGIN(Controller, move)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Controller, setMoveDirection)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Controller, applyFriction)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Controller, applyVerticalFriction)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Controller, accelerate)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Controller, applyGravity)
{
    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(Controller)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Controller_getProperty,                     // getProperty
    Controller_setProperty,                     // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    Controller_finalize,                        // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Controller)
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

JS_BEGINCONST(Controller)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Controller)
{
    JS_FASTNATIVE(Controller, move, 0),
    JS_FASTNATIVE(Controller, setMoveDirection, 3),
    JS_FASTNATIVE(Controller, applyFriction, 1),
    JS_FASTNATIVE(Controller, applyVerticalFriction, 1),
    JS_FASTNATIVE(Controller, accelerate, 3),
    JS_FASTNATIVE(Controller, applyGravity, 1),
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Controller)
{
    JS_FS_END
};

JS_CONSTRUCTOR(Controller)
{
    aController_t *ctrl;

    ctrl = (aController_t*)JS_malloc(cx, sizeof(animstate_t));
    if(ctrl == NULL)
        return JS_FALSE;

    memset(ctrl, 0, sizeof(aController_t));

    JS_NEWOBJECT_SETPRIVATE(ctrl, &Controller_class);
    return JS_TRUE;
}
