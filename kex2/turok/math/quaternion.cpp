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
// DESCRIPTION: Quaternion operations
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "common.h"
#include "mathlib.h"

//
// kexQuat::kexQuat
//

kexQuat::kexQuat() {
    Clear();
}

//
// kexQuat::kexQuat
//

kexQuat::kexQuat(const float angle, const float x, const float y, const float z) {
    float s = (float)sin(angle * 0.5f);
    float c = (float)cos(angle * 0.5f);

    this->x = x * s;
    this->y = y * s;
    this->z = z * s;
    this->w = c;
}

//
// kexQuat::kexQuat
//

kexQuat::kexQuat(const float angle, kexVec3 &vector) {
    float s = (float)sin(angle * 0.5f);
    float c = (float)cos(angle * 0.5f);

    this->x = vector.x * s;
    this->y = vector.y * s;
    this->z = vector.z * s;
    this->w = c;
}

//
// kexQuat::Set
//

void kexQuat::Set(const float x, const float y, const float z, const float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

//
// kexQuat::Clear
//

void kexQuat::Clear(void) {
    x = y = z = 0.0f;
    w = 1.0f;
}

//
// kexVec3::UnitSq
//

float kexQuat::UnitSq(void) const {
    return x * x + y * y + z * z + w * w;
}

//
// kexVec3::Unit
//

float kexQuat::Unit(void) const {
    return (float)sqrt(UnitSq());
}

//
// kexQuat::Normalize
//

kexQuat &kexQuat::Normalize(void) {
    float d = Unit();
    if(d != 0.0f) {
        d = 1.0f / d;
        *this *= d;
    }
    return *this;
}

//
// kexQuat::operator-
//

kexQuat kexQuat::operator-(void) const {
    kexQuat out;
    out.Set(-x, -y, -z, -w);
    return out;
}

//
// kexQuat::operator+
//

kexQuat kexQuat::operator+(const kexQuat &quat) {
    kexQuat out;
    out.x = x + quat.x;
    out.y = y + quat.y;
    out.z = z + quat.z;
    out.w = w + quat.w;
    return out;
}

//
// kexQuat::operator+=
//

kexQuat &kexQuat::operator+=(const kexQuat &quat) {
    x += quat.x;
    y += quat.y;
    z += quat.z;
    w += quat.w;
    return *this;
}

//
// kexQuat::operator-
//

kexQuat kexQuat::operator-(const kexQuat &quat) {
    kexQuat out;
    out.x = x - quat.x;
    out.y = y - quat.y;
    out.z = z - quat.z;
    out.w = w - quat.w;
    return out;
}

//
// kexQuat::operator-=
//

kexQuat &kexQuat::operator-=(const kexQuat &quat) {
    x -= quat.x;
    y -= quat.y;
    z -= quat.z;
    w -= quat.w;
    return *this;
}

//
// kexQuat::operator*
//

kexQuat kexQuat::operator*(const kexQuat &quat) {
    kexQuat out;

    out.x = x * quat.w - y * quat.z + quat.x * w + quat.y * z;
    out.y = x * quat.z + y * quat.w - quat.x * z + w * quat.y;
    out.z = quat.x * y + w * quat.z + z * quat.w - x * quat.y;
    out.w = w * quat.w - quat.y * y + z * quat.z + quat.x * x;

    return out;
}

//
// kexQuat::operator*=
//

kexQuat &kexQuat::operator*=(const kexQuat &quat) {
    float tx = x;
    float ty = y;
    float tz = z;
    float tw = w;

    x = tx * quat.w - ty * quat.z + quat.x * tw + quat.y * z;
    y = tx * quat.z + ty * quat.w - quat.x * tz + tw * quat.y;
    z = quat.x * ty + tw * quat.z + tz * quat.w - tx * quat.y;
    w = tw * quat.w - quat.y * ty + tz * quat.z + quat.x * x;

    return *this;
}

//
// kexQuat::operator*
//

kexQuat kexQuat::operator*(const float val) const {
    kexQuat out;
    out.x = x * val;
    out.y = y * val;
    out.z = z * val;
    out.w = w * val;
    return out;
}

//
// kexQuat::operator*=
//

kexQuat &kexQuat::operator*=(const float val) {
    x *= val;
    y *= val;
    z *= val;
    w *= val;

    return *this;
}

//
// kexQuat::Dot
//

float kexQuat::Dot(const kexQuat &quat) const {
    return (x * quat.x + y * quat.y + z * quat.z + w * quat.w);
}

//
// kexQuat::Slerp
//

kexQuat kexQuat::Slerp(const kexQuat &quat, float movement) const {
    kexQuat rdest = quat;
    float d1 = Dot(quat);
    float d2 = Dot(-quat);

    if(d1 < d2) {
        rdest = -quat;
        d1 = d2;
    }

    if(d1 <= 0.7071067811865001f) {
        float halfcos = (float)acos(d1);
        float halfsin = (float)sin(halfcos);

        if(halfsin == 0) {
            kexQuat out;
            out.Set(x, y, z, w);
            return out;
        }
        else {
            float d;
            float ms1;
            float ms2;

            d = 1.0f / halfsin;
            ms1 = (float)sin((1.0f - movement) * halfcos) * d;
            ms2 = (float)sin(halfcos * movement) * d;

            if(ms2 < 0) {
                rdest = -quat;
            }

            return *this * ms1 + rdest * ms2;
        }
    }
    else {
        kexQuat out = (rdest - *this) * movement + *this;
        out.Normalize();
        return out;
    }
}

//
// kexQuat::operator=
//

kexQuat &kexQuat::operator=(const kexQuat &quat) {
    x = quat.x;
    y = quat.y;
    z = quat.z;
    w = quat.w;
    return *this;
}

//
// kexQuat::ObjectConstruct1
//

void kexQuat::ObjectConstruct1(kexQuat *thisq) {
    new(thisq)kexQuat();
}

//
// kexQuat::ObjectConstruct2
//

void kexQuat::ObjectConstruct2(float a, float x, float y, float z, kexVec3 *thisq) {
    new(thisq)kexQuat(a, x, y, z);
}

//
// kexQuat::ObjectConstruct3
//

void kexQuat::ObjectConstruct3(float a, kexVec3 &in, kexQuat *thisq) {
    new(thisq)kexQuat(a, in);
}

//
// kexQuat::ObjectConstructCopy
//

void kexQuat::ObjectConstructCopy(const kexQuat &in, kexQuat *thisq) {
    new(thisq)kexQuat(in);
}
