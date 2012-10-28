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
#include "render.h"
#include "level.h"

typedef enum
{
    AF_NOALIGNPITCH     = 0x1,
    AF_CLIENTJUMP       = 0x2
} actorflags_t;

typedef enum
{
    TT_NORMAL           = 0,
    TT_WATER_SHALLOW    = 1,
    TT_WATER_SURFACE    = 2,
    TT_WATER_UNDER      = 3,
    TT_LAVA             = 4,
    TT_DEATHPIT         = 5,
    TT_NOCLIP           = 6
} terriantype_t;

typedef struct actor_s
{
    vec3_t              origin;
    vec3_t              velocity;
    unsigned int        flags;
    float               yaw;
    float               pitch;
    int                 svclient_id;
    int                 health;
    short               skin;
    object_t            object;
    plane_t             *plane;
    terriantype_t       terriantype;
    struct actor_s      *target;
    struct actor_s      *prev;
    struct actor_s      *next;
} actor_t;

extern actor_t actorlist[MAXMAPS];
extern actor_t *g_actorlist;

void G_LinkActor(actor_t *actor);
void G_UnlinkActor(actor_t* actor);
void G_ActorMovement(actor_t *actor);
actor_t *G_SpawnActor(void);
void G_SetActorLinkList(int map);
float G_GetActorMeleeRange(actor_t *actor, vec3_t targetpos);

#endif
