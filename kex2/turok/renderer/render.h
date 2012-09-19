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

#ifndef _RENDER_H_
#define _RENDER_H_

//
// MODELS
//
#define MDF_UNKNOWN1                1
#define MDF_FULLBRIGHT              2
#define MDF_NOCULLFACES             4
#define MDF_RENDERSPECULAR          8
#define MDF_SHINYSURFACE            16
#define MDF_UNKNOWN32               32
#define MDF_SOLID                   64
#define MDF_MASKED                  128
#define MDF_TRANSPARENT1            256
#define MDF_TRANSPARENT2            512
#define MDF_COLORIZE                1024
#define MDF_METALSURFACE            2048
#define MDF_UNKNOWN4096             4096
#define MDF_UNKNOWN8192             8192
#define MDF_UNKNOWN16384            16384
#define MDF_UNKNOWN32768            32768
#define MDF_UNKNOWN65536            65536

typedef struct
{
    unsigned int    flags;
    unsigned int    numverts;
    unsigned int    numtris;
    vec3_t          *xyz;
    float           *coords;
    float           *normals;
    word            *tris;
    char            texpath[MAX_FILEPATH];
    rcolor          color1;
    rcolor          color2;
} mdlsection_t;

typedef struct
{
    unsigned int    numsections;
    mdlsection_t    *sections;
} mdlmesh_t;

typedef struct
{
    unsigned int    numvariants;
    word            *variants;
    unsigned int    nummeshes;
    mdlmesh_t       *meshes;
    unsigned int    numchildren;
    word            *children;
} mdlnode_t;

typedef struct kmodel_s
{
    char            mdlpath[MAX_FILEPATH];
    bbox_t          bbox;
    unsigned int    numbehaviors;
    unsigned int    numnodes;
    mdlnode_t       *nodes;
    struct kmodel_s *next;
} kmodel_t;

kmodel_t *Mdl_Find(const char *name);
kmodel_t *Mdl_Load(const char *file);
void Mdl_Init(void);

//
// MAIN
//
void R_DrawFrame(void);
void R_FinishFrame(void);
void R_Shutdown(void);
void R_Init(void);

#endif