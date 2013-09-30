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
    if(!world->IsLoaded()) {
        dglClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        return;
    }

    client.LocalPlayer().PlayerEvent("onPreRender");

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

    client.LocalPlayer().PlayerEvent("onRender");
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

void kexRenderWorld::DrawSurface(const mdlsection_t *section, const char *texturePath) {
    texture_t *tex;
    rcolor color;

    if(texturePath == NULL) {
        texturePath = section->texpath;
    }

    renderSystem.SetState(GLSTATE_CULL, (section->flags & MDF_NOCULLFACES) == 0);
    renderSystem.SetState(GLSTATE_BLEND, (section->flags & (MDF_MASKED|MDF_TRANSPARENT1)) != 0);
    renderSystem.SetState(GLSTATE_ALPHATEST, (section->flags & (MDF_MASKED|MDF_TRANSPARENT1)) != 0);
    renderSystem.SetState(GLSTATE_TEXGEN_S, (section->flags & MDF_SHINYSURFACE) != 0);
    renderSystem.SetState(GLSTATE_TEXGEN_T, (section->flags & MDF_SHINYSURFACE) != 0);

    if(section->flags & MDF_COLORIZE) {
        color = section->color1;
        renderSystem.SetState(GLSTATE_LIGHTING, false);
    }
    else {
        color = COLOR_WHITE;
        renderSystem.SetState(GLSTATE_LIGHTING, true);
    }

    if(section->flags & MDF_MASKED) {
        renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.6525f);
    }
    else {
        renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    }

    dglNormalPointer(GL_FLOAT, sizeof(float)*3, section->normals);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, section->coords);
    dglVertexPointer(3, GL_FLOAT, sizeof(vec3_t), section->xyz);

    tex = Tex_CacheTextureFile(texturePath, GL_REPEAT,
        section->flags & MDF_MASKED);

    if(tex) {
        renderSystem.SetState(GLSTATE_LIGHTING, true);
        GL_BindTexture(tex);
    }
    else {
        renderSystem.SetState(GLSTATE_LIGHTING, false);
        GL_BindTextureName("textures/white.tga");
        color = section->color1;
    }

    if(section->flags & MDF_TRANSPARENT1) {
        renderSystem.SetState(GLSTATE_LIGHTING, false);
        color = color & 0xffffff;
        color |= (160 << 24);
    }

    dglColor4ubv((byte*)&color);

    dglDrawElements(GL_TRIANGLES, section->numtris, GL_UNSIGNED_SHORT, section->tris);
}

//
// kexRenderWorld::TraverseDrawActorNode
//

void kexRenderWorld::TraverseDrawActorNode(const kexWorldActor *actor,
                                           const mdlnode_t *node,
                                           animstate_t *animstate) {
    unsigned int i;
    const kmodel_t *model = actor->Model();
    int nodenum = node - model->nodes;

    if(animstate != NULL) {
        anim_t *anim;
        anim_t *prevanim;
        int frame;
        int nextframe;
        float delta;

        dglPushMatrix();

        anim        = animstate->track.anim;
        prevanim    = animstate->prevtrack.anim;
        frame       = animstate->track.frame;
        nextframe   = animstate->track.nextframe;
        delta       = animstate->deltatime;

        if(anim && frame < (int)anim->numframes) {
        }
    }

    if(node->nummeshes > 0) {
        unsigned int var;

        var = (actor->Variant() >= (int)node->numvariants) ?
            0 : node->variants[actor->Variant()];

        if(var >= node->nummeshes) {
            var = 0;
        }

        for(i = 0; i < node->meshes[var].numsections; i++) {
            mdlsection_t *section = &node->meshes[var].sections[i];
            char *texturepath = NULL;

            if(actor->textureSwaps != NULL) {
                char *meshTexture = actor->textureSwaps[nodenum][var][i];

                if(meshTexture != NULL && meshTexture[0] != '-')
                    texturepath = meshTexture;
            }

            DrawSurface(section, texturepath);
        }
    }

    for(i = 0; i < node->numchildren; i++) {
        TraverseDrawActorNode(actor,
            &model->nodes[node->children[i]], animstate);
    }

    if(animstate != NULL) {
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
                if(actor->bHidden)
                    continue;

                kexBBox box;

                box.min = actor->BoundingBox().min + actor->GetOrigin();
                box.max = actor->BoundingBox().max + actor->GetOrigin();

                actor->bCulled = !frustum.TestBoundingBox(box);

                if(actor->bCulled)
                    continue;

                dglPushMatrix();
                dglMultMatrixf(actor->Matrix().ToFloatPtr());
                TraverseDrawActorNode(actor, &actor->Model()->nodes[0], NULL);
                dglPopMatrix();

                if(bShowClipMesh) {
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

    for(kexWorldActor *actor = world->actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
            if(actor->bHidden)
                continue;

            kexBBox box;

            box.min = actor->BoundingBox().min * actor->GetScale() + actor->GetOrigin();
            box.max = actor->BoundingBox().max * actor->GetScale() + actor->GetOrigin();

            actor->bCulled = !frustum.TestBoundingBox(box);

            if(actor->bCulled)
                continue;

            // TEMP
            //DrawBoundingBox(box, 255, 0, 0);
    }
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
        float s1 = (float)sin(an * i);
        float c1 = (float)cos(an * i);
        float s2 = (float)sin(an * ((i+1)%31));
        float c2 = (float)cos(an * ((i+1)%31));

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
