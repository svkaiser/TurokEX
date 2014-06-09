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
//
// DESCRIPTION: World rendering
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "mathlib.h"
#include "renderModel.h"
#include "actor.h"
#include "world.h"
#include "renderBackend.h"
#include "renderWorld.h"
#include "renderMain.h"
#include "gameManager.h"
#include "worldModel.h"
#include "renderUtils.h"
#include "ai.h"

extern kexCvar cvarRenderLightScatter;
extern kexCvar cvarRenderNodeOcclusionQueries;
extern kexCvar cvarRenderActorOcclusionQueries;

kexRenderWorld renderWorld;

#define EXTENDED_BOX_SIZE   16.0f

//-----------------------------------------------------------------------------
//
// command functions
//
//-----------------------------------------------------------------------------

//
// showclipmesh
//

COMMAND(showclipmesh) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowClipMesh ^= 1;
    renderWorld.bWireframe = 0;
}

//
// showgridbounds
//

COMMAND(showgridbounds) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowGrid ^= 1;
}

//
// showbbox
//

COMMAND(showbbox) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowBBox ^= 1;
}

//
// showradius
//

COMMAND(showradius) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowRadius ^= 1;
}

//
// showorigin
//

COMMAND(showorigin) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowOrigin ^= 1;
}

//
// showrendernodes
//

COMMAND(showrendernodes) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowRenderNodes ^= 1;
}

//
// showcollision
//

COMMAND(showcollision) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowCollisionMap ^= 1;
    renderWorld.bWireframe = 0;
}

//
// drawwireframe
//

COMMAND(drawwireframe) {
    if(command.GetArgc() < 1)
        return;

    renderWorld.bWireframe ^= 1;
    renderWorld.bShowClipMesh = 0;
    renderWorld.bShowCollisionMap = 0;
}

//
// showareanode
//

COMMAND(showareanode) {
    if(command.GetArgc() < 2) {
        return;
    }

    renderWorld.showAreaNode = atoi(command.GetArgv(1));
}

//
// statscene
//

COMMAND(statscene) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bPrintStats ^= 1;
}

//
// rendernodestep
//

COMMAND(rendernodestep) {
    if(command.GetArgc() < 1) {
        return;
    }
    
    if(command.GetArgc() < 2) {
        renderWorld.renderNodeStep++;
        return;
    }
    
    renderWorld.renderNodeStep = atoi(command.GetArgv(1));
}

//-----------------------------------------------------------------------------
//
// initialization and shutdown
//
//-----------------------------------------------------------------------------

//
// kexRenderWorld::kexRenderWorld
//

kexRenderWorld::kexRenderWorld(void) {
    this->world             = &localWorld;
    this->bShowBBox         = false;
    this->bShowGrid         = false;
    this->bShowNodes        = false;
    this->bShowNormals      = false;
    this->bShowOrigin       = false;
    this->bShowRadius       = false;
    this->bWireframe        = false;
    this->bShowClipMesh     = false;
    this->bShowCollisionMap = false;
    this->bPrintStats       = false;
    this->numDrawnFX        = 0;
    this->showAreaNode      = -1;
    this->renderNodeStep    = -1;
}

//
// kexRenderWorld::InitSunData
//

void kexRenderWorld::InitSunData(void) {
    blackMat        = renderBackend.CacheMaterial("materials/default.kmat@black");
    sunSpotShader   = renderBackend.CacheShader("defs/shaders.def@sunSpot");

}

//
// kexRenderWorld::Shutdown
//
// Called after world has unloaded
//

void kexRenderWorld::Shutdown(void) {
    renderNodes.Destroy();
    
    for(unsigned int i = 0; i < actorQueries.Length(); i++) {
        dglDeleteQueriesARB(1, &actorQueries[i]);
    }
    
    for(unsigned int i = 0; i < nodeQueries.Length(); i++) {
        dglDeleteQueriesARB(1, &nodeQueries[i]);
    }

    for(unsigned int i = 0; i < staticQueries.Length(); i++) {
        dglDeleteQueriesARB(1, &staticQueries[i]);
    }

    actorQueries.Empty();
    nodeQueries.Empty();
    staticQueries.Empty();
}

//-----------------------------------------------------------------------------
//
// scene rendering
//
//-----------------------------------------------------------------------------

//
// kexRenderWorld::SetCameraView
//

void kexRenderWorld::SetCameraView(kexCamera *camera) {
    camera->SetupMatrices();
    
    dglMatrixMode(GL_PROJECTION);
    dglLoadMatrixf(camera->Projection().ToFloatPtr());
    dglMatrixMode(GL_MODELVIEW);
    dglLoadMatrixf(camera->ModelView().ToFloatPtr());

    worldLightTransform = (world->worldLightOrigin * world->Camera()->RotationMatrix()).ToVec3();
}

//
// kexRenderWorld::RenderScene
//

