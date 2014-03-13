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
#include "renderSystem.h"
#include "renderWorld.h"
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
        dglClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        return;
    }

    SetupGlobalFog();

    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    renderSystem.SetCull(GLCULL_BACK);
    renderSystem.SetState(GLSTATE_DEPTHTEST, true);

    dglDisableClientState(GL_COLOR_ARRAY);
    dglEnableClientState(GL_NORMAL_ARRAY);

    world->Camera()->SetupMatrices();

    dglMatrixMode(GL_PROJECTION);
    dglLoadMatrixf(world->Camera()->Projection().ToFloatPtr());
    dglMatrixMode(GL_MODELVIEW);
    dglLoadMatrixf(world->Camera()->ModelView().ToFloatPtr());

    if(bWireframe) {
        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    SetupGlobalLight();
    DrawStaticActors();
    DrawActors();

    if(showWorldNode >= 0 || showAreaNode >= 0) {
        renderSystem.SetState(GLSTATE_DEPTHTEST, false);
        if(showWorldNode >= 0) {
            DrawWorldNode(&world->worldNode);
        }
        if(showAreaNode >= 0) {
            DrawAreaNode();
        }
        renderSystem.SetState(GLSTATE_DEPTHTEST, true);
    }

    if(bShowCollisionMap) {
        world->CollisionMap().DebugDraw();
    }

    DrawFX();
    DrawViewActors();

    dglEnableClientState(GL_COLOR_ARRAY);
    dglDisableClientState(GL_NORMAL_ARRAY);

    if(bWireframe) {
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    renderSystem.SetState(GLSTATE_DEPTHTEST, false);
    renderSystem.SetState(GLSTATE_CULL, true);
    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    renderSystem.SetState(GLSTATE_BLEND, false);
    renderSystem.SetState(GLSTATE_ALPHATEST, false);
    renderSystem.SetState(GLSTATE_TEXGEN_S, false);
    renderSystem.SetState(GLSTATE_TEXGEN_T, false);
    renderSystem.SetState(GLSTATE_LIGHTING, false);
    renderSystem.SetState(GLSTATE_FOG, false);

    // TODO - NEEDS SHADERS
    dglDisable(GL_LIGHT0);
    dglDisable(GL_COLOR_MATERIAL);
}

//
// kexRenderWorld::SetupGlobalFog
//

void kexRenderWorld::SetupGlobalFog(void) {
    if((!bShowClipMesh && !bShowCollisionMap) && cvarRenderFog.GetBool() &&
        localWorld.FogEnabled() && !bWireframe) {
        float *rgb = localWorld.GetCurrentFogRGB();
        dglClearColor(rgb[0], rgb[1], rgb[2], 1.0f);

        dglFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
        dglFogfv(GL_FOG_COLOR, rgb);
        dglFogf(GL_FOG_START, localWorld.GetFogNear());
        dglFogf(GL_FOG_END, localWorld.GetFogFar());
    }
    else {
        dglClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    }
}

//
// kexRenderWorld::SetupGlobalLight
//

void kexRenderWorld::SetupGlobalLight(void) {
    if(bWireframe) {
        return;
    }

    // TODO - NEEDS SHADERS
    renderSystem.SetState(GLSTATE_LIGHTING, true);

    dglEnable(GL_LIGHT0);
    dglEnable(GL_COLOR_MATERIAL);
    dglColorMaterial(GL_FRONT, GL_DIFFUSE);
    dglColorMaterial(GL_BACK, GL_DIFFUSE);

    dglLightfv(GL_LIGHT0, GL_POSITION, world->worldLightOrigin.ToFloatPtr());
    dglMaterialfv(GL_FRONT, GL_DIFFUSE, world->worldLightColor.ToFloatPtr());
    dglMaterialfv(GL_FRONT, GL_AMBIENT, world->worldLightAmbience.ToFloatPtr());
    dglLightModelfv(GL_LIGHT_MODEL_AMBIENT, world->worldLightModelAmbience.ToFloatPtr());
}

//
// kexRenderWorld::DrawSurface
//

void kexRenderWorld::DrawSurface(const surface_t *surface, const char *texturePath) {
    kexTexture *tex;
    rcolor color;

    if(texturePath == NULL) {
        texturePath = surface->texturePath;
    }

    renderSystem.SetState(GLSTATE_FOG, (localWorld.FogEnabled() && !bWireframe && !bShowCollisionMap));
    renderSystem.SetState(GLSTATE_CULL, (surface->flags & MDF_NOCULLFACES) == 0);
    renderSystem.SetState(GLSTATE_BLEND, (surface->flags & (MDF_MASKED|MDF_TRANSPARENT1)) != 0);
    renderSystem.SetState(GLSTATE_ALPHATEST, (surface->flags & (MDF_MASKED|MDF_TRANSPARENT1)) != 0);
    renderSystem.SetState(GLSTATE_TEXGEN_S, (surface->flags & MDF_SHINYSURFACE) != 0);
    renderSystem.SetState(GLSTATE_TEXGEN_T, (surface->flags & MDF_SHINYSURFACE) != 0);

    if(surface->flags & MDF_COLORIZE) {
        color = surface->color1;
        renderSystem.SetState(GLSTATE_LIGHTING, false);
    }
    else {
        color = COLOR_WHITE;
        renderSystem.SetState(GLSTATE_LIGHTING, true);
    }

    if(surface->flags & MDF_MASKED) {
        renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.6525f);
    }
    else {
        renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    }

    dglNormalPointer(GL_FLOAT, sizeof(float)*3, surface->normals);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, surface->coords);
    dglVertexPointer(3, GL_FLOAT, sizeof(kexVec3), surface->vertices);

    if(!bWireframe) {
        tex = renderSystem.CacheTexture(texturePath, TC_REPEAT);

        if(tex) {
            renderSystem.SetState(GLSTATE_LIGHTING, true);
            tex->Bind();
        }
        else {
            renderSystem.SetState(GLSTATE_LIGHTING, false);
            renderSystem.whiteTexture.Bind();
            color = surface->color1;
        }

        if(surface->flags & MDF_TRANSPARENT1) {
            renderSystem.SetState(GLSTATE_LIGHTING, false);
            color = color & 0xffffff;
            color |= (160 << 24);
        }

        dglColor4ubv((byte*)&color);
    }
    else {
        renderSystem.SetState(GLSTATE_LIGHTING, false);
        renderSystem.whiteTexture.Bind();
    }

    dglDrawElements(GL_TRIANGLES, surface->numIndices, GL_UNSIGNED_SHORT, surface->indices);
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
                if(animState->flags & ANF_ROOTMOTION) {
                    if(nextframe >= frame && animState->frameTime > 0) {
                        kexMatrix mtx(DEG2RAD(-90), 1);
                        mtx.Scale(-1, 1, 1);

                        kexVec3 offs = (pos_next - pos_cur) | mtx;
                        animState->baseOffset = -pos_cur[2] * actor->GetScale()[1];
                        animState->rootMotion = (offs * actor->GetScale()) *
                            (60.0f / animState->frameTime);
                    }
                }

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

    if(node->numSurfaceGroups > 0) {
        unsigned int var;

        if(node->variants == NULL) {
            var = 0;
        }
        else {
            var = (actor->Variant() >= (int)node->numVariants) ?
                0 : node->variants[actor->Variant()];
        }

        if(var >= node->numSurfaceGroups) {
            var = 0;
        }

        for(i = 0; i < node->surfaceGroups[var].numSurfaces; i++) {
            surface_t *surface = &node->surfaceGroups[var].surfaces[i];
            char *texturepath = NULL;

            if(actor->textureSwaps != NULL) {
                char *meshTexture = actor->textureSwaps[nodenum][var][i];

                if(meshTexture != NULL && meshTexture[0] != '-')
                    texturepath = meshTexture;
            }

            DrawSurface(surface, texturepath);
        }
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
                    DrawBoundingBox(box, 255, 0, 0);
                    actor->bTraced = false;
                }
                else {
                    DrawBoundingBox(box, 255, 255, 0);
                }
            }
            if(bShowRadius && actor->bCollision) {
                kexVec3 org = actor->GetOrigin();
                DrawRadius(org[0], org[1], org[2],
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
                    DrawOrigin(0, 0, 0, 32);
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
                    DrawBoundingBox(box, 255, 0, 0);
                    actor->bTraced = false;
                }
                else {
                    DrawBoundingBox(box,
                        actor->bTouch ? 0 : 255,
                        actor->bTouch ? 255 : 128,
                        actor->bTouch ? 0 : 128);
                }
            }
            if(bShowRadius && actor->bCollision) {
                kexVec3 org = actor->GetOrigin();
                DrawRadius(org[0], org[1], org[2],
                    actor->Radius(), actor->BaseHeight(), 255, 128, 128);
                DrawRadius(org[0], org[1], org[2],
                    actor->Radius() * 0.5f, actor->Height(), 128, 128, 255);
                DrawRadius(org[0], org[1], org[2],
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

    renderSystem.SetCull(GLCULL_FRONT);
    renderSystem.SetState(GLSTATE_LIGHTING, true);

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

    renderSystem.SetState(GLSTATE_LIGHTING, false);
}

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
    static const word   spriteIndices[6] = { 0, 1, 2, 2, 1, 3 };
    static float        spriteTexCoords[8] = { 0, 1, 1, 1, 0, 0, 1, 0 };
    static float        spriteVertices[4][3];
    static byte         spriteColors[4][4];
    bool                bShowTexture;
    int                 fxDisplayNum;
    int                 i;
    int                 j;
    kexMatrix           mtx;
    kexMatrix           scalemtx;
    kexMatrix           finalmtx;
    kexFx               *fx;
    float               scale;
    float               w;
    float               h;
    float               y;
    fxinfo_t            *fxinfo;
    kexTexture          *texture;

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

            fxDisplayList[fxDisplayNum++].fx = world->fxRover;
    }

    if(fxDisplayNum <= 0) {
        return;
    }

    renderSystem.SetState(GLSTATE_LIGHTING, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_ALPHATEST, true);
    renderSystem.SetState(GLSTATE_TEXGEN_S, false);
    renderSystem.SetState(GLSTATE_TEXGEN_T, false);

    renderSystem.SetCull(GLCULL_FRONT);
    renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);

    bShowTexture = cvarRenderFxTexture.GetBool();

    qsort(fxDisplayList, fxDisplayNum, sizeof(fxDisplay_t), kexRenderWorld::SortSprites);

    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, spriteTexCoords);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, spriteVertices);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, spriteColors);

    dglDisableClientState(GL_NORMAL_ARRAY);

    if(!bWireframe) {
        dglEnableClientState(GL_COLOR_ARRAY);
        dglEnable(GL_POLYGON_OFFSET_FILL);
    }

    // draw sorted fx list
    for(i = 0; i < fxDisplayNum; i++) {
        fx = fxDisplayList[i].fx;
        fxinfo = fx->fxInfo;

        if(fxinfo == NULL) {
            continue;
        }

        scale = fx->drawScale * 0.01f;
        scalemtx = kexMatrix(fx->rotationOffset + DEG2RAD(180), 2);

        if(fxinfo->screen_offset_x != 0 || fxinfo->screen_offset_y != 0) {
            scalemtx *= kexVec3(fxinfo->screen_offset_x, fxinfo->screen_offset_y, 0);
        }

        scalemtx.Scale(scale, scale, scale);

        switch(fxinfo->drawtype) {
        case VFX_DRAWFLAT:
        case VFX_DRAWDECAL:
            mtx = kexMatrix(DEG2RAD(90), 1);
            renderSystem.SetState(GLSTATE_CULL, false);
            break;
        case VFX_DRAWBILLBOARD:
            mtx = kexMatrix(kexQuat(world->Camera()->GetAngles().yaw, 0, 1, 0));
            renderSystem.SetState(GLSTATE_CULL, true);
            break;
        case VFX_DRAWSURFACE:
            mtx = kexMatrix(fx->GetRotation());
            renderSystem.SetState(GLSTATE_CULL, false);
            break;
        default:
            mtx = kexMatrix(world->Camera()->GetRotation());
            renderSystem.SetState(GLSTATE_CULL, true);
            break;
        }

        texture = fx->Texture();
        y = fx->GetOrigin().y;

        // snap sprite to floor based on texture height
        if(fxinfo->bOffsetFromFloor) {
            y += 3.42f;
            if(fxinfo->drawtype == VFX_DRAWBILLBOARD) {
                y += (float)texture->OriginalHeight();
            }
        }

        finalmtx = (scalemtx | mtx);
        finalmtx.AddTranslation(fx->GetOrigin().x, y, fx->GetOrigin().z);

        dglPushMatrix();
        dglMultMatrixf(finalmtx.ToFloatPtr());

        w = (float)texture->OriginalWidth();
        h = (float)texture->OriginalHeight();

        spriteVertices[0][0] = -w;
        spriteVertices[0][1] = -h;
        spriteVertices[1][0] =  w;
        spriteVertices[1][1] = -h;
        spriteVertices[2][0] = -w;
        spriteVertices[2][1] =  h;
        spriteVertices[3][0] =  w;
        spriteVertices[3][1] =  h;

        for(j = 0; j < 4; j++) {
            spriteColors[j][0] = fx->color1[0];
            spriteColors[j][1] = fx->color1[1];
            spriteColors[j][2] = fx->color1[2];
            spriteColors[j][3] = fx->color1[3];
        }

        renderSystem.SetState(GLSTATE_DEPTHTEST, fxinfo->bDepthBuffer);

        if(bWireframe) {
            renderSystem.whiteTexture.Bind();
            dglColor4ub(192, 0, 0, 255);
        }
        else {
            if(bShowTexture) {
                texture->Bind();
                if(fxinfo->bTextureWrapMirror) {
                    texture->ChangeParameters(TC_MIRRORED, TF_LINEAR);
                    spriteTexCoords[1] = 2;
                    spriteTexCoords[2] = 2;
                    spriteTexCoords[3] = 2;
                    spriteTexCoords[6] = 2;
                }
                else {
                    texture->ChangeParameters(TC_CLAMP, TF_LINEAR);
                    spriteTexCoords[1] = 1;
                    spriteTexCoords[2] = 1;
                    spriteTexCoords[3] = 1;
                    spriteTexCoords[6] = 1;
                }
            }
            else {
                renderSystem.whiteTexture.Bind();
            }
        }

        if(!bWireframe) {
            dglPolygonOffset(-i*1.5f, -i*1.5f);
        }

        dglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, spriteIndices);

        if(bShowOrigin) {
            DrawOrigin(0, 0, 0, 32);
        }

        dglPopMatrix();

        if(fxinfo->lifetime.value == 1 && fx->bClientOnly) {
            fx->Remove();
        }
    }

    if(!bWireframe) {
        dglPolygonOffset(0, 0);
        dglDisable(GL_POLYGON_OFFSET_FILL);
        dglDisableClientState(GL_COLOR_ARRAY);
    }

    renderSystem.SetState(GLSTATE_DEPTHTEST, true);
    dglEnableClientState(GL_NORMAL_ARRAY);
}

