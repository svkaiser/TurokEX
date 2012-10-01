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

#include "type.h"
#include "mathlib.h"
#include "render.h"

typedef enum
{
    AF_NOALIGNPITCH     = 0x1
} actorflags_t;

typedef struct actor_s
{
    vec3_t              origin;
    vec4_t              rotation;
    vec3_t              velocity;
    vec3_t              scale;
    plane_t             *plane;
    plane_t             *hitplane;
    actorflags_t        flags;
    float               yaw;
    float               pitch;
    int                 svclient_id;
    kmodel_t            *model;
    short               type;
    int                 health;
    float               meleerange;
    float               width;
    float               height;
    int                 mapactor_id;
    struct actor_s      *target;
    struct actor_s      *prev;
    struct actor_s      *next;
} actor_t;

void G_LinkActor(actor_t *actor);
void G_UnlinkActor(actor_t* actor);
kbool G_ActorOnPlane(actor_t *actor);
void G_ActorMovement(actor_t *actor);
actor_t *G_SpawnActor(float x, float y, float z, float yaw, const char *model, short type);
float G_GetActorMeleeRange(actor_t *actor, vec3_t targetpos);
void G_RotateActorToPlane(vec4_t rot, actor_t *actor);

#endif
