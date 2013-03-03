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
// DESCRIPTION: Actor system
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"
#include "actor.h"
#include "level.h"
#include "zone.h"

//
// Actor_HasComponent
//

kbool Actor_HasComponent(gActor_t *actor, const char *component)
{
    kbool found;

    if(!JS_HasProperty(js_context, actor->components, component, &found))
        return false;

    return found;
}

//
// Actor_OnTouchEvent
//

void Actor_OnTouchEvent(gActor_t *actor)
{
    if(!actor->bTouch)
        return;

    Actor_CallEvent(actor, "onTouch");
}

//
// Actor_UpdateBox
//

void Actor_UpdateBox(gActor_t *actor, gObject_t *meshComponent)
{
    JSContext *cx = js_context;
    jsval minx, maxx;
    jsval miny, maxy;
    jsval minz, maxz;
    jsdouble d;

    if(!JS_GetProperty(cx, meshComponent, "box_min_x", &minx))
        return;
    if(!JS_GetProperty(cx, meshComponent, "box_min_y", &miny))
        return;
    if(!JS_GetProperty(cx, meshComponent, "box_min_z", &minz))
        return;
    if(!JS_GetProperty(cx, meshComponent, "box_max_x", &maxx))
        return;
    if(!JS_GetProperty(cx, meshComponent, "box_max_y", &maxy))
        return;
    if(!JS_GetProperty(cx, meshComponent, "box_max_z", &maxz))
        return;

    JS_ValueToNumber(cx, minx, &d);
    actor->bbox.min[0] = (float)d + actor->origin[0];
    JS_ValueToNumber(cx, miny, &d);
    actor->bbox.min[1] = (float)d + actor->origin[1];
    JS_ValueToNumber(cx, minz, &d);
    actor->bbox.min[2] = (float)d + actor->origin[2];
    JS_ValueToNumber(cx, maxx, &d);
    actor->bbox.max[0] = (float)d + actor->origin[0];
    JS_ValueToNumber(cx, maxy, &d);
    actor->bbox.max[1] = (float)d + actor->origin[1];
    JS_ValueToNumber(cx, maxz, &d);
    actor->bbox.max[2] = (float)d + actor->origin[2];
}

//
// Actor_UpdateMesh
//

void Actor_UpdateMesh(gActor_t *actor, gObject_t *meshComponent)
{
    JSContext *cx = js_context;
    jsval val;
    kbool found = false;

    if(JS_HasProperty(cx, meshComponent, "mesh", &found) && found)
    {
        gObject_t *modelObject;
        
        if(!JS_GetProperty(cx, meshComponent, "mesh", &val))
            return;
        if(!JS_ValueToObject(cx, val, &modelObject))
            return;
            
        actor->model = (kmodel_t*)JS_GetInstancePrivate(cx, modelObject, &Model_class, NULL);
    }
}

//
// Actor_UpdateVariant
//

void Actor_UpdateVariant(gActor_t *actor, gObject_t *meshComponent)
{
    JSContext *cx = js_context;
    jsval val;
    kbool found = false;

    if(JS_HasProperty(cx, meshComponent, "variant", &found) && found)
    {
        if(!JS_GetProperty(cx, meshComponent, "variant", &val))
            return;
        
        actor->variant = JSVAL_TO_INT(val);
    }
}

//
// Actor_UpdateTransform
//

void Actor_UpdateTransform(gActor_t *actor)
{
    Mtx_ApplyRotation(actor->rotation, actor->matrix);
    Mtx_Scale(actor->matrix,
        actor->scale[0],
        actor->scale[1],
        actor->scale[2]);
    Mtx_AddTranslation(actor->matrix,
        actor->origin[0],
        actor->origin[1],
        actor->origin[2]);
}

//
// Actor_CallEvent
//

