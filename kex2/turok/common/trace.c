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
// DESCRIPTION: Ray Tracing System
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "actor.h"

#define EPSILON_FLOOR 0.5f

//
// Trace_Object
//
// Line-circle intersection test on an object
//

static kbool Trace_Object(trace_t *trace, vec3_t objpos, float radius)
{
    vec3_t dir;
    vec3_t tdir;
    vec3_t ndir;
    float vu;
    float px;
    float pz;
    float cx;
    float cz;
    float vd;
    float d;
    float icv1;
    float icv2;
    float len;

    Vec_Sub(dir, trace->end, trace->start);
    Vec_Copy3(tdir, dir);
    Vec_Normalize3(dir);
    Vec_Copy3(ndir, dir);

    vu = Vec_Unit3(tdir);

    cx = objpos[0] - trace->start[0];
    cz = objpos[2] - trace->start[2];
    icv1 = tdir[0] * tdir[0] + tdir[2] * tdir[2];
    icv2 = cx * tdir[0] + cz * tdir[2];

    if(icv1 == 0)
        return false;

    if(icv2 < 0)
        return false;

    vd = icv2 / icv1;

    if(vd < 0) vd = 0;
    if(vd > 1) vd = 1;

    px = ((tdir[0] * vd) + trace->start[0]) - objpos[0];
    pz = ((tdir[2] * vd) + trace->start[2]) - objpos[2];

    len = radius * radius - (px * px + pz * pz);

    if(Vec_Length2(trace->start, objpos) < radius)
        len = 1;

    if(len > 0)
    {
        vec3_t lerp;
        vec3_t n;
        float f;

        f = (px * dir[0] + pz * dir[2]) - (float)sqrt(len) * vd;

        Vec_Lerp3(lerp, f, trace->start, objpos);
        Vec_Set3(n,
            lerp[0] - trace->end[0],
            0,
            lerp[2] - trace->end[2]);

        Vec_Sub(dir, objpos, trace->start);
        Vec_Normalize3(dir);
        d = Vec_Dot(n, dir);

        if(d != 0)
        {
            trace->frac = (px * dir[0] + pz * dir[2]) / d;

            Vec_Normalize3(n);
            Vec_Copy3(trace->normal, n);

            // TODO
            // get the intersect vector for bullet shots
            if(trace->bFullTrace)
                Vec_Lerp3(trace->hitvec, (1 + trace->frac),
                trace->start, trace->end);

            return true;
        }
    }

    return false;
}

//
// Trace_Objects
//
// Scans through planes for linked objects and test
// collision against them
//

static kbool Trace_Objects(trace_t *trace)
{
    unsigned int i;
    vec3_t pos;
    gActor_t *rover;

    Vec_Lerp3(pos, trace->frac, trace->end, trace->start);

    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *grid = &gLevel.gridBounds[i];

        if((pos[0] > grid->minx && pos[0] < grid->maxx) &&
            (pos[2] > grid->minz && pos[2] < grid->maxz))
        {
            unsigned int j;

            for(j = 0; j < grid->numStatics; j++)
            {
                gActor_t *actor = &grid->statics[j];

                if(!actor->bCollision && !actor->bTouch)
                    continue;

                if(actor->bTouch && trace->actor &&
                    trace->actor->components)
                {
                    if(Vec_Length3(pos, actor->origin) < trace->width)
                        Actor_OnTouchEvent(actor, trace->actor);
                }

                if(actor->bCollision)
                {
                    if(pos[1] > actor->origin[1] + actor->height ||
                        pos[1] + trace->offset < actor->origin[1])
                    {
                        continue;
                    }

                    if(Trace_Object(trace, actor->origin, actor->radius))
                    {
                        trace->type = TRT_OBJECT;
                        trace->hitActor = actor;
                        return true;
                    }
                }
            }
        }
    }

    // TODO
    for(rover = gLevel.actorRoot.next; rover != &gLevel.actorRoot; rover = rover->next)
    {
        gActor_t *actor = rover;

        if(actor == trace->actor || actor == trace->source)
            continue;

        if(trace->actor && trace->actor->owner == actor)
            continue;

        if(actor->bTouch && trace->actor &&
            trace->actor->components)
        {
            if(Vec_Length3(pos, actor->origin) < trace->width)
                Actor_OnTouchEvent(actor, trace->actor);
        }

        if(actor->bCollision)
        {
            if(pos[1] > actor->origin[1] + actor->height ||
                pos[1] + trace->offset < actor->origin[1])
            {
                continue;
            }

            if(Trace_Object(trace, actor->origin, actor->radius))
            {
                trace->type = TRT_OBJECT;
                trace->hitActor = actor;
                return true;
            }
        }
    }

    return false;
}