void kexRenderWorld::RenderScene(void) {
    if(!world->IsLoaded()) {
        return;
    }

    if(bPrintStats) {
        renderSceneMS = sysMain.GetMS();
        numDrawnStatics = numDrawnSDNodes = numDrawnActors = 0;
    }
    
    SetCameraView(world->Camera());
    
    // create draw lists
    DrawStaticActors();
    DrawActors();

    if(cvarRenderLightScatter.GetBool()) {
        PreProcessLightScatter();
    }

    if(!bWireframe) {
        kexVec4 fogRGB = world->GetCurrentFogRGB();
        dglClearColor(fogRGB[0], fogRGB[1], fogRGB[2], fogRGB[3]);
    }
    else {
        dglClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    }

    // force the fog clear color to be visible
    if(cvarRenderLightScatter.GetBool()) {
        dglClear(GL_COLOR_BUFFER_BIT);
    }

    DrawForegroundActors();

    SetCameraView(world->Camera());
    
    // render surfaces normally
    renderer.DrawSurfaceList(bWireframe ?
                             &kexRenderer::DrawWireFrameSurface :
                             &kexRenderer::DrawSurface);

    renderer.ClearSurfaceList();
    PreProcessOcclusionQueries();

    DebugObjects();

    if(showAreaNode >= 0 || bShowRenderNodes) {
        renderBackend.SetState(GLSTATE_DEPTHTEST, false);
        renderBackend.DisableShaders();
        if(showAreaNode >= 0) {
            DrawAreaNode();
        }
        if(bShowRenderNodes) {
            DrawRenderNode();
        }
        renderBackend.SetState(GLSTATE_DEPTHTEST, true);
    }

    if(bShowCollisionMap) {
        DrawSectors(world->CollisionMap().sectors,
            world->CollisionMap().numSectors);
    }

    if(bPrintStats) {
        renderFXMS = sysMain.GetMS();
    }

    DrawFX();

    if(bPrintStats) {
        renderFXMS = sysMain.GetMS() - renderFXMS;
    }

    DrawViewActors(bWireframe ?
                   &kexRenderer::DrawWireFrameSurface :
                   &kexRenderer::DrawSurface);

    if(bPrintStats) {
        renderSceneMS = sysMain.GetMS() - renderSceneMS;
    }
}

//-----------------------------------------------------------------------------
//
// preprocessing
//
//-----------------------------------------------------------------------------

//
// kexRenderWorld::CameraRenderSky
//

const bool kexRenderWorld::CameraRenderSky(void) const {
    kexSector *sector = world->Camera()->Sector();

    return (sector && sector->area && (sector->area->Flags() & AAF_DRAWSKY));
}

//
// kexRenderWorld::PreProcessLightScatter
//

void kexRenderWorld::PreProcessLightScatter(void) {
    int vp[4];
    bool bSunOccluded;

    sunPosition = world->worldLightOrigin.ToVec3() * 32768.0f;

    if(!CameraRenderSky()) {
        bSunOccluded = false;
    }
    else {
        bSunOccluded = world->Camera()->Frustum().TestSphere(sunPosition, 8192.0f);
    }
    
    renderer.ToggleLightScatter(bSunOccluded);

    if(!bSunOccluded) {
        return;
    }
    
    dglGetIntegerv(GL_VIEWPORT, vp);
    dglPushAttrib(GL_VIEWPORT_BIT);
    dglViewport(0, 0, renderer.FBOLightScatter().Width(), renderer.FBOLightScatter().Height());

    renderer.FBOLightScatter().Bind();
    dglClearColor(world->worldLightAmbience[0],
                  world->worldLightAmbience[1],
                  world->worldLightAmbience[2], 1);
    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawSun(true);
    SetCameraView(world->Camera());

    projectedSunCoords = world->Camera()->ProjectPoint(sunPosition, 0, 0);

    projectedSunCoords.x /= (float)vp[2];
    projectedSunCoords.y /= (float)vp[3];

    // redraw surfaces
    renderer.DrawSurfaceList(&kexRenderer::DrawBlackSurface);
    DrawViewActors(&kexRenderer::DrawBlackSurface);

    renderer.FBOLightScatter().UnBind();
    dglPopAttrib();
}

//
// kexRenderWorld::PreProcessOcclusionQueries
//

