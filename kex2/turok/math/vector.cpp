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

#include <math.h>
#include "common.h"
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
// Vec_Magnitude2
//

float Vec_Magnitude2(vec3_t vec)
{
    return vec[0]*vec[0]+vec[2]*vec[2];
}

//
// Vec_Magnitude3
//

float Vec_Magnitude3(vec3_t vec)
{
    return vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2];
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

    return kexMath::Sqrt(x * x + z * z);
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

    return kexMath::Sqrt(x * x + y * y + z * z);
}

//
// Vec_Unit2
//

float Vec_Unit2(vec3_t vec)
{
    return kexMath::Sqrt(
        vec[0] * vec[0] +
        vec[2] * vec[2]);
}

//
// Vec_Unit3
//

float Vec_Unit3(vec3_t vec)
{
    return kexMath::Sqrt(
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

    d = kexMath::Sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);

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

    d = kexMath::Sqrt(out[1] * out[1] + out[2] * out[2] + out[3] * out[3] + out[0] * out[0]);

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
        halfcos = kexMath::ACos(d1);
        halfsin = kexMath::Sin(halfcos);

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
            ms1 = kexMath::Sin((1.0f - movement) * halfcos) * d;
            ms2 = kexMath::Sin(halfcos * movement) * d;

            if(ms2 < 0)
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
    float sin_a = kexMath::Sin(angle * 0.5f);
    float cos_a = kexMath::Cos(angle * 0.5f);

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
    float d = kexMath::Sqrt(1 - vec4[3] * vec4[3]);

    *angle  = 2 * kexMath::ACos(vec4[3]);

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

    an = kexMath::ACos(scv[2]);

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

    s = kexMath::Sin((angle + M_PI) * 0.5f);
    c = kexMath::Cos((angle + M_PI) * 0.5f);

    vec[0] = 0;
    vec[1] = s;
    vec[2] = 0;
    vec[3] = c;
    
    Vec_MultQuaternion(out, vec, rot);
}

//
// Vec_PointAt
//

void Vec_PointAt(vec3_t org1, vec3_t org2, vec4_t rotation, float maxAngle, vec4_t out)
{
    vec3_t axis;
    vec3_t dir;
    vec3_t cp;
    vec4_t prot;
    float an;

    Vec_Set3(axis, 0, 0, 1);
    Vec_ApplyQuaternion(dir, axis, rotation);

    Vec_Sub(axis, org2, org1);
    Vec_Normalize3(axis);
    Vec_Cross(cp, dir, axis);
    Vec_Normalize3(cp);

    an = kexMath::ACos(Vec_Dot(axis, dir));

    if(maxAngle != 0 && an >= maxAngle)
        an = maxAngle;

    Vec_SetQuaternion(prot, an, cp[0], cp[1], cp[2]);
    Vec_MultQuaternion(out, rotation, prot);
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

    out[0] = kexMath::Sin(an1);
    out[1] = kexMath::Cos(an2);
    out[2] = kexMath::Cos(an1);
}

const kexVec3 kexVec3::vecRight(1, 0, 0);
const kexVec3 kexVec3::vecUp(0, 1, 0);
const kexVec3 kexVec3::vecForward(0, 0, 1);

//
// kexVec3::kexVec3
//

kexVec3::kexVec3(void) {
    Clear();
}

//
// kexVec3::kexVec3
//

kexVec3::kexVec3(const float x, const float y, const float z) {
    Set(x, y, z);
}

//
// kexVec3::Set
//

