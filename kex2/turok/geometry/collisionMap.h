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
    CLF_SOLID           = 0x10000
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

class kexSector;

typedef struct {
    kexSector                       *sector;
    kexSector                       *contactSector;
    kexVec3                         position;
    kexVec3                         normal;
    float                           fraction;
    bool                            bClippedEdge;
} cMapTraceResult_t;

typedef struct {
    kexVec3                         start;
    kexVec3                         end;
    kexVec3                         direction;
    kexSector                       *sector;
    int                             flags;
    cMapTraceResult_t               *result;
} cMapTrace_t;

typedef struct {
    unsigned int                    flags;
    float                           waterplane;
    unsigned int                    targetID;
    char                            *triggerSound;
    unsigned short                  fSurfaceID;
    unsigned short                  cSurfaceID;
    unsigned short                  wSurfaceID;
} cMapArea_t;

class kexSector {
public:
                                    kexSector(void);

    bool                            Wall(void);
    bool                            InRange(const kexVec3 &origin);
    bool                            CheckHeight(const kexVec3 &pos);
    bool                            IntersectEdge(cMapTrace_t *trace, const int edgeNum);
    bool                            Trace(cMapTrace_t *trace);
    kexSector                       *CrossEdge(cMapTrace_t *trace, const int edge);

    cMapArea_t                      *area;
    unsigned int                    flags;
    kexTri                          lowerTri;
    kexTri                          upperTri;
    kexSector                       *link[3];
    bool                            bTraced;
};

class kexCollisionMap {
public:
                                    kexCollisionMap(void);
                                    ~kexCollisionMap(void);

    void                            Load(const char *name);
    void                            Trace(cMapTraceResult_t *result,
                                          const kexVec3 &start, const kexVec3 &end,
                                          kexSector *sector,
                                          const int flags);
    void                            TraverseSectors(cMapTrace_t *trace, kexSector *sector);
    kexSector                       *PointInSector(const kexVec3 &origin);
    void                            DebugDraw(void);

    kexVec3                         *points[2];
    word                            *indices;
    int                             numSectors;
    int                             numPoints;
    kexSector                       *sectors;
    cMapArea_t                      *areas;

    const bool                      IsLoaded(void) const { return bLoaded; }

    static kexHeapBlock             hb_collisionMap;

private:
    bool                            bLoaded;
};

#endif
