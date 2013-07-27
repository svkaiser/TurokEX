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
#include "game.h"

#define EPSILON_FLOOR   0.5f
#define STEPHEIGHT      12.0f

//
// Trace_Object
//
// Line-circle intersection test on an object
//

static kbool Trace_Object(trace_t *trace, vec3_t objpos, float radius)
{
    vec3_t tdir;
    vec3_t odir;

    Vec_Sub(tdir, trace->start, trace->end);
    Vec_Sub(odir, trace->start, objpos);

    if((tdir[0] * odir[0] + tdir[2] * odir[2]) > 0)
    {
        float len = Vec_Unit2(tdir);

        if(len != 0)
        {
            vec3_t ndir;
            vec3_t cDist;
            float rd;
            float cp;

            Vec_Scale(ndir, tdir, 1.0f / len);
            cp = ndir[0] * odir[0] + ndir[2] * odir[2];
            Vec_Scale(ndir, ndir, cp);
            Vec_Sub(cDist, odir, ndir);

            rd = radius * radius - (cDist[0] * cDist[0] + cDist[2] * cDist[2]);

            if(rd > 0)
            {
                float frac = (cp - (float)sqrt(rd)) * (1.0f / len);

                if(frac <= 1.0f && frac < trace->frac)
                {
                    vec3_t lerp;

                    trace->frac = frac;
                    trace->tfrac = trace->frac;

                    Vec_Scale(lerp, tdir, frac);
                    Vec_Sub(trace->hitvec, trace->start, lerp);

                    Vec_Copy3(trace->normal, odir);
                    trace->normal[1] = 0;
                    Vec_Normalize3(trace->normal);
                    return true;
                }
            }
        }
    }

    return false;
}

//
// Trace_TestActor
//

static void Trace_TestActor(trace_t *trace, vec3_t pos, gActor_t *actor, plane_t *plane)
{
    if(actor->bStale)
        return;
    if(!(trace->physics & PF_CLIPSTATICS) && actor->bStatic)
        return;
    if(!(trace->physics & PF_CLIPACTORS) && !actor->bStatic)
        return;
    if(actor == trace->source)
        return;
    if(trace->source && trace->source->owner == actor)
        return;

    if(actor->bCollision || actor->bTouch)
    {
        if(actor->plane >= 0 && Map_IndexToPlane(actor->plane)->area_id != plane->area_id)
            return;

        if(actor->bTouch && trace->source && trace->source->components &&
            trace->physics & PF_TOUCHACTORS)
        {
            bbox_t box;

            Vec_Copy3(box.min, actor->bbox.min);
            Vec_Copy3(box.max, actor->bbox.max);
            Vec_Mult(box.min, box.min, actor->scale);
            Vec_Mult(box.max, box.max, actor->scale);
            Vec_Add(box.min, box.min, actor->origin);
            Vec_Add(box.max, box.max, actor->origin);

            box.min[0] -= trace->offset;
            box.min[1] -= trace->offset;
            box.min[2] -= trace->offset;
            box.max[0] += trace->offset;
            box.max[1] += trace->offset;
            box.max[2] += trace->offset;

            if((pos[0] > box.min[0] && pos[0] < box.max[0]) &&
                (pos[1] > box.min[1] && pos[1] < box.max[1]) &&
                (pos[2] > box.min[2] && pos[2] < box.max[2]))
            {
                Actor_OnTouchEvent(actor, trace->source);
                return;
            }
        }

        if(pos[1] > actor->origin[1] + (actor->height+actor->viewHeight) ||
            pos[1] + trace->offset < actor->origin[1])
        {
            return;
        }

        if(Trace_Object(trace, actor->origin, actor->radius))
        {
            if(actor->bCollision)
            {
                trace->type = TRT_OBJECT;
                trace->hitActor = actor;
            }
        }
    }
}

//
// Trace_Objects
//
// Scans through planes for linked objects and test
// collision against them
//

