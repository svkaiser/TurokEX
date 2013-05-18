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
// DESCRIPTION: Vector operations
//
//-----------------------------------------------------------------------------

#include "common.h"
#include <math.h>
#include "mathlib.h"

//
// Vec_Copy3
//

void Vec_Copy3(vec3_t out, vec3_t vec)
{
    out[0] = vec[0];
    out[1] = vec[1];
    out[2] = vec[2];
}

//
// Vec_Copy4
//

void Vec_Copy4(vec4_t out, vec4_t vec)
{
    out[0] = vec[0];
    out[1] = vec[1];
    out[2] = vec[2];
    out[3] = vec[3];
}

//
// Vec_Set3
//

void Vec_Set3(vec3_t vec, float x, float y, float z)
{
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
}

//
// Vec_Set4
//

void Vec_Set4(vec4_t vec, float x, float y, float z, float w)
{
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = w;
}

//
// Vec_Compare3
//

kbool Vec_Compare3(vec3_t v1, vec3_t v2)
{
    return (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2]);
}

//
// Vec_ToString
//

char *Vec_ToString(vec3_t vec)
{
    return kva("%f %f %f", vec[0], vec[1], vec[2]);
}

//
// Vec_Cross
//

void Vec_Cross(vec3_t out, vec3_t vec1, vec3_t vec2)
{
    out[0] = vec2[2] * vec1[1] - vec1[2] * vec2[1];
    out[1] = vec2[0] * vec1[2] - vec1[0] * vec2[2];
    out[2] = vec1[0] * vec2[1] - vec2[0] * vec1[1];
}

//
// Vec_Dot
//

float Vec_Dot(vec3_t vec1, vec3_t vec2)
{
    return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

//
// Vec_Add
//

void Vec_Add(vec3_t out, vec3_t vec1, vec3_t vec2)
{
    out[0] = vec1[0] + vec2[0];
    out[1] = vec1[1] + vec2[1];
    out[2] = vec1[2] + vec2[2];
}

//
// Vec_Sub
//

void Vec_Sub(vec3_t out, vec3_t vec1, vec3_t vec2)
{
    out[0] = vec1[0] - vec2[0];
    out[1] = vec1[1] - vec2[1];
    out[2] = vec1[2] - vec2[2];
}

//
// Vec_Mult
//

void Vec_Mult(vec3_t out, vec3_t vec1, vec3_t vec2)
{
    out[0] = vec1[0] * vec2[0];
    out[1] = vec1[1] * vec2[1];
    out[2] = vec1[2] * vec2[2];
}

//
// Vec_Scale
//

void Vec_Scale(vec3_t out, vec3_t vec1, float val)
{
    out[0] = vec1[0] * val;
    out[1] = vec1[1] * val;
    out[2] = vec1[2] * val;
}

//
// Vec_Length2
//

float Vec_Length2(vec3_t v1, vec3_t v2)
{
    float x;
    float z;

    x = v1[0] - v2[0];
    z = v1[2] - v2[2];

    return (float)sqrt(x * x + z * z);
}

//
// Vec_Length3
//

float Vec_Length3(vec3_t v1, vec3_t v2)
{
    float x;
    float y;
    float z;

    x = v1[0] - v2[0];
    y = v1[1] - v2[1];
    z = v1[2] - v2[2];

    return (float)sqrt(x * x + y * y + z * z);
}

//
// Vec_Unit2
//

float Vec_Unit2(vec3_t vec)
{
    return (float)sqrt(
        vec[0] * vec[0] +
        vec[2] * vec[2]);
}

//
// Vec_Unit3
//

float Vec_Unit3(vec3_t vec)
{
    return (float)sqrt(
        vec[0] * vec[0] +
        vec[1] * vec[1] +
        vec[2] * vec[2]);
}

//
// Vec_Normalize3
//

void Vec_Normalize3(vec3_t out)
{
    float d;

    d = (float)sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);

    if(d != 0.0f)
    {
        out[0]  = out[0] * 1.0f / d;
        out[1]  = out[1] * 1.0f / d;
        out[2]  = out[2] * 1.0f / d;
    }
}

//
// Vec_Normalize4
//

