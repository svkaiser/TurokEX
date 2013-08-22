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
#include "sound.h"
#include "fx.h"
#include "parse.h"

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
    scactor_bRotor,
    scactor_scale,
    scactor_radius,
    scactor_height,
    scactor_centerheight,
    scactor_viewheight,
    scactor_classFlags,
    scactor_physics,
    scactor_rotorSpeed,
    scactor_rotorVector,
    scactor_rotorFriction,
    scactor_tickDistance,
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
    { scactor_bRotor,           "bRotor"            },
    { scactor_scale,            "scale"             },
    { scactor_radius,           "radius"            },
    { scactor_height,           "height"            },
    { scactor_centerheight,     "centerheight"      },
    { scactor_viewheight,       "viewheight"        },
    { scactor_classFlags,       "classFlags"        },
    { scactor_physics,          "physics"           },
    { scactor_rotorSpeed,       "rotorSpeed"        },
    { scactor_rotorVector,      "rotorVector"       },
    { scactor_rotorFriction,    "rotorFriction"     },
    { scactor_tickDistance,     "tickDistance"      },
    { -1,                       NULL                }
};

//
// Actor_ParseTemplate
//

static void Actor_ParseTemplate(kexLexer *lexer, gActorTemplate_t *ac)
{
    int numComponents = 0;
    int j;

    Vec_Set3(ac->actor.scale, 1, 1, 1);

    // read into nested actor block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK)
    {
        switch(lexer->GetIDForTokenList(mapactortokens, lexer->Token()))
        {
        case scactor_mesh:
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            ac->actor.model = Kmesh_Load(lexer->StringToken());
            break;

        case scactor_bounds:
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->ExpectNextToken(TK_LBRACK);
            ac->actor.bbox.min[0] = (float)lexer->GetFloat();
            ac->actor.bbox.min[1] = (float)lexer->GetFloat();
            ac->actor.bbox.min[2] = (float)lexer->GetFloat();
            ac->actor.bbox.max[0] = (float)lexer->GetFloat();
            ac->actor.bbox.max[1] = (float)lexer->GetFloat();
            ac->actor.bbox.max[2] = (float)lexer->GetFloat();
            lexer->ExpectNextToken(TK_RBRACK);
            break;

        case scactor_components:
            lexer->ExpectNextToken(TK_LSQBRACK);
            numComponents = lexer->GetNumber();
            lexer->ExpectNextToken(TK_RSQBRACK);
            if(numComponents > 0)
            {
                ac->components = (char**)Z_Calloc(sizeof(char*) *
                    numComponents, PU_ACTOR, NULL);
            }
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->ExpectNextToken(TK_LBRACK);
            for(j = 0; j < numComponents; j++)
            {
                lexer->GetString();
                ac->components[j] = Z_Strdup(lexer->StringToken(), PU_ACTOR, NULL);
            }
            ac->numComponents = numComponents;
            lexer->ExpectNextToken(TK_RBRACK);
            break;

        case scactor_bCollision:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&ac->actor.bCollision,
                scactor_bCollision, false);
            break;

        case scactor_bStatic:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&ac->actor.bStatic,
                scactor_bStatic, false);
            break;

        case scactor_bTouch:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&ac->actor.bTouch,
                scactor_bTouch, false);
            break;

        case scactor_bHidden:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&ac->actor.bHidden,
                scactor_bHidden, false);
            break;

        case scactor_bOrientOnSlope:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&ac->actor.bOrientOnSlope,
                scactor_bOrientOnSlope, false);
            break;

        case scactor_bRotor:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&ac->actor.bRotor,
                scactor_bRotor, false);
            break;

        case scactor_scale:
            lexer->AssignVectorFromTokenList(mapactortokens, ac->actor.scale,
                scactor_scale, false);
            break;

        case scactor_radius:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.radius,
                scactor_radius, false);
            break;

        case scactor_height:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.baseHeight,
                scactor_height, false);
            break;

        case scactor_centerheight:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.centerHeight,
                scactor_centerheight, false);
            break;

        case scactor_viewheight:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.viewHeight,
                scactor_viewheight, false);
            break;

        case scactor_classFlags:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.classFlags,
                scactor_classFlags, false);

            if(ac->actor.classFlags & AC_AI)
                AI_Spawn(&ac->actor);
            break;

        case scactor_physics:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.physics,
                scactor_physics, false);
            break;

        case scactor_rotorSpeed:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.rotorSpeed,
                scactor_rotorSpeed, false);
            break;

        case scactor_rotorVector:
            lexer->AssignVectorFromTokenList(mapactortokens, ac->actor.rotorVector,
                scactor_rotorVector, false);
            break;

        case scactor_rotorFriction:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.rotorFriction,
                scactor_rotorFriction, false);
            break;

        case scactor_tickDistance:
            lexer->AssignFromTokenList(mapactortokens, &ac->actor.tickDistance,
                scactor_tickDistance, false);
            break;

        default:
            if(lexer->TokenType() == TK_IDENIFIER)
            {
                parser.Error("Actor_ParseTemplate: Unknown token: %s\n",
                    lexer->Token());
            }
            break;
        }

        lexer->Find();
    }
}

