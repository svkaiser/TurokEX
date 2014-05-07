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

kexCvar cvarRenderBloom("r_bloom", CVF_BOOL|CVF_CONFIG, "0", "TODO");
kexCvar cvarRenderBloomThreshold("r_bloomthreshold", CVF_FLOAT|CVF_CONFIG, "0.6", 0.01f, 1.0f, "TODO");
kexCvar cvarRenderFXAA("r_fxaa", CVF_BOOL|CVF_CONFIG, "0", "TODO");
kexCvar cvarRenderFXAAMaxSpan("r_fxaamaxspan", CVF_FLOAT|CVF_CONFIG, "8.0", 1.0f, 8.0f, "TODO");
kexCvar cvarRenderFXAAReduceMax("r_fxaareducemax", CVF_FLOAT|CVF_CONFIG, "8.0", 1.0f, 128.0f, "TODO");
kexCvar cvarRenderFXAAReduceMin("r_fxaareducemin", CVF_FLOAT|CVF_CONFIG, "128.0", 1.0f, 512.0f, "TODO");
kexCvar cvarRenderMotionBlur("r_motionblur", CVF_BOOL|CVF_CONFIG, "0", "TODO");
kexCvar cvarRenderMotionBlurSamples("r_motionblursamples", CVF_INT|CVF_CONFIG, "32", "TODO");
kexCvar cvarRenderMotionBlurRampSpeed("r_motionblurrampspeed", CVF_FLOAT|CVF_CONFIG, "0.0", "TODO");
kexCvar cvarRenderLightScatter("r_lightscatter", CVF_BOOL|CVF_CONFIG, "0", "TODO");
kexCvar cvarRenderNodeOcclusionQueries("r_nodeocclusionqueries", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarRenderActorOcclusionQueries("r_actorocclusionqueries", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarRenderNoSortMaterials("r_nosortmaterials", CVF_BOOL, "0", "TODO");

kexRenderer renderer;

//
// FCmd_PrintStats
//

static void FCmd_PrintStats(void) {
    if(command.GetArgc() < 1) {
        return;
    }

    renderer.bShowRenderStats ^= 1;
}

//
// SortDrawList
//

static int SortDrawList(const drawSurface_t *a, const drawSurface_t *b) {
    kexMaterial *xa = a->material;
    kexMaterial *xb = b->material;
    
    return (int)(xb - xa);
}

//
// kexRenderer::kexRenderer
//

kexRenderer::kexRenderer(void) {
    this->currentSurface        = NULL;
    this->motionBlurMaterial    = NULL;
    this->bRenderLightScatter   = false;
    this->bShowRenderStats      = false;

    memset(numDrawList, 0, sizeof(numDrawList));
}

//
// kexRenderer::~kexRenderer
//

kexRenderer::~kexRenderer(void) {
    for(int i = 0; i < NUMSORTORDERS; i++) {
        drawSurfaces[i].Empty();
    }
}

//
// kexRenderer::Init
//

void kexRenderer::Init(void) {
    motionBlurMaterial  = renderBackend.CacheMaterial("materials/motionBlur.kmat@motionBlur");
    wireframeMaterial   = renderBackend.CacheMaterial("materials/default.kmat@wireframe");
    shaderLightScatter  = renderBackend.CacheShader("defs/shaders.def@lightScatter");
    blackShader         = renderBackend.CacheShader("defs/shaders.def@black");
    fxaaShader          = renderBackend.CacheShader("defs/shaders.def@fxaa");
    blurShader          = renderBackend.CacheShader("defs/shaders.def@blur");
    bloomShader         = renderBackend.CacheShader("defs/shaders.def@bloomTest");

    int width   = kexMath::RoundPowerOfTwo(sysMain.VideoWidth());
    int height  = kexMath::RoundPowerOfTwo(sysMain.VideoHeight());
    
    fboLightScatter.InitColorAttachment(0, width >> 1, height >> 1);
    fboFXAA.InitColorAttachment(0, width, height);
    fboBloom.InitColorAttachment(0, width, height);
    fboBlur[0].InitColorAttachment(0, width >> 1, height >> 1);
    fboBlur[1].InitColorAttachment(0, width >> 3, height >> 3);

    renderWorld.InitSunData();

    command.Add("statrenderer", FCmd_PrintStats);
}

//
// kexRenderer::Draw
//

void kexRenderer::Draw(void) {
    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    renderWorld.RenderScene();
    renderBackend.SetOrtho();

    if(bShowRenderStats) {
        postProcessMS = sysMain.GetMS();
    }

    ProcessMotionBlur();
    ProcessBloom();
    ProcessLightScatter();
    ProcessFXAA();

    if(bShowRenderStats) {
        postProcessMS = sysMain.GetMS() - postProcessMS;
    }

    renderBackend.Canvas().Draw();
    gameManager.MenuCanvas().Draw();
    console.Draw();

    DrawStats();
    
    // finish frame
    renderBackend.SwapBuffers();
}

//
// kexRenderer::BindDrawPointers
//

void kexRenderer::BindDrawPointers(void) {
    dglNormalPointer(GL_FLOAT, sizeof(float)*3, drawVertices);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, drawTexCoords);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, drawVertices);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, drawRGB);
}

