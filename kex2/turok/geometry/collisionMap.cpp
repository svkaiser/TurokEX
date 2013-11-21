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

#ifndef EDITOR
#include "common.h"
#else
#include "editorCommon.h"
#endif
#include "binFile.h"
#include "fileSystem.h"
#include "collisionMap.h"

#define CM_ID_HEADER    0
#define CM_ID_DATASIZE  1
#define CM_ID_POINTS    2
#define CM_ID_SECTORS   3

kexHeapBlock kexCollisionMap::hb_collisionMap("collision map", false, NULL, NULL);

//
// kexCollisionSector::kexCollisionSector
//

kexCollisionSector::kexCollisionSector(void) {
    this->link[0] = NULL;
    this->link[1] = NULL;
    this->link[2] = NULL;
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
    this->bLoaded = false;
    this->points[0] = NULL;
    this->points[1] = NULL;
    this->sectors = NULL;
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
    kexBinFile binFile;
    kexCollisionSector *sec;
    float *pointPtrs;
    int pt[3];
    int edge[3];
    int i;
    int j;

    if(!binFile.Open(name)) {
        common.Warning("kexCollisionMap::Load: %s not found\n", name);
        return;
    }

    pointPtrs = (float*)binFile.GetOffset(CM_ID_POINTS, NULL, &numPoints);
    binFile.GetOffset(CM_ID_SECTORS, NULL, &numSectors);

    if(numSectors <= 0) {
        binFile.Close();
        return;
    }

    points[0] = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * numPoints,
        kexCollisionMap::hb_collisionMap);
    points[1] = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * numPoints,
        kexCollisionMap::hb_collisionMap);
    indices = (word*)Mem_Calloc((sizeof(word) * numSectors) * 3,
        kexCollisionMap::hb_collisionMap);

    for(i = 0; i < numPoints; i++) {
        points[0][i].Set(
            pointPtrs[i * 4 + 0],
            pointPtrs[i * 4 + 1],
            pointPtrs[i * 4 + 2]);
        points[1][i].Set(
            pointPtrs[i * 4 + 0],
            pointPtrs[i * 4 + 3],
            pointPtrs[i * 4 + 2]);
    }

    sectors = (kexCollisionSector*)Mem_Calloc(sizeof(kexCollisionSector) * numSectors,
        kexCollisionMap::hb_collisionMap);

    for(i = 0; i < numSectors; i++) {
        sec = &sectors[i];

        sec->area_id = binFile.Read16();
        sec->flags = binFile.Read16();

        for(j = 0; j < 3; j++) {
            pt[j] = binFile.Read16();
        }
        for(j = 0; j < 3; j++) {
            edge[j] = binFile.Read16();
        }
        for(j = 0; j < 3; j++) {
            sec->lowerTri.point[j] = reinterpret_cast<kexVec3*>(&points[0][pt[j]]);
            sec->upperTri.point[j] = reinterpret_cast<kexVec3*>(&points[1][pt[j]]);
            sec->lowerTri.edgeLink[j] = (edge[j] != -1) ? &sectors[edge[j]].lowerTri : NULL;
            sec->upperTri.edgeLink[j] = (edge[j] != -1) ? &sectors[edge[j]].upperTri : NULL;
            sec->link[j] = (edge[j] != -1) ? &sectors[edge[j]] : NULL;
        }

        indices[i * 3 + 0] = pt[0];
        indices[i * 3 + 1] = pt[1];
        indices[i * 3 + 2] = pt[2];

        // build plane for lower triangle
        sec->lowerTri.plane.SetNormal(
            *sec->lowerTri.point[0],
            *sec->lowerTri.point[1],
            *sec->lowerTri.point[2]);
        sec->lowerTri.plane.SetDistance(*sec->lowerTri.point[0]);
        sec->lowerTri.SetBounds();

        // build plane for upper triangle
        sec->upperTri.plane.SetNormal(
            *sec->upperTri.point[0],
            *sec->upperTri.point[1],
            *sec->upperTri.point[2]);
        sec->upperTri.plane.SetDistance(*sec->upperTri.point[0]);
        sec->upperTri.SetBounds();
    }

    binFile.Close();
    bLoaded = true;
}
