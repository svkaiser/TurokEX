// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _AI_H_
#define _AI_H_

#include "actor_old.h"

typedef enum
{
    AIF_TURNING         = 1,
    AIF_DORMANT         = 2,
    AIF_SEETARGET       = 4,
    AIF_SEEPLAYER       = 8,
    AIF_FINDACTOR       = 16,
    AIF_FINDPLAYERS     = 32,
    AIF_AVOIDWALLS      = 64,
    AIF_AVOIDACTORS     = 128,
    AIF_DISABLED        = 256,
    AIF_LOOKATTARGET    = 512
} aiFlags_e;

typedef struct ai_s
{
    float           idealYaw;
    float           turnSpeed;
    float           activeDistance;
    unsigned int    flags;
    vec3_t          goalOrigin;
    float           thinkTime;
    float           nextThinkTime;
    unsigned int    nodeHead;
    float           headYaw;
    float           headPitch;
    float           headTurnSpeed;
    float           maxHeadAngle;
    vec3_t          headYawAxis;
    vec3_t          headPitchAxis;
    gObject_t       *object;
    struct gActor_s *owner;
    struct gActor_s *target;
} ai_t;

ai_t *AI_Spawn(gActor_t *actor);
float AI_GetTargetDistance(ai_t *ai, gActor_t *target);
float AI_GetYawToTarget(ai_t *ai, gActor_t *target);
float AI_FindBestAngleToTarget(ai_t *ai, gActor_t *target, float extendRadius);
kbool AI_CheckPosition(ai_t *ai, vec3_t position, float radius, float angle);
void AI_FireProjectile(ai_t *ai, const char *fxname, float x, float y, float z,
                       float maxangle, kbool localToActor);
void AI_SetIdealYaw(ai_t *ai, float idealYaw, float turnSpeed);
void AI_Turn(ai_t *ai);
kbool AI_CanSeeTarget(ai_t *ai, gActor_t *target);
void AI_FindPlayers(ai_t *ai);
void AI_ClearTarget(ai_t *ai);
void AI_Think(ai_t *ai);

#endif
