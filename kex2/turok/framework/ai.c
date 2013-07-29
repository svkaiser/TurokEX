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
#include "fx.h"

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
    actor->ai->activeDistance   = 1024.0f;
    actor->ai->turnSpeed        = 4.096f;
    actor->ai->thinkTime        = 16;
    actor->ai->headTurnSpeed    = 4.096f;
    actor->ai->maxHeadAngle     = DEG2RAD(70);

    Vec_Set3(actor->ai->headYawAxis, 0, 1, 0);
    Vec_Set3(actor->ai->headPitchAxis, 1, 0, 0);

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
    y = (actor->origin[1] + actor->height) - (target->origin[1] + target->height);
    z = actor->origin[2] - target->origin[2];

    return (float)sqrt(x*x+y*y+z*z);
}

//
// AI_GetYawToTarget
//

float AI_GetYawToTarget(ai_t *ai, gActor_t *target)
{
    vec3_t vec;

    if(!target)
        return ai->owner->angles[0];

    Vec_PointToAxis(vec, target->origin, ai->owner->origin);
    return Ang_Round(Ang_ClampInvertSums(ai->owner->angles[0],
        Ang_VectorToAngle(vec)));
}

//
// AI_FindBestAngleToTarget
//

float AI_FindBestAngleToTarget(ai_t *ai, gActor_t *target, float extendRadius)
{
    float angle;
    vec3_t position;
    gActor_t *owner;

    owner = ai->owner;
    angle = AI_GetYawToTarget(ai, target);

    if(!target)
        return angle;

    Vec_Set3(position,
        owner->origin[0],
        owner->origin[1] + (owner->height * 0.8f),
        owner->origin[2]);

    if(!AI_CheckPosition(ai, position, extendRadius, owner->angles[0] + angle))
    {
        float an;
        float pAn;
        int i;

        pAn = M_PI / 8;

        for(i = 0; i < 8; i++)
        {
            float dir;

            an = ((i+1) * pAn);
            dir = an + angle;

            if(AI_CheckPosition(ai, position, extendRadius, owner->angles[0] + dir))
            {
                angle = DEG2RAD(15) + dir;
                Ang_Clamp(&angle);
                break;
            }

            dir = angle - an;

            if(AI_CheckPosition(ai, position, extendRadius, owner->angles[0] + dir))
            {
                angle = dir - DEG2RAD(15);
                Ang_Clamp(&angle);
                break;
            }
        }
    }

    return angle;
}

//
// AI_TracePosition
//

trace_t AI_TracePosition(ai_t *ai, vec3_t position, float radius, float angle)
{
    vec3_t dest;
    plane_t *plane;
    float s = (float)sin(angle);
    float c = (float)cos(angle);

    dest[0] = position[0] + (ai->owner->radius * radius * s);
    dest[1] = position[1];
    dest[2] = position[2] + (ai->owner->radius * radius * c);

    plane = ai->owner->plane != -1 ? &gLevel.planes[ai->owner->plane] : NULL;
    return Trace(position, dest, plane, ai->owner, ai->owner->physics);
}

//
// AI_CheckPosition
//

kbool AI_CheckPosition(ai_t *ai, vec3_t position, float radius, float angle)
{
    trace_t trace = AI_TracePosition(ai, position, radius, angle);
    kbool bHitWall = false;
    kbool bHitActor = false;

    if(ai->flags & AIF_AVOIDWALLS)
        bHitWall = !(trace.type != TRT_WALL && trace.type != TRT_EDGE);

    if(ai->flags & AIF_AVOIDACTORS)
        bHitActor = !(trace.type != TRT_OBJECT);

    return !(bHitWall | bHitActor);
}

//
// AI_FireProjectile
//

void AI_FireProjectile(ai_t *ai, const char *fxname, float x, float y, float z,
                       float maxangle, kbool localToActor)
{
    vec4_t frot;
    vec3_t origin;
    vec3_t torg;
    vec3_t tmp;
    gActor_t *actor;
    gActor_t *target;

    actor = ai->owner;
    target = ai->target;

    if(!target)
        return;

    if(!localToActor)
        Vec_Set3(origin, x, y, z);
    else
        Actor_GetLocalVectors(origin, actor, x, y, z);

    Vec_Copy3(torg, target->origin);
    torg[1] += 30.72f;

    Vec_PointAt(origin, torg, actor->rotation, maxangle, frot);
    Vec_Set3(tmp, 0, 0, 0);

    FX_Spawn(fxname, actor, tmp, origin, frot,
        Map_IndexToPlane(actor->plane));
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
    pos[1] = self->origin[1] + (self->baseHeight * 0.8f);
    pos[2] = self->origin[2];

    dest[0] = target->origin[0];
    dest[1] = target->origin[1] + (target->baseHeight * 0.8f);
    dest[2] = target->origin[2];

    trace = Trace(pos, dest, plane, self,
        PF_CLIPGEOMETRY | PF_CLIPEDGES | PF_DROPOFF);

    if(trace.type == TRT_NOHIT)
    {
        ai->flags |= AIF_SEETARGET;
        return true;
    }

    // something is obstructing its line of sight
    ai->flags &= ~AIF_SEETARGET;
    return false;
}

