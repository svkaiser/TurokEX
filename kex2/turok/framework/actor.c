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
#include "js_class.h"
#include "common.h"
#include "mathlib.h"
#include "actor.h"
#include "script.h"
#include "level.h"
#include "ai.h"
#include "zone.h"
#include "client.h"
#include "server.h"

static gActorTemplate_t *actorTemplateList[MAX_HASH];
static int numActors = 0;
static int numStaleActors = 0;

enum
{
    scactor_mesh = 0,
    scactor_bounds,
    scactor_components,
    scactor_bCollision,
    scactor_bStatic,
    scactor_bTouch,
    scactor_bHidden,
    scactor_bOrientOnSlope,
    scactor_scale,
    scactor_radius,
    scactor_height,
    scactor_centerheight,
    scactor_viewheight,
    scactor_classFlags,
    scactor_end
};

static const sctokens_t mapactortokens[scactor_end+1] =
{
    { scactor_mesh,             "mesh"              },
    { scactor_bounds,           "bounds"            },
    { scactor_components,       "components"        },
    { scactor_bCollision,       "bCollision"        },
    { scactor_bStatic,          "bStatic"           },
    { scactor_bTouch,           "bTouch"            },
    { scactor_bHidden,          "bHidden"           },
    { scactor_bOrientOnSlope,   "bOrientOnSlope"    },
    { scactor_scale,            "scale"             },
    { scactor_radius,           "radius"            },
    { scactor_height,           "height"            },
    { scactor_centerheight,     "centerheight"      },
    { scactor_viewheight,       "viewheight"        },
    { scactor_classFlags,       "classFlags"        },
    { -1,                       NULL                }
};

//
// Actor_ParseTemplate
//

static void Actor_ParseTemplate(scparser_t *parser, gActorTemplate_t *ac)
{
    int numComponents = 0;
    int j;

    Vec_Set3(ac->actor.scale, 1, 1, 1);

    // read into nested actor block
    SC_ExpectNextToken(TK_LBRACK);
    SC_Find();

    while(parser->tokentype != TK_RBRACK)
    {
        switch(SC_GetIDForToken(mapactortokens, parser->token))
        {
        case scactor_mesh:
            SC_ExpectNextToken(TK_EQUAL);
            SC_GetString();
            ac->actor.model = Mdl_Load(parser->stringToken);
            break;

        case scactor_bounds:
            SC_ExpectNextToken(TK_EQUAL);
            SC_ExpectNextToken(TK_LBRACK);
            ac->actor.bbox.min[0] = (float)SC_GetFloat();
            ac->actor.bbox.min[1] = (float)SC_GetFloat();
            ac->actor.bbox.min[2] = (float)SC_GetFloat();
            ac->actor.bbox.max[0] = (float)SC_GetFloat();
            ac->actor.bbox.max[1] = (float)SC_GetFloat();
            ac->actor.bbox.max[2] = (float)SC_GetFloat();
            SC_ExpectNextToken(TK_RBRACK);
            break;

        case scactor_components:
            SC_ExpectNextToken(TK_LSQBRACK);
            numComponents = SC_GetNumber();
            SC_ExpectNextToken(TK_RSQBRACK);
            if(numComponents > 0)
            {
                ac->components = (char**)Z_Calloc(sizeof(char*) *
                    numComponents, PU_ACTOR, NULL);
            }
            SC_ExpectNextToken(TK_EQUAL);
            SC_ExpectNextToken(TK_LBRACK);
            for(j = 0; j < numComponents; j++)
            {
                SC_GetString();
                ac->components[j] = Z_Strdup(parser->stringToken, PU_ACTOR, NULL);
            }
            ac->numComponents = numComponents;
            SC_ExpectNextToken(TK_RBRACK);
            break;

        case scactor_bCollision:
            SC_AssignInteger(mapactortokens, &ac->actor.bCollision,
                scactor_bCollision, parser, false);
            break;

        case scactor_bStatic:
            SC_AssignInteger(mapactortokens, &ac->actor.bStatic,
                scactor_bStatic, parser, false);
            break;

        case scactor_bTouch:
            SC_AssignInteger(mapactortokens, &ac->actor.bTouch,
                scactor_bTouch, parser, false);
            break;

        case scactor_bHidden:
            SC_AssignInteger(mapactortokens, &ac->actor.bHidden,
                scactor_bHidden, parser, false);
            break;

        case scactor_bOrientOnSlope:
            SC_AssignInteger(mapactortokens, &ac->actor.bOrientOnSlope,
                scactor_bOrientOnSlope, parser, false);
            break;

        case scactor_scale:
            SC_AssignVector(mapactortokens, ac->actor.scale,
                scactor_scale, parser, false);
            break;

        case scactor_radius:
            SC_AssignFloat(mapactortokens, &ac->actor.radius,
                scactor_radius, parser, false);
            break;

        case scactor_height:
            SC_AssignFloat(mapactortokens, &ac->actor.height,
                scactor_height, parser, false);
            break;

        case scactor_centerheight:
            SC_AssignFloat(mapactortokens, &ac->actor.centerHeight,
                scactor_centerheight, parser, false);
            break;

        case scactor_viewheight:
            SC_AssignFloat(mapactortokens, &ac->actor.viewHeight,
                scactor_viewheight, parser, false);
            break;

        case scactor_classFlags:
            SC_AssignInteger(mapactortokens, &ac->actor.classFlags,
                scactor_classFlags, parser, false);

            if(ac->actor.classFlags & AC_AI)
                AI_Spawn(&ac->actor);
            break;

        default:
            if(parser->tokentype == TK_IDENIFIER)
            {
                SC_Error("Actor_ParseTemplate: Unknown token: %s\n",
                    parser->token);
            }
            break;
        }

        SC_Find();
    }
}

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
// Actor_TransformBBox
//

