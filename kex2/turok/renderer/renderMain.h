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

#ifndef __RENDER_MAIN_H__
#define __RENDER_MAIN_H__

#include "fbo.h"
#include "material.h"

class kexMaterial;
class kexSector;
class kexTri;
class kexShaderObj;
class kexWorldObject;

typedef struct {
    unsigned int            flags;
    unsigned int            numVerts;
    unsigned int            numIndices;
    kexVec3                 *vertices;
    float                   *coords;
    float                   *normals;
    byte                    *rgb;
    word                    *indices;
    kexMaterial             *material;
} surface_t;

typedef struct {
    surface_t               *surf;
    kexMaterial             *material;
    kexMatrix               matrix;
    kexWorldObject          *refObj;
} drawSurface_t;

#define MAX_FX_DISPLAYS     2048

#define MAX_BLUR_SAMPLES    2

class kexRenderer;
typedef void                (kexRenderer::*drawSurfFunc_t)(const drawSurface_t*, kexMaterial*);

class kexFx;

typedef struct {
    kexFx                   *fx;
} fxDisplay_t;

class kexRenderer {
public:
                            kexRenderer(void);
                            ~kexRenderer(void);

    void                    Init(void);
    void                    AddSurface(const surface_t *surf,
                                       const kexMaterial *material,
                                       const kexMatrix mtx,
                                       const kexWorldObject *worldObject,
                                       const matSortOrder_t sortOrder = MSO_DEFAULT);
    void                    DrawSurfaceList(drawSurfFunc_t function, const int start, const int end);
    void                    DrawSurfaceList(drawSurfFunc_t function);
    void                    ClearSurfaceList(const int start, const int end);
    void                    ClearSurfaceList(void);
    void                    DrawSurface(const drawSurface_t *drawSurf, kexMaterial *material);
    void                    DrawWireFrameSurface(const drawSurface_t *drawSurf, kexMaterial *material);
    void                    DrawBlackSurface(const drawSurface_t *drawSurf, kexMaterial *material);
    void                    Draw(void);
    void                    DrawFX(const fxDisplay_t *fxList, const int count);
    void                    DrawScreenQuad(const kexFBO *fbo = NULL);
    void                    PrepareOcclusionQuery(void);
    void                    TestBoundsForOcclusionQuery(const unsigned int &query, const kexBBox &bounds);
    void                    EndOcclusionQueryTest(void);
    bool                    GetOcclusionSampleResult(const unsigned int &query, const kexBBox &bounds);

    kexFBO                  &FBOLightScatter(void) { return fboLightScatter; }
    void                    ToggleLightScatter(const bool bEnable) { bRenderLightScatter = bEnable; }
    
    const surface_t         *currentSurface;

    bool                    bShowRenderStats;

private:
    void                    ProcessMotionBlur(void);
    void                    ProcessLightScatter(void);
    void                    ProcessFXAA(void);
    void                    ProcessBloom(void);
    void                    DrawStats(void);

    int                     postProcessMS;
    kexMaterial             *motionBlurMaterial;
    kexMaterial             *wireframeMaterial;
    kexMatrix               prevMVMatrix;
    kexFBO                  fboLightScatter;
    kexFBO                  fboFXAA;
    kexFBO                  fboBloom;
    kexFBO                  fboBlur[MAX_BLUR_SAMPLES];
    kexShaderObj            *shaderLightScatter;
    kexShaderObj            *blackShader;
    kexShaderObj            *fxaaShader;
    kexShaderObj            *blurShader;
    kexShaderObj            *bloomShader;
    bool                    bRenderLightScatter;
    kexArray<drawSurface_t> drawSurfaces[NUMSORTORDERS];
    int                     numDrawList[NUMSORTORDERS];
};

extern kexRenderer renderer;

#endif