//
// AI_SetHeadLook
//

static void AI_SetHeadLook(ai_t *ai)
{
    gActor_t *actor;
    float yan;
    float current;
    float speed;
    float diff;
    vec4_t roty;

    actor = ai->owner;

    if(!actor->model)
        return;

    if(ai->nodeHead == 0 || ai->nodeHead >= actor->model->numnodes)
        return;

    Vec_Set4(roty, 0, 0, 0, 1);
    yan = 0;

    if(ai->target && (ai->flags & AIF_LOOKATTARGET &&
        ai->flags & AIF_SEETARGET))
    {
        vec3_t p1;
        vec3_t p2;
        vec3_t dir;
        gActor_t *target;

        target = ai->target;

        p2[0] = actor->origin[0];
        p2[1] = actor->origin[1] + actor->height;
        p2[2] = actor->origin[2];

        p1[0] = target->origin[0];
        p1[1] = target->origin[1] + target->height;
        p1[2] = target->origin[2];

        yan = M_PI - (actor->angles[0] - (float)atan2(p2[0] - p1[0], p2[2] - p1[2]));
        Ang_Clamp(&yan);

        Vec_Sub(dir, p2, p1);
        Vec_Normalize3(dir);

        if(yan >  ai->maxHeadAngle) yan =  ai->maxHeadAngle;
        if(yan < -ai->maxHeadAngle) yan = -ai->maxHeadAngle;
    }

    speed = ai->headTurnSpeed * ai->owner->timestamp;

    current = Ang_Round(ai->headYaw);
    if(fabs(current - yan) <= 0.001f)
        ai->headYaw = yan;
    else
    {
        diff = yan - current;
        Ang_Clamp(&diff);

        if(diff > 0)
        {
            if(diff > speed)
                diff = speed;
        }
        else if(diff < -speed)
            diff = -speed;

        ai->headYaw = Ang_Round(current + diff);
        Ang_Clamp(&ai->headYaw);
    }

    Vec_SetQuaternion(actor->nodeOffsets_r[ai->nodeHead],
        ai->headYaw, ai->headYawAxis[0], ai->headYawAxis[1], ai->headYawAxis[2]);
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
            jsval val;

            Actor_SetTarget(&ai->target, client.playerActor);

            if(Actor_ToVal(ai->target, &val))
                Actor_CallEvent(ai->owner, "onTargetFound", &val, 1);
        }

        ai->flags |= AIF_SEEPLAYER;
    }
}

//
// AI_ClearTarget
//

void AI_ClearTarget(ai_t *ai)
{
    Actor_SetTarget(&ai->target, NULL);
    ai->flags &= ~AIF_SEEPLAYER;
    ai->flags &= ~AIF_SEETARGET;
    ai->flags &= ~AIF_LOOKATTARGET;
}

//
// AI_CheckSleepRange
//

void AI_CheckSleepRange(ai_t *ai)
{
    // TODO - Handle network players
    if(Vec_Length2(ai->owner->origin,
        client.playerActor->origin) > ai->activeDistance)
    {
        if(!(ai->flags & AIF_DORMANT))
        {
            ai->flags |= AIF_DORMANT;
            Actor_CallEvent(ai->owner, "onSleep", NULL, 0);
        }
        return;
    }

    if(ai->flags & AIF_DORMANT)
        Actor_CallEvent(ai->owner, "onWake", NULL, 0);

    ai->flags &= ~AIF_DORMANT;
}

//
// AI_Think
//

void AI_Think(ai_t *ai)
{
    if(ai == NULL)
        return;

    if(!ai->owner)
        return;

    if(ai->owner->bStale)
    {
        Actor_SetTarget(&ai->owner, NULL);
        return;
    }

    if(ai->flags & AIF_DISABLED)
        return;

    AI_SetHeadLook(ai);
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

    Actor_CallEvent(ai->owner, "onThink", NULL, 0);
}