void Vec_Normalize4(vec4_t out)
{
    float d;

    d = (float)sqrt(out[1] * out[1] + out[2] * out[2] + out[3] * out[3] + out[0] * out[0]);

    if(d != 0.0f)
    {
        out[0]  = out[0] * 1.0f / d;
        out[1]  = out[1] * 1.0f / d;
        out[2]  = out[2] * 1.0f / d;
        out[3]  = out[3] * 1.0f / d;
    }
}

//
// Vec_TransformToWorld
//

void Vec_TransformToWorld(mtx_t m, vec3_t vec, vec3_t out)
{
    out[0] = m[ 4] * vec[1] + m[ 8] * vec[2] + m[ 0] * vec[0] + m[12];
    out[1] = m[ 5] * vec[1] + m[ 9] * vec[2] + m[ 1] * vec[0] + m[13];
    out[2] = m[ 6] * vec[1] + m[10] * vec[2] + m[ 2] * vec[0] + m[14];
}

//
// Vec_ApplyQuaternion
//

void Vec_ApplyQuaternion(vec3_t out, vec3_t vec, vec4_t rot)
{
    float xx = rot[0] * rot[0];
    float yx = rot[1] * rot[0];
    float zx = rot[2] * rot[0];
    float wx = rot[3] * rot[0];
    float yy = rot[1] * rot[1];
    float zy = rot[2] * rot[1];
    float wy = rot[3] * rot[1];
    float zz = rot[2] * rot[2];
    float wz = rot[3] * rot[2];
    float ww = rot[3] * rot[3];

    out[0] = ((yx + yx) - (wz + wz)) * vec[1] +
        ((wy + wy + zx + zx)) * vec[2] +
        (((ww + xx) - yy) - zz) * vec[0];
    out[1] = ((yy + (ww - xx)) - zz) * vec[1] +
        ((zy + zy) - (wx + wx)) * vec[2] +
        ((wz + wz) + (yx + yx)) * vec[0];
    out[2] = ((wx + wx) + (zy + zy)) * vec[1] +
        (((ww - xx) - yy) + zz) * vec[2] +
        ((zx + zx) - (wy + wy)) * vec[0];
}

//
// Vec_Lerp3
//

void Vec_Lerp3(vec3_t out, float movement, vec3_t curr, vec3_t next)
{
    out[0] = (next[0] - curr[0]) * movement + curr[0];
    out[1] = (next[1] - curr[1]) * movement + curr[1];
    out[2] = (next[2] - curr[2]) * movement + curr[2];
}

//
// Vec_Slerp
//

void Vec_Slerp(vec4_t out, float movement, vec4_t vec1, vec4_t vec2)
{
    float d1;
    float d2;
    float halfcos;
    float halfsin;
    vec4_t rdest;

    Vec_Copy4(rdest, vec2);

    d1 =  vec2[3] * vec1[3] +  vec2[2] * vec1[2] +  vec2[1] * vec1[1] +  vec2[0] * vec1[0];
    d2 = -vec2[3] * vec1[3] + -vec2[2] * vec1[2] + -vec2[1] * vec1[1] + -vec2[0] * vec1[0];

    if(d1 < d2)
    {
        Vec_Set4(rdest, -vec2[0], -vec2[1], -vec2[2], -vec2[3]);
        d1 = d2;
    }

    if(d1 <= 0.7071067811865001f)
    {
        halfcos = (float)acos(d1);
        halfsin = (float)sin(halfcos);

        if(halfsin == 0)
        {
            out[0] = vec1[0];
            out[1] = vec1[1];
            out[2] = vec1[2];
            out[3] = vec1[3];
        }
        else
        {
            float d;
            float ms1;
            float ms2;

            d = 1.0f / halfsin;
            ms1 = (float)sin((1.0f - movement) * halfcos) * d;
            ms2 = (float)sin(halfcos * movement) * d;

            if(ms2)
            {
                Vec_Set4(rdest, -vec2[0], -vec2[1], -vec2[2], -vec2[3]);
            }

            out[0] = ms1 * vec1[0] + rdest[0] * ms2;
            out[1] = ms1 * vec1[1] + rdest[1] * ms2;
            out[2] = ms1 * vec1[2] + rdest[2] * ms2;
            out[3] = ms1 * vec1[3] + rdest[3] * ms2;
        }
    }
    else
    {
        out[0] = (rdest[0] - vec1[0]) * movement + vec1[0];
        out[1] = (rdest[1] - vec1[1]) * movement + vec1[1];
        out[2] = (rdest[2] - vec1[2]) * movement + vec1[2];
        out[3] = (rdest[3] - vec1[3]) * movement + vec1[3];

        Vec_Normalize4(out);
    }
}

