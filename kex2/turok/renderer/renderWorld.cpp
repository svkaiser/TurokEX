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

kexRenderWorld renderWorld;

//
// FCmd_ShowCollision
//

static void FCmd_ShowCollision(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderWorld.bShowClipMesh ^= 1;
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
// kexRenderWorld::kexRenderWorld
//

kexRenderWorld::kexRenderWorld(void) {
    this->world         = &localWorld;
    this->bShowBBox     = false;
    this->bShowGrid     = false;
    this->bShowNodes    = false;
    this->bShowNormals  = false;
    this->bShowOrigin   = false;
    this->bShowRadius   = false;
    this->bWireframe    = false;
    this->bShowClipMesh = false;
}

//
// kexRenderWorld::Init
//

void kexRenderWorld::Init(void) {
    command.Add("showcollision", FCmd_ShowCollision);
    command.Add("showgridbounds", FCmd_ShowGridBounds);
    command.Add("showbbox", FCmd_ShowBoundingBox);
    command.Add("showradius", FCmd_ShowRadius);
}

//
// kexRenderWorld::RenderScene
//

void kexRenderWorld::RenderScene(void) {
    dglClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(!world->IsLoaded()) {
        return;
    }

    world->Camera()->SetupMatrices();
    renderSystem.SetCull(GLCULL_BACK);
    renderSystem.SetState(GLSTATE_DEPTHTEST, true);

    dglDisableClientState(GL_COLOR_ARRAY);

    dglMatrixMode(GL_PROJECTION);
    dglLoadMatrixf(world->Camera()->Projection().ToFloatPtr());
    dglMatrixMode(GL_MODELVIEW);
    dglLoadMatrixf(world->Camera()->ModelView().ToFloatPtr());

    SetupGlobalLight();
    DrawStaticActors();
    DrawActors();
    DrawFX();
    DrawViewActors();

    dglEnableClientState(GL_COLOR_ARRAY);

    renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    renderSystem.SetState(GLSTATE_DEPTHTEST, false);
    renderSystem.SetState(GLSTATE_CULL, true);
    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    renderSystem.SetState(GLSTATE_BLEND, false);
    renderSystem.SetState(GLSTATE_ALPHATEST, false);
    renderSystem.SetState(GLSTATE_TEXGEN_S, false);
    renderSystem.SetState(GLSTATE_TEXGEN_T, false);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    // TODO - NEEDS SHADERS
    dglDisable(GL_LIGHT0);
    dglDisable(GL_COLOR_MATERIAL);
}

//
// kexRenderWorld::SetupGlobalLight
//

void kexRenderWorld::SetupGlobalLight(void) {
    if(bWireframe)
        return;

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
    texture_t *tex;
    rcolor color;

    if(texturePath == NULL) {
        texturePath = surface->texturePath;
    }

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

    tex = Tex_CacheTextureFile(texturePath, GL_REPEAT,
        surface->flags & MDF_MASKED);

    if(tex) {
        renderSystem.SetState(GLSTATE_LIGHTING, true);
        GL_BindTexture(tex);
    }
    else {
        renderSystem.SetState(GLSTATE_LIGHTING, false);
        GL_BindTextureName("textures/white.tga");
        color = surface->color1;
    }

    if(surface->flags & MDF_TRANSPARENT1) {
        renderSystem.SetState(GLSTATE_LIGHTING, false);
        color = color & 0xffffff;
        color |= (160 << 24);
    }

    dglColor4ubv((byte*)&color);

    dglDrawElements(GL_TRIANGLES, surface->numIndices, GL_UNSIGNED_SHORT, surface->indices);
}

//
// kexRenderWorld::TraverseDrawActorNode
//

void kexRenderWorld::TraverseDrawActorNode(kexWorldActor *actor,
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

            rot *= actor->GetNodeRotations()[nodenum];
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
    kexFrustum frustum = world->Camera()->Frustum();

    for(unsigned int i = 0; i < world->gridBounds.Length(); i++) {
        gridBound_t *grid = world->gridBounds[i];

        for(kexWorldActor *actor = grid->staticActors.Next();
            actor != NULL; actor = actor->worldLink.Next()) {
                if(actor->bHidden) {
                    if(bShowClipMesh) {
                        actor->ClipMesh().DebugDraw();
                    }
                    continue;
                }

                kexBBox box;

                box.min = actor->BoundingBox().min + actor->GetOrigin();
                box.max = actor->BoundingBox().max + actor->GetOrigin();

                actor->bCulled = !frustum.TestBoundingBox(box);

                if(actor->bCulled) {
                    continue;
                }

                if(!bShowClipMesh) {
                    dglPushMatrix();
                    dglMultMatrixf(actor->Matrix().ToFloatPtr());
                    TraverseDrawActorNode(actor, &actor->Model()->nodes[0], NULL);
                    dglPopMatrix();
                }
                else {
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

        if(bShowGrid) {
            if(grid->bTraced) {
                DrawBoundingBox(grid->box, 255, 0, 0);
            }
            else {
                DrawBoundingBox(grid->box, 224, 224, 224);
            }
        }
    }
}

//
// kexRenderWorld::DrawActors
//

void kexRenderWorld::DrawActors(void) {
    kexFrustum frustum = world->Camera()->Frustum();
    kexMatrix mtx(DEG2RAD(-90), 1);
    mtx.Scale(-1, 1, 1);

    for(kexWorldActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bHidden || actor->bClientView) {
                continue;
            }

            kexBBox box;

            box.min = actor->BoundingBox().min * actor->GetScale() + actor->GetOrigin();
            box.max = actor->BoundingBox().max * actor->GetScale() + actor->GetOrigin();

            actor->bCulled = !frustum.TestBoundingBox(box);

            if(actor->bCulled) {
                continue;
            }

            if(actor->Model()) {
                dglPushMatrix();
                dglMultMatrixf(actor->Matrix().ToFloatPtr());
                dglPushMatrix();

                dglMultMatrixf(mtx.ToFloatPtr());

                TraverseDrawActorNode(actor, &actor->Model()->nodes[0], actor->AnimState());

                dglPopMatrix();
                dglPopMatrix();
            }

            if(bShowBBox) {
                if(actor->bTraced) {
                    DrawBoundingBox(box, 255, 0, 0);
                    actor->bTraced = false;
                }
                else {
                    DrawBoundingBox(box, 255, 128, 128);
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

    renderSystem.SetState(GLSTATE_LIGHTING, true);

    for(kexWorldActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bHidden || !actor->bClientView) {
                continue;
            }

            if(actor->Model()) {
                dglPushMatrix();
                dglMultMatrixf(actor->Matrix().ToFloatPtr());
                dglPushMatrix();

                dglMultMatrixf(modelMatrix.ToFloatPtr());

                TraverseDrawActorNode(actor, &actor->Model()->nodes[0], actor->AnimState());

                dglPopMatrix();
                dglPopMatrix();
            }
    }

    renderSystem.SetState(GLSTATE_LIGHTING, false);
}

//
// kexRenderWorld::DrawFX
//

void kexRenderWorld::DrawFX(void) {
    renderSystem.SetState(GLSTATE_LIGHTING, false);
    renderSystem.SetCull(GLCULL_FRONT);
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