//
// kexRenderer::AddTriangle
//

void kexRenderer::AddTriangle(int v0, int v1, int v2) {
    if(indiceCount + 3 >= GL_MAX_INDICES) {
        common.Warning("Static triangle indice overflow");
        return;
    }

    drawIndices[indiceCount++] = v0;
    drawIndices[indiceCount++] = v1;
    drawIndices[indiceCount++] = v2;
}

//
// kexRenderer::AddVertex
//

void kexRenderer::AddVertex(float x, float y, float z, float s, float t,
                                byte r, byte g, byte b, byte a) {
    if((vertexCount * 4 + 3) >= GL_MAX_VERTICES) {
        common.Warning("Static vertex draw overflow");
        return;
    }

    drawVertices[vertexCount * 3 + 0]   = x;
    drawVertices[vertexCount * 3 + 1]   = y;
    drawVertices[vertexCount * 3 + 2]   = z;
    drawTexCoords[vertexCount * 2 + 0]  = s;
    drawTexCoords[vertexCount * 2 + 1]  = t;
    drawRGB[vertexCount * 4 + 0]        = r;
    drawRGB[vertexCount * 4 + 1]        = g;
    drawRGB[vertexCount * 4 + 2]        = b;
    drawRGB[vertexCount * 4 + 3]        = a;

    vertexCount++;
}

//
// kexRenderer::AddLine
//

void kexRenderer::AddLine(float x1, float y1, float z1,
                              float x2, float y2, float z2,
                              byte r, byte g, byte b, byte a) {
    
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x1, y1, z1, 0, 0, r, g, b, a);
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x2, y2, z2, 0, 0, r, g, b, a);
}

//
// kexRenderer::AddLine
//

void kexRenderer::AddLine(float x1, float y1, float z1,
                              float x2, float y2, float z2,
                              byte r1, byte g1, byte b1, byte a1,
                              byte r2, byte g2, byte b2, byte a2) {
    
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x1, y1, z1, 0, 0, r1, g1, b1, a1);
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x2, y2, z2, 0, 0, r2, g2, b2, a2);
}

//
// kexRenderer::DrawElements
//

void kexRenderer::DrawElements(const bool bClearCount) {
    dglDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_SHORT, drawIndices);

    if(bClearCount) {
        indiceCount = 0;
        vertexCount = 0;
    }
}

//
// kexRenderer::DrawElements
//
// Draws using the specified material. A temp. surface
// is created in order to draw the material
//

void kexRenderer::DrawElements(const kexMaterial *material, const bool bClearCount) {
    surface_t surf;
    
    surf.numVerts   = vertexCount;
    surf.numIndices = indiceCount;
    surf.vertices   = reinterpret_cast<kexVec3*>(drawVertices);
    surf.coords     = drawTexCoords;
    surf.rgb        = drawRGB;
    surf.normals    = drawVertices;
    surf.indices    = drawIndices;
    
    DrawSurface(&surf, (kexMaterial*)material);
    
    if(bClearCount) {
        indiceCount = 0;
        vertexCount = 0;
    }
}

