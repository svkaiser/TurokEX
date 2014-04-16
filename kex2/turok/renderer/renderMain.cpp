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
#include "renderBackend.h"
#include "material.h"
#include "renderMain.h"
#include "renderWorld.h"
#include "world.h"
#include "console.h"
#include "gameManager.h"
#include "renderUtils.h"

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
    motionBlurMaterial = renderBackend.CacheMaterial("materials/motionBlur.kmat@motionBlur");
}

//
// kexRenderer::Draw
//

void kexRenderer::Draw(void) {
    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    renderWorld.RenderScene();
    renderBackend.SetOrtho();

    ProcessMotionBlur();

    renderBackend.Canvas().Draw();
    gameManager.MenuCanvas().Draw();
    console.Draw();
    
    // draw debug stats
    kexHeap::DrawHeapInfo();
    renderWorld.PrintStats();
    scriptManager.DrawGCStats();
    kexRenderUtils::ClearDebugLine();
    
    // finish frame
    renderBackend.SwapBuffers();
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
    
    if(shader == NULL) {
        return;
    }

    shader->Enable();
    shader->CommitGlobalUniforms(material);
    
    renderBackend.SetState(material->StateBits());
    renderBackend.SetAlphaFunc(material->AlphaFunction(), material->AlphaMask());
    renderBackend.SetCull(material->CullType());
    renderBackend.SetDepthMask(material->DepthMask());
    
    for(unsigned int i = 0; i < material->NumUnits(); i++) {
        matSampler_t *sampler = material->Sampler(i);
        
        renderBackend.SetTextureUnit(sampler->unit);
        
        if(sampler->texture == NULL) {
            renderBackend.defaultTexture.Bind();
        }
        else {
            if(sampler->texture == renderBackend.frameBuffer) {
                sampler->texture->BindFrameBuffer();
            }
            else if(sampler->texture == renderBackend.depthBuffer) {
                sampler->texture->BindDepthBuffer();
            }
            else {
                sampler->texture->Bind();
            }
            sampler->texture->ChangeParameters(sampler->clamp, sampler->filter);
        }
    }
    
    renderBackend.SetTextureUnit(0);

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
// kexRenderer::DrawFX
//

void kexRenderer::DrawFX(const fxDisplay_t *fxList, const int count) {
    static const word   spriteIndices[6] = { 0, 1, 2, 2, 1, 3 };
    static float        spriteTexCoords[8] = { 0, 1, 1, 1, 0, 0, 1, 0 };
    static float        spriteVertices[4][3];
    static byte         spriteColors[4][4];
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

    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetState(GLSTATE_ALPHATEST, true);
    renderBackend.SetState(GLSTATE_TEXGEN_S, false);
    renderBackend.SetState(GLSTATE_TEXGEN_T, false);
    
    renderBackend.SetCull(GLCULL_FRONT);
    renderBackend.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    renderBackend.SetDepthMask(GLDEPTHMASK_NO);

    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, spriteTexCoords);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, spriteVertices);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, spriteColors);

    dglDisableClientState(GL_NORMAL_ARRAY);

    frustum = localWorld.Camera()->Frustum();

    // draw sorted fx list
    for(i = 0; i < count; i++) {
        fx = fxList[i].fx;
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
                renderBackend.SetState(GLSTATE_CULL, false);
                break;
            case VFX_DRAWBILLBOARD:
                mtx = kexMatrix(kexQuat(localWorld.Camera()->GetAngles().yaw, 0, 1, 0));
                renderBackend.SetState(GLSTATE_CULL, true);
                break;
            case VFX_DRAWSURFACE:
                mtx = kexMatrix(fx->GetRotation());
                renderBackend.SetState(GLSTATE_CULL, false);
                break;
            default:
                mtx = kexMatrix(localWorld.Camera()->GetRotation());
                renderBackend.SetState(GLSTATE_CULL, true);
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

        renderBackend.SetState(GLSTATE_DEPTHTEST, fxinfo->bDepthBuffer);

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

        if(fxinfo->bLensFlares && fxinfo->lensFlares) {
            fxinfo->lensFlares->Origin() = fx->GetOrigin();
            fxinfo->lensFlares->Draw();

            // TODO - fx draw pointers needs to be rebinded and state reset
        }

        if(fxinfo->lifetime.value == 1 && fx->bClientOnly) {
            fx->Remove();
        }
    }

    renderBackend.SetState(GLSTATE_DEPTHTEST, true);
    renderBackend.SetDepthMask(GLDEPTHMASK_YES);
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

    renderBackend.AddVertex((float)vp[0], (float)vp[1], 0, 0, 1, 255, 255, 255, 255);
    renderBackend.AddVertex((float)vp[2], (float)vp[1], 0, 1, 1, 255, 255, 255, 255);
    renderBackend.AddVertex((float)vp[0], (float)vp[3], 0, 0, 0, 255, 255, 255, 255);
    renderBackend.AddVertex((float)vp[2], (float)vp[3], 0, 1, 0, 255, 255, 255, 255);
    renderBackend.AddTriangle(0, 1, 2);
    renderBackend.AddTriangle(2, 1, 3);
    renderBackend.DrawElements(motionBlurMaterial);
}
