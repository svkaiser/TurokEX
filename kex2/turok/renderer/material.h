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

#include "shaderProg.h"
#include "textureObject.h"
#include "cachefilelist.h"
#include "script.h"

typedef enum {
    MC_CLAMP    = 0,
    MC_REPEAT,
    MC_MIRRORED
} matClampMode_t;

typedef enum {
    MF_LINEAR   = 0,
    MF_NEAREST
} matFilterMode_t;

typedef enum {
    MTF_SOLID       = BIT(0),
    MTF_MASKED      = BIT(1),
    MTF_TRANSPARENT = BIT(2),
    MTF_FULLBRIGHT  = BIT(3),
    MTF_NOFOG       = BIT(4),
    MTF_NODRAW      = BIT(5)
} materialFlags_t;

typedef enum {
    MCT_NONE   = 0,
    MCT_FRONT,
    MCT_BACK,
} matCullType_t;

typedef enum {
    MPT_FLOAT   = 0,
    MPT_INT,
    MPT_VEC2,
    MPT_VEC3,
    MPT_VEC4,
    MPT_MAT4
} matParamType_t;

#define MAX_SAMPLER_UNITS   8

typedef struct {
    kexStr          param;
    int             unit;
    kexTexture      *texture;
    matClampMode_t  clamp;
    matFilterMode_t filter;
    float           scroll[2];
    float           scale[3];
    float           rotate;
} matSampler_t;

class kexMaterial {
public:
                                        kexMaterial(void);
                                        ~kexMaterial(void);
    
    void                                Bind(void);
    matSampler_t                        *Sampler(const int which);
    void                                Init(void);
    void                                Parse(kexLexer *lexer);
    
    const matCullType_t                 CullType(void) const { return cullType; }
    const kexShaderObj                  *ShaderObj(void) const { return &shaderObj; }
    const unsigned int                  Flags(void) const { return flags; }

    filepath_t                          fileName;
    kexMaterial                         *next;

private:
    void                                ParseParam(kexLexer *lexer);
    void                                ParseSampler(kexLexer *lexer);
    
    static unsigned int                 matID;
    static unsigned int                 bindID;
    
    template<typename t>
    struct matParam_t {
        kexStr                          param;
        t                               value;
    };
    
    typedef struct {
        kexArray<matParam_t<float>>     floatParams;
        kexArray<matParam_t<int>>       intParams;
        kexArray<matParam_t<kexVec2>>   vec2Params;
        kexArray<matParam_t<kexVec3>>   vec3Params;
        kexArray<matParam_t<kexVec4>>   vec4Params;
        kexArray<matParam_t<kexMatrix>> mat4Params;
    } matShaderParam_t;
    
    kexShaderObj                        shaderObj;
    matSampler_t                        samplers[MAX_SAMPLER_UNITS];
    matShaderParam_t                    params;
    matCullType_t                       cullType;
    unsigned int                        flags;
    unsigned int                        units;
    unsigned int                        genID;
    bool                                bShaderErrors;
};

class kexMaterialManager {
    friend class kexMaterial;
public:
                                        kexMaterialManager(void);
                                        ~kexMaterialManager(void);
    
    kexMaterial                         *LoadMaterial(const char *file);
    void                                Shutdown(void);
    
private:
    kexHashList<kexMaterial>            materials;
};

#endif
