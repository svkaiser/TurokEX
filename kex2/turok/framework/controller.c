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
// DESCRIPTION: Actor controller system
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "mathlib.h"
#include "actor.h"
#include "level.h"

#define VELOCITY_EPSILON    0.0001f

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

//
// Ctrl_SetDirection
//

void Ctrl_SetDirection(aController_t *ctrl,
                       float yaw, float pitch, float roll)
{
    float sy = (float)sin(yaw);
    float cy = (float)cos(yaw);
    float sp = (float)sin(pitch);
    float cp = (float)cos(pitch);
    float sr = (float)sin(roll);
    float cr = (float)cos(roll);

    ctrl->forward[0] = sy * cp;
    ctrl->forward[1] = -sp;
    ctrl->forward[2] = cy * cp;

    ctrl->right[0] = sr * sp * sy + cr * cy;
    ctrl->right[1] = sr * cp;
    ctrl->right[2] = sr * sp * cy + cr * -sy;

    ctrl->up[0] = cr * sp * sy + -sr * cy;
    ctrl->up[1] = cr * cp;
    ctrl->up[2] = cr * sp * cy + -sr * -sy;
}

//
// Ctrl_ApplyFriction
//

void Ctrl_ApplyFriction(aController_t *ctrl, float friction, kbool effectY)
{
    float speed;

    speed = Vec_Unit3(ctrl->velocity);

    if(speed < VELOCITY_EPSILON)
    {
        ctrl->velocity[0] = 0;
        ctrl->velocity[2] = 0;
    }
    else
    {
        float clipspeed = speed - (speed * friction);

        if(clipspeed < 0) clipspeed = 0;
        clipspeed /= speed;

        // de-accelerate velocity
        ctrl->velocity[0] = ctrl->velocity[0] * clipspeed;
        ctrl->velocity[2] = ctrl->velocity[2] * clipspeed;

        if(effectY)
            ctrl->velocity[1] = ctrl->velocity[1] * clipspeed;
    }
}

//
// Ctrl_Accelerate
//

void Ctrl_Accelerate(aController_t *ctrl, float speed, float accel, int v)
{
    float t;

    if(v < 0 || v > 2)
        return;

    speed = speed * ctrl->frameTime;

    t = (accel * (60 * ctrl->frameTime));
    if(t > 1)
    {
        ctrl->accel[v] = speed;
        return;
    }
    
    ctrl->accel[v] = (speed - ctrl->accel[v]) * t + ctrl->accel[v];
}

//
// Ctrl_ApplyGravity
//

void Ctrl_ApplyGravity(aController_t *ctrl, float gravity)
{
    plane_t *plane = ctrl->plane;
    gActor_t *actor = ctrl->owner;

    if((actor->origin[1] - Plane_GetDistance(plane, actor->origin)) > 0.01f)
        actor->origin[1] -= ((gravity * ctrl->frameTime) * ctrl->frameTime);
}
