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
// DESCRIPTION: Collision detection
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "game.h"
#include "actor.h"
#include "level.h"

#define TRYMOVE_COUNT           4
#define ONESIDED_FLOOR_DISTMAX  1.024f
#define EPSILON_FLOOR           0.975f
#define EPSILON_MOVE            0.1f
#define STEPHEIGHT              32

typedef enum
{
    TRT_NOHIT       = 0,
    TRT_SLOPE       = 1,
    TRT_STEEPSLOPE  = 2,
    TRT_OUTEREDGE   = 3,
    TRT_OBJECT      = 4
} tracetype_e;

typedef struct
{
    vec3_t      start;
    vec3_t      end;
    plane_t     *pl;
    plane_t     *hitpl;
    vec3_t      normal;
    vec3_t      hit;
    float       frac;
    float       sidedist;
    tracetype_e type;
} trace_t;

//
// G_FindClosestPlane
//

plane_t *G_FindClosestPlane(vec3_t coord)
{
    unsigned int i;
    float dist;
    float curdist;
    plane_t *plane;
    kbool ok;

    // VERY TEMP

    ok = false;
    curdist = 0;
    plane = NULL;

    for(i = 0; i < g_currentmap->numplanes; i++)
    {
        plane_t *p;

        p = &g_currentmap->planes[i];

        if(Plane_PointInRange(p, coord[0], coord[2]))
        {
            dist = coord[1] - Plane_GetDistance(p, coord);

            if(p->flags & CLF_ONESIDED && dist < -16)
            {
                continue;
            }

            if(dist < 0)
            {
                dist = -dist;
            }

            if(ok)
            {
                if(dist < curdist)
                {
                    curdist = dist;
                    plane = p;
                }
            }
            else
            {
                plane = p;
                curdist = dist;
                ok = true;
            }
        }
    }

    return plane;
}

//
// G_ClipVelocity
//

static void G_ClipVelocity(vec3_t out, vec3_t velocity, vec3_t normal, float fudge)
{
    float d;
    vec3_t n;

    d = Vec_Dot(velocity, normal) * fudge;
    Vec_Scale(n, normal, d);
    Vec_Sub(out, velocity, n);
    d = Vec_Unit3(out);

    if(d != 0)
    {
        Vec_Scale(out, out, Vec_Unit3(velocity) / d);
    }
}

//
// G_TraceObject
//

static kbool G_TraceObject(trace_t *trace, vec3_t objpos, float radius)
{
    vec3_t dir;
    float x;
    float z;
    float vd;
    float d;

    Vec_Sub(dir, trace->end, trace->start);
    vd = Vec_Unit3(dir);
    Vec_Normalize3(dir);

    x = objpos[0] - trace->start[0];
    z = objpos[2] - trace->start[2];
    d = dir[0] * x + dir[2] * z;

    if(d >= 0 && vd != 0)
    {
        float dx;
        float dz;
        float ld;
        float len;

        vd = 1.0f / vd;

        dx = vd * dir[0];
        dz = vd * dir[2];
        ld = dx * x + dz * z;

        x = x - ld * dx;
        z = z - ld * dz;

        len = radius * radius - (x * x + z * z);

        if(len > 0)
        {
            vec3_t end;
            vec3_t lerp;

            trace->frac = ld - (float)sqrt(len) * vd;

            Vec_Add(end, trace->start, dir);
            Vec_Lerp3(lerp, trace->frac, trace->start, end);
            Vec_Set3(trace->normal, lerp[0] - objpos[0], 0, lerp[2] - objpos[2]);
            Vec_Normalize3(trace->normal);

            return true;
        }
    }

    return false;
}

//
// G_CheckObjects
//