static void Actor_TransformBBox(gActor_t *actor)
{
    bbox_t box;
    vec3_t c;
    vec3_t h;
    vec3_t ct;
    vec3_t ht;
    mtx_t m;

    Vec_Copy3(box.min, actor->bbox.omin);
    Vec_Copy3(box.max, actor->bbox.omax);

    Mtx_Copy(m, actor->rotMtx);

    c[0] = (box.min[0] + box.max[0]) * 0.5f;
    c[1] = (box.min[1] + box.max[1]) * 0.5f;
    c[2] = (box.min[2] + box.max[2]) * 0.5f;

    Vec_Sub(h, box.max, c);

    Vec_TransformToWorld(m, c, ct);

    ct[0] = -ct[0];
    ct[2] = -ct[2];

    m[ 0] = (float)fabs(m[ 0]);
    m[ 1] = (float)fabs(m[ 1]);
    m[ 2] = (float)fabs(m[ 2]);
    m[ 4] = (float)fabs(m[ 4]);
    m[ 5] = (float)fabs(m[ 5]);
    m[ 6] = (float)fabs(m[ 6]);
    m[ 8] = (float)fabs(m[ 8]);
    m[ 9] = (float)fabs(m[ 9]);
    m[10] = (float)fabs(m[10]);
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;

    Vec_TransformToWorld(m, h, ht);

    Vec_Sub(box.min, ct, ht);
    Vec_Add(box.max, ct, ht);

    Vec_Copy3(actor->bbox.min, box.min);
    Vec_Copy3(actor->bbox.max, box.max);
}

//
// Actor_UpdateTransform
//

void Actor_UpdateTransform(gActor_t *actor)
{
    if(!actor->bStatic)
    {
        vec4_t yaw;
        vec4_t pitch;
        vec4_t roll;
        vec4_t rot;

        Ang_Clamp(&actor->angles[0]);
        Ang_Clamp(&actor->angles[1]);

        Vec_SetQuaternion(yaw, actor->angles[0], 0, 1, 0);
        Vec_SetQuaternion(pitch, actor->angles[1], 1, 0, 0);
        Vec_SetQuaternion(roll, actor->angles[2], 0, 0, 1);
        Vec_MultQuaternion(rot, yaw, roll);
        Vec_MultQuaternion(actor->rotation, pitch, rot);

        if(actor->plane != -1 && actor->bOrientOnSlope &&
            client.playerActor != actor /*TODO*/)
        {
            plane_t *plane = &gLevel.planes[actor->plane];

            if(!Plane_IsAWall(plane))
            {
                Plane_GetRotation(rot, plane);
                Vec_AdjustQuaternion(actor->rotation, rot, actor->angles[0] + M_PI);
            }
        }
    }

    Mtx_ApplyRotation(actor->rotation, actor->matrix);
    Mtx_Copy(actor->rotMtx, actor->matrix);

    Mtx_Scale(actor->matrix,
        actor->scale[0],
        actor->scale[1],
        actor->scale[2]);

    Mtx_AddTranslation(actor->matrix,
        actor->origin[0],
        actor->origin[1],
        actor->origin[2]);

    if(!actor->bStatic)
        Actor_TransformBBox(actor);
}

