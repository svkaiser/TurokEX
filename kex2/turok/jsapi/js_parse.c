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
// DESCRIPTION: Script parsing for defining new Javascript components
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "js_class.h"
#include "script.h"
#include "zone.h"

//
// JParse_GetJSONBuffer
//

char *JParse_GetJSONBuffer(scparser_t *parser)
{
    char *start;
    char *end;
    char *out;

    SC_ExpectNextToken(TK_LBRACK);
    start = &sc_parser->buffer[sc_parser->buffpos-1];

    while(parser->tokentype != TK_RBRACK)
    {
        SC_Find();
        end = &sc_parser->buffer[sc_parser->buffpos];
    }

    SC_Find();
    if(strcmp(parser->token, "EndObject"))
        SC_Error("Expected 'EndObject', found %s", parser->token);

    out = Z_Calloc((end-start)+1, PU_STATIC, NULL);
    strncpy(out, start, (end-start));

    return out;
}

//
// JParse_BeginObject
//

kbool JParse_BeginObject(scparser_t *parser, gObject_t *object)
{
    JSBool found;
    jsval val;
    JSContext *cx;
    gObject_t *cObject;
    gObject_t *fObject;
    gObject_t *newObject;
    char *json;
    jsval argv;
    jsval rval;

    SC_Find();
    if(strcmp(parser->token, "BeginObject"))
        SC_Error("Expected 'BeginObject', found %s", parser->token);

    SC_ExpectNextToken(TK_EQUAL);
    SC_GetString();

    cx = js_context;

    // get class name of the prototype object
    if(!JS_HasProperty(cx, js_gobject, parser->stringToken, &found))
        return false;

    if(!found)
        SC_Error("Unknown object class: %s", parser->stringToken);

    // get prototype
    if(!JS_GetProperty(cx, js_gobject, parser->stringToken, &val))
        return false;
    if(!JS_ValueToObject(cx, val, &cObject))
        return false;

    // construct class object
    if(!(newObject = classObj.create(cObject)))
        return false;

    // add new object as property
    if(!JS_DefineProperty(cx, object, parser->stringToken, OBJECT_TO_JSVAL(newObject),
        NULL, NULL, JSPROP_ENUMERATE))
        return false;

    // mark as active
    val = BOOLEAN_TO_JSVAL(true);
    if(!JS_SetProperty(cx, newObject, "active", &val))
        return false;

    val = OBJECT_TO_JSVAL(object);
    if(!JS_SetProperty(cx, newObject, "parent", &val))
        return false;

    // deserialize data
    json = JParse_GetJSONBuffer(parser);
    JS_GET_PROPERTY_OBJECT(newObject, "deSerialize", fObject);
    if(JS_ObjectIsFunction(cx, fObject))
    {
        argv = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, json));
        JS_CallFunctionValue(cx, newObject, OBJECT_TO_JSVAL(fObject), 1, &argv, &rval);
    }

    Z_Free(json);
    return true;
}

//
// JParse_Start
//

kbool JParse_Start(scparser_t *parser, gObject_t **object, int count)
{
    int i;

    // create a new object that's being parsed
    *object = J_NewObjectEx(js_context, &Component_class, NULL, NULL);
    JS_AddRoot(js_context, &(*object));

    for(i = 0; i < count; i++)
    {
        // entered 'BeginObject' block
        if(!JParse_BeginObject(parser, *object))
            return false;
    }

    return true;
}
