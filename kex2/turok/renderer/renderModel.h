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

#ifndef __RENDER_MODEL_H__
#define __RENDER_MODEL_H__

#include "cachefilelist.h"

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

typedef struct {
    unsigned int                    flags;
    unsigned int                    numVerts;
    unsigned int                    numIndices;
    kexVec3                         *vertices;
    float                           *coords;
    float                           *normals;
    byte                            *rgb;
    word                            *indices;
    filepath_t                      texturePath;
    rcolor                          color1;
    rcolor                          color2;
} surface_t;

typedef struct {
    unsigned int                    numSurfaces;
    surface_t                       *surfaces;
} surfaceGroup_t;

typedef struct {
    unsigned int                    numVariants;
    word                            *variants;
    unsigned int                    numSurfaceGroups;
    surfaceGroup_t                  *surfaceGroups;
    unsigned int                    numChildren;
    word                            *children;
} modelNode_t;

typedef struct kexModel_s {
    filepath_t                      filePath;
    kexBBox                         bounds;
    unsigned int                    numNodes;
    unsigned int                    numAnimations;
    modelNode_t                     *nodes;
#ifndef EDITOR
    struct kexAnim_s                *anims;
#endif
} kexModel_t;

class kexModelManager {
public:
                                    kexModelManager(void);
                                    ~kexModelManager(void);

    kexModel_t                      *LoadModel(const char *file);
    void                            Shutdown(void);

private:
    void                            ParseKMesh(kexModel_t *model, kexLexer *lexer);
    void                            ParseWavefrontObj(kexModel_t *model, kexLexer *lexer);

    kexHashList<kexModel_t>         modelList;
};

extern kexModelManager modelManager;
extern kexHeapBlock hb_model;

#endif