//
// Actor_HasComponent
//

kbool Actor_HasComponent(gActor_t *actor, const char *component)
{
    JSBool found;

    if(!JS_HasProperty(js_context, actor->components, component, &found))
        return false;

    return found;
}

//
// Actor_OnTouchEvent
//

void Actor_OnTouchEvent(gActor_t *actor, gActor_t *instigator)
{
    jsval val;

    if(actor->bStatic)
        return;
    if(!actor->bTouch)
        return;
    if(!Actor_ToVal(instigator, &val))
        return;

    Actor_CallEvent(actor, "onTouch", &val, 1);
}

//
// Actor_AlignToPlane
//

static kbool Actor_AlignToPlane(gActor_t *actor)
{
    if(!actor->bStatic && actor->physics & PF_SLIDEMOVE &&
        actor->plane != -1 && actor->bOrientOnSlope &&
        client.playerActor != actor /*TODO*/)
    {
        plane_t *plane = &gLevel.planes[actor->plane];
        mtx_t m1;
        mtx_t m2;
        vec4_t rot;

        if(!Plane_IsAWall(plane))
            Plane_GetRotation(rot, plane);
        else
            Vec_SetQuaternion(rot, Plane_GetYaw(plane) + M_PI, 0, 1, 0);

        Vec_Slerp(actor->lerpRotation, 4.0f * actor->timestamp,
            actor->lerpRotation, rot);

        Mtx_ApplyRotation(actor->lerpRotation, m1);
        Mtx_ApplyRotation(actor->rotation, m2);
        Mtx_MultiplyRotation(actor->matrix, m2, m1);

        return true;
    }

    return false;
}

//
// Actor_UpdateTransform
//

void Actor_UpdateTransform(gActor_t *actor)
{
    if(actor->bRotor)
    {
        actor->angles[0] += (actor->rotorVector[1] * actor->rotorSpeed * actor->timestamp);
        actor->angles[1] += (actor->rotorVector[0] * actor->rotorSpeed * actor->timestamp);
        actor->angles[2] += (actor->rotorVector[2] * actor->rotorSpeed * actor->timestamp);
    }

    if(!actor->bStatic || actor->classFlags & AC_FX || actor->bRotor)
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
    }

    if(!Actor_AlignToPlane(actor))
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
        BBox_Transform(actor->baseBBox, actor->rotMtx, &actor->bbox);
}

//
// Actor_OnGround
//

kbool Actor_OnGround(gActor_t *actor)
{
    return (actor->origin[1] - Plane_GetDistance(Map_IndexToPlane(actor->plane),
        actor->origin)) <= ONPLANE_EPSILON;
}

//
// Actor_ToVal
//

kbool Actor_ToVal(gActor_t *actor, long *val)
{
    gObject_t *aObject;

    if(actor == NULL)
    {
        *val = JSVAL_NULL;
        return true;
    }

    if(!(aObject = JPool_GetFree(&objPoolGameActor, &GameActor_class)) ||
        !(JS_SetPrivate(js_context, aObject, actor)))
    {
        *val = JSVAL_NULL;
        return false;
    }

    *val = (jsval)OBJECT_TO_JSVAL(aObject);
    return true;
}

