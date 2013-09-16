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
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "triangle.h"

//
// kexTri::kexTri
//

kexTri::kexTri(void) {
    for(int i = 0; i < 3; i++) {
        point[i] = NULL;
        edgeLink[i] = NULL;
        bEdgeBlock[i] = true;
    }
}

//
// kexTri::SetPoint
//

void kexTri::SetPoint(const int idx, float *p) {
    point[idx] = reinterpret_cast<kexVec3*>(p);
}

//
// kexTri::GetDistance
//

const float kexTri::GetDistance(const kexVec3 &pos) const {
    float dist = 0;
    kexVec3 n = plane.Normal();
    
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
    float d = (float)sqrt(x * x + z * z);
    
    if(d != 0) {
        float an = (float)acos(x / d);
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