void kexRenderWorld::PreProcessOcclusionQueries(void) {
    bool bNodeOcclusion = cvarRenderActorOcclusionQueries.GetBool();
    bool bActorOcclusion = cvarRenderActorOcclusionQueries.GetBool();

    // node occlusion query pass
    if(bNodeOcclusion | bActorOcclusion) {
        kexBBox box;

        if(bPrintStats) {
            renderOcclusionPrePassMS = sysMain.GetMS();
        }

        renderer.PrepareOcclusionQuery();
        
        if(bNodeOcclusion) {
            kexWorldModel *wm;

            for(unsigned int i = 0; i < renderNodes.numNodes; i++) {
                kexSDNodeObj<kexWorldModel> *node = &renderNodes.nodes[i];
                if(node->objects.Next() == NULL) {
                    continue;
                }
                if(!world->Camera()->Frustum().TestBoundingBox(node->bounds)) {
                    continue;
                }

                box = node->bounds;
                box += EXTENDED_BOX_SIZE;
                
                // test bounds of the node against the occlusion query
                renderer.TestBoundsForOcclusionQuery(nodeQueries[i], box);

                for(wm = node->objects.Next(); wm != NULL; wm = wm->renderNode.link.Next()) {
                    if(wm->bHidden || wm->bCulled || wm->queryIndex == -1) {
                        continue;
                    }

                    box = wm->Bounds();
                    box += EXTENDED_BOX_SIZE;

                    renderer.TestBoundsForOcclusionQuery(staticQueries[wm->queryIndex], box);
                }
            }
        }

        if(bActorOcclusion) {
            unsigned int queryIndex = 0;

            for(kexActor *actor = world->actors.Next();
                actor != NULL; actor = actor->worldLink.Next()) {
                    if(actor->bStatic   ||
                        actor->bHidden  ||
                        actor->bCulled  ||
                        actor->DisplayType() != ODT_NORMAL) {
                            continue;
                    }

                    // generate a new query if we need to
                    if(actorQueries.Length() <= queryIndex) {
                        GLuint query;

                        dglGenQueriesARB(1, &query);
                        actorQueries.Push(query);
                    }

                    // keep reference for next frame
                    actor->queryIndex = queryIndex++;

                    box = actor->Bounds();
                    box += EXTENDED_BOX_SIZE;

                    // query the result based on the bounding box
                    renderer.TestBoundsForOcclusionQuery(actorQueries[actor->queryIndex], box);
            }
        }

        renderer.EndOcclusionQueryTest();

        if(bPrintStats) {
            renderOcclusionPrePassMS = sysMain.GetMS() - renderOcclusionPrePassMS;
        }
    }
}

//-----------------------------------------------------------------------------
//
// world model / actor rendering
//
//-----------------------------------------------------------------------------

//
// kexRenderWorld::BuildNodes
//

void kexRenderWorld::BuildNodes(void) {
    kexWorldModel *wm;
    unsigned int idx = 0;
    
    renderNodes.Init(8);
    
    for(wm = world->staticActors.Next(); wm != NULL; wm = wm->worldLink.Next()) {
        GLuint query = 0;

        renderNodes.AddBoxToRoot(wm->Bounds());

        // create a new query for every world model
        dglGenQueriesARB(1, &query);
        staticQueries.Push(query);

        // keep a reference to it so we'll know which query belongs to which model
        wm->queryIndex = idx++;
    }
    
    renderNodes.BuildNodes();
    
    for(wm = world->staticActors.Next(); wm != NULL; wm = wm->worldLink.Next()) {
        wm->renderNode.Link(renderNodes, wm->Bounds());
    }
    
    nodeQueries.Resize(renderNodes.numNodes);
    
    for(unsigned int i = 0; i < nodeQueries.Length(); i++) {
        dglGenQueriesARB(1, &nodeQueries[i]);
    }
}

//
// kexRenderWorld::DrawWorldModel
//

void kexRenderWorld::DrawWorldModel(kexWorldModel *wm) {
    const kexModel_t *model;
    const modelNode_t *node;
    
    if(!bShowClipMesh && !bShowCollisionMap) {
        if(!(model = wm->Model())) {
            return;
        }
        
        node = &model->nodes[0];
        
        for(unsigned i = 0; i < model->nodes[0].numSurfaces; i++) {
            surface_t *surface = &node->surfaces[i];

            kexMaterial *material = surface->material;
            
            if(wm->materials != NULL) {
                kexMaterial *mat = wm->materials[i];
                
                if(mat != NULL) {
                    material = mat;
                }
            }

            if(material == NULL) {
                material = renderBackend.defaultMaterial;
            }

            renderer.AddSurface(surface,
                                material,
                                wm->Matrix(),
                                wm,
                                material->SortOrder());
        }
    }
    else if(bShowClipMesh) {
        wm->ClipMesh().DebugDraw();
    }
    
    if(bPrintStats) {
        numDrawnStatics++;
    }
}

//
// kexRenderWorld::TraverseDrawActorNode
//