//
// kexRenderWorld::DrawBoundingBox
//

void kexRenderWorld::DrawBoundingBox(const kexBBox &bbox, byte r, byte g, byte b) {
    kexFrustum frustum = world->Camera()->Frustum();

    if(!frustum.TestBoundingBox(bbox))
        return;

    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_CULL, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    dglColor4ub(r, g, b, 255);

    if(!bWireframe)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.max[2]);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.max[2]);
    dglEnd();

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.max[2]);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.max[2]);
    dglEnd();

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.max[2]);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.max[2]);
    dglEnd();

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.max[2]);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.max[2]);
    dglEnd();
    
    if(!bWireframe)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    renderSystem.SetState(GLSTATE_CULL, true);
    renderSystem.SetState(GLSTATE_BLEND, false);
    renderSystem.SetState(GLSTATE_LIGHTING, true);
}

//
// kexRenderWorld::DrawRadius
//

void kexRenderWorld::DrawRadius(float x, float y, float z, float radius, float height,
                  byte r, byte g, byte b) {
    float an;
    int i;

    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_CULL, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    an = DEG2RAD(360 / 32);

    dglBegin(GL_LINES);
    dglColor4ub(r, g, b, 255);

    for(i = 0; i < 32; i++)
    {
        float s1 = kexMath::Sin(an * i);
        float c1 = kexMath::Cos(an * i);
        float s2 = kexMath::Sin(an * ((i+1)%31));
        float c2 = kexMath::Cos(an * ((i+1)%31));

        dglVertex3f(x + (radius * s1), y, z + (radius * c1));
        dglVertex3f(x + (radius * s1), y + height, z + (radius * c1));
        dglVertex3f(x + (radius * s1), y, z + (radius * c1));
        dglVertex3f(x + (radius * s2), y, z + (radius * c2));
        dglVertex3f(x + (radius * s1), y + height, z + (radius * c1));
        dglVertex3f(x + (radius * s2), y + height, z + (radius * c2));
    }

    dglEnd();

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    renderSystem.SetState(GLSTATE_CULL, true);
    renderSystem.SetState(GLSTATE_BLEND, false);
    renderSystem.SetState(GLSTATE_LIGHTING, true);
}

