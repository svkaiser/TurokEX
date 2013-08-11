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
// DESCRIPTION: Javascript Sound Class
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "sound.h"

JS_CLASSOBJECT(Snd);

JS_FASTNATIVE_BEGIN(Snd, play)
{
    JSString *str;
    char *bytes;
    gActor_t *actor = NULL;
    jsval *v = JS_ARGV(cx, vp);

    if(argc <= 0)
        return JS_FALSE;

    JS_GETSTRING(str, bytes, v, 0);

    if(argc == 2)
    {
        JSObject *object;

        JS_GETOBJECT(object, v, 1);
        actor = (gActor_t*)JS_GetInstancePrivate(cx, object,
            &GameActor_class, NULL);
    }

    Snd_PlayShader(bytes, actor);
    JS_free(cx, bytes);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

JS_BEGINCLASS(Snd)
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

JS_BEGINPROPS(Snd)
{
    { NULL, 0, 0, NULL, NULL }
};

JS_BEGINCONST(Snd)
{
    { 0, 0, 0, { 0, 0, 0 } }
};

JS_BEGINFUNCS(Snd)
{
    JS_FASTNATIVE(Snd, play, 2),
    JS_FS_END
};
