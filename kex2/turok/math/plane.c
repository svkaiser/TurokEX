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
// DESCRIPTION: Plane operations
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "common.h"
#include "mathlib.h"
#include "zone.h"

//
// Plane_SetTemp
//

void Plane_SetTemp(plane_t *plane, vec3_t p1, vec3_t p2, vec3_t p3)
{
    plane->points = Z_Alloca(sizeof(vec3_t) * 3);

    memcpy(&plane->points[0], p1, sizeof(vec3_t));
    memcpy(&plane->points[1], p2, sizeof(vec3_t));
    memcpy(&plane->points[2], p3, sizeof(vec3_t));

    plane->link1 = plane->link2 = plane->link3 = NULL;
    plane->area = -1;
    plane->flags = 0;
}

//
// Plane_GetNormal
//

void Plane_GetNormal(vec3_t normal, plane_t *plane)
{
    vec3_t vp1;
    vec3_t vp2;
    vec3_t vn;

    if(plane->points)
    {
        Vec_Sub(vp1, plane->points[1], plane->points[0]);
        Vec_Sub(vp2, plane->points[2], plane->points[1]);
        Vec_Cross(vn, vp1, vp2);
    }
    else
    {
        vn[0] = 0;
        vn[1] = 1.0f;
        vn[2] = 0;
    }

    normal[0] = vn[0];
    normal[1] = vn[1];
    normal[2] = vn[2];
}

//
// Plane_IsFacing
//

kbool Plane_IsFacing(plane_t *plane, float angle)
{
    float s;
    float c;
    vec3_t n;

    s = (float)sin(angle);
    c = (float)cos(angle);

    Plane_GetNormal(n, plane);

    return -s * n[0] + -c * n[2] < 0.0f;
}

//
// Plane_GetDistance
//

float Plane_GetDistance(plane_t *plane, vec3_t pos)
{
    vec3_t normal;
    float dist;

    Plane_GetNormal(normal, plane);
    
    if(plane->points)
    {
        if(normal[1] == 0.0f)
        {
            dist = (
                plane->points[0][1] + 
                plane->points[1][1] +
                plane->points[2][1]) * 0.3333333432674408f;
        }
        else
        {
            vec3_t vec;

            Vec_Set3(vec,
                plane->points[0][0] - pos[0],
                plane->points[0][1],
                plane->points[0][2] - pos[2]);

            dist = Vec_Dot(vec, normal) / normal[1];
        }
        
        return dist;
    }

    return -(float)D_MAXINT;
}

//
// Plane_CheckYSlope
//

kbool Plane_CheckYSlope(plane_t *plane)
{
    vec3_t normal;

    Plane_GetNormal(normal, plane);
    Vec_Normalize3(normal);

    return (normal[1] <= 0.5f);
}

//
// Plane_GetAngle
//

float Plane_GetAngle(plane_t *p)
{
    if(Plane_CheckYSlope(p))
    {
        vec3_t t1;
        vec3_t t2;
        vec3_t n;
        float an;

        Plane_GetNormal(n, p);
        Vec_Set3(t1, 0, 1.0f, 0);
        Vec_Normalize3(n);
        Vec_Cross(t2, t1, n);
        Vec_Normalize3(t2);

        an = (float)acos(t1[0] * n[0] + t1[1] * n[1] + t1[2] * n[2]);

        return an;
    }

    return 0.0f;
}

//
// Plane_GetQuaternion
//

void Plane_GetQuaternion(vec4_t vec, plane_t *p)
{
    vec3_t n1;
    vec3_t n2;
    vec3_t cp;
    float an;
    float s;
    float c;

    Plane_GetNormal(n1, p);
    Vec_Normalize3(n1);
    Vec_Set3(n2, 0, 1, 0);
    Vec_Cross(cp, n2, n1);
    Vec_Normalize3(cp);

    an = (float)acos(n2[0] * n1[0] + n2[1] * n1[1] + n2[2] * n1[2]) * 0.5f;
    s = (float)sin(an);
    c = (float)cos(an);

    vec[0] = cp[0] * s;
    vec[1] = cp[1] * s;
    vec[2] = cp[2] * s;
    vec[3] = c;
}

//
// Plane_AdjustQuaternion
//

void Plane_AdjustQuaternion(vec4_t out, plane_t *p)
{
    vec3_t n1;
    vec3_t n2;
    vec3_t cp;
    float d;

    Plane_GetNormal(n1, p);
    Vec_Normalize3(n1);
    Vec_Set3(n2, 0, 1, 0);
    Vec_Cross(cp, n2, n1);

    d = (float)sqrt(cp[0] * cp[0] + cp[1] * cp[1] + cp[2] * cp[2]);

    if(d == 0.0f)
    {
        Vec_Set4(out, 0, 0, 0, 1);
    }
    else
    {
        float an;
        float s;
        float c;

        Vec_MultValue(cp, cp, 1.0f / d);

        an = (float)acos(n2[0] * n1[0] + n2[1] * n1[1] + n2[2] * n1[2]) * 0.5f;
        s = (float)sin(an);
        c = (float)cos(an);

        out[0] = cp[0] * s;
        out[1] = cp[1] * s;
        out[2] = cp[2] * s;
        out[3] = c;
    }
}

//
// Plane_PointInRange
//

kbool Plane_PointInRange(plane_t *p, float x, float z)
{
    int i;

    if(p->points)
    {
        for(i = 0; i < 3; i++)
        {
            if((x - p->points[i][0]) * (p->points[i+1][2] -
                p->points[i][2]) + (p->points[i][0] -
                p->points[i+1][0]) * (z - p->points[i][2]) < 0.0f)
            {
                return false;
            }
        }
    }

    return true;
}

