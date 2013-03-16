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

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "render.h"
#include "level.h"

typedef struct
{
    gActor_t    *owner;
    vec3_t      velocity;
    vec3_t      forward;
    vec3_t      right;
    vec3_t      up;
    vec3_t      accel;
    vec3_t      angles;
    float       moveTime;
    float       frameTime;
    float       timeStamp;
    plane_t     *plane;
} aController_t;

void Ctrl_SetDirection(aController_t *ctrl,
                       float yaw, float pitch, float roll);
void Ctrl_ApplyFriction(aController_t *ctrl, float friction, kbool effectY);
void Ctrl_Accelerate(aController_t *ctrl, float speed, float accel, int v);
void Ctrl_ApplyGravity(aController_t *ctrl, float gravity);

#endif
