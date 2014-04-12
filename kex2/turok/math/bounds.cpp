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
// DESCRIPTION: Bounding box operations
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "mathlib.h"

//
// kexBBox::kexBBox
//

kexBBox::kexBBox(void) {
    Clear();
}

//
// kexBBox::kexBBox
//

kexBBox::kexBBox(const kexVec3 &vMin, const kexVec3 &vMax) {
    this->min = vMin;
    this->max = vMax;
}

//
// kexBBox::Clear
//

void kexBBox::Clear(void) {
    min.Clear();
    max.Clear();
}

//
// kexBBox::Center
//

kexVec3 kexBBox::Center(void) const {
    return kexVec3(
        (max.x + min.x) * 0.5f,
        (max.y + min.y) * 0.5f,
        (max.z + min.z) * 0.5f);
}

//
// kexBBox::Radius
//

float kexBBox::Radius(void) const {
    int i;
    float r = 0;
    float r1;
    float r2;

    for(i = 0; i < 3; i++) {
        r1 = kexMath::Fabs(min[i]);
        r2 = kexMath::Fabs(max[i]);

        if(r1 > r2) {
            r += r1 * r1;
        }
        else {
            r += r2 * r2;
        }
    }

    return kexMath::Sqrt(r);
}

//
// kexBBox::PointInside
//

bool kexBBox::PointInside(const kexVec3 &vec) const {
    return !(vec[0] < min[0] || vec[1] < min[1] || vec[2] < min[2] ||
             vec[0] > max[0] || vec[1] > max[1] || vec[2] > max[2]);
}

//
// kexBBox::IntersectingBox
//

bool kexBBox::IntersectingBox(const kexBBox &box) const {
    return !(box.max[0] < min[0] || box.max[1] < min[1] || box.max[2] < min[2] ||
             box.min[0] > max[0] || box.min[1] > max[1] || box.min[2] > max[2]);
}

//
// kexBBox::IntersectingBox2D
//

bool kexBBox::IntersectingBox2D(const kexBBox &box) const {
    return !(box.max[0] < min[0] || box.max[2] < min[2] ||
             box.min[0] > max[0] || box.min[2] > max[2]);
}

//
// kexBBox::DistanceToPlane
//

float kexBBox::DistanceToPlane(kexPlane &plane) {
    kexVec3 c;
    float distStart;
    float distEnd;
    float dist = 0;
    
    c = Center();
    
    distStart = plane.Distance(c);
    distEnd = kexMath::Fabs((max.x - c.x) * plane.a) +
              kexMath::Fabs((max.y - c.y) * plane.b) +
              kexMath::Fabs((max.z - c.z) * plane.c);

    dist = distStart - distEnd;
    
    if(dist > 0) {
        // in front
        return dist;
    }
    
    dist = distStart + distEnd;
    
    if(dist < 0) {
        // behind
        return dist;
    }
    
    return 0;
}

//
// kexBBox::operator+
//

kexBBox kexBBox::operator+(const float radius) const {
    kexVec3 vmin = min;
    kexVec3 vmax = max;

    vmin.x -= radius;
    vmin.y -= radius;
    vmin.z -= radius;

    vmax.x += radius;
    vmax.y += radius;
    vmax.z += radius;

    return kexBBox(vmin, vmax);
}

//
// kexBBox::operator+=
//

kexBBox &kexBBox::operator+=(const float radius) {
    min.x -= radius;
    min.y -= radius;
    min.z -= radius;
    max.x += radius;
    max.y += radius;
    max.z += radius;
    return *this;
}

//
// kexBBox::operator+
//

kexBBox kexBBox::operator+(const kexVec3 &vec) const {
    kexVec3 vmin = min;
    kexVec3 vmax = max;

    vmin.x += vec.x;
    vmin.y += vec.y;
    vmin.z += vec.z;

    vmax.x += vec.x;
    vmax.y += vec.y;
    vmax.z += vec.z;

    return kexBBox(vmin, vmax);
}

//
// kexBBox::operator-
//

kexBBox kexBBox::operator-(const float radius) const {
    kexVec3 vmin = min;
    kexVec3 vmax = max;

    vmin.x += radius;
    vmin.y += radius;
    vmin.z += radius;

    vmax.x -= radius;
    vmax.y -= radius;
    vmax.z -= radius;

    return kexBBox(vmin, vmax);
}

//
// kexBBox::operator-
//

