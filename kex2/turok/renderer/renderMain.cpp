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
//
// DESCRIPTION: Main drawing system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "renderSystem.h"
#include "material.h"
#include "renderMain.h"
#include "renderWorld.h"
#include "world.h"
#include "console.h"
#include "gameManager.h"

kexCvar cvarRenderMotionBlur("r_motionblur", CVF_BOOL|CVF_CONFIG, "0", "TODO");
kexCvar cvarRenderMotionBlurSamples("r_motionblursamples", CVF_INT|CVF_CONFIG, "32", "TODO");
kexCvar cvarRenderMotionBlurRampSpeed("r_motionblurrampspeed", CVF_FLOAT|CVF_CONFIG, "0.0", "TODO");

kexRenderer renderer;

//
// kexRenderer::kexRenderer
//

kexRenderer::kexRenderer(void) {
    this->currentSurface        = NULL;
    this->motionBlurMaterial    = NULL;
}

//
// kexRenderer::~kexRenderer
//

kexRenderer::~kexRenderer(void) {
}

//
// kexRenderer::Init
//

void kexRenderer::Init(void) {
    motionBlurMaterial = renderSystem.CacheMaterial("materials/motionBlur.kmat@motionBlur");
}

//
// kexRenderer::Draw
//

void kexRenderer::Draw(void) {
    if(!localWorld.IsLoaded()) {
        dglClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        return;
    }

    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    renderWorld.RenderScene();
    renderSystem.SetOrtho();

    ProcessMotionBlur();

    renderSystem.Canvas().Draw();
    gameManager.MenuCanvas().Draw();
    console.Draw();
    
    // draw debug stats
    kexHeap::DrawHeapInfo();
    scriptManager.DrawGCStats();
    
    // finish frame
    renderSystem.SwapBuffers();
}

//
// kexRenderer::DrawElements
//

void kexRenderer::DrawSurface(const surface_t *surface)  {
    kexMaterial *material;
    
    if(surface == NULL) {
        return;
    }
    if(surface->material == NULL) {
        return;
    }
    if(surface->material->Flags() & MTF_NODRAW) {
        return;
    }
    
    material = surface->material;
    material->ShaderObj()->Enable();
    
    renderSystem.SetState(material->StateBits());
    renderSystem.SetAlphaFunc(material->AlphaFunction(), material->AlphaMask());
    renderSystem.SetCull(material->CullType());
    
    for(unsigned int i = 0; i < material->NumUnits(); i++) {
        matSampler_t *sampler = material->Sampler(i);
        
        renderSystem.SetTextureUnit(sampler->unit);
        
        if(sampler->texture == NULL) {
            renderSystem.defaultTexture.Bind();
        }
        else {
            if(sampler->texture == renderSystem.frameBuffer) {
                sampler->texture->BindFrameBuffer();
            }
            else if(sampler->texture == renderSystem.depthBuffer) {
                sampler->texture->BindDepthBuffer();
            }
            else {
                sampler->texture->Bind();
            }
            sampler->texture->ChangeParameters(sampler->clamp, sampler->filter);
        }
    }
    
    renderSystem.SetTextureUnit(0);

    if(currentSurface != surface) {
        dglNormalPointer(GL_FLOAT, sizeof(float)*3, surface->normals);
        dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, surface->coords);
        dglVertexPointer(3, GL_FLOAT, sizeof(kexVec3), surface->vertices);
        dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, surface->rgb);
        
        currentSurface = surface;
    }
    
    dglDrawElements(GL_TRIANGLES, surface->numIndices, GL_UNSIGNED_SHORT, surface->indices);
}

//
// kexRenderer::ProcessMotionBlur
//

