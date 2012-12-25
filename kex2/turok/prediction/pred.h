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

#ifndef _PRED_H_
#define _PRED_H_

#include "shared.h"

typedef enum
{
    PMF_JUMP        = 0x1,
    PMF_SUBMERGED   = 0x2,
    PMF_ONGROUND    = 0x4
} pmflags_t;

typedef enum
{
    MT_NORMAL = 0,
    MT_WATER_SHALLOW,
    MT_WATER_SURFACE,
    MT_WATER_UNDER,
    MT_LAVA,
    MT_DEATHPIT,
    MT_NOCLIP,
    MT_CLIMB,
    NUMMOVETYPES
} movetype_t;

typedef struct
{
    vec3_t      origin;
    vec3_t      velocity;
    vec3_t      forward;
    vec3_t      right;
    vec3_t      up;
    vec3_t      accel;
    float       width;
    float       height;
    float       center_y;
    float       view_y;
    float       yaw;
    float       pitch;
    float       roll;
    float       deltatime;
    int         movetype;
    plane_t     *plane;
    pmflags_t   flags;
    ticcmd_t    *cmd;
} move_t;

extern move_t movecontroller;

typedef struct
{
    vec3_t      origin;
    vec3_t      velocity;
    vec3_t      accel;
    float       angles[3];
    float       radius;
    float       height;
    float       centerheight;
    float       viewheight;
    pmflags_t   flags;
    int         movetype;
    int         plane;
} pmove_t;

typedef struct
{
    pmove_t     pmove;
    ticcmd_t    cmd;
} pred_t;

void Pred_SetDirection(move_t *move, float yaw, float pitch, float roll);
void Pred_ProcessMove(move_t *move, float friction, float gravity);
void Pred_ClientMovement(void);
void Pred_ServerMovement(void);

#endif