// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "actor.h"

typedef enum
{
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
} collisionflags_t;

typedef enum
{
    AAF_WATER           = 0x1,
    AAF_UNKNOWN2        = 0x2,
    AAF_UNKNOWN4        = 0x4,
    AAF_UNKNOWN8        = 0x8,
    AAF_CLIMB           = 0x10,
    AAF_ONESIDED        = 0x20,
    AAF_REVERB          = 0x40,
    AAF_CRAWL           = 0x80,
    AAF_UNKNOWN100      = 0x100,
    AAF_TOUCH           = 0x200,
    AAF_UNKNOWN400      = 0x400,
    AAF_UNKNOWN800      = 0x800,
    AAF_UNKNOWN1000     = 0x1000,
    AAF_UNKNOWN2000     = 0x2000,
    AAF_DEATHPIT        = 0x4000,
    AAF_UNKNOWN8000     = 0x8000,
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
} areaflags_t;

typedef struct plane_s
{
    unsigned short      area_id;
    unsigned int        flags;
    vec3_t              points[3];
    float               height[3];
    vec3_t              normal;
    vec3_t              ceilingNormal;
    struct plane_s      *link[3];
} plane_t;

typedef struct
{
    vec3_t              origin;
    vec3_t              velocity;
    vec3_t              forward;
    vec3_t              right;
    vec3_t              up;
    vec3_t              accel;
    vec3_t              angles;
    float               moveTime;
    float               frameTime;
    float               timeStamp;
    plane_t             *plane;
    gActor_t            *actor;
} worldState_t;

typedef struct
{
    float               minx;
    float               minz;
    float               maxx;
    float               maxz;
    gActor_t            *statics;
    unsigned int        numStatics;
} gridBounds_t;

typedef struct
{
    unsigned int        flags;
    float               waterplane;
    unsigned int        targetID;
    char                *triggerSound;
    unsigned short      fSurfaceID;
    unsigned short      cSurfaceID;
    unsigned short      wSurfaceID;
    gActor_t            actorRoot;
    gObject_t           *components;
    gObject_t           *iterator;
} gArea_t;

typedef struct
{
    kbool               loaded;
    kbool               bReadyUnload;
    unsigned int        numActors;
    unsigned int        numGridBounds;
    unsigned int        numplanes;
    unsigned int        numAreas;
    gActor_t            actorRoot;
    gActor_t            *actorRover;
    plane_t             *planes;
    gridBounds_t        *gridBounds;
    gArea_t             *areas;
    int                 mapID;
    int                 nextMap;
    char                name[64];
    vec4_t              worldLightOrigin;
    vec4_t              worldLightColor;
    vec4_t              worldLightAmbience;
    vec4_t              worldLightModelAmbience;
    int                 tics;
    float               time;
    float               deltaTime;
} gLevel_t;

extern gLevel_t gLevel;

//
// Map_PlaneToIndex
//

d_inline int Map_PlaneToIndex(plane_t *plane)
{
    return (plane != NULL) ? (plane - gLevel.planes) : -1;
}

//
// Map_IndexToPlane
//

d_inline plane_t *Map_IndexToPlane(int index)
{
    return index != -1 ? &gLevel.planes[index] : NULL;
}

gArea_t *Map_GetArea(plane_t *plane);
kbool Map_CallAreaEvent(gArea_t *area, const char *function, long *args, unsigned int nargs);
void Map_LinkActorToWorld(gActor_t *actor);
void Map_UnlinkActorFromWorld(gActor_t *actor);
int Map_GetWaterLevel(vec3_t origin, float height, plane_t *plane);
plane_t *Map_FindClosestPlane(vec3_t coord);
gActor_t *Map_SpawnActor(const char *classname, float x, float y, float z,
                         float yaw, float pitch, int plane);
void Map_AddActor(gLevel_t *level, gActor_t *actor);
void Map_RemoveActor(gLevel_t *level, gActor_t* actor);
void Map_TraverseChangePlaneHeight(plane_t *plane, float destHeight, int area_id);
void Map_ToggleBlockingPlanes(plane_t *pStart, kbool toggle);
void Map_TriggerActors(int targetID, int classFilter);
gObject_t *Map_GetActorsInRadius(float radius, float x, float y, float z, plane_t *plane,
                                 int classFilter, kbool bTrace);
void Map_Tick(void);
void Map_Load(int map);
void Map_Unload(void);
void Map_Init(void);

#endif