//
// Actor_CallEvent
//

kbool Actor_CallEvent(gActor_t *actor, const char *function, long *args, unsigned int nargs)
{
    jsval val;
    kbool ok;

    if(actor->components == NULL)
        return true;

#ifdef JS_LOGNEWOBJECTS
    Com_Printf("\nLogging Event (%s)...\n", function);
#endif

    ok = false;

    JS_ITERATOR_START(actor, val);
    JS_ITERATOR_LOOP(actor, val, function);
    {
        gObject_t *func;
        jsval rval;
        jsval argv = JSVAL_VOID;

        if(!JS_ValueToObject(js_context, vp, &func))
            continue;
        if(!JS_ObjectIsFunction(js_context, func))
            continue;

        if(args == NULL)
            args = &argv;

        JS_CallFunctionName(js_context, component, function, nargs, args, &rval);
        ok = true;
    }
    JS_ITERATOR_END(actor, val);

    return ok;
}

//
// Actor_UpdateMove
//

static void Actor_UpdateMove(gActor_t *actor)
{
    plane_t *plane;
    float time;
    animstate_t *astate;

    plane = Map_IndexToPlane(actor->plane);
    time = actor->timestamp;
    astate = &actor->animState;

    if(astate->frametime != 0)
        actor->height = ((actor->baseHeight * 0.72f) - astate->baseOffset) * 0.5f;

    if(actor->height < 0 || astate->frametime == 0)
        actor->height = 0;

    if(Actor_OnGround(actor) && !Plane_IsAWall(plane) &&
        astate->flags & ANF_ROOTMOTION && !(astate->flags & ANF_STOPPED))
    {
        vec3_t dir;
        float blendFrac;

        blendFrac = 1.0f;

        if(astate->flags & ANF_BLEND && astate->blendtime != 0)
            blendFrac = (astate->frametime/astate->blendtime);

        Vec_ApplyQuaternion(dir, astate->rootMotion, actor->rotation);
        dir[1] = 0;
        Vec_Scale(dir, dir, blendFrac);
        Vec_Add(actor->velocity, actor->velocity, dir);
    }

    G_ApplyGravity(actor->origin, actor->velocity, plane, actor->mass, time);
    G_ClipMovement(actor->origin, actor->velocity, time, &plane, actor);

    if(Actor_OnGround(actor))
    {
        if(!Plane_IsAWall(plane))
            G_ApplyFriction(actor->velocity, actor->friction, false);

        if(actor->bRotor && actor->rotorFriction != 0)
        {
            float speed;

            speed = Vec_Unit3(actor->velocity);

            if(speed < VELOCITY_EPSILON)
                actor->rotorSpeed = 0;
            else
            {
                float clipspeed = speed - (speed * actor->rotorFriction);

                if(clipspeed < 0) clipspeed = 0;
                clipspeed /= speed;

                // de-accelerate
                actor->rotorSpeed *= clipspeed;
            }
        }
    }
    
    Map_UnlinkActorFromWorld(actor);
    actor->plane = Map_PlaneToIndex(plane);
    Map_LinkActorToWorld(actor);
}

//
// Actor_UpdateModelYaw
//

static void Actor_UpdateModelYaw(gActor_t *actor)
{
    float angle;
    float time;
    int frame;
    animstate_t *astate;
    anim_t *anim;

    astate = &actor->animState;

    if(!(astate->flags & ANF_ROOTMOTION) ||
        astate->flags & (ANF_STOPPED|ANF_BLEND))
        return;

    if(astate->frametime <= 0)
        return;

    if(!(anim = astate->track.anim))
        return;

    frame = astate->track.frame;
    angle = M_PI * anim->yawOffsets[frame];

    if(frame > 0)
        angle -= (M_PI * anim->yawOffsets[frame-1]);

    Ang_Clamp(&angle);

    time = (4.0f * astate->frametime);
    actor->angles[0] -= angle * (actor->timestamp * time);
}