static kbool G_CheckObjects(trace_t *trace, plane_t *plane)
{
    object_t *obj;
    sector_t *sector;

    if(plane == NULL)
    {
        return false;
    }

    sector = &g_currentmap->sectors[plane - g_currentmap->planes];

    for(obj = sector->blocklist.next; obj != &sector->blocklist; obj = obj->next)
    {
        if(G_TraceObject(trace, obj->origin, obj->width))
        {
            /*if(trace->end[1] > obj->origin[1] + obj->height)
            {
                continue;
            }*/

            trace->type = TRT_OBJECT;
            return true;
        }
    }

    return false;
}

//
// G_TracePlane
//

static kbool G_TracePlane(trace_t *trace, plane_t *pl)
{
    float d;
    float dstart;
    float dend;
    
    d       = Vec_Dot(pl->points[0], pl->normal);
    dstart  = Vec_Dot(trace->start, pl->normal) - d;
    dend    = Vec_Dot(trace->end, pl->normal) - d;

    if(pl->flags & CLF_ONESIDED && !Plane_IsAWall(pl))
    {
        if(trace->start[1] -
            Plane_GetDistance(pl, trace->start) < -ONESIDED_FLOOR_DISTMAX)
        {
            return false;
        }
    }
    else if(!Plane_IsAWall(pl))
    {
        if(pl->normal[1] >= EPSILON_FLOOR)
        {
            return false;
        }
    }

    if(dstart > 0 && dend >= dstart)
    {
        return false;
    }

    if(dend <= 0 && dend < dstart)
    {
        trace->type = Plane_IsAWall(pl) ? TRT_STEEPSLOPE : TRT_SLOPE;
        trace->frac = (dstart - 0.03125f) / (dstart - dend);
        trace->hitpl = pl;

        Vec_Copy3(trace->normal, pl->normal);
        return true;
    }

    return false;
}

//
// G_CheckEdgeSide
//

static kbool G_CheckEdgeSide(trace_t *trace, vec3_t vp1, vec3_t vp2)
{
    float x;
    float z;
    float dx;
    float dz;
    float d;

    x = vp1[0] - vp2[0];
    z = vp2[2] - vp1[2];

    d = z * (trace->end[0] - trace->start[0]) +
        x * (trace->end[2] - trace->start[2]);

    if(d < 0)
    {
        dx = vp1[0] - trace->end[0];
        dz = vp1[2] - trace->end[2];

        d = (dz * x + dx * z) / d;

        if(d < trace->sidedist)
        {
            trace->sidedist = d;
            return true;
        }
    }

    return false;
}

//
// G_TraceEdge
//

static void G_TraceEdge(trace_t *trace, vec3_t vp1, vec3_t vp2)
{
    float x;
    float z;

    x = vp1[0] - vp2[0];
    z = vp2[2] - vp1[2];

    Vec_Set3(trace->normal, z, 0, x);
    Vec_Normalize3(trace->normal);

    trace->type = TRT_OUTEREDGE;
}

//
// G_PathTraverse
//

void G_PathTraverse(plane_t *plane, trace_t *trace)
{
    int i;
    int point;
    plane_t *pl;

    if(plane == NULL)
    {
        return;
    }

    if(G_CheckObjects(trace, plane))
    {
        return;
    }

    trace->pl = plane;

    pl = NULL;
    point = 0;
    trace->sidedist = 0;

    for(i = 0; i < 3; i++)
    {
        vec3_t vp1;
        vec3_t vp2;

        if(G_CheckObjects(trace, plane->link[i]))
        {
            return;
        }

        Vec_Copy3(vp1, plane->points[i]);
        Vec_Copy3(vp2, plane->points[(i + 1) % 3]);

        if(G_CheckEdgeSide(trace, vp1, vp2))
        {
            pl = plane->link[i];
            point = i;

            trace->frac = trace->sidedist;

            if(pl == NULL)
            {
                G_TraceEdge(trace, vp1, vp2);
                return;
            }
        }
    }

    if(pl)
    {
        if(Plane_IsAWall(pl))
        {
            float y = trace->start[1] + 1.024f;

            if( y <= pl->points[0][1]   ||
                y <= pl->points[1][1]   ||
                y <= pl->points[2][1])
            {
                if(G_TracePlane(trace, pl))
                {
                    return;
                }
            }
        }

        trace->type = TRT_NOHIT;

        G_PathTraverse(pl, trace);
    }
}

