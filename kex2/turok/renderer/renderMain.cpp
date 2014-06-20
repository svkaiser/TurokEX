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
#include "ai.h"
#include "gui.h"

kexCvar cvarRenderBloom("r_bloom", CVF_BOOL|CVF_CONFIG, "0", "TODO");
kexCvar cvarRenderBloomThreshold("r_bloomthreshold", CVF_FLOAT|CVF_CONFIG, "0.54", 0.01f, 1.0f, "TODO");
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
// statrenderer
//

COMMAND(statrenderer) {
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
    // setup materials
    motionBlurMaterial  = kexMaterial::manager.Load("materials/motionBlur.kmat@motionBlur");
    wireframeMaterial   = kexMaterial::manager.Load("materials/default.kmat@wireframe");
    
    // setup shaders
    shaderLightScatter  = kexShaderObj::manager.Load("defs/shaders.def@lightScatter");
    blackShader         = kexShaderObj::manager.Load("defs/shaders.def@black");
    fxaaShader          = kexShaderObj::manager.Load("defs/shaders.def@fxaa");
    blurShader          = kexShaderObj::manager.Load("defs/shaders.def@blur");
    bloomShader         = kexShaderObj::manager.Load("defs/shaders.def@bloomTest");

    int width   = kexMath::RoundPowerOfTwo(sysMain.VideoWidth());
    int height  = kexMath::RoundPowerOfTwo(sysMain.VideoHeight());

    // setup framebuffer objects
    fboLightScatter.InitColorAttachment(0, width >> 1, height >> 1);
    fboFXAA.InitColorAttachment(0, width, height);
    fboBloom.InitColorAttachment(0, width, height);
    fboBlur[0].InitColorAttachment(0, width >> 1, height >> 1);
    fboBlur[1].InitColorAttachment(0, width >> 3, height >> 3);

    renderWorld.InitSunData();
    
    // setup custom shader uniforms
    kexShaderObj::RegisterParam<kexClient>("uTime", &client, &kexClient::GetTime);
    kexShaderObj::RegisterParam<kexClient>("uRunTime", &client, &kexClient::GetRunTime);
    kexShaderObj::RegisterParam<kexRenderWorld>("uLightDirection", &renderWorld, &kexRenderWorld::WorldLightTransform);
    kexShaderObj::RegisterParam<kexWorld>("uLightDirectionColor", &localWorld, &kexWorld::GetLightColor);
    kexShaderObj::RegisterParam<kexWorld>("uLightAmbience", &localWorld, &kexWorld::GetLightAmbience);
    kexShaderObj::RegisterParam<kexWorld>("uFogNear", &localWorld, &kexWorld::GetFogNear);
    kexShaderObj::RegisterParam<kexWorld>("uFogFar", &localWorld, &kexWorld::GetFogFar);
    kexShaderObj::RegisterParam<kexWorld>("uFogColor", &localWorld, &kexWorld::GetCurrentFogRGB);

    common.Printf("Renderer Initialized\n");
}

//
// kexRenderer::Draw
//

