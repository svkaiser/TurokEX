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

#define SLOPE_THRESHOLD     25.0f

#define FRICTION_GROUND     0.5f
#define FRICTION_LAVA       0.205f
#define FRICTION_WATERMASS  0.902f
#define FRICTION_WTRIMPACT  0.905f

#define GRAVITY_NORMAL      0.62f
#define GRAVITY_WATER       0.005f
#define GRAVITY_FLOAT       0.45f

#define VELOCITY_EPSILON    0.0001f
#define ONPLANE_EPSILON     0.512f

#define WATERHEIGHT         15.36f
#define SHALLOWHEIGHT       20.8f

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

    actor->svclient_id  = -1;

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
// G_ActorOnPlane
//

kbool G_ActorOnPlane(actor_t *actor)
{
    if(actor->plane == NULL)
    {
        return false;
    }

    if((actor->origin[1] + actor->velocity[1]) -
        Plane_GetDistance(actor->plane, actor->origin) < ONPLANE_EPSILON)
    {
        if(Plane_IsAWall(actor->plane))
        {
            return false;
        }

        return true;
    }

    if(actor->velocity[1] < 0.0f && actor->velocity[1] > -16)
    {
        return true;
    }

    return false;
}

//
// G_CheckObjectStep
//
// Very basic check to see if an actor can stand on top
// of an object
//

static void G_CheckObjectStep(actor_t *actor)
{
    blockobj_t *obj;
    sector_t *sector;

    if(actor->plane == NULL)
    {
        return;
    }

    // TODO: this doesn't seem like the right way to do this...
    sector = &g_currentmap->sectors[actor->plane - g_currentmap->planes];

    // go through the list
    for(obj = sector->blocklist.next; obj != &sector->blocklist; obj = obj->next)
    {
        float height; 

        height = obj->object->origin[1] + obj->object->height;

        // allow upwards movement in case actor gets stuck inside object
        if(Vec_Length2(actor->origin, obj->object->origin) < obj->object->width &&
            actor->velocity[1] <= 0)
        {
            if(actor->origin[1] >= height && actor->origin[1] + actor->velocity[1] < height)
            {
                actor->origin[1] = height;
                actor->velocity[1] = 0;
                return;
            }
        }
    }
}

//
// G_GetTerrianType
//

static void G_GetTerrianType(actor_t *actor)
{
    float dist;

    if(actor->terriantype == TT_NOCLIP)
    {
        // ignore while in noclip mode
        return;
    }

    if(!actor->plane)
    {
        actor->terriantype = TT_NORMAL;
        return;
    }

    dist = Plane_GetDistance(actor->plane, actor->origin);

    if(actor->plane->flags & CLF_WATER)
    {
        float waterheight = Map_GetArea(actor->plane)->waterplane;

        // shallow water
        if(dist - actor->origin[1] <= WATERHEIGHT &&
            waterheight - dist <= SHALLOWHEIGHT)
        {
            actor->terriantype = TT_WATER_SHALLOW;
            return;
        }
        
        // underwater
        if(actor->origin[1] + actor->meleerange +
            WATERHEIGHT < waterheight)
        {
            actor->terriantype = TT_WATER_UNDER;
            return;
        }
        // water surface
        else if(actor->origin[1] +
            actor->meleerange < waterheight)
        {
            actor->terriantype = TT_WATER_SURFACE;
            return;
        }
    }
    
    // lava
    if(actor->plane->flags & CLF_DAMAGE_LAVA &&
        actor->svclient_id != -1 && dist < ONPLANE_EPSILON)
    {
        actor->terriantype = TT_LAVA;
        return;
    }

    // normal ground
    actor->terriantype = TT_NORMAL;
}


//
// G_ActorMovement
//
// Actor velocity is updated here
//

void G_ActorMovement(actor_t *actor)
{
    vec3_t position;
    float dist;
    float friction;

    // TEMP
    if(g_currentmap == NULL)
    {
        return;
    }

    // clip velocity before we update it
    G_ClipMovement(actor);

    // save previous origin first
    Vec_Copy3(actor->prevorigin, actor->origin);

    // set the next desired position
    Vec_Add(position, actor->origin, actor->velocity);
    friction = FRICTION_GROUND;

    // hit surface and update position/velocity
    if(actor->plane && actor->terriantype != TT_NOCLIP)
    {
        plane_t *pl = actor->plane;

        if(pl->flags & CLF_CHECKHEIGHT)
        {
            dist = Plane_GetHeight(pl, position) -
                (actor->meleerange + actor->object.viewheight);

            if(position[1] > dist)
            {
                // hit ceiling
                position[1] = dist;
                actor->velocity[1] = 0;
            }
        }

        // get floor distance
        dist = position[1] - Plane_GetDistance(pl, position);

        if(!Plane_IsAWall(pl) && dist < ONPLANE_EPSILON)
        {
            // lerp player back to the surface
            if(dist < 0)
            {
                vec3_t lerp;

                Vec_Set3(lerp, position[0], position[1] - dist, position[2]);
                Vec_Lerp3(position, 0.125f, position, lerp);
            }

            // surface was hit, kill vertical velocity
            actor->velocity[1] = 0;
        }

        G_GetTerrianType(actor);

        //
        // update gravity
        //
        switch(actor->terriantype)
        {
        case TT_WATER_SHALLOW:
            // normal gravity
            actor->velocity[1] -= GRAVITY_NORMAL;
            break;

        case TT_WATER_SURFACE:
            friction = FRICTION_WATERMASS;
            // stay afloat on the surface
            actor->velocity[1] *= GRAVITY_FLOAT;
            break;

        case TT_WATER_UNDER:
            friction = FRICTION_WATERMASS;
            if(actor->velocity[1] > 0.25f)
            {
                // water mass affects movement
                actor->velocity[1] *= FRICTION_WATERMASS;
            }
            else if(actor->velocity[1] < -1)
            {
                // friction from impact
                actor->velocity[1] *= FRICTION_WTRIMPACT;
            }
            else
            {
                // sink
                actor->velocity[1] -= GRAVITY_WATER;

                if(actor->velocity[1] <= -FRICTION_WATERMASS)
                {
                    actor->velocity[1] = -FRICTION_WATERMASS;
                }
            }
            break;

        case TT_LAVA:
            // slow movement in lava
            friction = FRICTION_LAVA;
            break;

        case TT_NOCLIP:
            actor->velocity[1] = actor->velocity[1] * friction;

            if(actor->velocity[1] < VELOCITY_EPSILON &&
                actor->velocity[1] > -VELOCITY_EPSILON)
            {
                actor->velocity[1] = 0;
            }
            break;

        default:
            // normal gravity
            actor->velocity[1] -= GRAVITY_NORMAL;
            break;
        }
    }

    // de-accelerate velocity
    actor->velocity[0] = actor->velocity[0] * friction;
    actor->velocity[2] = actor->velocity[2] * friction;

    if(actor->velocity[0] < VELOCITY_EPSILON &&
        actor->velocity[0] > -VELOCITY_EPSILON)
    {
        actor->velocity[0] = 0;
    }

    if(actor->velocity[2] < VELOCITY_EPSILON &&
        actor->velocity[2] > -VELOCITY_EPSILON)
    {
        actor->velocity[2] = 0;
    }

    // lerp actor to updated position
    Vec_Lerp3(actor->origin, 1, actor->origin, position);
    G_CheckObjectStep(actor);
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