void kexRenderWorld::TraverseDrawActorNode(const kexActor *actor,
                                           const modelNode_t *node,
                                           const kexAnimState *animState,
                                           const kexMatrix &matrix) {
    unsigned int i;
    const kexModel_t *model = actor->Model();
    int nodenum = node - model->nodes;
    kexMatrix mtx = matrix;

    if(animState != NULL) {
        kexAnim_t *anim;
        kexAnim_t *prevanim;
        int frame;
        int nextframe;
        float delta;

        anim        = animState->track.anim;
        prevanim    = animState->prevTrack.anim;
        frame       = animState->track.frame;
        nextframe   = animState->track.nextFrame;
        delta       = animState->deltaTime;

        if(anim && frame < (int)anim->numFrames) {
            kexQuat r1 = animState->GetRotation(anim, nodenum, frame);
            kexQuat r2 = animState->GetRotation(anim, nodenum, nextframe);
            kexVec3 t1 = animState->GetTranslation(anim, nodenum, frame);
            kexVec3 t2 = animState->GetTranslation(anim, nodenum, nextframe);

            kexQuat rot_cur;
            kexQuat rot_next;
            kexVec3 pos_cur;
            kexVec3 pos_next;

            if(!(animState->flags & ANF_BLEND)) {
                rot_cur     = r1;
                rot_next    = r2;
                pos_cur     = t1;
                pos_next    = t2;
            }
            else {
                frame       = animState->prevTrack.frame;
                nextframe   = animState->prevTrack.nextFrame;

                rot_cur     = animState->GetRotation(prevanim, nodenum, frame).Slerp(r1, delta);
                rot_next    = animState->GetRotation(prevanim, nodenum, nextframe).Slerp(r2, delta);
                pos_cur     = animState->GetTranslation(prevanim, nodenum, frame).Lerp(t1, delta);
                pos_next    = animState->GetTranslation(prevanim, nodenum, nextframe).Lerp(t2, delta);
            }

            kexQuat rot = rot_cur.Slerp(rot_next, delta);
            kexVec3 pos = pos_cur.Lerp(pos_next, delta);

            if(nodenum == 0) {
                if(animState->flags & ANF_ROOTMOTION ||
                    animState->prevFlags & ANF_ROOTMOTION) {
                    pos.x = 0;
                    pos.y = 0;
                }
            }

            rot = rot * actor->GetNodeRotations()[nodenum];
            kexMatrix translation(rot);
            translation.AddTranslation(pos);
            
            mtx = translation * matrix;
        }
    }

    for(i = 0; i < node->numSurfaces; i++) {
        surface_t *surface = &node->surfaces[i];
        kexMaterial *material;
        matSortOrder_t sortOrder;

        material = surface->material;

        if(actor->DisplayType() == ODT_CLIENTVIEW || actor->DisplayType() == ODT_FOREGROUND) {
            sortOrder = MSO_RESERVED;
        }
        else if(material) {
            sortOrder = material->SortOrder();
        }
        else {
            sortOrder = MSO_DEFAULT;
            material = renderBackend.defaultMaterial;
        }

        if(material) {
            renderer.AddSurface(surface,
                                material,
                                mtx,
                                actor,
                                sortOrder);
        }
    }

    for(i = 0; i < node->numChildren; i++) {
        TraverseDrawActorNode(actor, &model->nodes[node->children[i]], animState, mtx);
    }
}

//
// kexRenderWorld::TraversePortalView
//

void kexRenderWorld::TraversePortalView(kexPortal *portal, kexPortal *prevPortal,
                                        const int side, kexViewBounds *viewBounds) {
    kexViewBounds vb;

    if(portal == NULL) {
        return;
    }

    if(!world->Camera()->Frustum().TestBoundingBox(portal->Bounds())) {
        return;
    }

    portal->bInView = true;

    for(unsigned int i = 0; i < portal->links.Length(); i++) {
        if(!world->Camera()->Frustum().TestBoundingBox(portal->links[i].bounds)) {
            continue;
        }
        if(portal->links[i].portal == NULL) {
            continue;
        }
        if(portal->links[i].portal->bInView == true) {
            continue;
        }
        if(portal->links[i].portal == prevPortal) {
            continue;
        }
        if(portal->links[i].sideRef == (side ^ 1)) {
            continue;
        }

        portal->ClipLinkToViewBounds(world->Camera(), &portal->links[i], vb);

        if(viewBounds) {
            if(!viewBounds->ViewBoundInside(vb)) {
                continue;
            }
            
            vb = *viewBounds;
        }

        portal->links[i].bInView = true;

        TraversePortalView(portal->links[i].portal, portal, portal->links[i].sideRef, &vb);
    }
}

//
// kexRenderWorld::RecursiveSDNode
//

