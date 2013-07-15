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

JS_CLASSOBJECT(Game);

JS_PROP_FUNC_GET(Game)
{
    switch(JSVAL_TO_INT(id))
    {
    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_PROP_FUNC_SET(Game)
{
    switch(JSVAL_TO_INT(id))
    {
    default:
        return JS_TRUE;
    }

    return JS_TRUE;
}

JS_FASTNATIVE_BEGIN(Game, main)
{
    JS_CHECKARGS(0);
    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(Game, onLevelLoad)
{
    JS_CHECKARGS(0);
    JS_SAFERETURN();
}

JS_FASTNATIVE_BEGIN(Game, onLevelUnLoad)
{
    JS_CHECKARGS(0);
    JS_SAFERETURN();
}

JS_BEGINCLASS(Game)
    0,                                          // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    Game_getProperty,                           // getProperty
    Game_setProperty,                           // setProperty
    JS_EnumerateStub,                           // enumerate
    JS_ResolveStub,                             // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
JS_ENDCLASS();

JS_BEGINPROPS(Game)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Game)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Game)
{
    JS_FASTNATIVE(Game, main, 0),
    JS_FASTNATIVE(Game, onLevelLoad, 0),
    JS_FASTNATIVE(Game, onLevelUnLoad, 0),
    JS_FS_END
};
