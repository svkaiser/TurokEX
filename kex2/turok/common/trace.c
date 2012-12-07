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

#define EPSILON_FLOOR 0.975f

typedef enum
{
    TRT_NOHIT       = 0,
    TRT_SLOPE       = 1,
    TRT_WALL        = 2,
    TRT_EDGE        = 3,
    TRT_OBJECT      = 4,
    TRT_INTERACT    = 5
} tracetype_e;

typedef struct
{
    vec3_t      start;
    vec3_t      end;
    float       offset;
    float       width;
    float       yaw;
    plane_t     *pl;
    plane_t     *hitpl;
    vec3_t      normal;
    float       frac;
    tracetype_e type;
} trace_t;

//
// Trace_Object
//
// Line-circle intersection test on an object
//

static kbool Trace_Object(trace_t *trace, vec3_t objpos, float radius)
{
    vec3_t dir;
    float x;
    float z;
    float vd;
    float d;

    Vec_Sub(dir, trace->end, trace->start);
    vd = Vec_Unit2(dir);
    Vec_Normalize3(dir);

    // validate position
    x = objpos[0] - trace->start[0];
    z = objpos[2] - trace->start[2];
    d = dir[0] * x + dir[2] * z;

    if(d >= 0 && vd != 0)
    {
        float dx;
        float dz;
        float ld;
        float len;
        float r;

        vd = 1.0f / vd;

        dx = vd * dir[0];
        dz = vd * dir[2];
        ld = dx * x + dz * z;

        x = x - ld * dx;
        z = z - ld * dz;

        r = trace->width * trace->width + radius * radius;
        len = r - (x * x + z * z);

        // is the ray inside the radius?
        if(len > 0)
        {
            vec3_t end;
            vec3_t lerp;
            vec3_t n;
            vec3_t vec;
            float f;

            f = ld - (float)sqrt(len) * vd;

            // setup normal to clip the raytrace with
            Vec_Add(end, trace->start, dir);
            Vec_Lerp3(lerp, f, trace->start, end);
            Vec_Set3(n,
                lerp[0] - objpos[0],
                0,
                lerp[2] - objpos[2]);

            Vec_Sub(dir, trace->end, trace->start);
            d = Vec_Dot(n, dir);

            if(d != 0)
            {
                Vec_Sub(vec, lerp, trace->start);
                d = Vec_Dot(n, vec) / d;

                trace->frac = d;

                Vec_Normalize3(n);
                Vec_Copy3(trace->normal, n);
                return true;
            }
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

static kbool Trace_Objects(trace_t *trace, plane_t *plane)
{
    blockobj_t *obj;

    if(plane == NULL)
    {
        return false;
    }

    // go through the list
    for(obj = plane->blocklist.next; obj != &plane->blocklist; obj = obj->next)
    {
        if(Trace_Object(trace, obj->object->origin, obj->object->width))
        {
            float offset;

            offset = trace->offset;

            // check object height
            if(trace->end[1] > obj->object->origin[1] + obj->object->height ||
                trace->end[1] + offset < obj->object->origin[1])
            {
                continue;
            }

            trace->type = TRT_OBJECT;
            return true;
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
        float y = trace->end[1] + 1.024f;

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
    {
        return false;
    }

    if(!Plane_IsAWall(pl))
    {
        // ignore if the plane isn't steep enough
        if(pl->normal[1] <= EPSILON_FLOOR)
        {
            return false;
        }
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
        Plane_GetHeight(link, pos) < pos[1])
    {
        // above ceiling height
        return NULL;
    }

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

            // check climbable plane
            if(link->flags & CLF_CLIMB)
            {
                float angle = Plane_GetEdgeYaw(p, point) + M_PI;
                Ang_Clamp(&angle);

                angle = Ang_Diff(angle, trace->yaw);
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

            if(Plane_GetDistance(link, trace->end) <=
                Plane_GetDistance(p, trace->start))
            {
                // able to step off into this plane
                return link;
            }

            Vec_Sub(dir, trace->end, trace->start);

            // special case for planes flagged to block
            // from the front side. these will be treated as
            // solid walls. direction of ray must be facing
            // towards the plane
            if(Trace_CheckPlaneHeight(trace, link) &&
                Plane_IsFacing(link, Ang_VectorToAngle(dir)))
            {
                return NULL;
            }
        }
    }

    // crossed into a floor plane
    return link;
}

//
// Trace_TraversePlanes
//

void Trace_TraversePlanes(plane_t *plane, trace_t *trace)
{
    int i;
    plane_t *pl;

    if(plane == NULL)
    {
        return;
    }

    // we've entered a new plane
    trace->pl = plane;

    if(Trace_Objects(trace, plane))
    {
        // an object was hit
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

        if(Trace_Objects(trace, plane->link[i]))
        {
            // an object was hit
            return;
        }

        if(Trace_PlaneEdge(trace, vp1, vp2))
        {
            pl = Trace_GetPlaneLink(trace, plane, i);

            if(pl == NULL)
            {
                // treat it as a solid wall
                Trace_GetEdgeNormal(trace, vp1, vp2);
                return;
            }
            else if(trace->type == TRT_INTERACT)
            {
                trace->pl = pl;
                break;
            }
        }
    }

    if(pl)
    {
        // check to see if the ray can bump into it
        if(Plane_IsAWall(pl))
        {
            // trace it to see if its valid or not
            if(Trace_Plane(trace, pl))
            {
                return;
            }
        }

        trace->type = TRT_NOHIT;

        // traverse into next plane
        Trace_TraversePlanes(pl, trace);
    }
}

//
// Trace
//
//

trace_t Trace(vec3_t start, vec3_t end, plane_t *plane, float width, float offset, float yaw)
{
    trace_t trace;

    Vec_Copy3(trace.start, start);
    Vec_Copy3(trace.end, end);
    Vec_Set3(trace.normal, 0, 0, 0);

    trace.pl        = plane;
    trace.hitpl     = NULL;
    trace.frac      = 0;
    trace.type      = TRT_NOHIT;
    trace.width     = width;
    trace.offset    = offset;
    trace.yaw       = yaw;

    // try to trace something as early as possible
    if(!Trace_Plane(&trace, trace.pl))
    {
        if(!Trace_Objects(&trace, trace.pl))
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
                {
                    Trace_GetEdgeNormal(&trace, vp1, vp2);
                    break;
                }
            }
        }

        // start traversing into other planes if we couldn't trace anything
        if(trace.type == TRT_NOHIT)
            Trace_TraversePlanes(plane, &trace);
    }

    return trace;
}