//
// Actor_ExecuteFrameActions
//

static void Actor_ExecuteFrameActions(gActor_t *actor)
{
    animstate_t *astate;
    anim_t *anim;
    int i;
    int j;
    unsigned int inc;
    int frame;

    astate = &actor->animState;
    anim = astate->track.anim;

    if(actor->components == NULL    ||
        astate == NULL              ||
        anim == NULL                ||
        astate->flags & ANF_PAUSED  ||
        anim->actions == NULL       ||
        anim->numactions <= 0)
        return;

    frame = astate->track.frame;

    if(astate->currentFrame == frame)
        return;

    inc = 0;
    for(i = astate->currentFrame; i != frame; i++, inc++)
    {
        if(inc >= anim->numframes)
            break;

        if(i == anim->numframes)
            i = 0;

        for(j = 0; j < (int)anim->numactions; j++)
        {
            action_t *action = &astate->track.anim->actions[j];
            jsval val = 0;
            kbool ok = false;

            if(action->frame != i)
                continue;

            if(!strcmp(action->function, "playSound"))
            {
                Snd_PlayShader(action->argStrings[0], actor);
                continue;
            }
            else if(!strcmp(action->function, "fx"))
            {
                Actor_SpawnBodyFX(actor, action->argStrings[0],
                    action->args[1], action->args[2], action->args[3]);
                continue;
            }

            JS_ITERATOR_START(actor, val);
            JS_ITERATOR_LOOP(actor, val, action->function);
            {
                gObject_t *func;
                jsval rval;
                jsval argv[4];

                if(!JS_ValueToObject(js_context, vp, &func))
                    continue;
                if(!JS_ObjectIsFunction(js_context, func))
                    continue;

                J_NewDoubleEx(js_context, action->args[0], &argv[0]);
                J_NewDoubleEx(js_context, action->args[1], &argv[1]);
                J_NewDoubleEx(js_context, action->args[2], &argv[2]);
                J_NewDoubleEx(js_context, action->args[3], &argv[3]);

#ifdef JS_LOGNEWOBJECTS
                Com_Printf("\nLogging Event (%s)...\n", action->function);
#endif

                JS_CallFunctionName(js_context, component, action->function, 4, argv, &rval);
                ok = true;
            }
            JS_ITERATOR_END(actor, val);

            if(cvarDeveloper.GetBool() && ok == false)
            {
                common.DPrintf("Actor_ExecuteFrameActions: Couldn't execute \"%s\":%i\n",
                    action->function, action->frame);
            }
        }
    }

    astate->currentFrame = frame;
}

//
// Actor_FXEvent
//

kbool Actor_FXEvent(gActor_t *actor, gActor_t *target,
                    vec3_t fxOrigin, vec3_t fxVelocity,
                    int plane, action_t *action)
{
    jsval args[5];
    kbool ok;
    JSContext *cx;

    if(!actor)
        return false;

    if(!Actor_ToVal(target, &args[0]))
        return false;

    cx = js_context;

    J_NewDoubleEx(js_context, action->args[0], &args[1]);
    JS_VECTORTOVAL(fxOrigin, args[2]);
    JS_VECTORTOVAL(fxVelocity, args[3]);
    args[4] = INT_TO_JSVAL(plane);

    ok = Actor_CallEvent(actor, action->function, args, 5);

    if(cvarDeveloper.GetBool() && !ok)
    {
        common.DPrintf("Actor_FXEvent: Couldn't execute \"%s\"\n",
            action->function);
    }

    return true;
}

//
// Actor_GetLocalVectors
//

void Actor_GetLocalVectors(vec3_t out, gActor_t *actor, float x, float y, float z)
{
    mtx_t mtx;
    vec3_t pos;
    vec3_t tmp;

    Mtx_IdentityY(mtx, DEG2RAD(-90));
    Mtx_Scale(mtx, -1, 1, 1);
    Vec_Set3(tmp, x, y, z);
    Vec_TransformToWorld(mtx, tmp, pos);
    Vec_TransformToWorld(actor->matrix, pos, out);
}

