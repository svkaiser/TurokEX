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
// G_ApplyGravity
//

void G_ApplyGravity(vec3_t origin, vec3_t velocity, plane_t *plane,
                    float mass, float timeDelta)
{
    if(plane == NULL)
        return;

    if(origin[1] - Plane_GetDistance(plane, origin) > 0.01f)
        velocity[1] -= (mass * timeDelta);
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
// G_ApplyBounceVelocity
//

void G_ApplyBounceVelocity(vec3_t velocity, vec3_t reflection, float amount)
{
    float d = Vec_Unit3(velocity);
    float bounce = 1.0f;

    if(d >= 1.05f)
    {
        bounce = (1 + amount);

        if(d < 16.8f && amount < 1.0f)
            bounce = 0.2f;
    }

    G_ClipVelocity(velocity, velocity, reflection, bounce);
}

//
// G_TryMove
//

kbool G_TryMove(gActor_t *source, vec3_t origin, vec3_t dest, plane_t **plane)
{
    plane_t *newPlane = NULL;
    trace_t trace;

    trace = Trace(origin, dest, *plane, NULL, source, true);
    *plane = trace.pl;

    if(trace.type != TRT_NOHIT)
        Vec_Copy3(dest, trace.hitvec);

    return Plane_PointInRange(*plane, dest[0], dest[2]);
}

//
// G_ClipMovement
//
// Trace against surrounding planes and slide
// against it if needed, clipping velocity
// along the way
//

kbool G_ClipMovement(vec3_t origin, vec3_t velocity, float time, plane_t **plane,
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
    kbool hitOk;

    if(*plane == NULL)
    {
        if(t)
        {
            t->type = TRT_STUCK;
            t->hitpl = NULL;
            t->pl = NULL;
            Vec_Copy3(t->hitvec, origin);
            Vec_Copy3(t->normal, end);
            Vec_Normalize3(t->normal);
        }

        return true;
    }

    hitOk = false;

    if(t) memset(t, 0, sizeof(trace_t));

    // set start point
    Vec_Copy3(start, origin);
    Vec_Copy3(vel, velocity);
    moves = 0;

    for(i = 0; i < TRYMOVE_COUNT; i++)
    {
        // set end point
        //Vec_Add(end, start, vel);
        end[0] = start[0] + (vel[0] * time);
        end[1] = start[1] + (vel[1] * time);
        end[2] = start[2] + (vel[2] * time);

        // get trace results
        trace = Trace(start, end, *plane, actor, NULL, false);

        *plane = trace.pl;

        if(trace.type == TRT_NOHIT)
        {
            // went the entire distance
            break;
        }

        if(t)
        {
            if(trace.hitpl)
                t->hitpl = trace.hitpl;

            if(trace.hitActor)
                t->hitActor = trace.hitActor;

            Vec_Copy3(t->hitvec, end);

            if(trace.type != TRT_NOHIT)
            {
                Vec_Copy3(t->normal, trace.normal);
                t->type = trace.type;
                Vec_Lerp3(t->hitvec, (1 + trace.frac), start, end);
            }

            if(trace.frac != 0)
                t->frac = trace.frac;

            t->pl = trace.pl;
        }

        hitOk = true;

        Vec_Copy3(normals[moves++], trace.normal);

        // try all interacted normals
        for(hits = 0; hits < moves; hits++)
        {
            if(Vec_Dot(vel, normals[hits]) < 0)
            {
                int j;
                int k;
                float b;

                if(trace.type != TRT_OBJECT)
                    b = 1 - (1 + trace.frac) + 0.01f;
                else
                    b = 1;

                // slide along this plane
                G_ClipVelocity(vel, vel, normals[hits], b);

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
                                return true;
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

    Vec_Scale(vel, velocity, time);

    // stop all movement if not in a valid plane
    Vec_Add(end, origin, vel);
    if(!Plane_PointInRange(*plane, end[0], end[2]))
    {
        if(t)
        {
            t->type = TRT_STUCK;
            t->hitpl = *plane;
            t->pl = *plane;
            Vec_Copy3(t->hitvec, origin);
            Vec_Copy3(t->normal, end);
            Vec_Normalize3(t->normal);
        }

        hitOk = true;
        velocity[0] = 0;
        velocity[2] = 0;
    }

    // advance position
    Vec_Add(origin, origin, vel);

    // clip origin/velocity for ceiling and floors
    if(actor)
    {
        float dist;

        // test the floor and adjust height
        dist = origin[1] - Plane_GetDistance(*plane, origin);
        if(dist <= 0.512f)
        {
            if(!((*plane)->flags & CLF_CLIMB))
                origin[1] = origin[1] - dist;
            else
            {
                float d;

                d = Vec_Dot(origin, (*plane)->normal) -
                    Vec_Dot((*plane)->points[0], (*plane)->normal);

                if(d > 0)
                {
                    vec3_t dir;

                    Vec_Copy3(dir, origin);
                    Vec_Normalize3(dir);
                    Vec_Scale(dir, dir, d);
                    Vec_Sub(origin, origin, dir);
                }
            }

            if(t)
            {
                float d;

                t->hitpl = *plane;
                t->pl = *plane;
                Vec_Copy3(t->hitvec, start);
                Vec_Copy3(t->normal, (*plane)->normal);
                t->type = TRT_SLOPE;

                d = Vec_Dot(start, (*plane)->normal) -
                    Vec_Dot((*plane)->points[0], (*plane)->normal);

                if(d > 0)
                {
                    vec3_t dir;

                    Vec_Copy3(dir, velocity);
                    Vec_Normalize3(dir);
                    Vec_Scale(dir, dir, d - 3.42f);
                    Vec_Add(t->hitvec, start, dir);
                }
            }

            hitOk = true;
            G_ClipVelocity(velocity, velocity, (*plane)->normal, 1);

            if(velocity[1] > 0)
                velocity[1] = 0;
        }

        // test the ceiling and adjust height
        if((*plane)->flags & CLF_CHECKHEIGHT)
        {
            float offset = actor->centerHeight + actor->viewHeight;

            dist = Plane_GetHeight(*plane, origin);
            if((dist - (origin[1] + offset) < 1.024f))
            {
                origin[1] = dist - (1.024f + offset);

                if(t)
                {
                    t->hitpl = *plane;
                    t->pl = *plane;
                    Vec_Copy3(t->hitvec, start);
                    Vec_Copy3(t->normal, (*plane)->ceilingNormal);
                    t->type = TRT_SLOPE;
                }

                hitOk = true;
                G_ClipVelocity(velocity, velocity, (*plane)->ceilingNormal, 1);
            }
        }
    }

    return hitOk;
}