void Actor_CallEvent(gActor_t *actor, const char *function)
{
    gObject_t *iter;
    jsid id;

    iter = JS_NewPropertyIterator(js_context, actor->components);

    while(JS_NextProperty(js_context, iter, &id))
    {
        jsval vp;
        gObject_t *obj;
        gObject_t *component;

        if(id == JSVAL_VOID)
            break;

        if(!JS_GetMethodById(js_context, actor->components, id, &obj, &vp))
            continue;
        if(!JS_ValueToObject(js_context, vp, &component))
            continue;

        J_CallFunctionOnObject(js_context, component, function);
    }
}

//
// Actor_ComponentFunc
//

void Actor_ComponentFunc(const char *function)
{
    unsigned int i;

    if(!gLevel.loaded)
        return;

    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *gb = &gLevel.gridBounds[i];
        unsigned int j;

        for(j = 0; j < gb->numStatics; j++)
        {
            gActor_t *actor = &gb->statics[j];

            if(actor->bHidden)
                continue;

            if(actor->bStatic && !actor->bTouch)
                continue;

            Actor_CallEvent(actor, function);
        }
    }
}

//
// Actor_Setup
//

void Actor_Setup(gActor_t *actor)
{
    kbool found = false;
    gObject_t *ownerObject;
    jsval ownerVal;
    JSContext *cx = js_context;

    if( actor->rotation[0] == 0 &&
        actor->rotation[1] == 0 &&
        actor->rotation[2] == 0 &&
        actor->rotation[3] == 0)
    {
        Vec_SetQuaternion(actor->rotation, actor->angles[0], 0, 1, 0);
    }

    Vec_Normalize4(actor->rotation);

    Actor_UpdateTransform(actor);

    // create and set owner property
    ownerObject = JS_NewObject(cx, &GameActor_class, NULL, NULL);

    if(!JS_SetPrivate(cx, ownerObject, actor))
        return;

    ownerVal = OBJECT_TO_JSVAL(ownerObject);
    if(!JS_SetProperty(cx, actor->components, "owner", &ownerVal))
        return;

    if(Actor_HasComponent(actor, "ComponentTouchBox"))
        actor->bTouch = true;

    if(Actor_HasComponent(actor, "ComponentMesh"))
    {
        gObject_t *meshComponent;
        jsval val;

        if(!JS_GetProperty(cx, actor->components, "ComponentMesh", &val))
            return;
        if(!JS_ValueToObject(cx, val, &meshComponent))
            return;

        Actor_UpdateBox(actor, meshComponent);
        Actor_UpdateMesh(actor, meshComponent);
        Actor_UpdateVariant(actor, meshComponent);

        // TODO - allow updates for texture swap array?
        if(JS_HasProperty(cx, meshComponent, "textureSwaps", &found) && found)
        {
            gObject_t *tSwapObject;
            jsuint length;

            if(!JS_GetProperty(cx, meshComponent, "textureSwaps", &val))
                return;
            if(!JS_ValueToObject(cx, val, &tSwapObject))
                return;
            if(!JS_IsArrayObject(cx, tSwapObject))
                return;
            if(!JS_GetArrayLength(cx, tSwapObject, &length))
                return;

            if(length > 0)
            {
                jsuint i;

                actor->textureSwaps = (char**)Z_Calloc(sizeof(char*) * length, PU_ACTOR, NULL);

                for(i = 0; i < length; i++)
                {
                    JSString *str;
                    char *bytes;

                    val = J_GetObjectElement(cx, tSwapObject, i);

                    actor->textureSwaps[i] = NULL;

                    if(JSVAL_IS_NULL(val))
                        continue;

                    if((str = JS_ValueToString(cx, val)))
                    {
                        if(!(bytes = JS_EncodeString(cx, str)))
                            continue;
                            
                        actor->textureSwaps[i] = Z_Strdup(bytes, PU_ACTOR, NULL);
                        JS_free(cx, bytes);
                    }
                }
            }
        }

        if(actor->bStatic)
        {
            // TODO
            JS_DeleteProperty(cx, actor->components, "ComponentTouchBox");
            JS_DeleteProperty(cx, actor->components, "ComponentMesh");
        }
    }
}

