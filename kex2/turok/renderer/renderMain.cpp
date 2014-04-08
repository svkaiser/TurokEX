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

void kexRenderer::DrawSurface(const surface_t *surface, kexMaterial *material)  {
    kexShaderObj *shader;

    if(surface == NULL) {
        return;
    }
    if(material == NULL) {
        return;
    }
    if(material->Flags() & MTF_NODRAW) {
        return;
    }

    shader = material->ShaderObj();

    shader->Enable();
    shader->SetGlobalUniform(RSP_DIFFUSE_COLOR, material->DiffuseColor());
    shader->SetGlobalUniform(RSP_FOG_COLOR, localWorld.GetCurrentFogRGB());
    shader->SetGlobalUniform(RSP_FOG_NEAR, localWorld.GetFogNear());
    shader->SetGlobalUniform(RSP_FOG_FAR, localWorld.GetFogFar());
    shader->SetGlobalUniform(RSP_LIGHT_DIRECTION, renderWorld.WorldLightTransform());
    shader->SetGlobalUniform(RSP_LIGHT_DIRECTION_COLOR, localWorld.worldLightColor);
    shader->SetGlobalUniform(RSP_LIGHT_AMBIENCE, localWorld.worldLightAmbience);
    
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
// kexRenderer::SortSprites
//

int kexRenderer::SortSprites(const void *a, const void *b) {
    kexFx *xa = ((fxDisplay_t*)a)->fx;
    kexFx *xb = ((fxDisplay_t*)b)->fx;

    return (int)(xb->Distance() - xa->Distance());
}

//
// kexRenderer::DrawFX
//

void kexRenderer::DrawFX(void) {
    static const word   spriteIndices[6] = { 0, 1, 2, 2, 1, 3 };
    static float        spriteTexCoords[8] = { 0, 1, 1, 1, 0, 0, 1, 0 };
    static float        spriteVertices[4][3];
    static byte         spriteColors[4][4];
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
    kexFrustum          frustum;
    kexShaderObj        *shader;

    memset(fxDisplayList, 0, sizeof(fxDisplay_t) * MAX_FX_DISPLAYS);

    // gather particle fx and add to display list for sorting
    for(localWorld.fxRover = localWorld.fxList.Next(), fxDisplayNum = 0;
        localWorld.fxRover != NULL; localWorld.fxRover = localWorld.fxRover->worldLink.Next()) {
            if(fxDisplayNum >= MAX_FX_DISPLAYS) {
                break;
            }
            if(localWorld.fxRover == NULL) {
                break;
            }
            if(localWorld.fxRover->restart > 0) {
                continue;
            }
            if(localWorld.fxRover->IsStale()) {
                continue;
            }

            fxDisplayList[fxDisplayNum++].fx = localWorld.fxRover;
    }

    if(fxDisplayNum <= 0) {
        return;
    }

    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_ALPHATEST, true);
    renderSystem.SetState(GLSTATE_TEXGEN_S, false);
    renderSystem.SetState(GLSTATE_TEXGEN_T, false);

    renderSystem.SetCull(GLCULL_FRONT);
    renderSystem.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    renderSystem.SetDepthMask(GLDEPTHMASK_NO);

    qsort(fxDisplayList, fxDisplayNum, sizeof(fxDisplay_t), kexRenderer::SortSprites);

    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, spriteTexCoords);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, spriteVertices);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, spriteColors);

    dglDisableClientState(GL_NORMAL_ARRAY);

    frustum = localWorld.Camera()->Frustum();

    // draw sorted fx list
    for(i = 0; i < fxDisplayNum; i++) {
        fx = fxDisplayList[i].fx;
        fxinfo = fx->fxInfo;

        if(fxinfo == NULL) {
            continue;
        }

        texture = fx->Texture();

        w = (float)texture->OriginalWidth();
        h = (float)texture->OriginalHeight();

        scale = fx->drawScale * 0.01f;

        if(!frustum.TestSphere(fx->GetOrigin(), w * scale)) {
            continue;
        }

        if((shader = fxinfo->shaderObj)) {
            kexVec4 diffuse_color;

            shader->Enable();

            diffuse_color.Set(
                fx->color2[0] / 255.0f,
                fx->color2[1] / 255.0f,
                fx->color2[2] / 255.0f,
                fx->color2[3] / 255.0f);

            shader->SetGlobalUniform(RSP_DIFFUSE_COLOR, diffuse_color);
            shader->SetUniform("diffuse", 0);
        }

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
                mtx = kexMatrix(kexQuat(localWorld.Camera()->GetAngles().yaw, 0, 1, 0));
                renderSystem.SetState(GLSTATE_CULL, true);
                break;
            case VFX_DRAWSURFACE:
                mtx = kexMatrix(fx->GetRotation());
                renderSystem.SetState(GLSTATE_CULL, false);
                break;
            default:
                mtx = kexMatrix(localWorld.Camera()->GetRotation());
                renderSystem.SetState(GLSTATE_CULL, true);
                break;
        }
        
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

        dglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, spriteIndices);
        dglPopMatrix();

        if(fxinfo->lifetime.value == 1 && fx->bClientOnly) {
            fx->Remove();
        }
    }

    renderSystem.SetState(GLSTATE_DEPTHTEST, true);
    renderSystem.SetDepthMask(GLDEPTHMASK_YES);
    dglEnableClientState(GL_NORMAL_ARRAY);
}

//
// kexRenderer::ProcessMotionBlur
//

void kexRenderer::ProcessMotionBlur(void) {
    if(cvarRenderMotionBlur.GetBool() == false || motionBlurMaterial == NULL) {
        return;
    }

    kexMatrix mat = localWorld.Camera()->RotationMatrix();
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

    renderSystem.DisableShaders();

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

    renderSystem.DisableShaders();
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

    renderSystem.DisableShaders();
    
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

    renderSystem.DisableShaders();

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
                renderSystem.AddVertex(tri->point[j]->x,
                                       sector->area->WaterPlane(),
                                       tri->point[j]->z,
                                       0,
                                       0,
                                       32,
                                       0,
                                       255,
                                       192);
            }
            renderSystem.AddTriangle(idx + 0, idx + 1, idx + 2);

            idx += 3;
            num++;
        }
    }

    if(num != 0) {
        renderSystem.SetState(GLSTATE_CULL, true);
        renderSystem.SetState(GLSTATE_TEXTURE0, false);
        renderSystem.SetState(GLSTATE_BLEND, true);
        renderSystem.SetState(GLSTATE_ALPHATEST, true);
        renderSystem.SetState(GLSTATE_LIGHTING, false);

        renderSystem.BindDrawPointers();
        renderSystem.DrawElements(false);

        // draw wireframe outline
        renderSystem.SetPolyMode(GLPOLY_LINE);

        dglDisableClientState(GL_COLOR_ARRAY);
        dglColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        renderSystem.DrawElements();
        renderSystem.SetPolyMode(GLPOLY_FILL);

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

        renderSystem.SetState(GLSTATE_TEXTURE0, true);
    }
}