static void Trace_Objects(trace_t *trace, plane_t *plane)
{
    unsigned int i;
    vec3_t pos;
    gActor_t *rover;

    if(plane == NULL)
        return;

    Vec_Lerp3(pos, trace->tfrac, trace->start, trace->end);

    if(trace->physics & PF_CLIPSTATICS)
    {
        trace->frac = 1;

        for(i = 0; i < gLevel.numGridBounds; i++)
        {
            gridBounds_t *grid = &gLevel.gridBounds[i];

            if((pos[0] > grid->minx && pos[0] < grid->maxx) &&
                (pos[2] > grid->minz && pos[2] < grid->maxz))
            {
                unsigned int j;

                for(j = 0; j < grid->numStatics; j++)
                {
                    int k = 0;

                    Trace_TestActor(trace, pos, &grid->statics[j], plane);

                    // early out for slide movers
                    if(trace->physics & PF_SLIDEMOVE && trace->type == TRT_OBJECT)
                        return;

                    for(k = 0; k < 3; k++)
                    {
                        if(plane->link[k] == NULL)
                            continue;

                        Trace_TestActor(trace, pos,
                            &grid->statics[j], plane->link[k]);
                    }
                }
            }
        }
    }

    if(trace->physics & PF_CLIPACTORS)
    {
        gArea_t *area;

        area = Map_GetArea(plane);
        trace->frac = 1;

        // scan through area for slide movers
        if(trace->physics & PF_SLIDEMOVE)
        {
            for(rover = area->actorRoot.linkNext;
                rover != &area->actorRoot;
                rover = rover->linkNext)
            {
                Trace_TestActor(trace, pos, rover, plane);
             
                // early out for slide movers
                if(trace->type == TRT_OBJECT)
                    return;
            }
        }
        else
        {
            // scan through all actors for full ray-tracing
            for(rover = gLevel.actorRoot.next;
                rover != &gLevel.actorRoot;
                rover = rover->next)
            {
                Trace_TestActor(trace, pos, rover, plane);
            }
        }
    }
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
            trace->tfrac = 1.0f + d;

            Vec_Lerp3(trace->hitvec, trace->tfrac,
                trace->start, trace->end);

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
// Trace_CrossPlane
//
// Cross into a plane link and check for valid clipping
// If NULL then assume it is a solid wall/edge
//

static plane_t *Trace_CrossPlane(trace_t *trace, plane_t *p, int point)
{
    vec3_t pos;
    float ty;
    plane_t *link;

    link = p->link[point];

    if(!link)
    {
        // crossed into an edge that doesn't link
        // to another plane
        return NULL;
    }

    Vec_Lerp3(pos, trace->tfrac, trace->start, trace->end);
    ty = pos[1];

    // add an extra 'lip' to height for slide movers
    if(trace->source && trace->physics & PF_SLIDEMOVE)
        ty = ty + trace->offset + (trace->source->viewHeight * 0.5f);

    // check for ceiling heights
    if(link->flags & CLF_CHECKHEIGHT)
    {
        // check against 'vertical' ceilings
        if(link->ceilingNormal[1] >= -0.5f)
        {
            float cy;

            if(trace->source && trace->physics & PF_SLIDEMOVE)
                cy = ty;
            else
                cy = pos[1];

            if(cy >= link->height[0] ||
                cy >= link->height[1] ||
                cy >= link->height[2])
                return NULL;
        }

        if(!(p->flags & CLF_CHECKHEIGHT) && Plane_GetHeight(link, pos) < ty)
        {
            // above ceiling height
            return NULL;
        }
    }

    // don't cross blocking planes
    if(link->flags & CLF_BLOCK && !(link->flags & CLF_TOGGLE))
    {
        trace->hitpl = link;
        return NULL;
    }

    // don't really need to do additional checks for
    // non-slide movers
    if(!(trace->physics & PF_SLIDEMOVE))
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

    //
    // moving in and out of water
    //

    if(Map_GetArea(p)->flags & AAF_WATER && !(Map_GetArea(link)->flags & AAF_WATER) &&
        trace->physics & PF_NOEXITWATER)
        return NULL;

    if(!(Map_GetArea(p)->flags & AAF_WATER) && Map_GetArea(link)->flags & AAF_WATER &&
        trace->physics & PF_NOENTERWATER)
        return NULL;


    // crossing into a wall or a very steep slope
    if(Plane_IsAWall(link) && !Plane_IsAWall(p))
    {
        vec3_t dir;
        float dist1 = Plane_GetDistance(link, trace->end);
        float dist2 = Plane_GetDistance(p, trace->start);

        // handle steps and drop-offs
        if(dist1 <= dist2)
        {
            float len = (
                link->points[0][1] +
                link->points[1][1] +
                link->points[2][1]) * (1.0f/3.0f);

            if(len < 0)
                len = -len;

            if(!Trace_CheckPlaneHeight(trace, link) &&
                len >= STEPHEIGHT &&
                trace->physics & PF_SLIDEMOVE &&
                !(trace->physics & PF_DROPOFF))
            {
                return NULL;
            }

            // able to step off into this plane
            return link;
        }

        // check climbable plane
        if(link->flags & CLF_CLIMB && trace->source &&
            trace->source->physics & (PF_SLIDEMOVE|PF_CLIMBSURFACES))
        {
            float angle = Plane_GetEdgeYaw(p, point) + M_PI;
            Ang_Clamp(&angle);

            angle = Ang_Diff(angle, trace->source->angles[0]);

            if(angle < 0)
                angle = -angle;

            // must be facing the wall
            if(angle >= DEG2RAD(140))
            {
                trace->type = TRT_CLIMB;
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
    if(!Plane_IsAWall(plane) && !(trace->physics & PF_SLIDEMOVE))
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
            pl = Trace_CrossPlane(trace, plane, i);

            if(pl == NULL)
            {
                // treat it as a solid wall
                Trace_GetEdgeNormal(trace, vp1, vp2);
            }
            else if(trace->type == TRT_CLIMB)
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
            if(!(trace->physics & PF_SLIDEMOVE))
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
              gActor_t *source, int physicFlags)
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
    trace.tfrac         = 1;
    trace.type          = TRT_NOHIT;
    trace.width         = source ? source->radius : 10.24f;
    trace.offset        = source ? source->height : 10.24f;
    trace.source        = source;

    if(physicFlags <= -1)
        trace.physics = source ? source->physics : 0;
    else
        trace.physics = physicFlags;

    if(source == NULL)
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

    if(trace.type == TRT_NOHIT)
        Trace_Objects(&trace, trace.pl);

    return trace;
}