void kexRenderer::ProcessMotionBlur(void) {
    if(cvarRenderMotionBlur.GetBool() == false || motionBlurMaterial == NULL) {
        return;
    }

    kexMatrix mat = localWorld.Camera()->ModelView();
    kexMatrix inverseMat;
    kexMatrix motionMat;
    int samples;
    int vp[4];
    float rampSpeed;
    float targetTimeMS;
    float currentTimeMS;
    float deltaTime;
    float velocity;

    targetTimeMS = (1.0f / 60.0f) * 1000.0f;
    currentTimeMS = client.GetRunTime() * 1000.0f;

    if(currentTimeMS <= 0) {
        return;
    }

    mat.SetTranslation(0, 0, 0);
    inverseMat = kexMatrix::Invert(mat);
    motionMat = inverseMat * prevMVMatrix;

    rampSpeed = cvarRenderMotionBlurRampSpeed.GetFloat() * targetTimeMS;

    if(rampSpeed <= 0.0f) {
        velocity = 1.0f;
    }
    else {
        rampSpeed = 1.0f / (rampSpeed / currentTimeMS);
        velocity = kexMath::Fabs(1.0f - mat.ToQuat().Dot(prevMVMatrix.ToQuat()));

        // huh... wonder why this happens...
        // keep it within the [0.0, 1.0] range
        if(velocity > 1.0f) {
            velocity = 2.0f - velocity;
        }

        velocity /= rampSpeed;

        kexMath::Clamp(velocity, 0.0f, 1.0f);
    }

    deltaTime = targetTimeMS / currentTimeMS;
    kexMath::Clamp(deltaTime, 0.0f, 1.0f);

    prevMVMatrix = mat;

    samples = cvarRenderMotionBlurSamples.GetInt();

    motionBlurMaterial->ShaderObj()->Enable();
    motionBlurMaterial->ShaderObj()->SetUniform("uMVMPrevious", motionMat, false);
    motionBlurMaterial->ShaderObj()->SetUniform("uSamples", samples);
    motionBlurMaterial->ShaderObj()->SetUniform("uVelocity", velocity);
    motionBlurMaterial->ShaderObj()->SetUniform("uDeltaTime", deltaTime);

    dglGetIntegerv(GL_VIEWPORT, vp);

    renderSystem.AddVertex((float)vp[0], (float)vp[1], 0, 0, 1, 255, 255, 255, 255);
    renderSystem.AddVertex((float)vp[2], (float)vp[1], 0, 1, 1, 255, 255, 255, 255);
    renderSystem.AddVertex((float)vp[0], (float)vp[3], 0, 0, 0, 255, 255, 255, 255);
    renderSystem.AddVertex((float)vp[2], (float)vp[3], 0, 1, 0, 255, 255, 255, 255);
    renderSystem.AddTriangle(0, 1, 2);
    renderSystem.AddTriangle(2, 1, 3);
    renderSystem.DrawElements(motionBlurMaterial);
}

//
// kexRenderer::DrawBoundingBox
//

void kexRenderer::DrawBoundingBox(const kexBBox &bbox, byte r, byte g, byte b) {
    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_CULL, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

#define ADD_LINE(ba1, ba2, ba3, bb1, bb2, bb3)                      \
    renderSystem.AddLine(bbox[ba1][0], bbox[ba2][1], bbox[ba3][2],  \
                         bbox[bb1][0], bbox[bb2][1], bbox[bb3][2],  \
                         r, g, b, 255)
    
    renderSystem.BindDrawPointers();
    ADD_LINE(0, 0, 0, 1, 0, 0);
    ADD_LINE(1, 0, 0, 1, 0, 1);
    ADD_LINE(1, 0, 1, 0, 0, 1);
    ADD_LINE(0, 0, 1, 0, 0, 0);
    ADD_LINE(0, 0, 0, 0, 1, 0);
    ADD_LINE(0, 1, 0, 0, 1, 1);
    ADD_LINE(0, 1, 1, 0, 0, 1);
    ADD_LINE(0, 0, 1, 0, 0, 0);
    ADD_LINE(0, 1, 0, 1, 1, 0);
    ADD_LINE(1, 1, 0, 1, 1, 1);
    ADD_LINE(1, 1, 1, 0, 1, 1);
    ADD_LINE(0, 1, 1, 0, 1, 0);
    ADD_LINE(1, 0, 0, 1, 1, 0);
    ADD_LINE(1, 1, 0, 1, 1, 1);
    ADD_LINE(1, 1, 1, 1, 0, 1);
    ADD_LINE(1, 0, 1, 1, 0, 0);
    renderSystem.DrawLineElements();
    
#undef ADD_LINE

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
}

//
// kexRenderer::DrawRadius
//

