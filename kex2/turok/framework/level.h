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
    CLF_TOGGLEOFF       = 0x2,
    CLF_TOGGLEON        = 0x4,
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
    CLF_DAMAGE_LAVA     = 0x8000,
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

typedef enum
{
    BLF_SECTORLINK      = 0x1,
    BLF_TOUCH           = 0x2
} blockflags_t;

typedef struct
{
    float               minx;
    float               minz;
    float               maxx;
    float               maxz;
} mapgrid_t;

//
// OBJECTS
//
// An object is usually a piece of geometry which can be
// anything such as world geometry, items, or characters
// and can also be used for particle emitters. These make up
// the majority of a level.
//
// Static objects are objects that cannot be modified and can be
// linked to planes for radial collision detection
//
// Dynamic objects can be touchable or shootable and is treated
// differently for radial collision
//

typedef enum
{
    OC_GENERIC  = 0,
    OC_AI       = 1,
    OC_AIBOSS   = 2,
    OC_DYNAMIC  = 3,
    OC_PICKUP   = 4,
    OC_WEAPON   = 5,
    OC_GIB      = 6
} objectclass_t;

typedef enum
{
    OT_TUROK = 0,

    OT_AI_RAPTOR = 1,
    OT_AI_GRUNT = 2,
    OT_AI_DINOSAUR1 = 4,
    OT_AI_RIDER = 5,
    OT_AI_SANDWORM = 8,
    OT_AI_STALKER = 9,
    OT_AI_ALIEN = 10,
    OT_AI_PURLIN = 11,
    OT_AI_MECH = 12,
    OT_AI_FISH = 14,
    OT_AI_SEWERCRAB = 15,
    OT_AI_KILLERPLANT = 16,
    OT_AI_UNKNOWN1 = 19,
    OT_AI_ANIMAL = 20,
    OT_AI_INSECT = 21,
    OT_AI_TURRET = 2001,

    OT_AIBOSS_MANTIS = 1000,
    OT_AIBOSS_TREX = 1001,
    OT_AIBOSS_CAMPAINGER = 1002,
    OT_AIBOSS_HUNTER = 1003,
    OT_AIBOSS_HUMMER = 1004,

    OT_DYNAMIC_DOOR = 300,
    OT_DYNAMIC_GENERIC = 301,
    OT_DYNAMIC_MANTISSTATUE = 302,
    OT_DYNAMIC_LIFT = 303,
    OT_DYNAMIC_UNKNOWN1 = 304,
    OT_DYNAMIC_MANTISWALL = 305,
    OT_DYNAMIC_LASERWALL = 308,
    OT_DYNAMIC_DESTROYABLE = 309,
    OT_DYNAMIC_MOVER = 310,
    OT_DYNAMIC_UNKNOWN2 = 311,
    OT_DYNAMIC_LOGO = 313,
    OT_DYNAMIC_TECHLIGHT = 314,
    OT_DYNAMIC_UNKNOWN3 = 316,
    OT_DYNAMIC_PORTAL = 317,
    OT_DYNAMIC_GATEKEYPANEL = 318,
    OT_DYNAMIC_WATER1 = 319,
    OT_DYNAMIC_WATER2 = 320,
    OT_DYNAMIC_PORTALGATE = 321,
    OT_DYNAMIC_ARROW = 322,
    OT_DYNAMIC_MANTISKEY = 323,
    OT_DYNAMIC_UNKNOWN4 = 324,
    OT_DYNAMIC_KEYPLAQUE = 325,
    OT_DYNAMIC_TEXTUREANIMATE = 327,
    OT_DYNAMIC_UNKNOWN5 = 328,
    OT_DYNAMIC_CUSTOM = 329,
    OT_DYNAMIC_UNKNOWN6 = 330,
    OT_DYNAMIC_TECHDOOR = 331,

    OT_UNKNOWN = 200,
    
    OT_GIB_ALIEN3 = 701,
    OT_GIB_STALKER4 = 702,
    OT_GIB_ALIEN2 = 703,
    OT_GIB_ALIEN1 = 704,
    OT_GIB_STALKER3 = 705,
    OT_GIB_STALKER5 = 706,
    OT_GIB_STALKER2 = 707,
    OT_GIB_STALKER1 = 708,

    OT_WEAPON_KNIFE = 100,
    OT_WEAPON_BOW = 102,
    OT_WEAPON_PISTOL = 103,
    OT_WEAPON_RIFLE = 104,
    OT_WEAPON_PULSERIFLE = 105,
    OT_WEAPON_SHOTGUN = 106,
    OT_WEAPON_ASHOTGUN = 107,
    OT_WEAPON_MINIGUN = 108,
    OT_WEAPON_GRENADE = 109,
    OT_WEAPON_ALIENGUN = 110,
    OT_WEAPON_CANNON = 111,
    OT_WEAPON_MISSILE = 112,
    OT_WEAPON_ACCELERATOR = 113,
    OT_WEAPON_CHRONO = 114,
    
    OT_PICKUP_SMALLHEALTH = 400,
    OT_PICKUP_HEALTH = 402,
    OT_PICKUP_FULLHEALTH = 403,
    OT_PICKUP_ULTRAHEALTH = 404,
    OT_PICKUP_MASK = 405,
    OT_PICKUP_BACKPACK = 406,
    OT_PICKUP_SPIRIT = 407,
    OT_PICKUP_PISTOL = 409,
    OT_PICKUP_ASSAULTRIFLE = 410,
    OT_PICKUP_SHOTGUN = 411,
    OT_PICKUP_ASHOTGUN = 412,
    OT_PICKUP_MINIGUN = 413,
    OT_PICKUP_GRENADELAUNCHER = 414,
    OT_PICKUP_PULSERIFLE = 415,
    OT_PICKUP_ALIENWEAPON = 416,
    OT_PICKUP_ROCKETLAUNCHER = 417,
    OT_PICKUP_ACCELERATOR = 418,
    OT_PICKUP_CANNON = 419,
    OT_PICKUP_QUIVER2 = 420,
    OT_PICKUP_ARROWS = 421,
    OT_PICKUP_QUIVER1 = 422,
    OT_PICKUP_CLIP = 423,
    OT_PICKUP_CLIPBOX = 424,
    OT_PICKUP_SHELLS = 425,
    OT_PICKUP_SHELLBOX = 426,
    OT_PICKUP_EXPSHELLS = 427,
    OT_PICKUP_EXPSHELLBOX = 428,
    OT_PICKUP_MINIGUNAMMO = 429,
    OT_PICKUP_GRENADE = 430,
    OT_PICKUP_GRENADEBOX = 431,
    OT_PICKUP_SMALLCELL = 432,
    OT_PICKUP_CELL = 433,
    OT_PICKUP_ROCKET = 434,
    OT_PICKUP_FUSIONCELL = 436,
    OT_PICKUP_ARMOR = 437,
    OT_PICKUP_COIN1 = 438,
    OT_PICKUP_COIN10 = 439,
    OT_PICKUP_KEY1 = 440,
    OT_PICKUP_KEY2 = 441,
    OT_PICKUP_KEY3 = 442,
    OT_PICKUP_KEY4 = 443,
    OT_PICKUP_KEY5 = 444,
    OT_PICKUP_KEY6 = 445,
    OT_PICKUP_FINALKEY1 = 446,
    OT_PICKUP_CHRONOPIECE1 = 447,
    OT_PICKUP_FINALKEY2 = 449,
    OT_PICKUP_CHRONOPIECE2 = 450,
    OT_PICKUP_CHRONOPIECE3 = 451,
    OT_PICKUP_CHRONOPIECE4 = 452,
    OT_PICKUP_CHRONOPIECE5 = 453,
    OT_PICKUP_CHRONOPIECE6 = 454,
    OT_PICKUP_CHRONOPIECE7 = 455,
    OT_PICKUP_CHRONOPIECE8 = 456,

    OT_MINIPORTAL = 600,
    OT_WATER = 800,

} objecttype_t;