//
// kexRenderer::DrawElementsNoShader
//

void kexRenderer::DrawElementsNoShader(const bool bClearCount) {
    renderBackend.DisableShaders();
    dglDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_SHORT, drawIndices);

    if(bClearCount) {
        indiceCount = 0;
        vertexCount = 0;
    }
}

//
// kexRenderer::DrawLineElements
//

void kexRenderer::DrawLineElements(void) {
    renderBackend.DisableShaders();
    dglDrawElements(GL_LINES, indiceCount, GL_UNSIGNED_SHORT, drawIndices);
    
    indiceCount = 0;
    vertexCount = 0;
}

//
// kexRenderer::PrepareOcclusionQuery
//

void kexRenderer::PrepareOcclusionQuery(void) {
    // make sure we don't write anything while testing the bounds
    renderBackend.SetColorMask(0);
    renderBackend.SetDepthMask(0);
    
    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    renderBackend.SetState(GLSTATE_CULL, false);
    renderBackend.SetState(GLSTATE_BLEND, false);
    
    renderBackend.DisableShaders();
    
    dglDisableClientState(GL_NORMAL_ARRAY);
    dglDisableClientState(GL_TEXTURE_COORD_ARRAY);
    dglDisableClientState(GL_COLOR_ARRAY);
}

//
// kexRenderer::TestBoundsForOcclusionQuery
//

void kexRenderer::TestBoundsForOcclusionQuery(const unsigned int &query, const kexBBox &bounds) {
    static const word indices[36] = { 0, 1, 3, 4, 7, 5, 0, 4, 1, 1, 5, 6,
                                      2, 6, 7, 4, 0, 3, 1, 2, 3, 7, 6, 5,
                                      2, 1, 6, 3, 2, 7, 7, 4, 3, 4, 5, 1 };
    float points[24];
    
    bounds.ToPoints(points);
    
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, points);
    
    dglBeginQueryARB(GL_SAMPLES_PASSED_ARB, query);
    dglDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, indices);
    dglEndQueryARB(GL_SAMPLES_PASSED_ARB);
}

//
// kexRenderer::EndOcclusionQueryTest
//