void kexRenderer::Draw(void) {
    renderBackend.ClearBuffer();
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
    guiManager.DrawGuis();
    
    console.Draw();
    
    DrawStats();
    
    // finish frame
    renderBackend.SwapBuffers();
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

void kexRenderer::AddSurface(const surface_t *surf,
                             const kexMaterial *material,
                             const kexMatrix mtx,
                             const kexWorldObject *worldObject,
                             const matSortOrder_t sortOrder) {
    drawSurface_t drawSurf;
    
    drawSurf.surf = (surface_t*)surf;
    drawSurf.material = (kexMaterial*)material;
    drawSurf.matrix = mtx;
    drawSurf.refObj = (kexWorldObject*)worldObject;
    
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

void kexRenderer::DrawSurfaceList(drawSurfFunc_t function, const int start, const int end) {
    drawSurface_t *drawSurf;
    kexMaterial *material;
    bool bNoSort = cvarRenderNoSortMaterials.GetBool();

    assert(start >= 0 && start < end);
    assert(end <= NUMSORTORDERS && end > start);
    
    for(int i = start; i < end; i++) {
        if(bNoSort == false) {
            drawSurfaces[i].Sort(SortDrawList, numDrawList[i]);
        }
        
        for(int j = 0; j < numDrawList[i]; j++) {
            drawSurf = &drawSurfaces[i][j];
            
            dglPushMatrix();
            dglMultMatrixf(drawSurf->matrix.ToFloatPtr());
            
            if(function == (&kexRenderer::DrawWireFrameSurface)) {
                wireframeMaterial->SetDiffuseColor(drawSurf->refObj->WireFrameColor());
                material = wireframeMaterial;
            }
            else {
                material = drawSurf->material;
            }

            (this->*function)(drawSurf, material);

            dglPopMatrix();
        }
    }
}

//
// kexRenderer::DrawSurfaceList
//

void kexRenderer::DrawSurfaceList(drawSurfFunc_t function) {
    DrawSurfaceList(function, 0, MSO_RESERVED);
}

//
// kexRenderer::ClearSurfaceList
//

void kexRenderer::ClearSurfaceList(const int start, const int end) {
    assert(start >= 0 && start < end);
    assert(end <= NUMSORTORDERS && end > start);

    for(int i = start; i < end; i++) {
        numDrawList[i] = 0;
    }
}

//
// kexRenderer::ClearSurfaceList
//

void kexRenderer::ClearSurfaceList(void) {
    ClearSurfaceList(0, NUMSORTORDERS);
}

//
// kexRenderer::DrawSurface
//

void kexRenderer::DrawSurface(const drawSurface_t *drawSurf, kexMaterial *material)  {
    kexShaderObj *shader;
    surface_t *surface = drawSurf->surf;

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

    if(drawSurf->refObj) {
        shader->SetGlobalUniform(RSP_GENERIC_PARAM1, drawSurf->refObj->ShaderParams()[0]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM2, drawSurf->refObj->ShaderParams()[1]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM3, drawSurf->refObj->ShaderParams()[2]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM4, drawSurf->refObj->ShaderParams()[3]);
    }
    
    material->SetRenderState();
    renderBackend.SetPolyMode(GLPOLY_FILL);
    
    material->BindImages();

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

void kexRenderer::DrawWireFrameSurface(const drawSurface_t *drawSurf, kexMaterial *material) {
    kexShaderObj *shader;
    surface_t *surface = drawSurf->surf;

    if(surface == NULL) {
        return;
    }

    shader = material->ShaderObj();
    
    if(shader == NULL) {
        return;
    }

    shader->Enable();
    shader->CommitGlobalUniforms(material);

    if(drawSurf->refObj) {
        shader->SetGlobalUniform(RSP_GENERIC_PARAM1, drawSurf->refObj->ShaderParams()[0]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM2, drawSurf->refObj->ShaderParams()[1]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM3, drawSurf->refObj->ShaderParams()[2]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM4, drawSurf->refObj->ShaderParams()[3]);
    }
    
    material->SetRenderState();
    renderBackend.SetPolyMode(GLPOLY_LINE);
    
    material->BindImages();

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

void kexRenderer::DrawBlackSurface(const drawSurface_t *drawSurf, kexMaterial *material)  {
    kexShaderObj *shader = blackShader;
    surface_t *surface = drawSurf->surf;
    
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

    if(drawSurf->refObj) {
        shader->SetGlobalUniform(RSP_GENERIC_PARAM1, drawSurf->refObj->ShaderParams()[0]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM2, drawSurf->refObj->ShaderParams()[1]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM3, drawSurf->refObj->ShaderParams()[2]);
        shader->SetGlobalUniform(RSP_GENERIC_PARAM4, drawSurf->refObj->ShaderParams()[3]);
    }
    
    material->SetRenderState();
    renderBackend.SetPolyMode(GLPOLY_FILL);
    
    if(material->SortOrder() == MSO_MASKED) {
        material->BindImages();
    }
    else {
        renderBackend.blackTexture.Bind();
    }
    
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

void kexRenderer::DrawScreenQuad(const kexFBO *fbo) {
    static const word  indices[6] = { 0, 1, 2, 2, 1, 3 };
    static const float tcoords[8] = { 0, 0, 1, 0, 0, 1, 1, 1 };
    float verts[12];
    int w, h;
    float offset = 0;
    
    if(fbo == NULL) {
        w = sysMain.VideoWidth();
        h = sysMain.VideoHeight();
    }
    else {
        w = fbo->Width();
        h = fbo->Height();

        if(h > sysMain.VideoHeight()) {
            offset = (float)(h - sysMain.VideoHeight());
        }
    }
    
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, tcoords);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, verts);
    
    dglDisableClientState(GL_NORMAL_ARRAY);
    dglDisableClientState(GL_COLOR_ARRAY);
    
    verts[0 * 3 + 0] = 0;
    verts[0 * 3 + 1] = -offset;
    verts[0 * 3 + 2] = 0;
    verts[1 * 3 + 0] = (float)w;
    verts[1 * 3 + 1] = -offset;
    verts[1 * 3 + 2] = 0;
    verts[2 * 3 + 0] = 0;
    verts[2 * 3 + 1] = (float)h - offset;
    verts[2 * 3 + 2] = 0;
    verts[3 * 3 + 0] = (float)w;
    verts[3 * 3 + 1] = (float)h - offset;
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

    cpuVertList.BindDrawPointers();
    cpuVertList.AddVertex((float)vp[0], (float)vp[1], 0, 0, 0, 255, 255, 255, 255);
    cpuVertList.AddVertex((float)vp[2], (float)vp[1], 0, 1, 0, 255, 255, 255, 255);
    cpuVertList.AddVertex((float)vp[0], (float)vp[3], 0, 0, 1, 255, 255, 255, 255);
    cpuVertList.AddVertex((float)vp[2], (float)vp[3], 0, 1, 1, 255, 255, 255, 255);
    cpuVertList.AddTriangle(0, 1, 2);
    cpuVertList.AddTriangle(2, 1, 3);
    cpuVertList.DrawElements(motionBlurMaterial);
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

    DrawScreenQuad(&fboFXAA);
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
    
    cpuVertList.BindDrawPointers();
    fboBloom.BindImage();
    
    renderBackend.SetBlend(GLSRC_ONE, GLDST_ONE);

    cpuVertList.AddVertex((float)vp[0], (float)vp[1], 0, 0, 1, 255, 255, 255, 255);
    cpuVertList.AddVertex((float)vp[2], (float)vp[1], 0, 1, 1, 255, 255, 255, 255);
    cpuVertList.AddVertex((float)vp[0], (float)vp[3], 0, 0, 0, 255, 255, 255, 255);
    cpuVertList.AddVertex((float)vp[2], (float)vp[3], 0, 1, 0, 255, 255, 255, 255);
    cpuVertList.AddTriangle(0, 1, 2);
    cpuVertList.AddTriangle(2, 1, 3);

    cpuVertList.DrawElementsNoShader();

    renderBackend.SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    dglPopAttrib();
}

//
// kexRenderer::DrawStats
//

void kexRenderer::DrawStats(void) {
    kexHeap::DrawHeapInfo();
    kexAI::PrintDebugInfo();
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

    gameManager.PrintDebugStats();
    scriptManager.DrawGCStats();
    kexRenderUtils::ClearDebugLine();
}
