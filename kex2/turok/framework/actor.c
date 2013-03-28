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
// DESCRIPTION: GameActor system
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "jsobj.h"
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

void Actor_OnTouchEvent(gActor_t *actor, gActor_t *instigator)
{
    if(actor->bStatic)
        return;

    if(!actor->bTouch)
        return;

    Actor_CallEvent(actor, "onTouch", instigator);
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

void Actor_CallEvent(gActor_t *actor, const char *function, gActor_t *instigator)
{
    jsid id;
    jsval val;
    JSScopeProperty *sprop;

    if(actor->components == NULL)
        return;

    JS_GetReservedSlot(js_context, actor->iterator, 0, &val);
    sprop = (JSScopeProperty*)JS_GetPrivate(js_context, actor->iterator);

    while(JS_NextProperty(js_context, actor->iterator, &id))
    {
        jsval vp;
        kbool found;
        gObject_t *func;
        gObject_t *obj;
        gObject_t *component;
        jsval rval;
        jsval argv = JSVAL_VOID;
        uintN nargs = 0;

        if(id == JSVAL_VOID)
            break;

        if(!JS_GetMethodById(js_context, actor->components, id, &obj, &vp))
            continue;
        if(!JS_ValueToObject(js_context, vp, &component))
            continue;
        if(component == NULL)
            continue;
        if(!JS_HasProperty(js_context, component, function, &found))
            continue;
        if(!found)
            continue;
        if(!JS_GetProperty(js_context, component, function, &vp))
            continue;
        if(!JS_ValueToObject(js_context, vp, &func))
            continue;
        if(!JS_ObjectIsFunction(js_context, func))
            continue;

        if(instigator)
        {
            gObject_t *aObject;

            if(!(aObject = JPool_GetFree(&objPoolGameActor, &GameActor_class)) ||
                !(JS_SetPrivate(js_context, aObject, instigator)))
                continue;

            argv = OBJECT_TO_JSVAL(aObject);
            nargs = 1;
        }

        JS_CallFunctionName(js_context, component, function, nargs, &argv, &rval);
    }

    JS_SetReservedSlot(js_context, actor->iterator, 0, val);
    JS_SetPrivate(js_context, actor->iterator, sprop);
}

//
// Actor_ComponentFunc
//

void Actor_ComponentFunc(const char *function)
{
    unsigned int i;

    if(!gLevel.loaded)
        return;

    // TODO - MOVE PICKUPS TO LINKED LIST
    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *gb = &gLevel.gridBounds[i];
        unsigned int j;

        for(j = 0; j < gb->numStatics; j++)
        {
            gActor_t *actor = &gb->statics[j];

            if(actor->bStatic)
                continue;

            Actor_CallEvent(actor, function, NULL);
        }
    }
}

//
// Actor_SetTarget
//

void Actor_SetTarget(gActor_t **self, gActor_t *target)
{
    // If there was a target already, decrease its refcount
    if(*self)
    {
        Z_Touch(*self); // validate pointer
        (*self)->refcount--;
    }

    // Set new target and if non-NULL, increase its counter
    if((*self = target))
        target->refcount++;
}

//
// Actor_Setup
//

void Actor_Setup(gActor_t *actor)
{
    gObject_t *ownerObject;
    anim_t *anim;
    jsval ownerVal;
    kbool found;
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

    if((anim = Mdl_GetAnim(actor->model, "anim00")))
    {
        Mdl_SetAnimState(&actor->animState, anim, 1, ANF_LOOP);
        Vec_SetQuaternion(actor->rotation, -actor->angles[0], 0, 1, 0);

        if(actor->plane != -1)
        {
            vec4_t rot;
            plane_t *plane = &gLevel.planes[actor->plane];

            Plane_GetRotation(rot, plane);
            Vec_AdjustQuaternion(actor->rotation, rot, -actor->angles[0] + M_PI);
        }

        Actor_UpdateTransform(actor);
    }

    if(actor->components == NULL)
        return;

    actor->iterator = JS_NewPropertyIterator(js_context, actor->components);
    JS_AddRoot(js_context, &actor->iterator);

    if(actor->bStatic)
        Com_Error("Static actor (%s) should not contain any components", actor->name);

    // create and set owner property
    ownerObject = JS_NewObject(cx, &GameActor_class, NULL, NULL);

    if(!JS_SetPrivate(cx, ownerObject, actor))
        return;

    ownerVal = OBJECT_TO_JSVAL(ownerObject);
    JS_SetProperty(cx, actor->components, "owner", &ownerVal);
    JS_SetPropertyAttributes(cx, actor->components, "owner", 0, &found);
}