//
// Actor_CallEvent
//

void Actor_CallEvent(gActor_t *actor, const char *function, gActor_t *instigator)
{
    jsval val;

    if(actor->components == NULL)
        return;

#ifdef JS_LOGNEWOBJECTS
    Com_Printf("\nLogging Event (%s)...\n", function);
#endif

    JS_ITERATOR_START(actor, val);
    JS_ITERATOR_LOOP(actor, val, function);
    {
        gObject_t *func;
        jsval rval;
        jsval argv = JSVAL_VOID;
        uintN nargs = 0;

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
    JS_ITERATOR_END(actor, val);
}

//
// Actor_UpdateMove
//

static void Actor_UpdateMove(gActor_t *actor)
{
    plane_t *plane = Map_IndexToPlane(actor->plane);

    if(actor->animState.flags & ANF_ROOTMOTION &&
        !(actor->animState.flags & ANF_STOPPED))
    {
        vec3_t dir;

        Vec_ApplyQuaternion(dir, actor->animState.rootMotion, actor->rotation);
        dir[1] = 0;
        Vec_Scale(dir, dir, 30.0f * actor->timestamp);
        Vec_Add(actor->velocity, actor->velocity, dir);
    }

    G_ApplyGravity(actor->origin, actor->velocity, plane, actor->mass, actor->timestamp);
    G_ClipMovement(actor->origin, actor->velocity, actor->timestamp, &plane, actor, NULL);
    G_ApplyFriction(actor->velocity, actor->friction, false);
    
    actor->plane = Map_PlaneToIndex(plane);
}

//
// Actor_LocalTick
//

void Actor_LocalTick(void)
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

            // TODO
            if(Vec_Length3(client.player->camera->origin, actor->origin) < 2048.0f)
                Actor_CallEvent(actor, "onLocalTick", NULL);
        }
    }

    for(gLevel.actorRover = gLevel.actorRoot.next;
        gLevel.actorRover != &gLevel.actorRoot;
        gLevel.actorRover = gLevel.actorRover->next)
    {
        gActor_t *actor = gLevel.actorRover;

        if(actor->bStatic)
            continue;

        Mdl_UpdateAnimState(&actor->animState);

        // TODO
        if(client.playerActor == actor)
            continue;

        if(actor->components)
            Actor_CallEvent(actor, "onLocalTick", NULL);
    }
}

//
// Actor_Tick
//

