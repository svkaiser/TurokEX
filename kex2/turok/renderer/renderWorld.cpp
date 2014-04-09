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

kexCvar cvarRenderFog("r_fog", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarRenderCull("r_cull", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarRenderFxTexture("r_fxtexture", CVF_BOOL|CVF_CONFIG, "1", "TODO");

kexRenderWorld renderWorld;

//
// FCmd_ShowCollision
//

static void FCmd_ShowCollision(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowClipMesh ^= 1;
    renderWorld.bWireframe = 0;
}

//
// FCmd_ShowGridBounds
//

static void FCmd_ShowGridBounds(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowGrid ^= 1;
}

//
// FCmd_ShowBoundingBox
//

static void FCmd_ShowBoundingBox(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowBBox ^= 1;
}

//
// FCmd_ShowRadius
//

static void FCmd_ShowRadius(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowRadius ^= 1;
}

//
// FCmd_ShowOrigin
//

static void FCmd_ShowOrigin(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowOrigin ^= 1;
}

//
// FCmd_ShowCollisionMap
//

static void FCmd_ShowCollisionMap(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowCollisionMap ^= 1;
    renderWorld.bWireframe = 0;
}

//
// FCmd_ShowWireframe
//

static void FCmd_ShowWireFrame(void) {
    if(command.GetArgc() < 1)
        return;

    renderWorld.bWireframe ^= 1;
    renderWorld.bShowClipMesh = 0;
    renderWorld.bShowCollisionMap = 0;
}

//
// FCmd_ShowWorldNode
//

static void FCmd_ShowWorldNode(void) {
    if(command.GetArgc() < 2) {
        return;
    }

    renderWorld.showWorldNode = atoi(command.GetArgv(1));
}

//
// FCmd_ShowAreaNode
//

static void FCmd_ShowAreaNode(void) {
    if(command.GetArgc() < 2) {
        return;
    }

    renderWorld.showAreaNode = atoi(command.GetArgv(1));
}

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
    this->showWorldNode     = -1;
    this->showAreaNode      = -1;
}

//
// kexRenderWorld::Init
//

void kexRenderWorld::Init(void) {
    command.Add("showclipmesh", FCmd_ShowCollision);
    command.Add("showgridbounds", FCmd_ShowGridBounds);
    command.Add("showbbox", FCmd_ShowBoundingBox);
    command.Add("showradius", FCmd_ShowRadius);
    command.Add("showorigin", FCmd_ShowOrigin);
    command.Add("drawwireframe", FCmd_ShowWireFrame);
    command.Add("showworldnode", FCmd_ShowWorldNode);
    command.Add("showareanode", FCmd_ShowAreaNode);
    command.Add("showcollision", FCmd_ShowCollisionMap);
}

//
// kexRenderWorld::RenderScene
//

void kexRenderWorld::RenderScene(void) {
    if(!world->IsLoaded()) {
        return;
    }

    kexVec4 fogRGB = world->GetCurrentFogRGB();
    dglClearColor(fogRGB[0], fogRGB[1], fogRGB[2], fogRGB[3]);

    world->Camera()->SetupMatrices();

    dglMatrixMode(GL_PROJECTION);
    dglLoadMatrixf(world->Camera()->Projection().ToFloatPtr());
    dglMatrixMode(GL_MODELVIEW);
    dglLoadMatrixf(world->Camera()->ModelView().ToFloatPtr());

    worldLightTransform = (world->worldLightOrigin | world->Camera()->RotationMatrix()).ToVec3();

    /*if(bWireframe) {
        renderBackend.SetPolyMode(GLPOLY_LINE);
    }*/

    DrawStaticActors();
    DrawActors();

    /*if(showWorldNode >= 0 || showAreaNode >= 0) {
        renderBackend.SetState(GLSTATE_DEPTHTEST, false);
        if(showWorldNode >= 0) {
            DrawWorldNode(&world->worldNode);
        }
        if(showAreaNode >= 0) {
            DrawAreaNode();
        }
        renderBackend.SetState(GLSTATE_DEPTHTEST, true);
    }*/

    if(bShowCollisionMap) {
        renderer.DrawSectors(world->CollisionMap().sectors,
            world->CollisionMap().numSectors);
    }

    renderer.DrawFX();
    DrawViewActors();

    /*if(bWireframe) {
        renderBackend.SetPolyMode(GLPOLY_FILL);
    }*/
}

//
// kexRenderWorld::TraverseDrawActorNode
//

void kexRenderWorld::TraverseDrawActorNode(kexActor *actor,
                                           const modelNode_t *node,
                                           kexAnimState *animState) {
    unsigned int i;
    const kexModel_t *model = actor->Model();
    int nodenum = node - model->nodes;

    if(animState != NULL) {
        kexAnim_t *anim;
        kexAnim_t *prevanim;
        int frame;
        int nextframe;
        float delta;

        dglPushMatrix();

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

            dglMultMatrixf(translation.ToFloatPtr());
        }
    }

    for(i = 0; i < node->numSurfaces; i++) {
        surface_t *surface = &node->surfaces[i];
        char *materialPath = surface->material;

        if(actor->materials != NULL) {
            char *mat = actor->materials[i];

            if(mat != NULL && mat[0] != '-')
                materialPath = mat;
        }

        kexMaterial *material = renderBackend.CacheMaterial(materialPath);
        renderer.DrawSurface(surface, material);
    }

    for(i = 0; i < node->numChildren; i++) {
        TraverseDrawActorNode(actor,
            &model->nodes[node->children[i]], animState);
    }

    if(animState != NULL) {
        dglPopMatrix();
    }
}

