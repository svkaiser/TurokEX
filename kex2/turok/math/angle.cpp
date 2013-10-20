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
#include "common.h"
#include "mathlib.h"

#define FULLCIRCLE  (M_PI * 2)

//
// Ang_Round
//

float Ang_Round(float angle)
{
    float an = DEG2RAD((360.0f / 65536.0f) *
        ((int)(RAD2DEG(angle) * (65536.0f / 360.0f)) & 65535));

    Ang_Clamp(&an);
    return an;
}

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

    return (float)acos(vec[1] / (float)sqrt(d)) - DEG2RAD(90);
}

//
// Ang_AlignYawToVector
//

float Ang_AlignYawToVector(float angle, vec3_t v1, vec3_t v2)
{
    float d;
    float v;
    float s;
    float c;
    float x;
    float z;
    float an;

    x = v1[0] - v2[0];
    z = v1[2] - v2[2];

    d = x * x + z * z;

    if(d == 0.0f)
        return 0.0f;

    s = -(float)sin(angle);
    c = -(float)cos(angle);

    v = (c * z + s * x);
    an = v / (float)sqrt(d);

    if(an >  1.0f) an =  1.0f;
    if(an < -1.0f) an = -1.0f;

    if(x >= 0)
        return -(float)acos(an);

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
        return 0.0f;

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
// Ang_ClampInvert
//

float Ang_ClampInvert(float angle)
{
    float an = angle;
    
    for(; an < -M_PI; an = an + FULLCIRCLE);
    for(; an >  M_PI; an = an - FULLCIRCLE);

    return -an;
}

//
// Ang_ClampInvertSums
//

float Ang_ClampInvertSums(float angle1, float angle2)
{
    return Ang_ClampInvert(Ang_ClampInvert(angle2) + angle1);
}

//
// Ang_Clamp
//

void Ang_Clamp(float *angle)
{
    float an = *angle;

    if(an < -M_PI) for(; an < -M_PI; an = an + FULLCIRCLE);
    if(an >  M_PI) for(; an >  M_PI; an = an - FULLCIRCLE);

    *angle = an;
}

//
// Ang_Diff
//

float Ang_Diff(float angle1, float angle2)
{
    float an1;
    float an2;

    Ang_Clamp(&angle1);
    Ang_Clamp(&angle2);

    an2 = 0.0f;

    if(angle1 <= angle2)
    {
        an1 = angle2 + FULLCIRCLE;
        if(angle1 - angle2 > an1 - angle1)
        {
            an2 = angle1 - an1;
        }
        else
        {
            an2 = angle1 - angle2;
        }
    }
    else
    {
        an1 = angle2 - FULLCIRCLE;
        if(angle2 - angle1 <= angle1 - an1)
        {
            an2 = angle1 - angle2;
        }
        else
        {
            an2 = angle1 - an1;
        }
    }

    return an2;
}

//
// kexAngle::kexAngle
//

kexAngle::kexAngle(void) {
    this->yaw   = 0;
    this->pitch = 0;
    this->roll  = 0;
}

//
// kexAngle::kexAngle
//

kexAngle::kexAngle(const float yaw, const float pitch, const float roll) {
    this->yaw   = yaw;
    this->pitch = pitch;
    this->roll  = roll;
}

//
// kexAngle::kexAngle
//

kexAngle::kexAngle(const kexVec3 &vector) {
    this->yaw   = vector.x;
    this->pitch = vector.y;
    this->roll  = vector.z;

    Clamp180();
}

//
// kexAngle::kexAngle
//

kexAngle::kexAngle(const kexAngle &an) {
    this->yaw   = an.yaw;
    this->pitch = an.pitch;
    this->roll  = an.roll;
}

//
// kexAngle::Clamp180
//

kexAngle &kexAngle::Clamp180(void) {
#define CLAMP180(x)                                             \
    if(x < -M_PI) for(; x < -M_PI; x = x + FULLCIRCLE);         \
    if(x >  M_PI) for(; x >  M_PI; x = x - FULLCIRCLE)
    CLAMP180(yaw);
    CLAMP180(pitch);
    CLAMP180(roll);
#undef CLAMP180

    return *this;
}

//
// kexAngle::Clamp180Invert
//

kexAngle &kexAngle::Clamp180Invert(void) {
#define CLAMP180(x)                                             \
    for(; x < -M_PI; x = x + FULLCIRCLE);                       \
    for(; x >  M_PI; x = x - FULLCIRCLE)
    CLAMP180(yaw);
    CLAMP180(pitch);
    CLAMP180(roll);
#undef CLAMP180

    yaw     = -yaw;
    pitch   = -pitch;
    roll    = -roll;

    return *this;
}

//
// kexAngle::Clamp180InvertSum
//

kexAngle &kexAngle::Clamp180InvertSum(const kexAngle &angle) {
    kexAngle an = angle;

    an.Clamp180Invert();

    an.yaw      += this->yaw;
    an.pitch    += this->pitch;
    an.roll     += this->roll;

    an.Clamp180Invert();

    this->yaw   = an.yaw;
    this->pitch = an.pitch;
    this->roll  = an.roll;

    return *this;
}

//
// kexAngle::Round
//

kexAngle &kexAngle::Round(void) {
#define ROUND(x)                                        \
    x = DEG2RAD((360.0f / 65536.0f) *                   \
    ((int)(RAD2DEG(x) * (65536.0f / 360.0f)) & 65535))
    yaw     = ROUND(yaw);
    pitch   = ROUND(pitch);
    roll    = ROUND(roll);
#undef ROUND

    return Clamp180();
}

//
// kexAngle::Diff
//

kexAngle kexAngle::Diff(kexAngle &angle) {
    float an;
    kexAngle out;

    Clamp180();
    angle.Clamp180();

#define DIFF(x)                     \
    if(x <= angle.x) {              \
        an = angle.x + FULLCIRCLE;  \
        if(x - angle.x > an - x) {  \
            out.x = x - an;         \
        }                           \
        else {                      \
            out.x = x - angle.x;    \
        }                           \
    }                               \
    else {                          \
        an = angle.x - FULLCIRCLE;  \
        if(angle.x - x <= x - an) { \
            out.x = x - angle.x;    \
        }                           \
        else {                      \
            out.x = x - an;         \
        }                           \
    }
    DIFF(yaw);
    DIFF(pitch);
    DIFF(roll);
#undef DIFF

    return out;
}

//
// kexAngle::ToAxis
//

void kexAngle::ToAxis(kexVec3 *forward, kexVec3 *up, kexVec3 *right) {
    float sy = (float)sin(yaw);
    float cy = (float)cos(yaw);
    float sp = (float)sin(pitch);
    float cp = (float)cos(pitch);
    float sr = (float)sin(roll);
    float cr = (float)cos(roll);

    if(forward) {
        forward->x  = sy * cp;
        forward->y  = -sp;
        forward->z  = cy * cp;
    }
    if(right) {
        right->x    = sr * sp * sy + cr * cy;
        right->y    = sr * cp;
        right->z    = sr * sp * cy + cr * -sy;
    }
    if(up) {
        up->x       = cr * sp * sy + -sr * cy;
        up->y       = cr * cp;
        up->z       = cr * sp * cy + -sr * -sy;
    }
}

//
// kexAngle::ToForwardAxis
//

kexVec3 kexAngle::ToForwardAxis(void) {
    kexVec3 vec;

    ToAxis(&vec, NULL, NULL);
    return vec;
}

//
// kexAngle::ToUpAxis
//

kexVec3 kexAngle::ToUpAxis(void) {
    kexVec3 vec;

    ToAxis(NULL, &vec, NULL);
    return vec;
}

//
// kexAngle::ToRightAxis
//

kexVec3 kexAngle::ToRightAxis(void) {
    kexVec3 vec;

    ToAxis(NULL, NULL, &vec);
    return vec;
}

//
// kexAngle::ToVec3
//

const kexVec3 &kexAngle::ToVec3(void) const {
    return *reinterpret_cast<const kexVec3*>(&yaw);
}

//
// kexAngle::ToVec3
//

kexVec3 &kexAngle::ToVec3(void) {
    return *reinterpret_cast<kexVec3*>(&yaw);
}

//
// kexAngle::operator+
//

kexAngle kexAngle::operator+(const kexAngle &angle) {
    return kexAngle(yaw + angle.yaw, pitch + angle.pitch, roll + angle.roll);
}

//
// kexAngle::operator-
//

kexAngle kexAngle::operator-(const kexAngle &angle) {
    return kexAngle(yaw - angle.yaw, pitch - angle.pitch, roll - angle.roll);
}

//
// kexAngle::operator-
//

kexAngle kexAngle::operator-(void) {
    return kexAngle(-yaw, -pitch, -roll);
}

//
// kexAngle::operator+=
//

kexAngle &kexAngle::operator+=(const kexAngle &angle) {
    yaw     += angle.yaw;
    pitch   += angle.pitch;
    roll    += angle.roll;
    return *this;
}

//
// kexAngle::operator-=
//

kexAngle &kexAngle::operator-=(const kexAngle &angle) {
    yaw     -= angle.yaw;
    pitch   -= angle.pitch;
    roll    -= angle.roll;
    return *this;
}

//
// kexAngle::operator=
//

kexAngle &kexAngle::operator=(const kexAngle &angle) {
    yaw     = angle.yaw;
    pitch   = angle.pitch;
    roll    = angle.roll;
    return *this;
}

//
// kexAngle::operator=
//

kexAngle &kexAngle::operator=(const kexVec3 &vector) {
    yaw     = vector.x;
    pitch   = vector.y;
    roll    = vector.z;
    return *this;
}

//
// kexAngle::operator=
//

kexAngle &kexAngle::operator=(const float *vecs) {
    yaw     = vecs[0];
    pitch   = vecs[1];
    roll    = vecs[2];
    return *this;
}

//
// kexAngle::operator[]
//

float kexAngle::operator[](int index) const {
    assert(index >= 0 && index < 3);
    return (&yaw)[index];
}

//
// kexAngle::operator[]
//

float kexAngle::operator[](int index) {
    assert(index >= 0 && index < 3);
    return (&yaw)[index];
}

//
// kexAngle::ObjectConstruct1
//

void kexAngle::ObjectConstruct1(kexAngle *an) {
    new(an)kexAngle();
}

//
// kexAngle::ObjectConstruct2
//

void kexAngle::ObjectConstruct2(const float a, const float b, const float c, kexAngle *an) {
    new(an)kexAngle(a, b, c);
}

//
// kexAngle::ObjectConstruct3
//

void kexAngle::ObjectConstruct3(const kexVec3 &vec, kexAngle *an) {
    new(an)kexAngle(vec);
}

//
// kexAngle::ObjectConstructCopy
//

void kexAngle::ObjectConstructCopy(const kexAngle &in, kexAngle *an) {
    new(an)kexAngle(in);
}
