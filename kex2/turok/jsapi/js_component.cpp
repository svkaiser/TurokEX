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
// DESCRIPTION: Javascript Component Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "actor.h"

JS_CLASSOBJECT(Component);

JS_PROP_FUNC_GET(Component)
{
    return JS_TRUE;
}

JS_PROP_FUNC_SET(Component)
{
    return JS_TRUE;
}

JS_FINALIZE_FUNC(Component)
{
}

JS_BEGINCLASS(Component)
    JSCLASS_HAS_PRIVATE,                        // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Component_getProperty,                      // getProperty
    Component_setProperty,                      // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    Component_finalize,                         // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Component)
{
    { "parent",  0, 0, NULL, NULL },
    { "active", 1, 0, NULL, NULL },
    { "owner", 2, 0, NULL, NULL },
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Component)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Component)
{
    JS_FS_END
};

JS_BEGINSTATICFUNCS(Component)
{
    JS_FS_END
};