void kexRenderWorld::RecursiveSDNode(int nodenum) {
    kexSDNodeObj<kexWorldModel> *node;
    kexWorldModel *wm;
    kexFrustum frustum;
    kexCamera *camera;
    kexBBox box;
    int side;
    float d;
    bool bNodeQueries;

    node = &renderNodes.nodes[nodenum];
    camera = world->Camera();
    frustum = camera->Frustum();

    if(node->axis != -1) {
        d = node->plane.Distance(camera->GetOrigin()) - node->plane.d;
        side = FLOATSIGNBIT(d);

        if(frustum.TestBoundingBox(node->children[side]->bounds)) {
            RecursiveSDNode(node->children[side]->nodeNum);
        }

        if(frustum.TestBoundingBox(node->children[side ^ 1]->bounds)) {
            RecursiveSDNode(node->children[side ^ 1]->nodeNum);
        }
    }
    
    if(node->objects.Next() == NULL) {
        return;
    }

    bNodeQueries = cvarRenderNodeOcclusionQueries.GetBool();
    
    if(bNodeQueries) {
        box = node->bounds;
        box += (EXTENDED_BOX_SIZE + 24.0f);

        // check if it's occluded from the previous frame
        if(renderer.GetOcclusionSampleResult(nodeQueries[nodenum], box)) {
            numOccludedNodes++;
            return;
        }
    }
    
    if(renderNodeStep >= 0) {
        if(renderNodeStepNum >= renderNodeStep) {
            return;
        }
        
        renderNodeStepNum++;
    }

    for(wm = node->objects.Next(); wm != NULL; wm = wm->renderNode.link.Next()) {
        if(wm->bHidden) {
            if(bShowClipMesh) {
                wm->ClipMesh().DebugDraw();
            }
            continue;
        }

        box = wm->Bounds();

        if(renderNodeStep >= 0) {
            dglDepthRange(0, 0);
            kexRenderUtils::DrawBoundingBox(box, 0, 255, 255);
            dglDepthRange(0, 1);
        }

        wm->bCulled = !frustum.TestBoundingBox(box);

        if(wm->bCulled) {
            numCulledStatics++;
            continue;
        }

        if(bNodeQueries) {
            box += (EXTENDED_BOX_SIZE + 24.0f);

            if(renderer.GetOcclusionSampleResult(staticQueries[wm->queryIndex], box)) {
                numOccludedStatics++;
                continue;
            }
        }

        DrawWorldModel(wm);
    }
    
    if(bPrintStats) {
        numDrawnSDNodes++;
    }
}

//
// kexRenderWorld::DrawStaticActors
//

void kexRenderWorld::DrawStaticActors(void) {
    if(bPrintStats) {
        renderStaticsMS = sysMain.GetMS();
        numCulledStatics = 0;
        numOccludedNodes = 0;
        numOccludedStatics = 0;
    }
    
    renderNodeStepNum = 0;

    RecursiveSDNode(0);

    if(bPrintStats) {
        renderStaticsMS = sysMain.GetMS() - renderStaticsMS;
    }
}

//
// kexRenderWorld::DrawSingleActor
//

void kexRenderWorld::DrawSingleActor(kexActor *actor, kexMatrix *matrix) {
    kexBBox box;
    kexMatrix curMatrix;

    box = actor->Bounds();

    if(actor->Model()) {
        curMatrix = actor->Matrix();

        if(actor->bNoFixedTransform == false && matrix) {
            curMatrix = *matrix * curMatrix;
        }

        TraverseDrawActorNode(actor, &actor->Model()->nodes[0], actor->AnimState(), curMatrix);
        
        if(bPrintStats) {
            numDrawnActors++;
        }
    }
}

//
// kexRenderWorld::DrawActors
//

void kexRenderWorld::DrawActors(void) {
    kexFrustum frustum = world->Camera()->Frustum();
    kexMatrix mtx(DEG2RAD(-90), 1);
    kexBBox box;
    kexVec3 camOrg;

    mtx.Scale(-1, 1, 1);

    if(bPrintStats) {
        numOccludedActors = 0;
        numCulledActors = 0;

        renderActorsMS = sysMain.GetMS();
    }

    camOrg = world->Camera()->GetOrigin();

    bool bUseQueries = cvarRenderActorOcclusionQueries.GetBool();

    for(kexActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bStatic) {
                continue;
            }
            if(actor->bHidden || actor->DisplayType() != ODT_NORMAL) {
                continue;
            }

            if(actor->bNoCull == false) {
                if(actor->GetOrigin().DistanceSq(camOrg) >= (actor->CullDistance() * actor->CullDistance())) {
                    actor->bCulled = true;
                }
                else {
                    actor->bCulled = !frustum.TestBoundingBox(actor->Bounds());
                }

                if(actor->bCulled) {
                    numCulledActors++;
                    continue;
                }
            }
            
            if(bUseQueries && actor->queryIndex != -1) {
                box = actor->Bounds();
                box += (EXTENDED_BOX_SIZE + 24.0f);
                if(renderer.GetOcclusionSampleResult(actorQueries[actor->queryIndex], box)) {
                    numOccludedActors++;
                    continue;
                }
            }

            DrawSingleActor(actor, &mtx);
    }

    if(bPrintStats) {
        renderActorsMS = sysMain.GetMS() - renderActorsMS;
    }
}

//
// kexRenderWorld::DrawViewActors
//

void kexRenderWorld::DrawViewActors(drawSurfFunc_t function) {
    kexMatrix projMatrix;
    kexMatrix curMatrix;
    kexMatrix modelMatrix(DEG2RAD(-90), 1);

    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    projMatrix.Identity();
    projMatrix.SetViewProjection(world->Camera()->Aspect(), 45.0f, 32, -1);
    dglMultMatrixf(projMatrix.ToFloatPtr());

    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();

    for(kexActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bHidden || actor->DisplayType() != ODT_CLIENTVIEW) {
                continue;
            }

            if(actor->Model()) {
                curMatrix = modelMatrix * actor->Matrix();
                TraverseDrawActorNode(actor, &actor->Model()->nodes[0], actor->AnimState(), curMatrix);
            }
    }

    renderer.DrawSurfaceList(function, MSO_RESERVED, MSO_RESERVED+1);
    renderer.ClearSurfaceList(MSO_RESERVED, MSO_RESERVED+1);
}

