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
// DESCRIPTION: Javascript Game Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "game.h"

JS_CLASSOBJECT(NGame);

enum ngame_enum
{
    NG_SUBCLASS
};

JS_PROP_FUNC_GET(NGame)
{
    switch(JSVAL_TO_INT(id))
    {
    case NG_SUBCLASS:
        return JS_TRUE;
    }

    return JS_FALSE;
}

JS_PROP_FUNC_SET(NGame)
{
    switch(JSVAL_TO_INT(id))
    {
    case NG_SUBCLASS:
        {
            JSObject *obj2;
            JS_GETOBJECT(obj2, vp, 0);
            JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj2));
            return JS_TRUE;
        }

    default:
        JS_ReportError(cx, "Unknown property");
        break;
    }

    return JS_FALSE;
}

JS_BEGINCLASS(NGame)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    NGame_getProperty,                          // getProperty
    NGame_setProperty,                          // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(NGame)
{
    { "subclass",   NG_SUBCLASS,    JSPROP_ENUMERATE,   NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(NGame)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(NGame)
{
    JS_FS_END
};
