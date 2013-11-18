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
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "fileSystem.h"
#include "collisionMap.h"

kexHeapBlock kexCollisionMap::hb_collisionMap("collision map", false, NULL, NULL);

//
// kexCollisionSector::kexCollisionSector
//

kexCollisionSector::kexCollisionSector(void) {
}

//
// kexCollisionSector::Wall
//

bool kexCollisionSector::Wall(void) {
    if(!(flags & CLF_SLOPETEST)) {
        return (flags & CLF_FRONTNOCLIP) != 0;
    }

    return (lowerTri.plane.b <= 0.5f);
}

//
// kexCollisionSector::kexCollisionSector
//

bool kexCollisionSector::InRange(const float x, const float z) {
    bool s1;
    bool s2;
    bool s3;

#define M_POINTONSIDE(v1, v2, v3, x, z) \
    ((x - v2[0])        *   \
    (v3[2] - v2[2])     -   \
    (v3[0] - v2[0])     *   \
    (z - v2[2]))        *   \
    ((v1[0] - v2[0])    *   \
    (v3[2] - v2[2])     -   \
    (v3[0] - v2[0])     *   \
    (v1[2] - v2[2]))

    s1 = M_POINTONSIDE(*lowerTri.point[0], *lowerTri.point[1], *lowerTri.point[2], x, z) >= 0;
    s2 = M_POINTONSIDE(*lowerTri.point[1], *lowerTri.point[0], *lowerTri.point[2], x, z) >= 0;
    s3 = M_POINTONSIDE(*lowerTri.point[2], *lowerTri.point[0], *lowerTri.point[1], x, z) >= 0;

#undef M_POINTONSIDE

    return (s1 && s2 && s3);
}

//
// kexCollisionMap::kexCollisionMap
//

kexCollisionMap::kexCollisionMap(void) {
}

//
// kexCollisionMap::~kexCollisionMap
//

kexCollisionMap::~kexCollisionMap(void) {
}

//
// kexCollisionMap::Load
//

void kexCollisionMap::Load(const char *name) {
    int buffsize;
    int *rover;
    int pointOffset;
    int planeOffset;
    int numPlanes;

    buffsize = fileSystem.ReadExternalTextFile(name, (byte**)(&data));
    if(buffsize <= 0) {
        buffsize = fileSystem.OpenFile(name, (byte**)(&data), hb_collisionMap);
    }

    if(buffsize <= 0) {
        common.Warning("kexCollisionMap::Load: %s not found\n", name);
        return;
    }

    rover = (int*)data;
    pointOffset = rover[2];
    planeOffset = rover[3];

    points = (kexVec4*)((data + pointOffset) + 4);
    numPlanes = *(int*)(data + planeOffset);
}
