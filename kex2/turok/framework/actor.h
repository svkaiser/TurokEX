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

#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "render.h"
#include "level.h"

typedef enum
{
    AC_AI       = 1,
    AC_MESH     = 2,
    AC_FX       = 4,
    AC_PLAYER   = 8
} actorClassFlags_e;

typedef enum
{
    WL_INVALID  = 0,
    WL_OVER     = 1,
    WL_BETWEEN  = 2,
    WL_UNDER    = 3
} actorWaterLevel_e;

typedef struct gActor_s
{
    vec3_t              origin;
    vec4_t              rotation;
    vec3_t              velocity;
    int                 refcount;
    kbool               bStale;
    kbool               bOrientOnSlope;
    kbool               bStatic;
    kbool               bCollision;
    kbool               bTouch;
    kbool               bClientOnly;
    kbool               bHidden;
    kbool               bNoDropOff;
    kbool               bRotor;
    bbox_t              bbox;
    float               angles[3];
    char                name[64];
    int                 plane;
    float               radius;
    float               height;
    float               baseHeight;
    float               centerHeight;
    float               viewHeight;
    float               friction;
    float               airfriction;
    float               mass;
    float               bounceDamp;
    float               cullDistance;
    unsigned int        targetID;
    mtx_t               matrix;
    mtx_t               rotMtx;
    vec4_t              lerpRotation;
    gObject_t           *components;
	gObject_t           *iterator;
    int                 numProperties;
    int                 classFlags;
    unsigned int        surfaceID;
    vec3_t              *nodeOffsets_t;
    vec4_t              *nodeOffsets_r;
    float               rotorSpeed;
    float               rotorFriction;
    vec3_t              rotorVector;
    struct gActor_s     *owner;
    struct gActor_s     *prev;
    struct gActor_s     *next;
    struct gActor_s     *linkPrev;
    struct gActor_s     *linkNext;
    vec3_t              scale;
    kmodel_t            *model;
    animstate_t         animState;
    int                 variant;
    char                ****textureSwaps;
    float               timestamp;
    float               tickDistance;
    kbool               bCulled;
    int                 physics;
    int                 waterlevel;
    struct ai_s         *ai;
} gActor_t;

typedef struct gActorTemplate_s
{
    char                    name[MAX_FILEPATH];
    gActor_t                actor;
    char                    **components;
    unsigned int            numComponents;
    struct gActorTemplate_s *next;
} gActorTemplate_t;

void Actor_Setup(gActor_t *actor);
void Actor_SetTarget(gActor_t **self, gActor_t *target);
void Actor_UpdateTransform(gActor_t *actor);
kbool Actor_CallEvent(gActor_t *actor, const char *function, long *args, unsigned int nargs);
void Actor_LocalTick(void);
void Actor_Tick(void);
kbool Actor_OnGround(gActor_t *actor);
kbool Actor_ToVal(gActor_t *actor, long *val);
kbool Actor_HasComponent(gActor_t *actor, const char *component);
kbool Actor_FXEvent(gActor_t *actor, gActor_t *target, vec3_t fxOrigin, vec3_t fxVelocity,
                    int plane, action_t *action);
void Actor_GetLocalVectors(vec3_t out, gActor_t *actor, float x, float y, float z);
void Actor_GetWaterLevel(gActor_t *actor);
void Actor_SpawnBodyFX(gActor_t *actor, const char *fx, float x, float y, float z);
void Actor_OnTouchEvent(gActor_t *actor, gActor_t *instigator);
void Actor_DrawDebugStats(void);
void Actor_UpdateModel(gActor_t *actor, const char *model);
void Actor_Remove(gActor_t *actor);
void Actor_ClearData(gActor_t *actor);
gActor_t *Actor_Spawn(const char *classname, float x, float y, float z,
                      float yaw, float pitch, int plane);
gActor_t *Actor_SpawnEx(float x, float y, float z, float yaw, float pitch, int plane,
                        int classFlags, const char *component, gObject_t *callback);

#endif
