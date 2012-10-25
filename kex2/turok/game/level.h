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

#include "type.h"

typedef enum
{
    CLF_WATER           = 0x1,
    CLF_UNKNOWN2        = 0x2,
    CLF_UNKNOWN4        = 0x4,
    CLF_NOSOLIDWALL     = 0x8,
    CLF_UNKNOWN16       = 0x10,
    CLF_ONESIDED        = 0x20,
    CLF_CHECKHEIGHT     = 0x40,
    CLF_UNKNOWN128      = 0x80,
    CLF_UNKNOWN256      = 0x100,
    CLF_HIDDEN          = 0x200,
    CLF_UNKNOWN1024     = 0x400,
    CLF_UNKNOWN2048     = 0x800,
    CLF_UNKNOWN4096     = 0x1000,
    CLF_UNKNOWN8192     = 0x2000,
    CLF_ENDLESSPIT      = 0x4000,
    CLF_DAMAGE_LAVA     = 0x8000,
    CLF_UNKNOWN65536    = 0x10000
} collisionflags_t;

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
    blockobj_t          blocklist;
    struct plane_s      *link[3];
} plane_t;

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

#define MAXMAPS     50

extern kmap_t kmaps[MAXMAPS];
extern kmap_t *g_currentmap;

area_t *Map_GetArea(plane_t *plane);
kmap_t *Map_Load(int map);
void Map_Init(void);

#endif