kexBBox kexBBox::operator-(const kexVec3 &vec) const {
    kexVec3 vmin = min;
    kexVec3 vmax = max;

    vmin.x -= vec.x;
    vmin.y -= vec.y;
    vmin.z -= vec.z;

    vmax.x -= vec.x;
    vmax.y -= vec.y;
    vmax.z -= vec.z;

    return kexBBox(vmin, vmax);
}

//
// kexBBox::operator-=
//

kexBBox &kexBBox::operator-=(const float radius) {
    min.x += radius;
    min.y += radius;
    min.z += radius;
    max.x -= radius;
    max.y -= radius;
    max.z -= radius;
    return *this;
}

//
// kexBBox::operator|
//

kexBBox kexBBox::operator|(const kexMatrix &matrix) const {
    kexVec3 c  = Center();
    kexVec3 ct = c | matrix;
    
    kexMatrix mtx(matrix);
    
    for(int i = 0; i < 3; i++) {
        mtx.vectors[i].x = kexMath::Fabs(mtx.vectors[i].x);
        mtx.vectors[i].y = kexMath::Fabs(mtx.vectors[i].y);
        mtx.vectors[i].z = kexMath::Fabs(mtx.vectors[i].z);
    }
    
    kexVec3 ht = (max - c) | mtx;
    
    return kexBBox(ct - ht, ct + ht);
}

//
// kexBBox::operator|=
//

kexBBox &kexBBox::operator|=(const kexMatrix &matrix) {
    kexVec3 c  = Center();
    kexVec3 ct = c | matrix;
    
    kexMatrix mtx(matrix);
    
    for(int i = 0; i < 3; i++) {
        mtx.vectors[i].x = kexMath::Fabs(mtx.vectors[i].x);
        mtx.vectors[i].y = kexMath::Fabs(mtx.vectors[i].y);
        mtx.vectors[i].z = kexMath::Fabs(mtx.vectors[i].z);
    }
    
    kexVec3 ht = (max - c) | mtx;
    
    min = (ct - ht);
    max = (ct + ht);
    
    return *this;
}

//
// kexBBox::operator|
//

kexBBox kexBBox::operator|(const kexVec3 &vec) const {
    kexBBox box = *this;

    if(vec.x < 0) box.min.x += (vec.x-1); else box.max.x += (vec.x+1);
    if(vec.y < 0) box.min.y += (vec.y-1); else box.max.y += (vec.y+1);
    if(vec.z < 0) box.min.z += (vec.z-1); else box.max.z += (vec.z+1);

    return box;
}

//
// kexBBox::operator|=
//

kexBBox &kexBBox::operator|=(const kexVec3 &vec) {
    if(vec.x < 0) min.x += (vec.x-1); else max.x += (vec.x+1);
    if(vec.y < 0) min.y += (vec.y-1); else max.y += (vec.y+1);
    if(vec.z < 0) min.z += (vec.z-1); else max.z += (vec.z+1);

    return *this;
}

//
// kexBBox::operator=
//

kexBBox &kexBBox::operator=(const kexBBox &bbox) {
    min = bbox.min;
    max = bbox.max;

    return *this;
}

//
// kexBBox::operator[]
//

kexVec3 kexBBox::operator[](int index) const {
    assert(index >= 0 && index < 2);
    return index == 0 ? min : max;
}

//
// kexBBox::operator[]
//

kexVec3 &kexBBox::operator[](int index) {
    assert(index >= 0 && index < 2);
    return index == 0 ? min : max;
}

//
// kexBBox:LineIntersect
//

bool kexBBox::LineIntersect(const kexVec3 &start, const kexVec3 &end) {
    float ld[3];
    kexVec3 center = Center();
    kexVec3 extents = max - center;
    kexVec3 lineDir = (end - start) * 0.5f;
    kexVec3 lineCenter = lineDir + start;
    kexVec3 dir = lineCenter - center;

    ld[0] = kexMath::Fabs(lineDir[0]);
    if(kexMath::Fabs(dir[0]) > extents[0] + ld[0]) return false;
    ld[1] = kexMath::Fabs(lineDir[1]);
    if(kexMath::Fabs(dir[1]) > extents[1] + ld[1]) return false;
    ld[2] = kexMath::Fabs(lineDir[2]);
    if(kexMath::Fabs(dir[2]) > extents[2] + ld[2]) return false;

    kexVec3 cross = lineDir.Cross(dir);

    if(kexMath::Fabs(cross[0]) > extents[1] * ld[2] + extents[2] * ld[1]) return false;
    if(kexMath::Fabs(cross[1]) > extents[0] * ld[2] + extents[2] * ld[0]) return false;
    if(kexMath::Fabs(cross[2]) > extents[0] * ld[1] + extents[1] * ld[0]) return false;

    return true;
}
