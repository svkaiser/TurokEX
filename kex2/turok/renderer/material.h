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

#include "renderBackend.h"
#include "shaderProg.h"
#include "textureObject.h"
#include "cachefilelist.h"
#include "script.h"

typedef enum {
    MTF_FULLBRIGHT  = BIT(0),
    MTF_NODRAW      = BIT(1)
} materialFlags_t;

#define MAX_SAMPLER_UNITS   4

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

typedef enum {
    MSO_DEFAULT     = 0,
    MSO_MASKED,
    MSO_TRANSPARENT,
    MSO_CUSTOM1,
    MSO_CUSTOM2,
    MSO_CUSTOM3,
    MSO_RESERVED,
    NUMSORTORDERS
} matSortOrder_t;

class kexMaterial {
public:
                                        kexMaterial(void);
                                        ~kexMaterial(void);
    
    matSampler_t                        *Sampler(const int which);
    void                                Init(void);
    void                                Parse(kexLexer *lexer);
    void                                Delete(void);
    void                                SetDiffuseColor(const rcolor color);
    
    const glCullType_t                  CullType(void) const { return cullType; }
    const glFunctions_t                 AlphaFunction(void) const { return alphaFunction; }
    const matSortOrder_t                SortOrder(void) const { return sortOrder; }
    const int                           DepthMask(void) const { return depthMask; }
    kexShaderObj                        *ShaderObj(void) { return shaderObj; }
    const unsigned int                  Flags(void) const { return flags; }
    const unsigned int                  StateBits(void) const { return stateBits; }
    const float                         AlphaMask(void) const { return alphaMask; }
    const unsigned int                  NumUnits(void) const { return units; }
    kexVec4                             &DiffuseColor(void) { return diffuseColor; }
    const kexVec4                       DiffuseColor(void) const { return diffuseColor; }

    filepath_t                          fileName;
    kexMaterial                         *next;

private:
    void                                ParseParam(kexLexer *lexer);
    void                                ParseSampler(kexLexer *lexer);
    glFunctions_t                       ParseFunction(kexLexer *lexer);
    void                                ParseShader(kexLexer *lexer);
    
    kexShaderObj                        *shaderObj;
    matSampler_t                        samplers[MAX_SAMPLER_UNITS];
    unsigned int                        stateBits;
    glCullType_t                        cullType;
    glFunctions_t                       alphaFunction;
    int                                 depthMask;
    matSortOrder_t                      sortOrder;
    float                               alphaMask;
    kexVec4                             diffuseColor;
    unsigned int                        flags;
    unsigned int                        units;
    unsigned int                        genID;
    bool                                bShaderErrors;
};

#endif