//
// kexRenderWorld::DrawStaticActors
//

void kexRenderWorld::DrawStaticActors(void) {
    kexBBox box;
    kexFrustum frustum = world->Camera()->Frustum();

    for(kexActor *actor = world->staticActors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bHidden) {
                if(bShowClipMesh) {
                    actor->ClipMesh().DebugDraw();
                }
                continue;
            }

            box = actor->Bounds();
            actor->bCulled = !frustum.TestBoundingBox(box);

            if(actor->bCulled) {
                continue;
            }

            if(!bShowClipMesh && !bShowCollisionMap) {
                dglPushMatrix();
                dglMultMatrixf(actor->Matrix().ToFloatPtr());

                if(bWireframe) {
                    dglColor4ub(0, 224, 224, 255);
                }
                TraverseDrawActorNode(actor, &actor->Model()->nodes[0], NULL);
                dglPopMatrix();
            }
            else if(bShowClipMesh) {
                actor->ClipMesh().DebugDraw();
            }

            if(bShowBBox) {
                if(actor->bTraced) {
                    renderer.DrawBoundingBox(box, 255, 0, 0);
                    actor->bTraced = false;
                }
                else {
                    renderer.DrawBoundingBox(box, 255, 255, 0);
                }
            }
            if(bShowRadius && actor->bCollision) {
                kexVec3 org = actor->GetOrigin();
                renderer.DrawRadius(org[0], org[1], org[2],
                    actor->Radius(), actor->Height(), 255, 128, 128);
            }
    }
}

//
// kexRenderWorld::DrawActors
//

void kexRenderWorld::DrawActors(void) {
    kexBBox box;
    kexFrustum frustum = world->Camera()->Frustum();
    kexMatrix mtx(DEG2RAD(-90), 1);
    mtx.Scale(-1, 1, 1);

    for(kexActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bStatic) {
                continue;
            }
            if(actor->bHidden || actor->bClientView) {
                continue;
            }

            box = actor->Bounds();
            actor->bCulled = !frustum.TestBoundingBox(box);

            if(actor->bCulled) {
                continue;
            }

            if(actor->Model()) {
                dglPushMatrix();
                dglMultMatrixf(actor->Matrix().ToFloatPtr());

                if(bShowOrigin) {
                    renderer.DrawOrigin(0, 0, 0, 32);
                }

                if(actor->bNoFixedTransform == false) {
                    dglPushMatrix();
                    dglMultMatrixf(mtx.ToFloatPtr());
                }

                if(bWireframe) {
                    dglColor4ub(192, 192, 192, 255);
                }

                TraverseDrawActorNode(actor, &actor->Model()->nodes[0], actor->AnimState());

                dglPopMatrix();

                if(actor->bNoFixedTransform == false) {
                    dglPopMatrix();
                }
            }

            if(bShowBBox) {
                if(actor->bTraced) {
                    renderer.DrawBoundingBox(box, 255, 0, 0);
                    actor->bTraced = false;
                }
                else {
                    renderer.DrawBoundingBox(box,
                        actor->bTouch ? 0 : 255,
                        actor->bTouch ? 255 : 128,
                        actor->bTouch ? 0 : 128);
                }
            }
            if(bShowRadius && actor->bCollision) {
                kexVec3 org = actor->GetOrigin();
                renderer.DrawRadius(org[0], org[1], org[2],
                    actor->Radius(), actor->BaseHeight(), 255, 128, 128);
                renderer.DrawRadius(org[0], org[1], org[2],
                    actor->Radius() * 0.5f, actor->Height(), 128, 128, 255);
                renderer.DrawRadius(org[0], org[1], org[2],
                    actor->Radius() * 0.5f, actor->GetViewHeight(), 128, 255, 128);
            }
    }
}

