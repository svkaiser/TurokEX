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
#include "areas.h"

typedef enum {
    CLF_WATER           = BIT(0),
    CLF_BLOCK           = BIT(1),
    CLF_TOGGLE          = BIT(2),
    CLF_FRONTNOCLIP     = BIT(3),
    CLF_CLIMB           = BIT(4),
    CLF_ONESIDED        = BIT(5),
    CLF_CHECKHEIGHT     = BIT(6),
    CLF_CRAWL           = BIT(7),
    CLF_ENTERCRAWL      = BIT(8),
    CLF_HIDDEN          = BIT(9),
    CLF_UNKNOWN1024     = BIT(10),
    CLF_UNKNOWN2048     = BIT(11),
    CLF_UNKNOWN4096     = BIT(12),
    CLF_SLOPETEST       = BIT(13),
    CLF_ENDLESSPIT      = BIT(14),
    CLF_MAPPED          = BIT(15),
    CLF_SOLID           = BIT(16)
} cMapFlags_t;

typedef enum {
    PF_CLIPEDGES        = BIT(0),
    PF_IGNOREBLOCKERS   = BIT(1),
    PF_DROPOFF          = BIT(2),
    PF_NOENTERWATER     = BIT(3),
    PF_NOEXITWATER      = BIT(4),
    PF_NOCLIPSTATICS    = BIT(5),
    PF_NOCLIPACTORS     = BIT(6)
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
    float                           height;
    cMapTraceResult_t               *result;
} cMapTrace_t;

class kexSector {
public:
                                    kexSector(void);

    bool                            Wall(void);
    bool                            InRange(const kexVec3 &origin);
    bool                            CheckHeight(const kexVec3 &pos);
    bool                            IntersectEdge(cMapTrace_t *trace, const int edgeNum);
    bool                            Trace(cMapTrace_t *trace, const bool bTestCeiling);
    kexSector                       *CrossEdge(cMapTrace_t *trace, const int edge);
    const int                       GetID(void) const;

    kexArea                         *area;
    unsigned int                    flags;
    kexTri                          lowerTri;
    kexTri                          upperTri;
    kexSector                       *link[3];
    bool                            bTraced;

    kexArray<kexSector*>            stacks;
};

class kexCollisionMap {
public:
                                    kexCollisionMap(void);
                                    ~kexCollisionMap(void);

    void                            Load(const char *name);
    void                            Unload(void);
    void                            Trace(cMapTraceResult_t *result,
                                          const kexVec3 &start, const kexVec3 &end,
                                          kexSector *sector,
                                          const int flags,
                                          const float height = 0);
    void                            TraverseSectors(cMapTrace_t *trace, kexSector *sector);
    kexSector                       *PointInSector(const kexVec3 &origin);
    void                            PlayerCrossAreas(kexSector *enter, kexSector *exit);
    void                            RecursiveChangeHeight(kexSector *sector, float destHeight,
                                        unsigned int areaID);
    void                            ToggleBlock(const kexVec3 pos, bool bToggle);

    kexVec3                         *points[2];
    word                            *indices;
    int                             numSectors;
    int                             numAreas;
    int                             numPoints;
    kexSector                       *sectors;
    kexArray<kexArea*>              areas;

    const bool                      IsLoaded(void) const { return bLoaded; }

    static kexHeapBlock             hb_collisionMap;

private:
    void                            SetupSectorStackList(void);
    void                            RecursiveToggleBlock(kexSector *sector, bool bToggle,
                                        unsigned int areaID);

    bool                            bLoaded;
};

#endif
