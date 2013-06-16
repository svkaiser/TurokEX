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
// DESCRIPTION: AI System
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"
#include "actor.h"
#include "client.h"
#include "zone.h"
#include "ai.h"

//
// AI_Spawn
//

ai_t *AI_Spawn(gActor_t *actor)
{
    if(actor->bStale)
        return NULL;

    actor->ai = (ai_t*)Z_Calloc(sizeof(ai_t), PU_AI, NULL);
    actor->ai->owner = actor;

    // defaults
    actor->ai->activeDistance = 2048.0f;
    actor->ai->turnSpeed = 4.096f;
    actor->ai->thinkTime = 16;

    // create AI script object
    actor->ai->object = J_NewObjectEx(js_context, &AI_class, NULL, NULL);
    JS_SetPrivate(js_context, actor->ai->object, actor->ai);
    JS_AddRoot(js_context, &actor->ai->object);

    return actor->ai;
}

//
// AI_GetTargetDistance
//

float AI_GetTargetDistance(ai_t *ai, gActor_t *target)
{
    float x;
    float y;
    float z;
    gActor_t *actor;

    actor = ai->owner;

    x = actor->origin[0] - target->origin[0];
    y = (actor->origin[1] + actor->centerHeight) - target->origin[1];
    z = actor->origin[2] - target->origin[2];

    return (float)sqrt(x*x+y*y+z*z);
}

//
// AI_GetYawToTarget
//

float AI_GetYawToTarget(ai_t *ai, gActor_t *target)
{
    vec3_t vec;

    Vec_PointToAxis(vec, target->origin, ai->owner->origin);
    return Ang_Round(Ang_ClampInvertSums(ai->owner->angles[0],
        Ang_VectorToAngle(vec)));
}

//
// AI_CheckPosition
//

kbool AI_CheckPosition(ai_t *ai, vec3_t position, float radius, float angle)
{
    vec3_t dest;
    trace_t trace;
    plane_t *plane;
    float s = (float)sin(angle + M_PI);
    float c = (float)cos(angle + M_PI);

    dest[0] = position[0] + (ai->owner->radius * radius * s);
    dest[1] = position[1];
    dest[2] = position[2] + (ai->owner->radius * radius * c);

    plane = ai->owner->plane != -1 ? &gLevel.planes[ai->owner->plane] : NULL;
    trace = Trace(position, dest, plane, NULL, ai->owner, false);

    return (trace.type != TRT_WALL && trace.type != TRT_EDGE);
}

//
// AI_SetIdealYaw
//

void AI_SetIdealYaw(ai_t *ai, float idealYaw, float turnSpeed)
{
    ai->idealYaw = Ang_Round(idealYaw);
    ai->turnSpeed = turnSpeed;
    ai->flags |= AIF_TURNING;
}

//
// AI_Turn
//

void AI_Turn(ai_t *ai)
{
    float speed;
    float current;
    float diff;

    speed = ai->turnSpeed * ai->owner->timestamp;
    current = Ang_Round(ai->owner->angles[0]);

    if(fabs(current - ai->idealYaw) <= 0.001f)
    {
        ai->flags &= ~AIF_TURNING;
        Actor_CallEvent(ai->owner, "onTurned", NULL);
        return;
    }

    diff = ai->idealYaw - current;
    Ang_Clamp(&diff);

    if(diff > 0)
    {
        if(diff > speed)
            diff = speed;
    }
    else if(diff < -speed)
        diff = -speed;

    ai->owner->angles[0] = Ang_Round(current + diff);
}

//
// AI_CanSeeTarget
//

kbool AI_CanSeeTarget(ai_t *ai, gActor_t *target)
{
    vec3_t pos;
    vec3_t dest;
    gActor_t *self;
    plane_t *plane;
    trace_t trace;

    self = ai->owner;
    plane = Map_IndexToPlane(self->plane);

    pos[0] = self->origin[0];
    pos[1] = self->origin[1] +
        ((self->viewHeight + self->centerHeight) * 0.8f);
    pos[2] = self->origin[2];

    dest[0] = target->origin[0];
    dest[1] = target->origin[1] +
        ((target->viewHeight + target->centerHeight) * 0.8f);
    dest[2] = target->origin[2];

    trace = Trace(pos, dest, plane, NULL, self, true);

    if(trace.hitActor && trace.hitActor == target)
    {
        ai->flags |= AIF_SEETARGET;
        return true;
    }

    ai->flags &= ~AIF_SEETARGET;
    return false;
}

//
// AI_FindPlayers
//

void AI_FindPlayers(ai_t *ai)
{
    ai->flags &= ~AIF_SEEPLAYER;

    if(AI_CanSeeTarget(ai, client.playerActor))
    {
        if(!ai->target)
        {
            Actor_SetTarget(&ai->target, client.playerActor);
            Actor_CallEvent(ai->owner, "onTargetFound", ai->target);
        }

        ai->flags |= AIF_SEEPLAYER;
    }
}

//
// AI_CheckSleepRange
//

void AI_CheckSleepRange(ai_t *ai)
{
    // TODO - Handle network players
    if(ai->activeDistance < Vec_Length2(ai->owner->origin,
        client.playerActor->origin))
    {
        if(!(ai->flags & AIF_DORMANT))
        {
            ai->flags |= AIF_DORMANT;
            Actor_CallEvent(ai->owner, "onSleep", NULL);
        }
        return;
    }

    if(ai->flags & AIF_DORMANT)
        Actor_CallEvent(ai->owner, "onWake", NULL);

    ai->flags &= ~AIF_DORMANT;
}

//
// AI_Think
//

void AI_Think(ai_t *ai)
{
    if(ai == NULL)
        return;

    AI_CheckSleepRange(ai);

    if(ai->flags & AIF_TURNING)
        AI_Turn(ai);

    if(ai->flags & AIF_DORMANT)
        return;

    if(ai->nextThinkTime >= gLevel.time)
        return;

    ai->nextThinkTime = gLevel.time + ai->thinkTime;

    // TODO - TEMP
    if(ai->flags & AIF_FINDPLAYERS)
        AI_FindPlayers(ai);

    Actor_CallEvent(ai->owner, "onThink", NULL);
}