void kexVec3::Set(const float x, const float y, const float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

//
// kexVec3::Clear
//

void kexVec3::Clear(void) {
    x = y = z = 0.0f;
}

//
// kexVec3::Dot
//

float kexVec3::Dot(const kexVec3 &vec) const {
    return (x * vec.x + y * vec.y + z * vec.z);
}

//
// kexVec3::Dot
//

float kexVec3::Dot(const kexVec3 &vec1, const kexVec3 &vec2) {
    return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
}

//
// kexVec3::Cross
//

kexVec3 kexVec3::Cross(const kexVec3 &vec) const {
    return kexVec3(
        vec.z * y - z * vec.y,
        vec.x * z - x * vec.z,
        x * vec.y - vec.x * y
    );
}

//
// kexVec3::Cross
//

kexVec3 &kexVec3::Cross(const kexVec3 &vec1, const kexVec3 &vec2) {
    x = vec2.z * vec1.y - vec1.z * vec2.y;
    y = vec2.x * vec1.z - vec1.x * vec2.z;
    z = vec1.x * vec2.y - vec2.x * vec1.y;

    return *this;
}

//
// kexVec3::UnitSq
//

float kexVec3::UnitSq(void) const {
    return x * x + y * y + z * z;
}

//
// kexVec3::Unit
//

float kexVec3::Unit(void) const {
    return kexMath::Sqrt(UnitSq());
}

//
// kexVec3::DistanceSq
//

float kexVec3::DistanceSq(const kexVec3 &vec) const {
    return (
        (x - vec.x) * (x - vec.x) +
        (y - vec.y) * (y - vec.y) +
        (z - vec.z) * (z - vec.z)
    );
}

//
// kexVec3::Distance
//

float kexVec3::Distance(const kexVec3 &vec) const {
    return kexMath::Sqrt(DistanceSq(vec));
}

//
// kexVec3::Normalize
//

kexVec3 &kexVec3::Normalize(void) {
    float d = Unit();
    if(d != 0.0f) {
        d = 1.0f / d;
        *this *= d;
    }
    return *this;
}

//
// kexVec3::PointAt
//

kexVec3 kexVec3::PointAt(kexVec3 &location) const {
    float an1 = (float)atan2(location.x - x, location.z - z);
    float an2 = (float)atan2(location.Distance(*this), location.y - y);

    return kexVec3(
        kexMath::Sin(an1),
        kexMath::Cos(an2),
        kexMath::Cos(an1)
    );
}

//
// kexVec3::Lerp
//

kexVec3 kexVec3::Lerp(const kexVec3 &next, float movement) const {
    return (next - *this) * movement + *this;
}

//
// kexVec3::Lerp
//

kexVec3 &kexVec3::Lerp(const kexVec3 &start, const kexVec3 &next, float movement) {
    *this = (next - start) * movement + start;
    return *this;
}

//
// kexVec3::ToQuat
//

kexQuat kexVec3::ToQuat(void) {
    float d = Unit();

    if(d == 0.0f)
        return kexQuat();

    kexVec3 scv = *this * (1.0f / d);
    float angle = kexMath::ACos(scv.z);

    return kexQuat(angle, vecForward.Cross(scv).Normalize());
}

//
// kexVec3::ToYaw
//

float kexVec3::ToYaw(void) const {
    float d = x * x + z * z;

    if(d == 0.0f)
        return 0.0f;

    float an = -(z / kexMath::Sqrt(d));

    if(an >  1.0f) an =  1.0f;
    if(an < -1.0f) an = -1.0f;

    if(-x <= 0.0f) {
        return -kexMath::ACos(an);
    }

    return kexMath::ACos(an);
}

//
// kexVec3::ToPitch
//

float kexVec3::ToPitch(void) const {
    float d = UnitSq();
    
    if(d == 0.0f)
        return 0.0f;
        
    return kexMath::ACos(y / kexMath::Sqrt(d));
}

//
// kexVec3::ToString
//

kexStr kexVec3::ToString(void) const {
    kexStr str;
    str = str + x + " " + y + " " + z;
    return str;
}

//
// kexVec3::operator+
//

kexVec3 kexVec3::operator+(const kexVec3 &vec) {
    return kexVec3(x + vec.x, y + vec.y, z + vec.z);
}

//
// kexVec3::operator+
//

kexVec3 kexVec3::operator+(const kexVec3 &vec) const {
    return kexVec3(x + vec.x, y + vec.y, z + vec.z);
}

//
// kexVec3::operator+
//

kexVec3 kexVec3::operator+(kexVec3 &vec) {
    return kexVec3(x + vec.x, y + vec.y, z + vec.z);
}

//
// kexVec3::operator+=
//

kexVec3 &kexVec3::operator+=(const kexVec3 &vec) {
    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
}

//
// kexVec3::operator-
//

kexVec3 kexVec3::operator-(const kexVec3 &vec) const {
    return kexVec3(x - vec.x, y - vec.y, z - vec.z);
}

//
// kexVec3::operator-
//

kexVec3 kexVec3::operator-(void) const {
    return kexVec3(-x, -y, -z);
}

//
// kexVec3::operator-=
//

kexVec3 &kexVec3::operator-=(const kexVec3 &vec) {
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
}

//
// kexVec3::operator*
//

kexVec3 kexVec3::operator*(const kexVec3 &vec) {
    return kexVec3(x * vec.x, y * vec.y, z * vec.z);
}

//
// kexVec3::operator*=
//

kexVec3 &kexVec3::operator*=(const kexVec3 &vec) {
    x *= vec.x;
    y *= vec.y;
    z *= vec.z;
    return *this;
}

//
// kexVec3::operator*
//

kexVec3 kexVec3::operator*(const float val) {
    return kexVec3(x * val, y * val, z * val);
}

//
// kexVec3::operator*
//

kexVec3 kexVec3::operator*(const float val) const {
    return kexVec3(x * val, y * val, z * val);
}

//
// kexVec3::operator*=
//

kexVec3 &kexVec3::operator*=(const float val) {
    x *= val;
    y *= val;
    z *= val;
    return *this;
}

//
// kexVec3::operator/
//

kexVec3 kexVec3::operator/(const kexVec3 &vec) {
    return kexVec3(x / vec.x, y / vec.y, z / vec.z);
}

//
// kexVec3::operator/=
//

kexVec3 &kexVec3::operator/=(const kexVec3 &vec) {
    x /= vec.x;
    y /= vec.y;
    z /= vec.z;
    return *this;
}

//
// kexVec3::operator/
//

kexVec3 kexVec3::operator/(const float val) {
    return kexVec3(x / val, y / val, z / val);
}

//
// kexVec3::operator/=
//

kexVec3 &kexVec3::operator/=(const float val) {
    x /= val;
    y /= val;
    z /= val;
    return *this;
}

//
// kexVec3::operator|
//

kexVec3 kexVec3::operator|(const kexQuat &quat) {
    float xx = quat.x * quat.x;
    float yx = quat.y * quat.x;
    float zx = quat.z * quat.x;
    float wx = quat.w * quat.x;
    float yy = quat.y * quat.y;
    float zy = quat.z * quat.y;
    float wy = quat.w * quat.y;
    float zz = quat.z * quat.z;
    float wz = quat.w * quat.z;
    float ww = quat.w * quat.w;

    return kexVec3(
        ((yx + yx) - (wz + wz)) * y +
        ((wy + wy + zx + zx)) * z +
        (((ww + xx) - yy) - zz) * x,
        ((yy + (ww - xx)) - zz) * y +
        ((zy + zy) - (wx + wx)) * z +
        ((wz + wz) + (yx + yx)) * x,
        ((wx + wx) + (zy + zy)) * y +
        (((ww - xx) - yy) + zz) * z +
        ((zx + zx) - (wy + wy)) * x
    );
}

//
// kexVec3::operator|
//

kexVec3 kexVec3::operator|(const kexMatrix &mtx) {
    return kexVec3(
        mtx.vectors[1].x * y + mtx.vectors[2].x * z + mtx.vectors[0].x * x + mtx.vectors[3].x,
        mtx.vectors[1].y * y + mtx.vectors[2].y * z + mtx.vectors[0].y * x + mtx.vectors[3].y,
        mtx.vectors[1].z * y + mtx.vectors[2].z * z + mtx.vectors[0].z * x + mtx.vectors[3].z);
}

//
// kexVec3::operator|=
//

kexVec3 &kexVec3::operator|=(const kexQuat &quat) {
    float xx = quat.x * quat.x;
    float yx = quat.y * quat.x;
    float zx = quat.z * quat.x;
    float wx = quat.w * quat.x;
    float yy = quat.y * quat.y;
    float zy = quat.z * quat.y;
    float wy = quat.w * quat.y;
    float zz = quat.z * quat.z;
    float wz = quat.w * quat.z;
    float ww = quat.w * quat.w;
    float vx = x;
    float vy = y;
    float vz = z;

    x = ((yx + yx) - (wz + wz)) * vy +
        ((wy + wy + zx + zx)) * vz +
        (((ww + xx) - yy) - zz) * vx;
    y = ((yy + (ww - xx)) - zz) * vy +
        ((zy + zy) - (wx + wx)) * vz +
        ((wz + wz) + (yx + yx)) * vx;
    z = ((wx + wx) + (zy + zy)) * vy +
        (((ww - xx) - yy) + zz) * vz +
        ((zx + zx) - (wy + wy)) * vx;

    return *this;
}

//
// kexVec3::operator|=
//

kexVec3 &kexVec3::operator|=(const kexMatrix &mtx) {
    float _x = x;
    float _y = y;
    float _z = z;
    
    x = mtx.vectors[1].x * _y + mtx.vectors[2].x * _z + mtx.vectors[0].x * _x + mtx.vectors[3].x;
    y = mtx.vectors[1].y * _y + mtx.vectors[2].y * _z + mtx.vectors[0].y * _x + mtx.vectors[3].y;
    z = mtx.vectors[1].z * _y + mtx.vectors[2].z * _z + mtx.vectors[0].z * _x + mtx.vectors[3].z;

    return *this;
}

//
// kexVec3::operator=
//

kexVec3 &kexVec3::operator=(const kexVec3 &vec) {
    x = vec.x;
    y = vec.y;
    z = vec.z;
    return *this;
}

//
// kexVec3::operator=
//

kexVec3 &kexVec3::operator=(const float *vecs) {
    x = vecs[0];
    y = vecs[1];
    z = vecs[2];
    return *this;
}

//
// kexVec3::operator[]
//

float kexVec3::operator[](int index) const {
    assert(index >= 0 && index < 3);
    return (&x)[index];
}

//
// kexVec3::operator[]
//

float kexVec3::operator[](int index) {
    assert(index >= 0 && index < 3);
    return (&x)[index];
}

//
// kexVec3::ObjectConstruct1
//

void kexVec3::ObjectConstruct1(kexVec3 *thisvec) {
    new(thisvec)kexVec3();
}

//
// kexVec3::ObjectConstruct2
//

void kexVec3::ObjectConstruct2(float x, float y, float z, kexVec3 *thisvec) {
    new(thisvec)kexVec3(x, y, z);
}

//
// kexVec3::ObjectConstructCopy
//

void kexVec3::ObjectConstructCopy(const kexVec3 &in, kexVec3 *thisvec) {
    new(thisvec)kexVec3(in);
}

//
// kexVec4::kexVec4
//

kexVec4::kexVec4(void) {
    Clear();
}

//
// kexVec4::kexVec4
//

kexVec4::kexVec4(const float x, const float y, const float z, const float w) {
    Set(x, y, z, w);
}

//
// kexVec4::Set
//

void kexVec4::Set(const float x, const float y, const float z, const float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

//
// kexVec4::Clear
//

void kexVec4::Clear(void) {
    x = y = z = w = 0.0f;
}

//
// kexVec4::ToVec3
//

kexVec3 const &kexVec4::ToVec3(void) const {
    return *reinterpret_cast<const kexVec3*>(this);
}

//
// kexVec4::ToVec3
//

kexVec3 &kexVec4::ToVec3(void) {
    return *reinterpret_cast<kexVec3*>(this);
}

//
// kexVec4::ToFloatPtr
//

float *kexVec4::ToFloatPtr(void) {
    return reinterpret_cast<float*>(&x);
}

//
// kexVec4::operator[]
//

float kexVec4::operator[](int index) const {
    assert(index >= 0 && index < 3);
    return (&x)[index];
}

//
// kexVec4::operator[]
//

float kexVec4::operator[](int index) {
    assert(index >= 0 && index < 3);
    return (&x)[index];
}
