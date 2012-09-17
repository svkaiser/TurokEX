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

typedef struct
{
    float minx;
    float minz;
    float maxx;
    float maxz;
} mapgrid_t;

typedef struct
{
    char        mdlpath[MAX_FILEPATH];
    int         textureindex;
    int         skin;
    int         tid;
    int         target;
    int         variant;
    int         leafindex;
    float       yaw;
    vec3_t      origin;
    vec3_t      scale;
    float       width;
    float       height;
} mapactor_t;

typedef struct
{
    vec3_t      origin;
    vec3_t      scale;
    bbox_t      box;
    char        mdlpath[MAX_FILEPATH];
    vec4_t      angle;
    int         textureindex;
} mapstaticinst_t;

typedef struct
{
    vec3_t      origin;
    vec3_t      scale;
    float       boundsize;
    char        mdlpath[MAX_FILEPATH];
    int         textureindex;
    int         tid;
    int         target;
} mapinst_t;

typedef struct
{
    unsigned int        numstaticinstances;
    unsigned int        numinstances;
    mapstaticinst_t     *staticinst;
    mapinst_t           *inst;
} mapinstgroup_t;

typedef struct
{
    unsigned int    numactors;
    unsigned int    numinstancegroups;
    unsigned int    numgridbounds;
    mapactor_t      *actors;
    mapinstgroup_t  *instgroups;
    mapgrid_t       *gridbounds;
    actor_t         actorlist;
} kmap_t;

#define MAXMAPS     50

extern kmap_t kmaps[MAXMAPS];

void Map_Init(void);

#endif