//
// Actor_SpawnBodyFX
//

void Actor_SpawnBodyFX(gActor_t *actor, const char *fx, float x, float y, float z)
{
    plane_t *plane;
    vec3_t org;
    vec3_t tmp;
    vec4_t rot;
    fx_t *vfx;

    // TODO
    if(actor->bStatic)
        return;

    if(actor->bCulled)
        return;

    Actor_GetLocalVectors(org, actor, x, y, z);
    Vec_Set3(tmp, 0, 0, 0);

    plane = Map_IndexToPlane(actor->plane);

    if(plane)
        Plane_GetRotation(rot, plane);
    else
        Vec_Set4(rot, 0, 0, 0, 1);

    if(!(vfx = FX_Spawn(fx, actor, tmp, org, actor->rotation, plane)))
        return;

    Vec_AdjustQuaternion(vfx->rotation, rot, actor->angles[0] + M_PI);
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

            if(actor->bRotor)
            {
                actor->timestamp = client.GetRunTime();
                Actor_UpdateTransform(actor);
            }

            if(actor->components && !actor->bCulled)
                Actor_CallEvent(actor, "onLocalTick", NULL, 0);
        }
    }

    for(gLevel.actorRover = gLevel.actorRoot.next;
        gLevel.actorRover != &gLevel.actorRoot;
        gLevel.actorRover = gLevel.actorRover->next)
    {
        gActor_t *actor = gLevel.actorRover;

        if(actor->bStatic)
            continue;

        if(!actor->ai && actor->animState.track.anim != NULL && actor->tickDistance != 0)
        {
            float x = actor->origin[0] - client.playerActor->origin[0];
            float z = actor->origin[2] - client.playerActor->origin[2];

            if(x*x+z*z >= (actor->tickDistance * 2.048f))
                actor->animState.flags |= ANF_PAUSED;
            else
                actor->animState.flags &= ~ANF_PAUSED;
        }

        Mdl_UpdateAnimState(&actor->animState);

        // TODO
        if(client.playerActor == actor)
            continue;

        if(actor->components && !actor->bCulled)
            Actor_CallEvent(actor, "onLocalTick", NULL, 0);
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

        actor->timestamp = ((float)server.GetElaspedTime() -
            gLevel.deltaTime) / 1000.0f;

        if(actor->timestamp < 0)
            continue;

        if(actor->components)
        {
            Actor_ExecuteFrameActions(actor);

            if(Actor_CallEvent(actor, "onTick", NULL, 0))
            {
                if(actor->classFlags & AC_AI)
                    AI_Think(actor->ai);

                JPool_ReleaseObjects(&objPoolVector);
                JPool_ReleaseObjects(&objPoolGameActor);
                JPool_ReleaseObjects(&objPoolAnimState);
            }
        }

        // TODO
        if(actor->physics != 0)
        {
            actor->waterlevel = Map_GetWaterLevel(actor->origin,
                actor->height,
                Map_IndexToPlane(actor->plane));

            Actor_UpdateMove(actor);
            Actor_UpdateModelYaw(actor);
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
// Actor_CopyProperties
//

void Actor_CopyProperties(gActor_t *actor, gActor_t *gTemplate)
{
    actor->bClientOnly      = gTemplate->bClientOnly;
    actor->bCollision       = gTemplate->bCollision;
    actor->bHidden          = gTemplate->bHidden;
    actor->bOrientOnSlope   = gTemplate->bOrientOnSlope;
    actor->bStatic          = gTemplate->bStatic;
    actor->bTouch           = gTemplate->bTouch;
    actor->bRotor           = gTemplate->bRotor;
    actor->centerHeight     = gTemplate->centerHeight;
    actor->viewHeight       = gTemplate->viewHeight;
    actor->radius           = gTemplate->radius;
    actor->baseHeight       = gTemplate->baseHeight;
    actor->model            = gTemplate->model;
    actor->classFlags       = gTemplate->classFlags;
    actor->physics          = gTemplate->physics;
    actor->mass             = gTemplate->mass;
    actor->friction         = gTemplate->friction;
    actor->bounceDamp       = gTemplate->bounceDamp;
    actor->rotorSpeed       = gTemplate->rotorSpeed;
    actor->rotorFriction    = gTemplate->rotorFriction;

    Actor_UpdateModel(actor, NULL);

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
    Vec_Copy3(actor->baseBBox.min, gTemplate->bbox.min);
    Vec_Copy3(actor->baseBBox.max, gTemplate->bbox.max);
    Vec_Copy3(actor->scale, gTemplate->scale);
    Vec_Copy3(actor->rotorVector, gTemplate->rotorVector);
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

    hash = common.HashFileName(name);

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
    kexLexer *lexer;

    at = (gActorTemplate_t*)Z_Calloc(sizeof(gActorTemplate_t), PU_STATIC, 0);

    // set default properties
    at->actor.mass          = 1200;
    at->actor.friction      = 1.0f;
    at->actor.airfriction   = 1.0f;
    at->actor.bounceDamp    = 0.0f;

    if(lexer = parser.Open(kva("actors/%s.kact", name)))
    {
        Actor_ParseTemplate(lexer, at);
        parser.Close();
    }

    hash = common.HashFileName(name);
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

    Actor_CopyProperties(actor, &at->actor);

    if(at->numComponents > 0)
    {
        unsigned int i;

        Actor_CreateComponent(actor);

        for(i = 0; i < at->numComponents; i++)
            Actor_AddComponent(actor, at->components[i]);
    }

    Vec_Set3(actor->origin, x, y, z);
    Vec_Set4(actor->rotation, 0, 0, 0, 1);
    Vec_Set4(actor->lerpRotation, 0, 0, 0, 1);

    actor->angles[0] = yaw;
    actor->angles[1] = pitch;
    actor->plane = plane;

    Actor_Setup(actor);
    return actor;
}

//
// Actor_SpawnEx
//

gActor_t *Actor_SpawnEx(float x, float y, float z, float yaw, float pitch, int plane,
                        int classFlags, const char *component, gObject_t *callback)
{
    gActor_t *actor;

    actor = (gActor_t*)Z_Calloc(sizeof(gActor_t), PU_ACTOR, NULL);

    // set default properties
    actor->classFlags   = classFlags;
    actor->mass         = 1200;
    actor->friction     = 1.0f;
    actor->airfriction  = 1.0f;
    actor->radius       = 30.72f;
    actor->baseHeight   = 30.72f;
    actor->viewHeight   = 16.384f;
    actor->centerHeight = 10.24f;

    Vec_Set3(actor->rotorVector, 0, 1, 0);
    Vec_Set3(actor->scale, 1, 1, 1);
    Vec_Set3(actor->bbox.min, -actor->radius, -actor->radius, -actor->radius);
    Vec_Set3(actor->bbox.max, actor->radius, actor->radius, actor->radius);
    Vec_Copy3(actor->baseBBox.min, actor->bbox.min);
    Vec_Copy3(actor->baseBBox.max, actor->bbox.max);

    if(actor->classFlags & AC_AI)
        AI_Spawn(actor);

    if(component != NULL)
    {
        Actor_CreateComponent(actor);
        Actor_AddComponent(actor, component);
    }

    actor->angles[0] = yaw;
    actor->angles[1] = pitch;
    actor->plane = plane;

    Vec_Set3(actor->origin, x, y, z);
    Vec_Set4(actor->lerpRotation, 0, 0, 0, 1);
    Vec_SetQuaternion(actor->rotation, actor->angles[0], 0, 1, 0);

    if(callback && JS_ObjectIsFunction(js_context, callback))
    {
        jsval actorVal;

        if(Actor_ToVal(actor, &actorVal))
        {
            jsval rval;
            JS_CallFunctionValue(js_context, js_gobject,
                OBJECT_TO_JSVAL(callback), 1, &actorVal, &rval);
        }
    }

    Actor_Setup(actor);
    return actor;
}

//
// Actor_ClearData
//

void Actor_ClearData(gActor_t *actor)
{
    if(actor->iterator)
        JS_RemoveRoot(js_context, &actor->iterator);

    if(actor->components)
    {
        JS_SetPrivate(js_context, actor->components, NULL);
        JS_RemoveRoot(js_context, &actor->components);
    }

    if(actor->classFlags & AC_AI && actor->ai)
    {
        if(actor->ai->object)
        {
            JS_SetPrivate(js_context, actor->ai->object, NULL);
            JS_RemoveRoot(js_context, &actor->ai->object);
        }

        Z_Free(actor->ai);
    }
}

//
// Actor_UpdateModel
//

void Actor_UpdateModel(gActor_t *actor, const char *model)
{
    if(model)
        actor->model = Kmesh_Load(model);

    if(actor->model)
    {
        unsigned int i;
        unsigned int j;
        anim_t *anim;
        kmodel_t *m;

        m = actor->model;

        // set initial animation
        // TODO - rename anim00 to something better
        if(anim = Mdl_GetAnim(m, "anim00"))
            Mdl_SetAnimState(&actor->animState, anim, 4, ANF_LOOP);

        // allocate node translation offset data
        actor->nodeOffsets_t = (vec3_t*)Z_Realloc(
            actor->nodeOffsets_t,
            sizeof(vec3_t) * m->numnodes,
            PU_ACTOR,
            0);

        // allocate node rotation offset data
        actor->nodeOffsets_r = (vec4_t*)Z_Realloc(
            actor->nodeOffsets_r,
            sizeof(vec4_t) * m->numnodes,
            PU_ACTOR,
            0);

        // set default rotation offsets
        for(i = 0; i < m->numnodes; i++)
            Vec_Set4(actor->nodeOffsets_r[i], 0, 0, 0, 1);

        // allocate data for texture swap array
        actor->textureSwaps = (char****)Z_Calloc(sizeof(char***) *
            m->numnodes, PU_ACTOR, NULL);

        for(j = 0; j < m->numnodes; j++)
        {
            unsigned int k;
            mdlnode_t *node;

            node = &m->nodes[j];

            actor->textureSwaps[j] = (char***)Z_Calloc(sizeof(char**) *
                node->nummeshes, PU_ACTOR, NULL);

            for(k = 0; k < node->nummeshes; k++)
            {
                mdlmesh_t *mesh;

                mesh = &node->meshes[k];

                actor->textureSwaps[j][k] = (char**)Z_Calloc(sizeof(char*) *
                    mesh->numsections, PU_ACTOR, NULL);
            }
        }
    }
}

//
// Actor_Remove
//

void Actor_Remove(gActor_t *actor)
{
    Map_UnlinkActorFromWorld(actor);
    actor->linkNext = actor->linkPrev = NULL;

    Actor_ClearData(actor);
    Z_Free(actor);
}

//
// Actor_Setup
//

void Actor_Setup(gActor_t *actor)
{
    gObject_t *ownerObject;
    jsval ownerVal;
    JSBool found;
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
    actor->timestamp = (float)server.GetRunTime();
    actor->height = actor->bStatic ? actor->baseHeight : 0;

    if(actor->bTouch)
    {
        actor->bCollision = false;
        actor->viewHeight = actor->baseHeight * 0.5f;
    }

    if(actor->components == NULL)
        return;

    actor->iterator = JS_NewPropertyIterator(js_context, actor->components);
    JS_AddRoot(js_context, &actor->iterator);

    if(actor->bStatic)
        common.Error("Static actor (%s) should not contain any components", actor->name);

    // create and set owner property
    ownerObject = J_NewObjectEx(cx, &GameActor_class, NULL, NULL);

    if(!JS_SetPrivate(cx, ownerObject, actor))
        return;

    ownerVal = OBJECT_TO_JSVAL(ownerObject);
    JS_SetProperty(cx, actor->components, "owner", &ownerVal);
    JS_SetPropertyAttributes(cx, actor->components, "owner", 0, &found);

    Actor_CallEvent(actor, "onReady", NULL, 0);
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