//
// G_Trace
//
//

trace_t G_Trace(vec3_t start, vec3_t end, plane_t *plane)
{
    trace_t trace;

    Vec_Copy3(trace.start, start);
    Vec_Copy3(trace.end, end);
    Vec_Set3(trace.normal, 0, 0, 0);

    trace.pl        = plane;
    trace.hitpl     = NULL;
    trace.frac      = 1;
    trace.sidedist  = 0;
    trace.type      = TRT_NOHIT;

    // try to trace something as early as possible
    if(!G_TracePlane(&trace, trace.pl))
    {
        int i;

        // look for edges in the initial plane that can be collided with
        for(i = 0; i < 3; i++)
        {
            vec3_t vp1;
            vec3_t vp2;

            if(plane->link[i] != NULL)
            {
                continue;
            }

            Vec_Copy3(vp1, plane->points[i]);
            Vec_Copy3(vp2, plane->points[(i + 1) % 3]);

            if(G_CheckEdgeSide(&trace, vp1, vp2))
            {
                G_TraceEdge(&trace, vp1, vp2);
                break;
            }
        }

        // start traversing into other planes if we couldn't trace anything
        if(trace.type == TRT_NOHIT)
        {
            G_PathTraverse(plane, &trace);
        }
    }

    Vec_Lerp3(trace.hit, trace.frac, start, end);

    return trace;
}

//
// G_GroundMove
//
// Trace against surrounding planes and slide
// against it if needed, clipping velocity
// along the way
//

void G_GroundMove(actor_t *actor)
{
    if(actor->plane == NULL ||
        (actor->plane &&
        !Plane_PointInRange(actor->plane,
        actor->origin[0], actor->origin[2])))
    {
        // if player is in the void or current plane isn't
        // in range then scan through all planes and find
        // the cloest one to the actor
        actor->plane = G_FindClosestPlane(actor->origin);
    }

    if(actor->plane != NULL)
    {
        trace_t trace;
        vec3_t start;
        vec3_t end;
        vec3_t vel;
        int i;

        // set start point
        Vec_Copy3(start, actor->origin);
        Vec_Copy3(vel, actor->velocity);

        for(i = 0; i < TRYMOVE_COUNT; i++)
        {
            float tmpy;

            // set end point
            Vec_Add(end, start, vel);

            // get trace results
            trace = G_Trace(start, end, actor->plane);

            actor->plane = trace.pl;

            if(trace.type == TRT_NOHIT)
            {
                // went the entire distance
                break;
            }

            // slide against the hit surface. ignore Y-velocity if on a steep slope
            tmpy = vel[1];
            G_ClipVelocity(vel, vel, trace.normal, 1.01f);
            if(trace.type == TRT_STEEPSLOPE && vel[1] > tmpy)
            {
                vel[1] = tmpy;
            }

            // handle vertical sliding if on a steep slope
            if(Plane_IsAWall(actor->plane) &&
                (actor->origin[1] -
                Plane_GetDistance(actor->plane, actor->origin)) <= 0)
            {
                vec3_t push;
                vec3_t n;

                Vec_Copy3(n, actor->plane->normal);

                // negate y-normal so its facing downward
                n[1] = -n[1];

                // scale the normal by the magnitude of velocity and
                // the steepness of the slope
                Vec_Scale(push, n, Vec_Unit3(vel) * actor->plane->normal[1]);

                // apply to velocity
                Vec_Add(vel, vel, push);
            }

            // force a deadstop if clipped velocity is against
            // the original velocity
            if(Vec_Dot(vel, actor->velocity) <= 0)
            {
                actor->velocity[0] = 0;
                actor->velocity[2] = 0;
                break;
            }

            // update velocity and try another move
            Vec_Copy3(actor->velocity, vel);
        }
    }
}
