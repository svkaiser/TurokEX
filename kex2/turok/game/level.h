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
#include "actor.h"
#include "mathlib.h"

typedef enum
{
    CLF_WATER           = 0x1,
    CLF_UNKNOWN2        = 0x2,
    CLF_UNKNOWN4        = 0x4,
    CLF_NOSOLIDWALL     = 0x8,
    CLF_UNKNOWN16       = 0x10,
    CLF_UNKNOWN32       = 0x20,
    CLF_TELEPORT        = 0x40,
    CLF_UNKNOWN128      = 0x80,
    CLF_UNKNOWN256      = 0x100,
    CLF_HIDDEN          = 0x200,
    CLF_UNKNOWN1024     = 0x400,
    CLF_UNKNOWN2048     = 0x800,
    CLF_DAMAGE_LAVA     = 0x1000,
    CLF_UNKNOWN8192     = 0x2000,
    CLF_ENDLESSPIT      = 0x4000,
    CLF_UNKNOWN32768    = 0x8000,
    CLF_UNKNOWN65536    = 0x10000
} collisionflags_t;

typedef struct
{
    char            mdlpath[MAX_FILEPATH];
    short           textureindex;
    short           skin;
    short           tid;
    short           target;
    short           variant;
    short           leafindex;
    float           yaw;
    vec3_t          origin;
    vec3_t          scale;
    float           width;
    float           height;
    float           meleerange;
    short           health;
    short           type;
} mapactor_t;

typedef struct
{
    float           minx;
    float           minz;
    float           maxx;
    float           maxz;
} mapgrid_t;

typedef struct
{
    vec3_t          origin;
    vec3_t          scale;
    bbox_t          box;
    char            mdlpath[MAX_FILEPATH];
    vec4_t          rotation;
    int             textureindex;
    mapgrid_t       *grid;
} mapinst_t;

typedef struct
{
    unsigned int    numstatics;
    unsigned int    numspecials;
    mapinst_t       *statics;
    mapinst_t       *specials;
} mapinstgroup_t;

typedef struct
{
    rcolor          fog_color;
    float           fog_far;
    float           fog_near;
    float           waterplane;
    unsigned int    flags;
    int             args[6];
} area_t;

typedef struct
{
    unsigned int    nummapactors;
    unsigned int    numinstancegroups;
    unsigned int    numgridbounds;
    unsigned int    numareas;
    unsigned int    numplanes;
    unsigned int    numzonebounds;
    mapactor_t      *mapactors;
    mapinstgroup_t  *instgroups;
    mapgrid_t       *gridbounds;
    area_t          *areas;
    plane_t         *planes;
    mapgrid_t       *zones;
    actor_t         actorlist;
    int             tics;
    float           time;
} kmap_t;

#define MAXMAPS     50

extern kmap_t kmaps[MAXMAPS];
extern kmap_t *g_currentmap;

kmap_t *Map_Load(int map);
void Map_Init(void);

#endif