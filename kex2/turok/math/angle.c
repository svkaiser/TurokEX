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
// DESCRIPTION: Angle operations
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "mathlib.h"

//
// Ang_AlignPitchToVector
//

float Ang_AlignPitchToVector(vec3_t vec)
{
    float d;

    d = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];

    if(d == 0.0f)
    {
        return 0.0f;
    }

    return (float)acos(vec[1] / (float)sqrt(d)) - (M_RAD * 90.0f);
}

//
// Ang_AlignYawToVector
//

float Ang_AlignYawToVector(float angle, vec3_t v1, vec3_t v2)
{
    float d;
    float s;
    float c;
    float x;
    float z;
    float an;

    x = v1[0] - v2[0];
    z = v1[2] - v2[2];

    d = (float)sqrt(x * x + z * z);

    if(d == 0.0f)
    {
        return 0.0f;
    }

    s = -(float)sin(angle);
    c = -(float)cos(angle);

    an = (c * z + s * x) / d;
    if(an >  1.0f) an =  1.0f;
    if(an < -1.0f) an = -1.0f;

    if(x >= 0)
    {
        return -(float)acos(an);
    }

    return (float)acos(an);
}

//
// Ang_VectorToAngle
//

float Ang_VectorToAngle(vec3_t vec)
{
    float d;
    float an;

    d = vec[0] * vec[0] + vec[2] * vec[2];

    if(d == 0.0f)
    {
        return 0.0f;
    }

    an = -(vec[2] / (float)sqrt(d));

    if(an >  1.0f) an =  1.0f;
    if(an < -1.0f) an = -1.0f;

    if(-vec[0] <= 0.0f)
    {
        return -(float)acos(an);
    }

    return (float)acos(an);
}

//
// Ang_Invert
//

float Ang_Invert(float angle)
{
    float an;
    
    for(an = angle; an < -M_PI; an = an - -(M_PI * 2));
    for(; an > M_PI; an = an - (M_PI * 2));

    return -an;
}

//
// Ang_InvertSums
//

float Ang_InvertSums(float angle1, float angle2)
{
    return Ang_Invert(Ang_Invert(angle2) + angle1);
}

//
// Ang_Clamp
//

void Ang_Clamp(float *angle)
{
    float an = *angle;

    if(*angle < -M_PI)
    {
        for(an = *angle; an < -M_PI; an = an - -(M_PI * 2));
    }

    if(*angle >= M_PI)
    {
        for(; an > M_PI; an = an - (M_PI * 2));
    }

    *angle = an;
}