//
// Trace_CheckPlaneHeight
//

static kbool Trace_CheckPlaneHeight(trace_t *trace, plane_t *pl)
{
    if(Plane_IsAWall(pl))
    {
        float y = trace->end[1] + 16.384f;

        if( y > pl->points[0][1] &&
            y > pl->points[1][1] &&
            y > pl->points[2][1])
        {
            return false;
        }
    }

    return true;
}

//
// Trace_GetEdgeIntersect
//

static void Trace_GetEdgeIntersect(trace_t *trace, vec3_t vp1, vec3_t vp2)
{
    float x = vp1[0] - vp2[0];
    float z = vp2[2] - vp1[2];
    float d;
    vec3_t normal;
    vec3_t dir;
    vec3_t spot;

    Vec_Set3(normal, z, 0, x);
    Vec_Normalize3(normal);
    Vec_Sub(dir, trace->end, trace->start);
    Vec_Normalize3(dir);

    d = Vec_Dot(normal, dir);

    if(d != 0)
    {
        Vec_Scale(spot, dir, Vec_Length3(trace->end, trace->start) -
            (Vec_Dot(trace->end, normal) - Vec_Dot(vp1, normal)) / d);

        Vec_Add(trace->hitvec, trace->start, spot);
    }
}

//
// Trace_GetPlaneIntersect
//

static kbool Trace_GetPlaneIntersect(trace_t *trace, plane_t *plane)
{
    float d;
    int i;

    if(plane == NULL)
        return false;

    for(i = 0; i < 3; i++)
    {
        d = Vec_Dot(trace->end, plane->normal) -
            Vec_Dot(plane->points[i], plane->normal);

        if(d < 0)
        {
            vec3_t dir;
            vec3_t spot;
            float vd;
        
            Vec_Sub(dir, trace->end, trace->start);
            Vec_Normalize3(dir);

            vd = Vec_Dot(plane->normal, dir);

            if(vd != 0)
            {
                Vec_Scale(spot, dir,
                    Vec_Length3(trace->end, trace->start) - d / vd);
                Vec_Add(trace->hitvec, trace->start, spot);
                return true;
            }
        }
    }

    return false;
}

//
// Trace_Plane
//
// Check if the ray intersected with plane
//

static kbool Trace_Plane(trace_t *trace, plane_t *pl)
{
    float d;
    float dstart;
    float dend;

    if(!Trace_CheckPlaneHeight(trace, pl))
        return false;

    if(!Plane_IsAWall(pl))
    {
        // ignore if the plane isn't steep enough
        if(pl->normal[1] > EPSILON_FLOOR)
            return false;
    }

    d       = Vec_Dot(pl->points[0], pl->normal);
    dstart  = Vec_Dot(trace->start, pl->normal) - d;
    dend    = Vec_Dot(trace->end, pl->normal) - d;

    if(dstart > 0 && dend >= dstart)
    {
        // in front of the plane
        return false;
    }

    if(dend <= 0 && dend < dstart)
    {
        // intersected
        trace->type = Plane_IsAWall(pl) ? TRT_WALL : TRT_SLOPE;
        trace->hitpl = pl;

        Vec_Copy3(trace->normal, pl->normal);
        return true;
    }

    return false;
}

//
// Trace_PlaneEdge
//
// Simple line-line intersection test to see if
// ray has crossed the plane's edge
//

