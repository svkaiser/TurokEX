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
// DESCRIPTION: Actor system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "server.h"
#include "kernel.h"
#include "mathlib.h"
#include "game.h"
#include "actor.h"
#include "level.h"
#include "zone.h"

static actor_t *g_currentactor;

actor_t *g_actorlist;
actor_t actorlist[MAXMAPS];

//
// G_LinkActor
//

void G_LinkActor(actor_t *actor)
{
    g_actorlist->prev->next = actor;
    actor->next = g_actorlist;
    actor->prev = g_actorlist->prev;
    g_actorlist->prev = actor;
}

//
// G_UnlinkActor
//

void G_UnlinkActor(actor_t* actor)
{
    /* Remove from main actor list */
    actor_t* next = g_currentactor->next;

    /* Note that g_currentactor is guaranteed to point to us,
    * and since we're freeing our memory, we had better change that. So
    * point it to actor->prev, so the iterator will correctly move on to
    * actor->prev->next = actor->next */
    (next->prev = g_currentactor = actor->prev)->next = next;
}

//
// G_SpawnActor
//

actor_t *G_SpawnActor(void)
{
    actor_t *actor;

    if(g_currentmap == NULL)
    {
        return NULL;
    }

    actor = (actor_t*)Z_Calloc(sizeof(*actor), PU_ACTOR, NULL);
    G_LinkActor(actor);

    return actor;
}

//
// G_SetActorLinkList
//

void G_SetActorLinkList(int map)
{
    g_actorlist = &actorlist[map];
}

//
// G_GetActorMeleeRange
//

float G_GetActorMeleeRange(actor_t *actor, vec3_t targetpos)
{
    float x;
    float y;
    float z;

    x = actor->origin[0] - targetpos[0];
    y = actor->object.height + actor->origin[1] - targetpos[1];
    z = actor->origin[2] - targetpos[2];

    return (float)sqrt(x * x + y * y + z * z);
}