//
// kexRenderWorld::DrawOrigin
//

void kexRenderWorld::DrawOrigin(float x, float y, float z, float size) {
    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_FOG, false);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    dglDepthRange(0.0f, 0.0f);
    dglLineWidth(2.0f);
    dglBegin(GL_LINES);

    // x
    dglColor4ub(255, 0, 0, 255);
    dglVertex3f(x, y, z);
    dglVertex3f(x + size, y, z);
    // y
    dglColor4ub(0, 255, 0, 255);
    dglVertex3f(x, y, z);
    dglVertex3f(x, y + size, z);
    // z
    dglColor4ub(0, 0, 255, 255);
    dglVertex3f(x, y, z);
    dglVertex3f(x, y, z + size);

    dglEnd();
    dglLineWidth(1.0f);
    dglDepthRange(0.0f, 1.0f);

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
}

//
// kexRenderWorld::DrawWorldNode
//

void kexRenderWorld::DrawWorldNode(worldNode_t *node) {
    if(node->bLeaf) {
        DrawBoundingBox(node->bounds, 0, 255, 0);
        if(showWorldNode >= 1) {
            for(unsigned int i = 0; i < node->actors.Length(); i++) {
                DrawBoundingBox(node->actors[i]->Bounds(), 0, 255, 255);
            }
        }
        return;
    }
    else {
        DrawBoundingBox(node->bounds, 255, 255, 0);
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
        DrawBoundingBox(nodes->bounds, 64, 128, 255);
    }

    for(int i = 0; i < world->NumAreaNodes(); i++) {
        nodes = &world->areaNodes[i];
        for(kexActor *actor = static_cast<kexActor*>(nodes->objects.Next());
            actor != NULL;
            actor = static_cast<kexActor*>(actor->areaLink.Next())) {
                if(actor == gameManager.localPlayer.Puppet()) {
                    DrawBoundingBox(nodes->bounds, 255, 0, 0);
                }
                else {
                    renderSystem.SetState(GLSTATE_TEXTURE0, false);
                    dglBegin(GL_LINES);
                    dglColor4ub(255, 0, 0, 255);
                    dglVertex3fv(nodes->bounds.Center().ToFloatPtr());
                    dglVertex3fv(actor->GetOrigin().ToFloatPtr());
                    dglEnd();
                    renderSystem.SetState(GLSTATE_TEXTURE0, true);
                }
        }
    }

    dglDepthRange(0.0f, 1.0f);
}
