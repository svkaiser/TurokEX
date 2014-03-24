// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "renderSystem.h"
#include "shaderProg.h"
#include "textureObject.h"
#include "cachefilelist.h"
#include "script.h"

typedef enum {
    MTF_SOLID       = BIT(0),
    MTF_MASKED      = BIT(1),
    MTF_TRANSPARENT = BIT(2),
    MTF_FULLBRIGHT  = BIT(3),
    MTF_NODRAW      = BIT(4)
} materialFlags_t;

#define MAX_SAMPLER_UNITS   8

typedef struct {
    kexStr          param;
    int             unit;
    kexTexture      *texture;
    texClampMode_t  clamp;
    texFilterMode_t filter;
    float           scroll[2];
    float           scale[3];
    float           rotate;
} matSampler_t;

class kexMaterial {
public:
                                        kexMaterial(void);
                                        ~kexMaterial(void);
    
    matSampler_t                        *Sampler(const int which);
    void                                Init(void);
    void                                Parse(kexLexer *lexer);
    
    const glCullType_t                  CullType(void) const { return cullType; }
    const kexShaderObj                  *ShaderObj(void) const { return &shaderObj; }
    const unsigned int                  Flags(void) const { return flags; }
    const unsigned int                  StateBits(void) const { return stateBits; }

    filepath_t                          fileName;
    kexMaterial                         *next;

private:
    void                                ParseParam(kexLexer *lexer);
    void                                ParseSampler(kexLexer *lexer);
    
    kexShaderObj                        shaderObj;
    matSampler_t                        samplers[MAX_SAMPLER_UNITS];
    unsigned int                        stateBits;
    glCullType_t                        cullType;
    unsigned int                        flags;
    unsigned int                        units;
    unsigned int                        genID;
    bool                                bShaderErrors;
};

#endif