void Actor_Tick(void)
{
    if(!gLevel.loaded)
        return;

    for(gLevel.actorRover = gLevel.actorRoot.next;
        gLevel.actorRover != &gLevel.actorRoot;
        gLevel.actorRover = gLevel.actorRover->next)
    {
        gActor_t *actor = gLevel.actorRover;
        numActors++;

        if(actor->bStatic)
            continue;

        // TODO
        if(client.playerActor == actor)
            continue;

        actor->timestamp = ((float)server.runtime -
            gLevel.deltaTime) / 1000.0f;

        if(actor->timestamp < 0)
            continue;

        if(actor->components)
        {
            Actor_CallEvent(actor, "onTick", NULL);
            if(actor->classFlags & AC_AI)
                AI_Think(actor->ai);

            JS_MaybeGC(js_context);
        }

        if(actor->physics != PT_NONE)
        {
            Actor_UpdateMove(actor);
            Actor_UpdateTransform(actor);
        }

        if(actor->bStale)
        {
            numStaleActors++;
            Map_RemoveActor(&gLevel, actor);
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
        (*self)->refcount--;

    // Set new target and if non-NULL, increase its counter
    if((*self = target))
        target->refcount++;
}

//
// Actor_NewProperty
//

static propKey_t *Actor_NewProperty(gActor_t *actor, const char *name, int id)
{
    if(id <= -1 || id < actor->numProperties)
        return NULL;

    actor->numProperties = (id + 1);

    actor->properties = (propKey_t*)Z_Realloc(actor->properties,
        sizeof(propKey_t) * actor->numProperties, PU_STATIC, 0);

    return &actor->properties[id];
}

//
// Actor_CopyProperties
//

void Actor_CopyProperties(gActor_t *actor, gActor_t *gTemplate)
{
    int size = sizeof(propKey_t) * gTemplate->numProperties;

    if(size > 0)
    {
        actor->properties = (propKey_t*)Z_Calloc(size, PU_ACTOR, 0);
        memcpy(actor->properties, gTemplate->properties, size);
    }

    actor->bClientOnly      = gTemplate->bClientOnly;
    actor->bCollision       = gTemplate->bCollision;
    actor->bHidden          = gTemplate->bHidden;
    actor->bOrientOnSlope   = gTemplate->bOrientOnSlope;
    actor->bStatic          = gTemplate->bStatic;
    actor->bTouch           = gTemplate->bTouch;
    actor->centerHeight     = gTemplate->centerHeight;
    actor->viewHeight       = gTemplate->viewHeight;
    actor->radius           = gTemplate->radius;
    actor->height           = gTemplate->height;
    actor->model            = gTemplate->model;
    actor->classFlags       = gTemplate->classFlags;

    if(actor->classFlags & AC_AI)
    {
        AI_Spawn(actor);
        actor->ai->activeDistance = gTemplate->ai->activeDistance;
        actor->ai->turnSpeed = gTemplate->ai->turnSpeed;
        actor->ai->thinkTime = gTemplate->ai->thinkTime;
        actor->ai->nextThinkTime = gLevel.time + actor->ai->thinkTime;
    }

    Vec_Copy3(actor->bbox.min, gTemplate->bbox.min);
    Vec_Copy3(actor->bbox.max, gTemplate->bbox.max);
    Vec_Copy3(actor->bbox.omin, gTemplate->bbox.min);
    Vec_Copy3(actor->bbox.omax, gTemplate->bbox.max);
    Vec_Copy3(actor->scale, gTemplate->scale);
}

//
// Actor_CreateComponent
//

void Actor_CreateComponent(gActor_t *actor)
{
    if(!(actor->components = J_NewObjectEx(js_context, &Component_class, NULL, NULL)))
        return;

    JS_AddRoot(js_context, &actor->components);
}

//
// Actor_AddComponent
//

void Actor_AddComponent(gActor_t *actor, const char *component)
{
    jsval val;
    JSContext *cx;
    gObject_t *cObject;
    gObject_t *newObject;

    if(actor->components == NULL)
        return;

    cx = js_context;

    // get prototype
    if(!JS_GetProperty(cx, js_gobject, component, &val))
        return;
    if(!JS_ValueToObject(cx, val, &cObject))
        return;

    // construct class object
    if(!(newObject = classObj.create(cObject)))
        return;

    // add new object as property
    if(!JS_DefineProperty(cx, actor->components, component, OBJECT_TO_JSVAL(newObject),
        NULL, NULL, JSPROP_ENUMERATE))
        return;

    // mark as active
    val = BOOLEAN_TO_JSVAL(true);
    JS_SetProperty(cx, newObject, "active", &val);

    // set parent
    val = OBJECT_TO_JSVAL(actor->components);
    JS_SetProperty(cx, newObject, "parent", &val);
}

//
// Actor_FindTemplate
//

gActorTemplate_t *Actor_FindTemplate(const char *name)
{
    gActorTemplate_t *at;
    unsigned int hash;

    hash = Com_HashFileName(name);

    for(at = actorTemplateList[hash]; at; at = at->next)
    {
        if(!strcmp(name, at->name))
            return at;
    }

    return NULL;
}

//
// Actor_NewTemplate
//

gActorTemplate_t *Actor_NewTemplate(const char *name)
{
    gActorTemplate_t *at;
    unsigned int hash;
    scparser_t *parser;

    at = (gActorTemplate_t*)Z_Calloc(sizeof(gActorTemplate_t), PU_STATIC, 0);

    if(parser = SC_Open(kva("actors/%s.kact", name)))
    {
        Actor_ParseTemplate(parser, at);
        SC_Close();
    }

    hash = Com_HashFileName(name);
    at->next = actorTemplateList[hash];
    actorTemplateList[hash] = at;

    return at;
}

//
// Actor_Spawn
//

gActor_t *Actor_Spawn(const char *classname, float x, float y, float z,
                      float yaw, float pitch, int plane)
{
    gActorTemplate_t *at;
    gActor_t *actor;

    if(!(at = Actor_FindTemplate(classname)))
    {
        if(!(at = Actor_NewTemplate(classname)))
            return NULL;
    }

    actor = (gActor_t*)Z_Calloc(sizeof(gActor_t), PU_ACTOR, NULL);

    // set default properties
    actor->physics      = PT_NONE;
    actor->mass         = 1200;
    actor->friction     = 1.0f;
    actor->airfriction  = 1.0f;
    actor->bounceDamp   = 0.0f;

    Actor_CopyProperties(actor, &at->actor);

    if(at->numComponents > 0)
    {
        unsigned int i;

        Actor_CreateComponent(actor);

        for(i = 0; i < at->numComponents; i++)
            Actor_AddComponent(actor, at->components[i]);
    }

    Vec_Set3(actor->origin, x, y, z);
    actor->angles[0] = yaw;
    actor->angles[1] = pitch;
    actor->plane = plane;

    Actor_Setup(actor);
    return actor;
}

//
// Actor_AddIntegerProperty
//

void Actor_AddIntegerProperty(gActor_t *actor, const char *name, int id, int value)
{
    propKey_t *key;

    if(!(key = Actor_NewProperty(actor, name, id)))
        return;

    key->val.i = value;
    key->type = 0;
}

//
// Actor_AddFloatProperty
//

void Actor_AddFloatProperty(gActor_t *actor, const char *name, int id, float value)
{
    propKey_t *key;

    if(!(key = Actor_NewProperty(actor, name, id)))
        return;

    key->val.f = value;
    key->type = 1;
}

//
// Actor_AddStringProperty
//

void Actor_AddStringProperty(gActor_t *actor, const char *name, int id, char *value)
{
    propKey_t *key;

    if(!(key = Actor_NewProperty(actor, name, id)))
        return;

    key->val.c = value;
    key->type = 2;
}

//
// Actor_AddDataProperty
//

void Actor_AddDataProperty(gActor_t *actor, const char *name, int id, void *value)
{
    propKey_t *key;

    if(!(key = Actor_NewProperty(actor, name, id)))
        return;

    key->val.p = value;
    key->type = 3;
}

//
// Actor_ClearData
//

void Actor_ClearData(gActor_t *actor)
{
    if(actor->iterator)
        JS_RemoveRoot(js_context, &actor->iterator);

    if(actor->components)
        JS_RemoveRoot(js_context, &actor->components);

    if(actor->properties && actor->numProperties > 0)
        Z_Free(actor->properties);

    if(actor->classFlags & AC_AI && actor->ai)
    {
        if(actor->ai->object)
            JS_RemoveRoot(js_context, &actor->ai->object);

        Z_Free(actor->ai);
    }
}

//
// Actor_Remove
//

void Actor_Remove(gActor_t *actor)
{
    Actor_ClearData(actor);
    Z_Free(actor);
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

    if(actor->model && (anim = Mdl_GetAnim(actor->model, "anim00")))
        Mdl_SetAnimState(&actor->animState, anim, 4, ANF_PAUSED);

    Actor_UpdateTransform(actor);
    actor->timestamp = (float)server.runtime;

    if(actor->components == NULL)
        return;

    actor->iterator = JS_NewPropertyIterator(js_context, actor->components);
    JS_AddRoot(js_context, &actor->iterator);

    if(actor->bStatic)
        Com_Error("Static actor (%s) should not contain any components", actor->name);

    // create and set owner property
    ownerObject = J_NewObjectEx(cx, &GameActor_class, NULL, NULL);

    if(!JS_SetPrivate(cx, ownerObject, actor))
        return;

    ownerVal = OBJECT_TO_JSVAL(ownerObject);
    JS_SetProperty(cx, actor->components, "owner", &ownerVal);
    JS_SetPropertyAttributes(cx, actor->components, "owner", 0, &found);

    Actor_CallEvent(actor, "onReady", NULL);
}

//
// Actor_DrawDebugStats
//

void Actor_DrawDebugStats(void)
{
    Draw_Text(32, 192, COLOR_GREEN, 1, "num actors: %i", numActors);
    Draw_Text(32, 208, COLOR_GREEN, 1, "num stale actors: %i", numStaleActors);

    numActors = 0;
    numStaleActors = 0;
}