//
// kexRenderWorld::DrawViewActors
//

void kexRenderWorld::DrawViewActors(void) {
    kexMatrix projMatrix;
    kexMatrix modelMatrix(DEG2RAD(-90), 1);

    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    projMatrix.Identity();
    projMatrix.SetViewProjection(world->Camera()->Aspect(), 45.0f, 32, -1);
    dglMultMatrixf(projMatrix.ToFloatPtr());

    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();

    renderBackend.SetCull(GLCULL_FRONT);

    for(kexActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bHidden || !actor->bClientView) {
                continue;
            }

            if(actor->Model()) {
                dglPushMatrix();
                dglMultMatrixf(actor->Matrix().ToFloatPtr());
                dglPushMatrix();

                dglMultMatrixf(modelMatrix.ToFloatPtr());

                if(bWireframe) {
                    if(actor->bStatic) {
                        dglColor4ub(0, 224, 224, 255);
                    }
                    else {
                        dglColor4ub(192, 192, 192, 255);
                    }
                }

                TraverseDrawActorNode(actor, &actor->Model()->nodes[0], actor->AnimState());

                dglPopMatrix();
                dglPopMatrix();
            }
    }
}

//
// kexRenderWorld::DrawWorldNode
//

void kexRenderWorld::DrawWorldNode(worldNode_t *node) {
    if(node->bLeaf) {
        renderer.DrawBoundingBox(node->bounds, 0, 255, 0);
        if(showWorldNode >= 1) {
            for(unsigned int i = 0; i < node->actors.Length(); i++) {
                renderer.DrawBoundingBox(node->actors[i]->Bounds(), 0, 255, 255);
            }
        }
        return;
    }
    else {
        renderer.DrawBoundingBox(node->bounds, 255, 255, 0);
    }

    if(showWorldNode >= 1) {
        float d = node->plane.Normal().Dot(world->Camera()->GetOrigin()) - node->plane.d;

        if(d >= 0) {
            if(node->children[0]) {
                DrawWorldNode(node->children[0]);
            }
        }
        else {
            if(node->children[1]) {
                DrawWorldNode(node->children[1]);
            }
        }
    }
    else {
        if(node->children[0]) {
            DrawWorldNode(node->children[0]);
        }
        if(node->children[1]) {
            DrawWorldNode(node->children[1]);
        }
    }
}

//
// kexRenderWorld::DrawAreaNode
//

void kexRenderWorld::DrawAreaNode(void) {
    areaNode_t *nodes;

    dglDepthRange(0.0f, 0.0f);

    for(int i = 0; i < world->NumAreaNodes(); i++) {
        nodes = &world->areaNodes[i];
        renderer.DrawBoundingBox(nodes->bounds, 64, 128, 255);
    }

    for(int i = 0; i < world->NumAreaNodes(); i++) {
        nodes = &world->areaNodes[i];
        for(kexActor *actor = static_cast<kexActor*>(nodes->objects.Next());
            actor != NULL;
            actor = static_cast<kexActor*>(actor->areaLink.Next())) {
                if(actor == gameManager.localPlayer.Puppet()) {
                    renderer.DrawBoundingBox(nodes->bounds, 255, 0, 0);
                }
                else {
                    renderBackend.SetState(GLSTATE_TEXTURE0, false);
                    dglBegin(GL_LINES);
                    dglColor4ub(255, 0, 0, 255);
                    dglVertex3fv(nodes->bounds.Center().ToFloatPtr());
                    dglVertex3fv(actor->GetOrigin().ToFloatPtr());
                    dglEnd();
                    renderBackend.SetState(GLSTATE_TEXTURE0, true);
                }
        }
    }

    dglDepthRange(0.0f, 1.0f);
}
