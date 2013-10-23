// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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
// DESCRIPTION: Triangle class
//              Triangles are represented as three vector points
//              and a plane. Each edge of a kexTri may contain a pointer
//              that links to another triangle, which is useful for
//              collision detection.
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "triangle.h"

int kexTri::globalID = 0;

//
// kexTri::kexTri
//

kexTri::kexTri(void) {
    for(int i = 0; i < 3; i++) {
        this->point[i] = NULL;
        this->edgeLink[i] = NULL;
        this->bEdgeBlock[i] = true;
    }

    this->id = kexTri::globalID++;
    this->bTraced = false;
}

//
// kexTri::SetPoint
// assumes that *p contains 3 floats
//

void kexTri::SetPoint(const int idx, float *p) {
    point[idx] = reinterpret_cast<kexVec3*>(p);
}

//
// kexTri::SetBounds
//

void kexTri::SetBounds(void) {
    float lowx = M_INFINITY;
    float lowy = M_INFINITY;
    float lowz = M_INFINITY;
    float hix = -M_INFINITY;
    float hiy = -M_INFINITY;
    float hiz = -M_INFINITY;

    bounds.min.Clear();
    bounds.max.Clear();

    for(int i = 0; i < 3; i++) {
        if(point[i]->x < lowx) lowx = point[i]->x;
        if(point[i]->y < lowy) lowy = point[i]->y;
        if(point[i]->z < lowz) lowz = point[i]->z;
        if(point[i]->x > hix) hix = point[i]->x;
        if(point[i]->y > hiy) hiy = point[i]->y;
        if(point[i]->z > hiz) hiz = point[i]->z;
    }

    bounds.min.Set(lowx, lowy, lowz);
    bounds.max.Set(hix, hiy, hiz);
}

//
// kexTri::PointInRange
//

bool kexTri::PointInRange(const kexVec3 &pt, const float expand) {
    float rSq = expand * expand;
    float eSq;
    kexVec3 cp;
    kexVec3 pt1;
    kexVec3 pt2;
    kexVec3 pt3;
    kexVec3 dp1;
    kexVec3 dp2;
    kexVec3 edge;

    for(int i = 0; i < 3; i++) {
        pt1 = *point[(i+0)%3];
        pt2 = *point[(i+1)%3];
        pt3 = *point[(i+2)%3];

        dp1 = pt1 - pt;
        dp2 = pt2 - pt;

        cp = dp1.Cross(dp2);

        if(plane.Normal().Dot(cp) >= 0) {
            continue;
        }

        if(expand == 0) {
            return false;
        }

        edge = pt1 - pt2;
        eSq = edge.UnitSq();

        if(cp.UnitSq() > eSq * rSq) {
            return false;
        }

        float d = edge.Dot(dp1);

        if(d < 0) {
            edge = pt1 - pt3;
            if(edge.Dot(dp1) < 0 && dp1.UnitSq() > rSq) {
                return false;
            }
        }
        else if(d > eSq) {
            edge = pt2 - pt3;
            if(edge.Dot(dp2) < 0 && dp2.UnitSq() > rSq) {
                return false;
            }
        }
    }

    return true;
}

//
// kexTri::GetDistance
//

const float kexTri::GetDistance(const kexVec3 &pos) const {
    float dist = 0;
    kexVec3 n = plane.Normal();
    
    //
    // if the plane is standing straight up (aka its a wall) then
    // get the sum of the height from all 3 points
    //
    if(n.y == 0) {
        dist = (
            point[0]->y +
            point[1]->y +
            point[2]->y) / 3;
    }
    else {
        dist = kexVec3::Dot(kexVec3(
            point[0]->x - pos.x,
            point[0]->y,
            point[0]->z - pos.z), n) / n.y;
    }
    
    return dist;
}

//
// kexTri::GetEdgeYaw
//

const float kexTri::GetEdgeYaw(const int idx) const {
    float x = point[(idx+1)%3]->x - point[idx]->x;
    float z = point[(idx+1)%3]->z - point[idx]->z;
    float d = kexMath::Sqrt(x * x + z * z);
    
    if(d != 0) {
        float an = kexMath::ACos(x / d);
        if(z >= 0) {
            an = -an;
        }
        
        return an;
    }
    
    return 0;
}

//
// kexTri::GetCenterPoint
//

const kexVec3 kexTri::GetCenterPoint(void) const {
    return kexVec3(
        (point[0]->x + point[1]->x + point[2]->x) / 3,
        (point[0]->y + point[1]->y + point[2]->y) / 3,
        (point[0]->z + point[1]->z + point[2]->z) / 3);
}
