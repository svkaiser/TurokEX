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

#ifndef __COLLISION_MAP_H__
#define __COLLISION_MAP_H__

#include "mathlib.h"
#include "triangle.h"

typedef enum {
    CLF_WATER           = 0x1,
    CLF_BLOCK           = 0x2,
    CLF_TOGGLE          = 0x4,
    CLF_FRONTNOCLIP     = 0x8,
    CLF_CLIMB           = 0x10,
    CLF_ONESIDED        = 0x20,
    CLF_CHECKHEIGHT     = 0x40,
    CLF_CRAWL           = 0x80,
    CLF_ENTERCRAWL      = 0x100,
    CLF_HIDDEN          = 0x200,
    CLF_UNKNOWN1024     = 0x400,
    CLF_UNKNOWN2048     = 0x800,
    CLF_UNKNOWN4096     = 0x1000,
    CLF_SLOPETEST       = 0x2000,
    CLF_ENDLESSPIT      = 0x4000,
    CLF_MAPPED          = 0x8000,
    CLF_UNKNOWN65536    = 0x10000
} cMapFlags_t;

typedef enum {
    PF_CLIPEDGES        = 0x1,
    PF_IGNOREBLOCKERS   = 0x2,
    PF_CLIMBSURFACES    = 0x4,
    PF_NOSTEPUP         = 0x8,
    PF_DROPOFF          = 0x10,
    PF_NOENTERWATER     = 0x20,
    PF_NOEXITWATER      = 0x40
} cMapClipFlags_t;

class kexCollisionSector {
public:
                                    kexCollisionSector(void);

    bool                            Wall(void);
    bool                            InRange(const float x, const float z);

    unsigned short                  area_id;
    unsigned int                    flags;
    kexTri                          lowerTri;
    kexTri                          upperTri;
    kexCollisionSector              *link[3];
};

class kexCollisionMap {
public:
                                    kexCollisionMap(void);
                                    ~kexCollisionMap(void);

    void                            Load(const char *name);

    kexVec3                         *points[2];
    word                            *indices;
    int                             numSectors;
    int                             numPoints;
    kexCollisionSector              *sectors;

    const bool                      IsLoaded(void) const { return bLoaded; }

    static kexHeapBlock             hb_collisionMap;

private:
    bool                            bLoaded;
};

#endif
