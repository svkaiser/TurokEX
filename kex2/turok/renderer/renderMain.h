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
} drawSurface_t;

#define MAX_FX_DISPLAYS     2048

#define GL_MAX_INDICES      0x10000
#define GL_MAX_VERTICES     0x10000

class kexFx;

typedef struct {
    kexFx                   *fx;
} fxDisplay_t;

class kexRenderer {
public:
                            kexRenderer(void);
                            ~kexRenderer(void);

    void                    Init(void);
    void                    AddSurface(const surface_t *surf, const kexMaterial *material,
                                       const kexMatrix mtx, const matSortOrder_t sortOrder = MSO_DEFAULT);
    void                    DrawSurfaceList(void);
    void                    DrawSurface(const surface_t *surface, kexMaterial *material);
    void                    DrawWireFrameSurface(const surface_t *surface, const rcolor color);
    void                    DrawBlackSurface(const surface_t *surface, kexMaterial *material);
    void                    Draw(void);
    void                    DrawFX(const fxDisplay_t *fxList, const int count);
    void                    BindDrawPointers(void);
    void                    AddTriangle(int v0, int v1, int v2);
    void                    AddVertex(float x, float y, float z, float s, float t,
                                      byte r, byte g, byte b, byte a);
    void                    AddLine(float x1, float y1, float z1,
                                    float x2, float y2, float z2,
                                    byte r, byte g, byte b, byte a);
    void                    AddLine(float x1, float y1, float z1,
                                    float x2, float y2, float z2,
                                    byte r1, byte g1, byte b1, byte a1,
                                    byte r2, byte g2, byte b2, byte a2);
    void                    DrawElements(const bool bClearCount = true);
    void                    DrawElements(const kexMaterial *material, const bool bClearCount = true);
    void                    DrawElementsNoShader(const bool bClearCount = true);
    void                    DrawLineElements(void);
    void                    DrawScreenQuad(void);
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
    void                    DrawStats(void);

    int                     postProcessMS;
    kexMaterial             *motionBlurMaterial;
    kexMaterial             *wireframeMaterial;
    kexMatrix               prevMVMatrix;
    kexFBO                  fboLightScatter;
    kexFBO                  fboFXAA;
    kexShaderObj            *shaderLightScatter;
    kexShaderObj            *blackShader;
    kexShaderObj            *fxaaShader;
    bool                    bRenderLightScatter;
    kexArray<drawSurface_t> drawSurfaces[NUMSORTORDERS];
    int                     numDrawList[NUMSORTORDERS];
    word                    indiceCount;
    word                    vertexCount;
    word                    drawIndices[GL_MAX_INDICES];
    float                   drawVertices[GL_MAX_VERTICES];
    float                   drawTexCoords[GL_MAX_VERTICES];
    byte                    drawRGB[GL_MAX_VERTICES];
};

extern kexRenderer renderer;

#endif
