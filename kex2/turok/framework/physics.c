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
// DESCRIPTION: Collision detection / Physics behavior
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "game.h"
#include "actor.h"
#include "level.h"

#define TRYMOVE_COUNT   3

//
// G_ClipVelocity
//

void G_ClipVelocity(vec3_t out, vec3_t velocity, vec3_t normal, float fudge)
{
    float d;
    vec3_t n;

    d = Vec_Dot(velocity, normal) * fudge;
    Vec_Scale(n, normal, d);
    Vec_Sub(out, velocity, n);
    d = Vec_Unit3(out);

    if(d != 0)
        Vec_Scale(out, out, Vec_Unit3(velocity) / d);
}

//
// G_SlideOnCrease
//

static void G_SlideOnCrease(vec3_t out, vec3_t velocity, vec3_t v1, vec3_t v2)
{
    vec3_t dir;

    Vec_Cross(dir, v1, v2);
    Vec_Normalize3(dir);
    Vec_Scale(out, dir, Vec_Dot(velocity, dir));
}

//
// G_ApplyFriction
//

void G_ApplyFriction(vec3_t velocity, float friction, kbool effectY)
{
    float speed;

    speed = Vec_Unit3(velocity);

    if(speed < VELOCITY_EPSILON)
    {
        velocity[0] = 0;
        velocity[2] = 0;
    }
    else
    {
        float clipspeed = speed - (speed * friction);

        if(clipspeed < 0) clipspeed = 0;
        clipspeed /= speed;

        // de-accelerate velocity
        velocity[0] = velocity[0] * clipspeed;
        velocity[2] = velocity[2] * clipspeed;

        if(effectY)
            velocity[1] = velocity[1] * clipspeed;
    }
}

//
// G_ClipMovement
//
// Trace against surrounding planes and slide
// against it if needed, clipping velocity
// along the way
//

void G_ClipMovement(vec3_t origin, vec3_t velocity, plane_t **plane,
                    gActor_t *actor, trace_t *t)
{
    trace_t trace;
    vec3_t start;
    vec3_t end;
    vec3_t vel;
    vec3_t normals[TRYMOVE_COUNT];
    int moves;
    int i;
    int hits;

    if(*plane == NULL)
        return;

    if(t) memset(t, 0, sizeof(trace_t));

    // set start point
    Vec_Copy3(start, origin);
    Vec_Copy3(vel, velocity);
    moves = 0;

    for(i = 0; i < TRYMOVE_COUNT; i++)
    {
        // set end point
        Vec_Add(end, start, vel);

        // get trace results
        trace = Trace(start, end, *plane, actor, NULL, false);

        if(t) *t = trace;
        *plane = trace.pl;

        if(trace.type == TRT_INTERACT || trace.pl->flags & CLF_CLIMB)
            break;

        if(trace.type == TRT_NOHIT)
        {
            // went the entire distance
            break;
        }

        Vec_Copy3(normals[moves++], trace.normal);

        // try all interacted normals
        for(hits = 0; hits < moves; hits++)
        {
            if(Vec_Dot(vel, normals[hits]) < 0)
            {
                int j;
                int k;

                // slide along this plane
                G_ClipVelocity(vel, vel, normals[hits], 1);

                if(trace.type == TRT_OBJECT)
                    break;

                // try bumping against another plane
                for(j = 0; j < moves; j++)
                {
                    if(j != hits && Vec_Dot(vel, normals[j]) < 0)
                    {
                        // slide along the crease between two planes
                        G_SlideOnCrease(vel, vel,
                            normals[hits], normals[j]);

                        // see if it bumps into a third plane
                        for(k = 0; k < moves; k++)
                        {
                            if(k != j && k != hits &&
                                Vec_Dot(vel, normals[k]) < 0)
                            {
                                // force a dead stop
                                Vec_Set3(velocity, 0, 0, 0);
                                return;
                            }
                        }
                    }
                }
            }
        }

        // force a deadstop if clipped velocity is against
        // the original velocity or if exceeded max amount of
        // attempted moves (don't count against objects hit)
        if(trace.type != TRT_OBJECT)
        {
            if(Vec_Dot(vel, velocity) <= 0 || i == (TRYMOVE_COUNT - 1))
            {
                velocity[0] = 0;
                velocity[2] = 0;
                break;
            }
        }

        // update velocity and try another move
        Vec_Copy3(velocity, vel);
    }

    // stop all movement if not in a valid plane
    Vec_Add(end, origin, velocity);
    if(!Plane_PointInRange(*plane, end[0], end[2]))
    {
        velocity[0] = 0;
        velocity[2] = 0;
    }

    Vec_Add(origin, origin, velocity);

    if(actor)
    {
        float dist;

        dist = origin[1] - Plane_GetDistance(*plane, origin);
        if(dist <= 0.512f)
        {
            origin[1] = origin[1] - dist;
            G_ClipVelocity(velocity, velocity, (*plane)->normal, 1);

            if(velocity[1] > 0)
                velocity[1] = 0;
        }

        if((*plane)->flags & CLF_CHECKHEIGHT)
        {
            float offset = actor->centerHeight + actor->viewHeight;

            dist = Plane_GetHeight(*plane, origin);
            if((dist - (origin[1] + offset) < 1.024f))
            {
                G_ClipVelocity(velocity, velocity, (*plane)->ceilingNormal, 1);
                origin[1] = dist - (1.024f + offset);
            }
        }
    }
}
