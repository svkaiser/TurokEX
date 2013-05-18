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
#include "actor.h"

#define ONPLANE_EPSILON     0.512f
#define VELOCITY_EPSILON    0.0001f
#define SLOPE_THRESHOLD     25.0f

void G_ClipVelocity(vec3_t out, vec3_t velocity, vec3_t normal, float fudge);
void G_ApplyFriction(vec3_t velocity, float friction, kbool effectY);
void G_ApplyBounceVelocity(vec3_t velocity, vec3_t reflection, float amount);
kbool G_TryMove(gActor_t *source, vec3_t origin, vec3_t dest, plane_t **plane);
kbool G_ClipMovement(vec3_t origin, vec3_t velocity, plane_t **plane,
                    gActor_t *actor, trace_t *t);

void G_Shutdown(void);
void G_Ticker(void);
void G_ClientThink(void);
void G_Init(void);

#endif

