// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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

#ifndef __RENDER_WORLD_H__
#define __RENDER_WORLD_H__

#include "world.h"
#include "sdNodes.h"

class kexRenderWorld {
public:
                                kexRenderWorld(void);

    void                        Shutdown(void);
    void                        SetCameraView(kexCamera *camera);
    void                        RenderScene(void);
    void                        BuildNodes(void);
    void                        DrawAreaNode(void);
    void                        DrawRenderNode(void);
    void                        PrintStats(void);
    void                        InitSunData(void);

    static void                 Init(void);
    static int                  SortSprites(const void *a, const void *b);

    bool                        bShowBBox;
    bool                        bShowGrid;
    bool                        bShowNormals;
    bool                        bShowRadius;
    bool                        bShowNodes;
    bool                        bShowOrigin;
    bool                        bWireframe;
    bool                        bShowClipMesh;
    bool                        bShowCollisionMap;
    bool                        bShowRenderNodes;
    bool                        bPrintStats;
    int                         renderNodeStep;

    int                         showAreaNode;
    kexSDNode<kexWorldModel>    renderNodes;

    kexVec3                     &WorldLightTransform(void) { return worldLightTransform; }
    kexVec3                     &ProjectedSunCoords(void) { return projectedSunCoords; }
    const bool                  LightScatterPass(void) const { return bLightScatterPass; }

private:
    void                        PreProcessLightScatter(void);
    void                        DrawSun(const bool bForceInfiniteProjection);
    void                        DrawSingleActor(kexActor *actor, kexMatrix *matrix);
    void                        DrawActors(void);
    void                        DrawStaticActors(void);
    void                        DrawViewActors(void);
    void                        DrawForegroundActors(void);
    void                        DrawFX(void);
    void                        DrawWorldModel(kexWorldModel *wm);
    void                        RecursiveSDNode(int nodenum);
    void                        TraverseDrawActorNode(kexActor *actor,
                                                      const modelNode_t *node,
                                                      kexAnimState *animState);
    void                        DrawSectors(kexSector *sectors, const int count);
    void                        DrawTriangle(const kexTri &tri, const word index,
                                             byte r, byte g, byte b, byte a);

    kexWorld                    *world;
    kexVec3                     worldLightTransform;
    int                         renderSceneMS;
    int                         renderStaticsMS;
    int                         renderActorsMS;
    int                         renderFXMS;
    int                         numDrawnStatics;
    int                         numCulledStatics;
    int                         numDrawnSDNodes;
    int                         numDrawnActors;
    int                         numOccludedActors;
    int                         numCulledActors;
    int                         renderNodeStepNum;
    kexMaterial                 *blackMat;
    kexMaterial                 *sunMaterial;
    kexModel_t                  *sunModel;
    kexVec3                     sunPosition;
    kexVec3                     projectedSunCoords;
    bool                        bLightScatterPass;
    kexArray<GLuint>            actorQueries;
};

extern kexRenderWorld renderWorld;

#endif