//
// Vec_SetQuaternion
//

void Vec_SetQuaternion(vec4_t vec, float angle, float x, float y, float z)
{
    float sin_a = (float)sin(angle * 0.5f);
    float cos_a = (float)cos(angle * 0.5f);

    vec[0]  = x * sin_a;
    vec[1]  = y * sin_a;
    vec[2]  = z * sin_a;
    vec[3]  = cos_a;

    Vec_Normalize4(vec);
}

//
// Vec_QuaternionToAxis
//

void Vec_QuaternionToAxis(float *angle, vec3_t vec3, vec4_t vec4)
{
    float d = (float)sqrt(1 - vec4[3] * vec4[3]);

    *angle  = 2 * (float)acos(vec4[3]);

    if(d != 0.0)
    {
        vec3[0] = vec4[0] / d;
        vec3[1] = vec4[1] / d;
        vec3[2] = vec4[2] / d;
    }
    else
    {
        vec3[0] = 1;
        vec3[1] = 0;
        vec3[2] = 0;
    }
}

//
// Vec_ToQuaternion
//

void Vec_ToQuaternion(vec4_t out, vec3_t vec)
{
    float d;
    float an;
    vec3_t scv;
    vec3_t fwd;
    vec3_t cp;

    d = Vec_Unit3(vec);

    if(d == 0)
    {
        Vec_Set4(out, 0, 0, 0, 1);
        return;
    }

    Vec_Set3(fwd, 0, 0, 1);
    Vec_Scale(scv, vec, 1 / d);
    Vec_Cross(cp, fwd, scv);
    Vec_Normalize3(cp);

    an = (float)acos(scv[2]);

    Vec_SetQuaternion(out, an, cp[0], cp[1], cp[2]);
}

//
// Vec_MultQuaternion
//

void Vec_MultQuaternion(vec4_t out, vec4_t q1, vec4_t q2)
{
    out[0] = q1[0] * q2[3] - q1[1] * q2[2] + q2[0] * q1[3] + q2[1] * q1[2];
    out[1] = q1[0] * q2[2] + q1[1] * q2[3] - q2[0] * q1[2] + q1[3] * q2[1];
    out[2] = q2[0] * q1[1] + q1[3] * q2[2] + q1[2] * q2[3] - q1[0] * q2[1];
    out[3] = q1[3] * q2[3] - q2[1] * q1[1] + q1[2] * q2[2] + q2[0] * q1[0];
}

//
// Vec_AdjustQuaternion
//

void Vec_AdjustQuaternion(vec4_t out, vec4_t rot, float angle)
{
    vec4_t vec;
    float s;
    float c;

    s = (float)sin((angle - -M_PI) * 0.5f);
    c = (float)cos((angle - -M_PI) * 0.5f);

    vec[0] = 0;
    vec[1] = s;
    vec[2] = 0;
    vec[3] = c;
    
    Vec_MultQuaternion(out, vec, rot);
}

//
// Vec_PointToAxis
//

void Vec_PointToAxis(vec3_t out, vec3_t p1, vec3_t p2)
{
    float an1;
    float an2;

    an1 = (float)atan2(p2[0] - p1[0], p2[2] - p1[2]);
    an2 = (float)atan2(Vec_Length3(p2, p1), p2[1]- p1[1]);

    out[0] = (float)sin(an1);
    out[1] = (float)cos(an2);
    out[2] = (float)cos(an1);
}

//
// Vec_PointToAngle
//

void Vec_PointToAngle(vec4_t out, vec3_t p1, vec3_t p2)
{
    float an1;
    float an2;
    vec4_t q1;
    vec4_t q2;

    an1 = (float)atan2(p2[0] - p1[0], p2[2] - p1[2]);
    an2 = (float)atan2(Vec_Length3(p2, p1), p2[1]- p1[1]);

    Vec_SetQuaternion(q1, an1, 0, 1, 0);
    Vec_SetQuaternion(q2, an2, 0, 0, 1);
    Vec_MultQuaternion(out, q2, q1);
}