void kexRenderer::DrawRadius(float x, float y, float z,
                             float radius, float height,
                             byte r, byte g, byte b) {
    float an;
    int i;

    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_CULL, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_LIGHTING, false);
    renderSystem.BindDrawPointers();

    an = DEG2RAD(360 / 32);

    for(i = 0; i < 32; i++) {
        float s1 = kexMath::Sin(an * i);
        float c1 = kexMath::Cos(an * i);
        float s2 = kexMath::Sin(an * ((i+1)%31));
        float c2 = kexMath::Cos(an * ((i+1)%31));
        float x1 = x + (radius * s1);
        float x2 = x + (radius * s2);
        float y1 = y;
        float y2 = y + height;
        float z1 = z + (radius * c1);
        float z2 = z + (radius * c2);
        
        renderSystem.AddLine(x1, y1, z1, x1, y2, z1, r, g, b, 255);
        renderSystem.AddLine(x1, y1, z1, x2, y1, z2, r, g, b, 255);
        renderSystem.AddLine(x1, y2, z1, x2, y2, z2, r, g, b, 255);
    }

    renderSystem.DrawLineElements();
    renderSystem.SetState(GLSTATE_TEXTURE0, true);
}

//
// kexRenderer::DrawOrigin
//

void kexRenderer::DrawOrigin(float x, float y, float z, float size) {
    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_FOG, false);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    dglDepthRange(0.0f, 0.0f);
    dglLineWidth(2.0f);
    
    renderSystem.BindDrawPointers();
    renderSystem.AddLine(x, y, z, x + size, y, z, 255, 0, 0, 255); // x
    renderSystem.AddLine(x, y, z, x, y + size, z, 0, 255, 0, 255); // y
    renderSystem.AddLine(x, y, z, x, y, z + size, 0, 0, 255, 255); // z
    renderSystem.DrawLineElements();
    
    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    
    dglLineWidth(1.0f);
    dglDepthRange(0.0f, 1.0f);
}

//
// kexRenderer::DrawTriangle
//

void kexRenderer::DrawTriangle(const kexTri &tri, const word index,
                               byte r, byte g, byte b, byte a) {
    for(int j = 0; j < 3; j++) {
        renderSystem.AddVertex(tri.point[j]->x,
                               tri.point[j]->y,
                               tri.point[j]->z,
                               0,
                               0,
                               r,
                               g,
                               b,
                               a);
    }
    
    renderSystem.AddTriangle(index + 0, index + 1, index + 2);
}

//
// kexRenderer::DrawSectors
//

void kexRenderer::DrawSectors(kexSector *sectors, const int count) {
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

    renderSystem.SetState(GLSTATE_CULL, true);
    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_ALPHATEST, true);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    renderSystem.BindDrawPointers();

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
                r = 255;
                g = b = a = 128;
            }

            DrawTriangle(*tri, idx, r, g, b, a);
            idx += 3;
            num++;
        }

        if(sector->flags & CLF_CHECKHEIGHT) {
            tri = &sector->upperTri;

            if(frustum.TestTriangle(*tri)) {
                DrawTriangle(*tri, idx, 128, 255, 128, 128);
                idx += 3;
                num++;
            }
        }
    }

    if(num != 0) {
        renderSystem.DrawElements(false);
        renderSystem.SetPolyMode(GLPOLY_LINE);
        renderSystem.SetState(GLSTATE_DEPTHTEST, false);

        // draw wireframe outline
        dglDisableClientState(GL_COLOR_ARRAY);
        dglColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        renderSystem.DrawElements();
        renderSystem.SetPolyMode(GLPOLY_FILL);
        renderSystem.SetState(GLSTATE_DEPTHTEST, true);

        dglEnableClientState(GL_COLOR_ARRAY);

        renderSystem.SetState(GLSTATE_CULL, false);
        dglLineWidth(2.0f);

        // draw plane normal vectors
        for(int i = 0; i < count; i++) {
            sector = &sectors[i];
            tri = &sector->lowerTri;

            if(frustum.TestTriangle(*tri)) {
                n = tri->plane.Normal();
                pt = tri->GetCenterPoint();

                renderSystem.AddLine(pt.x,
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

                    renderSystem.AddLine(pt.x,
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

        renderSystem.DrawLineElements();
        dglLineWidth(1.0f);
    }

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
}
