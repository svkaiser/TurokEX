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
    case 0:
        JS_NEWOBJECTPOOL(ctrl->owner, GameActor);
        return JS_TRUE;

    case 1:
        JS_NEWVECTOR2(ctrl->origin);
        return JS_TRUE;

    case 2:
        JS_NEWVECTOR2(ctrl->velocity);
        return JS_TRUE;

    case 3:
        JS_NEWVECTOR2(ctrl->forward);
        return JS_TRUE;

    case 4:
        JS_NEWVECTOR2(ctrl->right);
        return JS_TRUE;

    case 5:
        JS_NEWVECTOR2(ctrl->up);
        return JS_TRUE;

    case 6:
        JS_NEWVECTOR2(ctrl->accel);
        return JS_TRUE;

    case 7:
        return JS_NewDoubleValue(cx, ctrl->angles[0], vp);

    case 8:
        return JS_NewDoubleValue(cx, ctrl->angles[1], vp);

    case 9:
        return JS_NewDoubleValue(cx, ctrl->angles[2], vp);

    case 10:
        return JS_NewDoubleValue(cx, ctrl->moveTime, vp);

    case 11:
        return JS_NewDoubleValue(cx, ctrl->frameTime, vp);

    case 12:
        return JS_NewDoubleValue(cx, ctrl->timeStamp, vp);

    case 13:
        if(ctrl->plane == NULL)
            JS_SET_RVAL(cx, vp, JSVAL_NULL);
        else
            JS_NEWOBJECTPOOL(ctrl->plane, Plane);
        return JS_TRUE;

    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(AController)
{
    aController_t *ctrl;
    jsdouble dval;
    JSObject *object;

    if(!(ctrl = (aController_t*)JS_GetInstancePrivate(cx, obj, &AController_class, NULL)))
        return JS_TRUE;

    switch(JSVAL_TO_INT(id))
    {
    case 1:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ctrl->origin);
        return JS_TRUE;

    case 2:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ctrl->velocity);
        return JS_TRUE;

    case 3:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ctrl->forward);
        return JS_TRUE;

    case 4:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ctrl->right);
        return JS_TRUE;

    case 5:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ctrl->up);
        return JS_TRUE;

    case 6:
        JS_GETOBJECT(object, vp, 0);
        JS_GETVECTOR2(object, ctrl->accel);
        return JS_TRUE;

    case 7:
        JS_GETNUMBER(dval, vp, 0);
        ctrl->angles[0] = (float)dval;
        return JS_TRUE;

    case 8:
        JS_GETNUMBER(dval, vp, 0);
        ctrl->angles[1] = (float)dval;
        return JS_TRUE;

    case 9:
        JS_GETNUMBER(dval, vp, 0);
        ctrl->angles[2] = (float)dval;
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_FINALIZE_FUNC(AController)
{
    /*aController_t *ctrl;

    if(ctrl = (aController_t*)JS_GetPrivate(cx, obj))
        JS_free(cx, ctrl);*/
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
    { "origin",     1,  JSPROP_ENUMERATE, NULL, NULL },
    { "velocity",   2,  JSPROP_ENUMERATE, NULL, NULL },
    { "forward",    3,  JSPROP_ENUMERATE, NULL, NULL },
    { "right",      4,  JSPROP_ENUMERATE, NULL, NULL },
    { "up",         5,  JSPROP_ENUMERATE, NULL, NULL },
    { "accel",      6,  JSPROP_ENUMERATE, NULL, NULL },
    { "yaw",        7,  JSPROP_ENUMERATE, NULL, NULL },
    { "pitch",      8,  JSPROP_ENUMERATE, NULL, NULL },
    { "roll",       9,  JSPROP_ENUMERATE, NULL, NULL },
    { "moveTime",   10, JSPROP_ENUMERATE, NULL, NULL },
    { "frameTime",  11, JSPROP_ENUMERATE, NULL, NULL },
    { "timeStamp",  12, JSPROP_ENUMERATE, NULL, NULL },
    { "plane",      13, JSPROP_ENUMERATE, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(AController)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(AController)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(AController)
{
    JS_FS_END
};
