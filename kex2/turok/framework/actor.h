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

typedef struct gActor_s
{
    kbool               bOrientOnSlope;
    kbool               bStatic;
    kbool               bCollision;
    kbool               bTouch;
    kbool               bClientOnly;
    kbool               bHidden;
    vec3_t              origin;
    vec3_t              scale;
    vec4_t              rotation;
    bbox_t              bbox;
    kmodel_t            *model;
    animstate_t         animState;
    int                 variant;
    char                **textureSwaps;
    float               angles[3];
    char                name[64];
    int                 plane;
    float               radius;
    float               height;
    float               centerHeight;
    float               viewHeight;
    mtx_t               matrix;
    gObject_t           *components;
	gObject_t			*iterator;
} gActor_t;

void Actor_Setup(gActor_t *actor);
void Actor_UpdateTransform(gActor_t *actor);
void Actor_CallEvent(gActor_t *actor, const char *function, gActor_t *instigator);
void Actor_ComponentFunc(const char *function);
kbool Actor_HasComponent(gActor_t *actor, const char *component);
void Actor_OnTouchEvent(gActor_t *actor, gActor_t *instigator);

#endif