static kbool Trace_PlaneEdge(trace_t *trace, vec3_t vp1, vec3_t vp2)
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

        if(d < trace->frac)
        {
            trace->frac = d;

            if(trace->bFullTrace)
                Trace_GetEdgeIntersect(trace, vp1, vp2);
            return true;
        }
    }

    return false;
}

//
// Trace_GetEdgeNormal
//
// Sets up a normal vector for an edge that isn't linked
// into another plane. Basically treat it as a solid wall
//

static void Trace_GetEdgeNormal(trace_t *trace, vec3_t vp1, vec3_t vp2)
{
    float x;
    float z;

    x = vp1[0] - vp2[0];
    z = vp2[2] - vp1[2];

    Vec_Set3(trace->normal, z, 0, x);
    Vec_Normalize3(trace->normal);

    trace->type = TRT_EDGE;
}

//
// Trace_GetPlaneLink
//
// Fetches the next linked plane. If NULL then assume it is
// a solid wall/edge
//

static plane_t *Trace_GetPlaneLink(trace_t *trace, plane_t *p, int point)
{
    vec3_t pos;
    plane_t *link;

    link = p->link[point];

    if(!link)
    {
        // crossed into an edge
        return NULL;
    }

    Vec_Lerp3(pos, trace->frac, trace->end, trace->start);

    if(!(p->flags & CLF_CHECKHEIGHT) &&
        link->flags & CLF_CHECKHEIGHT &&
        Plane_GetHeight(link, pos) < (pos[1] + trace->offset + 30.72f))
    {
        // above ceiling height
        return NULL;
    }

    if(trace->bFullTrace)
        return link;

    if(Plane_IsAWall(p))
    {
        if(!Plane_IsAWall(link))
            return link;

        if(p->flags & CLF_CLIMB &&
            !(link->flags & CLF_CLIMB) &&
            Plane_GetDistance(p, pos) + 1.024f > pos[1])
        {
            return NULL;
        }
    }

    if(Plane_IsAWall(link))
    {
        if(!Plane_IsAWall(p))
        {
            vec3_t dir;
            float dist1 = Plane_GetDistance(link, trace->end);
            float dist2 = Plane_GetDistance(p, trace->start);

            if(dist1 <= dist2)
            {
                if(!Trace_CheckPlaneHeight(trace, link))
                {
                    if(trace->actor && trace->actor->bNoDropOff)
                        return NULL;

                    if(trace->source && trace->source->bNoDropOff)
                        return NULL;
                }

                // able to step off into this plane
                return link;
            }

            // check climbable plane
            if(link->flags & CLF_CLIMB && trace->actor)
            {
                float angle = Plane_GetEdgeYaw(p, point) + M_PI;
                Ang_Clamp(&angle);

                angle = Ang_Diff(angle, trace->actor->angles[0]);

                if(angle < 0)
                    angle = -angle;

                // must be facing the wall
                if(angle >= DEG2RAD(140))
                {
                    trace->type = TRT_INTERACT;
                    trace->hitpl = link;
                    return link;
                }
            }

            Vec_Sub(dir, trace->end, trace->start);

            // special case for planes flagged to block
            // from the front side. these will be treated as
            // solid walls. direction of ray must be facing
            // towards the plane
            if(Trace_CheckPlaneHeight(trace, link) &&
                Plane_IsFacing(link, Ang_VectorToAngle(dir)))
            {
                trace->hitpl = link;
                return NULL;
            }
        }
    }

    // crossed into a floor plane
    return link;
}

//
// Trace_CheckBulletRay
//

static kbool Trace_CheckBulletRay(trace_t *trace, plane_t *plane)
{
    if(!Trace_CheckPlaneHeight(trace, plane))
        return false;

    if(Trace_GetPlaneIntersect(trace, plane))
    {
        kbool ok = true;
        vec3_t oldvec;
        int i;

        Vec_Copy3(oldvec, trace->end);
        Vec_Copy3(trace->end, trace->hitvec);

        trace->frac = 0;

        // check if the bullet can go beyond the plane
        for(i = 0; i < 3; i++)
        {
            vec3_t vp1;
            vec3_t vp2;

            Vec_Copy3(vp1, plane->points[i]);
            Vec_Copy3(vp2, plane->points[(i + 1) % 3]);

            if(Trace_PlaneEdge(trace, vp1, vp2))
            {
                // went over or past the plane
                ok = false;
                break;
            }
        }

        Vec_Copy3(trace->end, oldvec);

        if(ok)
        {
            trace->type = Plane_IsAWall(plane) ? TRT_WALL : TRT_SLOPE;
            trace->hitpl = plane;
            Vec_Copy3(trace->normal, plane->normal);
            return true;
        }
    }

    return false;
}