typedef struct
{
    vec3_t              origin;
    vec3_t              scale;
    bbox_t              box;
    char                mdlpath[MAX_FILEPATH];
    vec4_t              rotation;
    char                **textureswaps;
    short               tid;
    short               target;
    short               variant;
    short               type;
    float               width;
    float               height;
    float               centerheight;
    float               viewheight;
    short               plane_id;
    byte                flags;
    blockflags_t        blockflag;
    mtx_t               matrix;
} object_t;

//
// INSTANCES
//
// Instances contain groups of both static and dynamic objects
// TODO: Finish description
//

typedef struct
{
    unsigned int        numstatics;
    unsigned int        numspecials;
    object_t            *statics;
    object_t            *specials;
} instance_t;

typedef struct
{
    rcolor              fog_color;
    float               fog_far;
    float               fog_near;
    float               waterplane;
    float               skyheight;
    unsigned int        flags;
    int                 args[6];
} area_t;

typedef struct blockobj_s
{
    struct blockobj_s   *prev;
    struct blockobj_s   *next;
    object_t            *object;
} blockobj_t;

typedef struct plane_s
{
    unsigned int        area_id;
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
    unsigned int        nummapactors;
    unsigned int        numinstances;
    unsigned int        numgridbounds;
    unsigned int        numareas;
    unsigned int        numplanes;
    unsigned int        numzonebounds;
    instance_t          *instances;
    mapgrid_t           *gridbounds;
    area_t              *areas;
    plane_t             *planes;
    mapgrid_t           *zones;
    int                 tics;
    float               time;
} kmap_t;

typedef struct
{
    int             (*length)(void);
    kbool           (*get)(int, gActor_t*);
} gActorList_t;

typedef struct
{
    float           minx;
    float           minz;
    float           maxx;
    float           maxz;
    gActor_t        *statics;
    unsigned int    numStatics;
} gridBounds_t;

typedef struct
{
    gObject_t       *components;
    gObject_t       *iterator;
} gArea_t;

typedef struct
{
    kbool           (*load)(const char*);
    kbool           (*tick)(void);
    kbool           loaded;
    kbool           bReadyUnload;
    unsigned int    numActors;
    unsigned int    numGridBounds;
    unsigned int    numplanes;
    unsigned int    numAreas;
    gActor_t        actorRoot;
    gActor_t        *actorRover;
    plane_t         *planes;
    gridBounds_t    *gridBounds;
    gArea_t         *areas;
    int             mapID;
    int             nextMap;
    char            name[64];
    int             tics;
    float           time;
    float           deltaTime;
} gLevel_t;

extern gLevel_t gLevel;

#define MAXMAPS     50

extern kmap_t kmaps[MAXMAPS];
extern kmap_t *g_currentmap;

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

int Obj_GetClassType(object_t *obj);
area_t *Map_GetArea(plane_t *plane);
plane_t *Map_FindClosestPlane(vec3_t coord);
gActor_t *Map_SpawnActor(const char *classname, float x, float y, float z,
                         float yaw, float pitch, int plane);
void Map_AddActor(gLevel_t *level, gActor_t *actor);
void Map_RemoveActor(gLevel_t *level, gActor_t* actor);
void Map_TraverseChangePlaneHeight(plane_t *plane, float destHeight, int area_id);
void Map_Tick(void);
void Map_Load(int map);
void Map_Unload(void);
void Map_Init(void);

#endif