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

#ifndef __GAME_H__
#define __GAME_H__

#include "client.h"
#include "actor_old.h"

#define ONPLANE_EPSILON     0.512f
#define VELOCITY_EPSILON    0.0001f
#define SLOPE_THRESHOLD     25.0f

typedef enum
{
    PF_CLIPSTATICS      = 0x1,
    PF_CLIPACTORS       = 0x2,
    PF_CLIPGEOMETRY     = 0x4,
    PF_CLIPEDGES        = 0x8,
    PF_IGNOREBLOCKERS   = 0x10,
    PF_CLIMBSURFACES    = 0x20,
    PF_SLIDEMOVE        = 0x40,
    PF_NOSTEPUP         = 0x80,
    PF_DROPOFF          = 0x100,
    PF_NOENTERWATER     = 0x200,
    PF_NOEXITWATER      = 0x400,
    PF_TOUCHACTORS      = 0x800,
} physics_flags_e;

#define PF_CLIP_ALL (PF_CLIPSTATICS|PF_CLIPACTORS|PF_CLIPGEOMETRY|PF_CLIPEDGES)
#define PF_EXCLUDE_FLAG(f, exclude) ((f & exclude) ? (f ^ exclude) : f)

void G_ClipVelocity(vec3_t out, vec3_t velocity, vec3_t normal, float fudge);
void G_ApplyFriction(vec3_t velocity, float friction, kbool effectY);
void G_ApplyBounceVelocity(vec3_t velocity, vec3_t reflection, float amount);
void G_ApplyGravity(vec3_t origin, vec3_t velocity, plane_t *plane,
                    float mass, float timeDelta);
kbool G_TryMove(gActor_t *source, vec3_t origin, vec3_t dest, plane_t **plane);
kbool G_ClipMovement(vec3_t origin, vec3_t velocity, float time, plane_t **plane,
                    gActor_t *actor);

void G_Shutdown(void);
void G_Ticker(void);
void G_ClientThink(void);
void G_Init(void);

#endif

