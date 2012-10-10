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

static actor_t *g_currentactor;

//
// G_LinkActor
//

void G_LinkActor(actor_t *actor)
{
    //g_currentmap->actorlist.prev->next = actor;
    //actor->next = &g_currentmap->actorlist;
    //actor->prev = g_currentmap->actorlist.prev;
    //g_currentmap->actorlist.prev = actor;
}

//
// G_UnlinkActor
//

void G_UnlinkActor(actor_t* actor)
{
    /* Remove from main actor list */
    //actor_t* next = g_currentactor->next;

    /* Note that g_currentactor is guaranteed to point to us,
    * and since we're freeing our memory, we had better change that. So
    * point it to actor->prev, so the iterator will correctly move on to
    * actor->prev->next = actor->next */
    //(next->prev = g_currentactor = actor->prev)->next = next;
}

//
// G_SpawnActor
//

actor_t *G_SpawnActor(float x, float y, float z,
                      float yaw, const char *model, short type)
{
    actor_t *actor;

    if(g_currentmap == NULL)
    {
        return NULL;
    }

    actor = (actor_t*)Z_Calloc(sizeof(*actor), PU_ACTOR, NULL);

    /*Vec_Set3(actor->origin, x, y, z);
    Vec_Set3(actor->prevorigin, x, y, z);
    Vec_Set3(actor->scale, 0.5f, 0.5f, 0.5f);
    Vec_SetQuaternion(actor->rotation, yaw, 1, 0, 0);

    actor->yaw          = yaw;
    actor->pitch        = 0;
    actor->type         = type;
    actor->health       = 100;
    actor->width        = 44.0f;
    actor->height       = 44.0f;
    actor->meleerange   = 0;
    actor->target       = NULL;
    actor->model        = Mdl_Load(model);
    actor->mapactor_id  = -1;
    actor->svclient_id  = -1;

    G_LinkActor(actor);*/

    return actor;
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

    if(Plane_IsAWall(actor->plane))
    {
        return false;
    }

    if((actor->origin[1] + actor->velocity[1]) -
        Plane_GetDistance(actor->plane, actor->origin) < 2.0f)
    {
        return true;
    }

    if(actor->velocity[1] < 0.0f && actor->velocity[1] > -16.0f)
    {
        return true;
    }

    return false;
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

    // TEMP
    if(g_currentmap == NULL)
    {
        return;
    }

    // normal movement; clip velocity before we update it
    G_ActorGroundMove(actor);

    // save previous origin first
    Vec_Copy3(actor->prevorigin, actor->origin);

    // set the next desired position
    Vec_Add(position, actor->origin, actor->velocity);

    if(actor->plane)
    {
        plane_t *pl = actor->plane;

        if(pl->flags & CLF_CHECKHEIGHT)
        {
            dist = Plane_GetHeight(pl, position) /*- actor->object.height*/ - 30.72f;

            if(position[1] > dist)
            {
                position[1] = dist;
                actor->velocity[1] = 0;
            }
        }

        // get floor distance
        dist = position[1] - Plane_GetDistance(pl, position);

        if(!(pl->flags & CLF_ONESIDED))
        {
            if(!Plane_IsAWall(pl) && dist < 1)
            {
                // lerp player back to the surface
                if(dist < -0.1f)
                {
                    vec3_t lerp;

                    Vec_Set3(lerp, position[0], position[1] - dist, position[2]);
                    Vec_Lerp3(position, 0.125f, position, lerp);
                }
                else
                {
                    // snap the position to the surface
                    position[1] = position[1] - dist;
                }

                // surface was hit, kill vertical velocity
                actor->velocity[1] = 0;
            }
            else if(Plane_IsAWall(pl) && dist < -1)
            {
                // freeze vertical velocity if under a steep slope
                actor->velocity[1] = 0;
            }
            else
            {
                // nothing was hit, continue freefall
                actor->velocity[1] -= 0.5f;
            }
        }
        else
        {
            // for one-sided surfaces only
            // snap position to surface if within a certain range
            if(dist < 0 && dist > -16)
            {
                position[1] = position[1] - dist;
            }
            else
            {
                // freefall
                actor->velocity[1] -= 0.5f;
            }
        }
    }

    // de-accelerate velocity
    actor->velocity[0] = actor->velocity[0] * 0.5f;
    actor->velocity[2] = actor->velocity[2] * 0.5f;

    // lerp actor to updated position
    Vec_Lerp3(actor->origin, 1, actor->origin, position);
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

//
// G_RotateActorToPlane
//

void G_RotateActorToPlane(vec4_t rot, actor_t *actor)
{
    vec3_t up;
    vec3_t cross;
    vec4_t lerp;
    vec4_t dir;
    vec3_t n;
    float an;
    float s;
    float c;
    float slope;

    if(actor->plane == NULL ||
        (actor->flags & AF_NOALIGNPITCH || Plane_IsAWall(actor->plane)))
    {
        Vec_Set4(rot, 0, 0, 0, 1);
        return;
    }
    else if(actor->svclient_id == -1)
    {
        Plane_GetRotation(rot, actor->plane);
        return;
    }

    Plane_GetNormal(n, actor->plane);
    Vec_Set3(up, 0, 1, 0);
    Vec_Normalize3(n);
    Vec_Cross(cross, up, n);
    Vec_Normalize3(cross);

    an = (float)acos(up[0] * n[0] + up[1] * n[1] + up[2] * n[2]);
    if(an > (SLOPE_THRESHOLD * M_RAD))
    {
        an = (SLOPE_THRESHOLD * M_RAD);
    }

    s = (float)sin(an * (1.0f - SLOPE_THRESHOLD) * 0.5f);
    c = (float)cos(an * (1.0f - SLOPE_THRESHOLD) * 0.5f);

    dir[0] = cross[0] * s;
    dir[1] = cross[1] * s;
    dir[2] = cross[2] * s;
    dir[3] = c;

    slope = Plane_GetSlope(actor->plane,
        actor->origin[0],
        actor->origin[2],
        actor->origin[0] - (float)sin(actor->yaw),
        actor->origin[2] - (float)cos(actor->yaw));

    if(!Plane_IsAWall(actor->plane))
    {
        if(slope <= SLOPE_THRESHOLD)
        {
            slope = slope * (1.0f - SLOPE_THRESHOLD);
        }
        else
        {
            slope = SLOPE_THRESHOLD * (1.0f - SLOPE_THRESHOLD);
        }
    }

    s = (float)sin(slope * 0.5f);
    c = (float)cos(slope * 0.5f);

    lerp[0] = (float)cos(actor->yaw) * s;
    lerp[1] = 0;
    lerp[2] = -(float)sin(actor->yaw) * s;
    lerp[3] = c;

    Vec_Slerp(rot, 0.5f, lerp, dir);
}