void kexRenderer::EndOcclusionQueryTest(void) {
    renderBackend.SetColorMask(1);
    renderBackend.SetDepthMask(1);
    
    dglEnableClientState(GL_NORMAL_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    dglEnableClientState(GL_COLOR_ARRAY);
    
    renderBackend.SetState(GLSTATE_TEXTURE0, true);
}

//
// kexRenderer::GetOcclusionSampleResult
//

bool kexRenderer::GetOcclusionSampleResult(const unsigned int &query, const kexBBox &bounds) {
    int samples = -1;
    
    dglGetQueryObjectivARB(query, GL_QUERY_RESULT_ARB, &samples);

    if(samples <= 16 && samples >= 0) {
        // omit if the camera is completely inside the bounds
        if(!bounds.PointInside(localWorld.Camera()->GetOrigin())) {
            return true;
        }
    }
    
    return false;
}

//
// kexRenderer::AddSurface
//

void kexRenderer::AddSurface(const surface_t *surf, const kexMaterial *material,
                             const kexMatrix mtx, const matSortOrder_t sortOrder) {
    drawSurface_t drawSurf;
    
    drawSurf.surf = (surface_t*)surf;
    drawSurf.material = (kexMaterial*)material;
    drawSurf.matrix = mtx;
    
    if(drawSurfaces[sortOrder].Length() <= (unsigned int)numDrawList[sortOrder]) {
        drawSurfaces[sortOrder].Push(drawSurf);
    }
    else {
        drawSurfaces[sortOrder][numDrawList[sortOrder]] = drawSurf;
    }

    numDrawList[sortOrder]++;
}

//
// kexRenderer::DrawSurfaceList
//

void kexRenderer::DrawSurfaceList(void) {
    drawSurface_t *drawSurf;
    bool bNoSort = cvarRenderNoSortMaterials.GetBool();
    
    for(int i = 0; i < NUMSORTORDERS; i++) {
        if(bNoSort == false) {
            drawSurfaces[i].Sort(SortDrawList, numDrawList[i]);
        }
        
        for(int j = 0; j < numDrawList[i]; j++) {
            drawSurf = &drawSurfaces[i][j];
            
            dglPushMatrix();
            dglMultMatrixf(drawSurf->matrix.ToFloatPtr());

            DrawSurface(drawSurf->surf, drawSurf->material);

            dglPopMatrix();
        }

        numDrawList[i] = 0;
    }
}

//
// kexRenderer::DrawSurface
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
    renderBackend.SetPolyMode(GLPOLY_FILL);
    
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
// kexRenderer::DrawWireFrameSurface
//

void kexRenderer::DrawWireFrameSurface(const surface_t *surface, const rcolor color) {
    kexShaderObj *shader;
    kexMaterial *material;

    if(surface == NULL) {
        return;
    }

    shader = wireframeMaterial->ShaderObj();
    
    if(shader == NULL) {
        return;
    }

    material = wireframeMaterial;
    material->SetDiffuseColor(color);

    shader->Enable();
    shader->CommitGlobalUniforms(material);
    
    renderBackend.SetState(material->StateBits());
    renderBackend.SetAlphaFunc(material->AlphaFunction(), material->AlphaMask());
    renderBackend.SetCull(material->CullType());
    renderBackend.SetDepthMask(material->DepthMask());
    renderBackend.SetPolyMode(GLPOLY_LINE);
    
    for(unsigned int i = 0; i < material->NumUnits(); i++) {
        matSampler_t *sampler = material->Sampler(i);
        
        renderBackend.SetTextureUnit(sampler->unit);
        
        if(sampler->texture == NULL) {
            renderBackend.defaultTexture.Bind();
        }
        else {
            sampler->texture->Bind();
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
// kexRenderer::DrawBlackSurface
//

void kexRenderer::DrawBlackSurface(const surface_t *surface, kexMaterial *material)  {
    kexShaderObj *shader = blackShader;
    
    if(surface == NULL) {
        return;
    }
    if(shader == NULL) {
        return;
    }
    if(material == NULL) {
        return;
    }
    if(material->Flags() & MTF_NODRAW) {
        return;
    }
    if(material->SortOrder() == MSO_TRANSPARENT) {
        return;
    }
    
    shader->Enable();
    shader->CommitGlobalUniforms(material);
    
    renderBackend.SetState(material->StateBits());
    renderBackend.SetAlphaFunc(material->AlphaFunction(), material->AlphaMask());
    renderBackend.SetCull(material->CullType());
    renderBackend.SetDepthMask(material->DepthMask());
    renderBackend.SetPolyMode(GLPOLY_FILL);
    
    for(unsigned int i = 0; i < material->NumUnits(); i++) {
        matSampler_t *sampler = material->Sampler(i);
        
        renderBackend.SetTextureUnit(sampler->unit);
        
        if(sampler->texture == NULL) {
            renderBackend.defaultTexture.Bind();
        }
        else {
            sampler->texture->Bind();
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
    renderBackend.SetDepthMask(0);

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

        if(fxinfo->animtype == VFX_ANIMDRAWSINGLEFRAME && fx->bClientOnly) {
            fx->Remove();
        }
    }

    renderBackend.SetState(GLSTATE_DEPTHTEST, true);
    renderBackend.SetDepthMask(1);
    dglEnableClientState(GL_NORMAL_ARRAY);
}

//
// kexRenderer::DrawScreenQuad
//

void kexRenderer::DrawScreenQuad(void) {
    static const word  indices[6] = { 0, 1, 2, 2, 1, 3 };
    static const float tcoords[8] = { 0, 0, 1, 0, 0, 1, 1, 1 };
    float verts[12];
    int w, h;
    
    w = sysMain.VideoWidth();
    h = sysMain.VideoHeight();
    
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, tcoords);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, verts);
    
    dglDisableClientState(GL_NORMAL_ARRAY);
    dglDisableClientState(GL_COLOR_ARRAY);
    
    verts[0 * 3 + 0] = 0;
    verts[0 * 3 + 1] = 0;
    verts[0 * 3 + 2] = 0;
    verts[1 * 3 + 0] = (float)w;
    verts[1 * 3 + 1] = 0;
    verts[1 * 3 + 2] = 0;
    verts[2 * 3 + 0] = 0;
    verts[2 * 3 + 1] = (float)h;
    verts[2 * 3 + 2] = 0;
    verts[3 * 3 + 0] = (float)w;
    verts[3 * 3 + 1] = (float)h;
    verts[3 * 3 + 2] = 0;
    
    dglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    dglEnableClientState(GL_NORMAL_ARRAY);
    dglEnableClientState(GL_COLOR_ARRAY);
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

    AddVertex((float)vp[0], (float)vp[1], 0, 0, 0, 255, 255, 255, 255);
    AddVertex((float)vp[2], (float)vp[1], 0, 1, 0, 255, 255, 255, 255);
    AddVertex((float)vp[0], (float)vp[3], 0, 0, 1, 255, 255, 255, 255);
    AddVertex((float)vp[2], (float)vp[3], 0, 1, 1, 255, 255, 255, 255);
    AddTriangle(0, 1, 2);
    AddTriangle(2, 1, 3);
    DrawElements(motionBlurMaterial);
}

//
// kexRenderer::ProcessLightScatter
//

void kexRenderer::ProcessLightScatter(void) {
    if(cvarRenderLightScatter.GetBool() == false || bRenderLightScatter == false) {
        return;
    }

    shaderLightScatter->Enable();

    shaderLightScatter->SetUniform("uLightCoordinate", renderWorld.ProjectedSunCoords());
    shaderLightScatter->SetUniform("uExposure", 0.0034f);
    shaderLightScatter->SetUniform("uDecay", 1.0f);
    shaderLightScatter->SetUniform("uDensity", 0.84f);
    shaderLightScatter->SetUniform("uWeight", 3.5f);
    shaderLightScatter->SetUniform("uDiffuse", 0);

    renderBackend.SetTextureUnit(0);
    fboLightScatter.BindImage();

    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetBlend(GLSRC_ONE, GLDST_ONE);

    DrawScreenQuad();

    renderBackend.DisableShaders();
    renderBackend.SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
}

//
// kexRenderer::ProcessFXAA
//

void kexRenderer::ProcessFXAA(void) {
    if(cvarRenderFXAA.GetBool() == false) {
        return;
    }
    
    int viewWidth = sysMain.VideoWidth();
    int viewHeight = sysMain.VideoHeight();
    
    fboFXAA.CopyBackBuffer();
    
    fxaaShader->Enable();
    fxaaShader->SetUniform("uDiffuse", 0);
    fxaaShader->SetUniform("uViewWidth", (float)viewWidth);
    fxaaShader->SetUniform("uViewHeight", (float)viewHeight);
    fxaaShader->SetUniform("uMaxSpan", cvarRenderFXAAMaxSpan.GetFloat());
    fxaaShader->SetUniform("uReduceMax", cvarRenderFXAAReduceMax.GetFloat());
    fxaaShader->SetUniform("uReduceMin", cvarRenderFXAAReduceMin.GetFloat());
    
    renderBackend.SetTextureUnit(0);
    fboFXAA.BindImage();
    
    renderBackend.SetState(GLSTATE_BLEND, true);
    
    // resize viewport to account for FBO dimentions
    dglPushAttrib(GL_VIEWPORT_BIT);
    dglViewport(0, 0, fboFXAA.Width(), fboFXAA.Height());

    DrawScreenQuad();

    dglPopAttrib();
    renderBackend.DisableShaders();
}

//
// kexRenderer::ProcessBloom
//

void kexRenderer::ProcessBloom(void) {
    if(cvarRenderBloom.GetBool() == false) {
        return;
    }

    renderBackend.SetTextureUnit(0);
    renderBackend.SetState(GLSTATE_BLEND, true);

    // pass 1: bloom
    renderBackend.frameBuffer->BindFrameBuffer();
    fboBloom.Bind();
    bloomShader->Enable();
    bloomShader->SetUniform("uDiffuse", 0);
    bloomShader->SetUniform("uBloomThreshold", cvarRenderBloomThreshold.GetFloat());
    DrawScreenQuad();
    fboBloom.UnBind();

    blurShader->Enable();
    blurShader->SetUniform("uDiffuse", 0);
    blurShader->SetUniform("uBlurRadius", 1.0f);

    // pass 2: blur
    for(int i = 0; i < MAX_BLUR_SAMPLES; i++) {
        // horizonal
        fboBlur[i].CopyFrameBuffer(fboBloom);
        fboBloom.Bind();
        fboBlur[i].BindImage();
        blurShader->SetUniform("uSize", (float)fboBlur[i].Width());
        blurShader->SetUniform("uDirection", 1);
        DrawScreenQuad();
        fboBloom.UnBind();

        // vertical
        fboBlur[i].CopyFrameBuffer(fboBloom);
        fboBloom.Bind();
        fboBlur[i].BindImage();
        blurShader->SetUniform("uSize", (float)fboBlur[i].Height());
        blurShader->SetUniform("uDirection", 0);
        DrawScreenQuad();
        fboBloom.UnBind();
    }

    renderBackend.DisableShaders();

    int vp[4];
    dglGetIntegerv(GL_VIEWPORT, vp);

    // resize viewport to account for FBO dimentions
    dglPushAttrib(GL_VIEWPORT_BIT);
    dglViewport(0, 0, fboBloom.Width(), fboBloom.Height());
    
    BindDrawPointers();
    fboBloom.BindImage();
    
    renderBackend.SetBlend(GLSRC_ONE, GLDST_ONE);

    AddVertex((float)vp[0], (float)vp[1], 0, 0, 1, 255, 255, 255, 255);
    AddVertex((float)vp[2], (float)vp[1], 0, 1, 1, 255, 255, 255, 255);
    AddVertex((float)vp[0], (float)vp[3], 0, 0, 0, 255, 255, 255, 255);
    AddVertex((float)vp[2], (float)vp[3], 0, 1, 0, 255, 255, 255, 255);
    AddTriangle(0, 1, 2);
    AddTriangle(2, 1, 3);

    DrawElementsNoShader();

    renderBackend.SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    dglPopAttrib();
}

//
// kexRenderer::DrawStats
//

void kexRenderer::DrawStats(void) {
    kexHeap::DrawHeapInfo();
    renderBackend.PrintStats();
    renderWorld.PrintStats();

#define DRAWSURF_SIZE(x) ((sizeof(drawSurface_t) * drawSurfaces[x].Length()) >> 10)

    if(bShowRenderStats) {
        kexRenderUtils::PrintStatsText("post process time", ": %ims", postProcessMS);
        kexRenderUtils::AddDebugLineSpacing();
        kexRenderUtils::PrintStatsText("draw surf default", ": %ikb", DRAWSURF_SIZE(MSO_DEFAULT));
        kexRenderUtils::PrintStatsText("draw surf masked", ": %ikb", DRAWSURF_SIZE(MSO_MASKED));
        kexRenderUtils::PrintStatsText("draw surf transparent", ": %ikb", DRAWSURF_SIZE(MSO_TRANSPARENT));
        kexRenderUtils::PrintStatsText("draw surf custom1", ": %ikb", DRAWSURF_SIZE(MSO_CUSTOM1));
        kexRenderUtils::PrintStatsText("draw surf custom2", ": %ikb", DRAWSURF_SIZE(MSO_CUSTOM2));
        kexRenderUtils::PrintStatsText("draw surf custom3", ": %ikb", DRAWSURF_SIZE(MSO_CUSTOM3));
        kexRenderUtils::AddDebugLineSpacing();
    }

#undef DRAWSURF_SIZE

    scriptManager.DrawGCStats();
    kexRenderUtils::ClearDebugLine();
}
