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

#ifndef __AREAS_H__
#define __AREAS_H__

#include "common.h"
#include "keymap.h"
#include "scriptAPI/component.h"

typedef enum {
    AAF_WATER           = 0x1,
    AAF_BLOCK           = 0x2,
    AAF_TOGGLE          = 0x4,
    AAF_FRONTNOCLIP     = 0x8,
    AAF_CLIMB           = 0x10,
    AAF_ONESIDED        = 0x20,
    AAF_CHECKHEIGHT     = 0x40,
    AAF_CRAWL           = 0x80,
    AAF_ENTERCRAWL      = 0x100,
    AAF_TOUCH           = 0x200,
    AAF_UNKNOWN400      = 0x400,
    AAF_UNKNOWN800      = 0x800,
    AAF_UNKNOWN1000     = 0x1000,
    AAF_SLOPETEST       = 0x2000,
    AAF_DEATHPIT        = 0x4000,
    AAF_MAPPED          = 0x8000,
    AAF_EVENT           = 0x10000,
    AAF_REPEATABLE      = 0x20000,
    AAF_TELEPORT        = 0x40000,
    AAF_DAMAGE          = 0x80000,
    AAF_DRAWSKY         = 0x100000,
    AAF_WARP            = 0x200000,
    AAF_UNKNOWN400000   = 0x400000,
    AAF_UNKNOWN800000   = 0x800000,
    AAF_UNKNOWN1000000  = 0x1000000,
    AAF_UNKNOWN2000000  = 0x2000000,
    AAF_CHECKPOINT      = 0x4000000,
    AAF_SAVEGAME        = 0x8000000
} areaFlags_t;

typedef enum {
    WLT_INVALID  = 0,
    WLT_OVER     = 1,
    WLT_BETWEEN  = 2,
    WLT_UNDER    = 3
} waterLevelType_t;

BEGIN_EXTENDED_CLASS(kexArea, kexObject);
public:
                                    kexArea(void);
                                    ~kexArea(void);

    void                            Setup(void);
    void                            Enter(void);
    waterLevelType_t                GetWaterLevel(const kexVec3 &origin, const float height);

    unsigned int                    &Flags(void) { return flags; }
    float                           &WaterPlane(void) { return waterplane; }
    int                             &TargetID(void) { return targetID; }
    kexVec3                         &FogRGB(void) { return globalFogRGB; }
    float                           &FogZFar(void) { return globalFogZFar; }
    word                            FloorSurfaceType(void) { return fSurfaceID; }
    word                            CeilingSurfaceType(void) { return cSurfaceID; }
    word                            WallSurfaceType(void) { return wSurfaceID; }
    const unsigned int              WorldID(void) const { return worldID; }

    static void                     InitObject(void);
    static unsigned int             id;

    kexKeyMap                       keyMap;
    kexAreaComponent                scriptComponent;

private:
    unsigned int                    flags;
    float                           waterplane;
    int                             targetID;
    char                            *triggerSound;
    word                            fSurfaceID;
    word                            cSurfaceID;
    word                            wSurfaceID;
    kexVec3                         globalFogRGB;
    float                           globalFogZFar;
    unsigned int                    worldID;
};

#endif