//
// Trace_TraversePlanes
//

void Trace_TraversePlanes(plane_t *plane, trace_t *trace)
{
    int i;
    plane_t *pl;

    if(plane == NULL)
        return;

    // we've entered a new plane
    trace->pl = plane;

    // bullet-trace detection for floors
    if(!Plane_IsAWall(plane) && trace->bFullTrace)
    {
        if(Trace_CheckBulletRay(trace, plane))
            return;
    }

    pl = NULL;
    trace->frac = 0;

    // check if ray crosses into an edge. if multiple edges
    // have been crossed then get the one closest to the ray
    for(i = 0; i < 3; i++)
    {
        vec3_t vp1;
        vec3_t vp2;

        Vec_Copy3(vp1, plane->points[i]);
        Vec_Copy3(vp2, plane->points[(i + 1) % 3]);

        if(Trace_PlaneEdge(trace, vp1, vp2))
        {
            pl = Trace_GetPlaneLink(trace, plane, i);

            if(pl == NULL)
            {
                // treat it as a solid wall
                Trace_GetEdgeNormal(trace, vp1, vp2);
            }
            else if(trace->type == TRT_INTERACT)
            {
                trace->pl = pl;
            }
        }
    }

    if(pl)
    {
        // check to see if the ray can bump into it
        if(Plane_IsAWall(pl))
        {
            // trace it to see if its valid or not
            if(trace->bFullTrace)
            {
                if(Trace_CheckBulletRay(trace, pl))
                    return;
            }
            else if(Trace_Plane(trace, pl))
                return;
        }

        trace->type = TRT_NOHIT;

        // traverse into next plane
        Trace_TraversePlanes(pl, trace);
    }
}

//
// Trace
//

trace_t Trace(vec3_t start, vec3_t end, plane_t *plane,
              gActor_t *actor, gActor_t *source, kbool bFullTrace)
{
    trace_t trace;

    Vec_Copy3(trace.start, start);
    Vec_Copy3(trace.end, end);
    Vec_Copy3(trace.hitvec, start);
    Vec_Set3(trace.normal, 0, 0, 0);

    trace.pl            = plane;
    trace.hitpl         = NULL;
    trace.hitActor      = NULL;
    trace.frac          = 0;
    trace.type          = TRT_NOHIT;
    trace.width         = actor ? actor->radius : 32.0f;
    trace.offset        = actor ? actor->centerHeight : 0;
    trace.bFullTrace    = bFullTrace;
    trace.actor         = actor;
    trace.source        = source;
    trace.dist          = 0;

    if(!Trace_Objects(&trace))
    {
        if(actor == NULL)
            Trace_TraversePlanes(plane, &trace);
        else
        {
            // try to trace something as early as possible
            if(!Trace_Plane(&trace, trace.pl))
            {
                int i;

                // look for edges in the initial plane that can be collided with
                for(i = 0; i < 3; i++)
                {
                    vec3_t vp1;
                    vec3_t vp2;

                    if(plane->link[i] != NULL)
                        continue;

                    Vec_Copy3(vp1, plane->points[i]);
                    Vec_Copy3(vp2, plane->points[(i + 1) % 3]);

                    // check to see if an edge was crossed
                    if(Trace_PlaneEdge(&trace, vp1, vp2))
                        Trace_GetEdgeNormal(&trace, vp1, vp2);
                }

                // start traversing into other planes if we couldn't trace anything
                if(trace.type == TRT_NOHIT)
                    Trace_TraversePlanes(plane, &trace);
            }
        }
    }

    return trace;
}