//
// kexRenderWorld::DrawSun
//

void kexRenderWorld::DrawSun(const bool bForceInfiniteProjection) {
    if(sunSpotShader == NULL) {
        return;
    }

    float zfar = world->Camera()->ZFar();

    if(bForceInfiniteProjection) {
        world->Camera()->ZFar() = -1;
        SetCameraView(world->Camera());
    }
    
    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    
    sunSpotShader->Enable();
    sunSpotShader->SetUniform("uCameraTransform", world->Camera()->RotationMatrix());
    sunSpotShader->SetUniform("uOrigin", sunPosition);
    
    renderBackend.defaultTexture.Bind();
    
    kexRenderUtils::DrawTexturedQuad(kexVec2(-1024, 0), kexVec2(1024, 0), -1024, 1024, 255, 255, 255);

    if(bForceInfiniteProjection) {
        world->Camera()->ZFar() = zfar;
        dglClear(GL_DEPTH_BUFFER_BIT);
    }
}

//
// kexRenderWorld::DrawForegroundActors
//

void kexRenderWorld::DrawForegroundActors(void) {
    kexFrustum frustum = world->Camera()->Frustum();
    float zfar = world->Camera()->ZFar();

    world->Camera()->ZFar() = -1;
    SetCameraView(world->Camera());

    for(kexActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bHidden || actor->DisplayType() != ODT_FOREGROUND) {
                continue;
            }

            if(actor->bNoCull == false) {
                actor->bCulled = !frustum.TestBoundingBox(actor->Bounds());

                if(actor->bCulled) {
                    continue;
                }
            }

            DrawSingleActor(actor, NULL);
    }

    renderer.DrawSurfaceList(bWireframe ?
                            &kexRenderer::DrawWireFrameSurface :
                            &kexRenderer::DrawSurface,
                            MSO_RESERVED, MSO_RESERVED+1);

    renderer.ClearSurfaceList(MSO_RESERVED, MSO_RESERVED+1);

    if(CameraRenderSky()) {
        DrawSun(false);
    }

    world->Camera()->ZFar() = zfar;
    dglClear(GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------
//
// particle fx rendering
//
//-----------------------------------------------------------------------------

//
// kexRenderWorld::SortSprites
//

int kexRenderWorld::SortSprites(const void *a, const void *b) {
    kexFx *xa = ((fxDisplay_t*)a)->fx;
    kexFx *xb = ((fxDisplay_t*)b)->fx;
    
    return (int)(xb->Distance() - xa->Distance());
}

//
// kexRenderWorld::DrawFX
//

void kexRenderWorld::DrawFX(void) {
    static fxDisplay_t fxDisplayList[MAX_FX_DISPLAYS];
    
    int fxDisplayNum;
    fxinfo_t *fxinfo;
    
    memset(fxDisplayList, 0, sizeof(fxDisplay_t) * MAX_FX_DISPLAYS);
    
    // gather particle fx and add to display list for sorting
    for(world->fxRover = world->fxList.Next(), fxDisplayNum = 0;
        world->fxRover != NULL; world->fxRover = world->fxRover->worldLink.Next()) {
        if(fxDisplayNum >= MAX_FX_DISPLAYS) {
            break;
        }
        if(world->fxRover == NULL) {
            break;
        }
        if(world->fxRover->restart > 0) {
            continue;
        }
        if(world->fxRover->IsStale()) {
            continue;
        }
        if(world->fxRover->fxInfo->drawtype == VFX_DRAWHIDDEN) {
            continue;
        }
        
        fxDisplayList[fxDisplayNum++].fx = world->fxRover;

        fxinfo = world->fxRover->fxInfo;

        if(fxinfo->bLensFlares && fxinfo->lensFlares) {
            fxinfo->lensFlares->Draw(world->fxRover->GetOrigin());
        }
    }

    numDrawnFX = fxDisplayNum;
    
    if(fxDisplayNum <= 0) {
        return;
    }
    
    qsort(fxDisplayList, fxDisplayNum, sizeof(fxDisplay_t), kexRenderWorld::SortSprites);
    
    renderer.DrawFX(fxDisplayList, fxDisplayNum);
}

//-----------------------------------------------------------------------------
//
// debugging stuff
//
//-----------------------------------------------------------------------------

//
// kexRenderWorld::PrintStats
//

void kexRenderWorld::PrintStats(void) {
    if(!bPrintStats) {
        return;
    }
    
    kexRenderUtils::PrintStatsText("scene time", ": %ims", renderSceneMS);
    kexRenderUtils::PrintStatsText("statics time", ": %ims", renderStaticsMS);
    kexRenderUtils::PrintStatsText("actor time", ": %ims", renderActorsMS);
    kexRenderUtils::PrintStatsText("fx time", ": %ims", renderFXMS);
    kexRenderUtils::PrintStatsText("query time", ": %ims", renderOcclusionPrePassMS);
    kexRenderUtils::AddDebugLineSpacing();
    kexRenderUtils::PrintStatsText("drawn statics", ": %i", numDrawnStatics);
    kexRenderUtils::PrintStatsText("nodes visited", ": %i", numDrawnSDNodes);
    kexRenderUtils::PrintStatsText("drawn actors", ": %i", numDrawnActors);
    kexRenderUtils::PrintStatsText("drawn fx", ": %i", numDrawnFX);
    kexRenderUtils::AddDebugLineSpacing();
    kexRenderUtils::PrintStatsText("culled statics", ": %i", numCulledStatics);
    kexRenderUtils::PrintStatsText("culled actors", ": %i", numCulledActors);
    kexRenderUtils::AddDebugLineSpacing();
    kexRenderUtils::PrintStatsText("occluded nodes", ": %i", numOccludedNodes);
    kexRenderUtils::PrintStatsText("occluded statics", ": %i", numOccludedStatics);
    kexRenderUtils::PrintStatsText("occluded actors", ": %i", numOccludedActors);
    kexRenderUtils::PrintStatsText("actor query size", ": %i", actorQueries.Length());
    kexRenderUtils::AddDebugLineSpacing();
}

//
// kexRenderWorld::DebugObjects
//

void kexRenderWorld::DebugObjects(void) {
    if(kexAI::debugAI) {
        kexRenderUtils::DrawBoundingBox(kexAI::debugAI->Bounds(), 255, 0, 255);
    }

    if(!bShowBBox && !bShowRadius && !bShowOrigin) {
        return;
    }

    for(kexWorldModel *wm = world->staticActors.Next(); wm != NULL; wm = wm->worldLink.Next()) {
        if(!world->Camera()->Frustum().TestBoundingBox(wm->Bounds())) {
            continue;
        }
        if(bShowBBox) {
            if(wm->bTraced) {
                kexRenderUtils::DrawBoundingBox(wm->Bounds(), 255, 0, 0);
                wm->bTraced = false;
            }
            else {
                kexRenderUtils::DrawBoundingBox(wm->Bounds(), 255, 255, 0);
            }
        }
        if(bShowRadius && wm->bCollision) {
            kexVec3 org = wm->GetOrigin();
            kexRenderUtils::DrawRadius(org[0], org[1], org[2],
                                       wm->Radius(), wm->Height(), 255, 128, 128);
        }
    }

    for(kexActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(!world->Camera()->Frustum().TestBoundingBox(actor->Bounds())) {
                continue;
            }
            if(bShowOrigin) {
                kexRenderUtils::DrawOrigin(actor->GetOrigin().x,
                                           actor->GetOrigin().y,
                                           actor->GetOrigin().z, 32);
            }

            if(bShowBBox) {
                if(actor->bTraced) {
                    kexRenderUtils::DrawBoundingBox(actor->Bounds(), 255, 0, 0);
                    actor->bTraced = false;
                }
                else {
                    kexRenderUtils::DrawBoundingBox(actor->Bounds(),
                        actor->bTouch ? 0 : 255,
                        actor->bTouch ? 255 : 128,
                        actor->bTouch ? 0 : 128);
                }
            }
            if(bShowRadius && actor->bCollision) {
                kexVec3 org = actor->GetOrigin();
                kexRenderUtils::DrawRadius(org[0], org[1], org[2],
                    actor->Radius(), actor->BaseHeight(), 255, 128, 128);
                kexRenderUtils::DrawRadius(org[0], org[1], org[2],
                    actor->Radius() * 0.5f, actor->Height(), 128, 128, 255);
                kexRenderUtils::DrawRadius(org[0], org[1], org[2],
                    actor->Radius() * 0.5f, actor->GetViewHeight(), 128, 255, 128);
            }
    }
}

//
// kexRenderWorld::DrawAreaNode
//

void kexRenderWorld::DrawAreaNode(void) {
    kexSDNodeObj<kexWorldObject> *nodes;

    dglDepthRange(0.0f, 0.0f);

    for(unsigned int i = 0; i < world->areaNodes.numNodes; i++) {
        nodes = &world->areaNodes.nodes[i];
        kexRenderUtils::DrawBoundingBox(nodes->bounds, 64, 128, 255);
    }

    nodes = gameManager.localPlayer.Puppet()->areaLink.node;
    if(nodes) {
        kexRenderUtils::DrawBoundingBox(nodes->bounds, 255, 0, 0);
    }

    dglDepthRange(0.0f, 1.0f);
}

//
// kexRenderWorld::DrawRenderNode
//

void kexRenderWorld::DrawRenderNode(void) {
    kexSDNodeObj<kexWorldModel> *nodes;
    kexWorldModel *wm;
    int nodenum;

    dglDepthRange(0.0f, 0.0f);

    for(unsigned int i = 0; i < renderNodes.numNodes; i++) {
        nodes = &renderNodes.nodes[i];
        kexRenderUtils::DrawBoundingBox(nodes->bounds, 0, 255, 0);
    }

    nodenum = renderNodes.PointInNode(world->Camera()->GetOrigin(), 16);
    nodes = &renderNodes.nodes[nodenum];

    kexRenderUtils::DrawBoundingBox(nodes->bounds, 255, 255, 0);

    for(wm = nodes->objects.Next(); wm != NULL; wm = wm->renderNode.link.Next()) {
        kexRenderUtils::DrawBoundingBox(wm->Bounds(), 0, 255, 255);
    }

    dglDepthRange(0.0f, 1.0f);
}

//
// kexRenderWorld::DrawTriangle
//

void kexRenderWorld::DrawTriangle(const kexTri &tri, const word index,
                               byte r, byte g, byte b, byte a) {
    for(int j = 0; j < 3; j++) {
        renderer.AddVertex(tri.point[j]->x,
                                tri.point[j]->y,
                                tri.point[j]->z,
                                0,
                                0,
                                r,
                                g,
                                b,
                                a);
    }
    
    renderer.AddTriangle(index + 0, index + 1, index + 2);
}

//
// kexRenderWorld::DrawSectors
//

void kexRenderWorld::DrawSectors(kexSector *sectors, const int count) {
    if(!localWorld.CollisionMap().IsLoaded() || sectors == NULL || count <= 0) {
        return;
    }
    
    kexFrustum frustum = localWorld.Camera()->Frustum();
    int idx = 0;
    int num = 0;
    kexVec3 pt;
    kexSector *sector;
    kexTri *tri;
    kexVec3 n;
    
    renderBackend.DisableShaders();
    
    for(int i = 0; i < count; i++) {
        byte r, g, b, a;
        
        sector = &sectors[i];
        tri = &sector->lowerTri;
        
        if(frustum.TestTriangle(*tri)) {
            if(sector->bTraced == true) {
                r = g = b = a = 255;
                sector->bTraced = false;
            }
            else {
                r = 192;
                a = 255;
                g = b = 96;
            }
            
            DrawTriangle(*tri, idx, r, g, b, a);
            idx += 3;
            num++;
        }
        
        if(sector->flags & CLF_CHECKHEIGHT) {
            tri = &sector->upperTri;
            
            if(frustum.TestTriangle(*tri)) {
                DrawTriangle(*tri, idx, 96, 192, 96, 255);
                idx += 3;
                num++;
            }
        }
        
        if(sector->area && sector->area->Flags() & AAF_WATER) {
            for(int j = 0; j < 3; j++) {
                renderer.AddVertex(tri->point[j]->x,
                                        sector->area->WaterPlane(),
                                        tri->point[j]->z,
                                        0,
                                        0,
                                        32,
                                        0,
                                        255,
                                        192);
            }
            renderer.AddTriangle(idx + 0, idx + 1, idx + 2);
            
            idx += 3;
            num++;
        }
    }
    
    if(num != 0) {
        renderBackend.SetState(GLSTATE_CULL, true);
        renderBackend.SetState(GLSTATE_TEXTURE0, false);
        renderBackend.SetState(GLSTATE_BLEND, true);
        renderBackend.SetState(GLSTATE_ALPHATEST, true);
        
        renderer.BindDrawPointers();
        renderer.DrawElementsNoShader(false);
        
        // draw wireframe outline
        renderBackend.SetPolyMode(GLPOLY_LINE);
        
        dglDisableClientState(GL_COLOR_ARRAY);
        dglColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
        
        renderer.DrawElementsNoShader();
        renderBackend.SetPolyMode(GLPOLY_FILL);
        
        dglEnableClientState(GL_COLOR_ARRAY);
        
        renderBackend.SetState(GLSTATE_CULL, false);
        dglLineWidth(2.0f);
        
        // draw plane normal vectors
        for(int i = 0; i < count; i++) {
            sector = &sectors[i];
            tri = &sector->lowerTri;
            
            if(frustum.TestTriangle(*tri)) {
                n = tri->plane.Normal();
                pt = tri->GetCenterPoint();
                
                renderer.AddLine(pt.x,
                                      pt.y,
                                      pt.z,
                                      pt.x + (16 * n[0]),
                                      pt.y + (16 * n[1]),
                                      pt.z + (16 * n[2]),
                                      0,
                                      32,
                                      255,
                                      255,
                                      0,
                                      255,
                                      0,
                                      255);
            }
            
            if(sector->flags & CLF_CHECKHEIGHT) {
                tri = &sector->upperTri;
                
                if(frustum.TestTriangle(*tri)) {
                    n = tri->plane.Normal();
                    pt = tri->GetCenterPoint();
                    
                    renderer.AddLine(pt.x,
                                          pt.y,
                                          pt.z,
                                          pt.x + (16 * n[0]),
                                          pt.y + (16 * n[1]),
                                          pt.z + (16 * n[2]),
                                          255,
                                          0,
                                          0,
                                          255,
                                          255,
                                          255,
                                          32,
                                          255);
                }
            }
        }
        
        renderer.DrawLineElements();
        dglLineWidth(1.0f);
        
        renderBackend.SetState(GLSTATE_TEXTURE0, true);
    }
}
