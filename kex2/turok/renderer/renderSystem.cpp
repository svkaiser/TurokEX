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
// DESCRIPTION: Renderer backend
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "renderSystem.h"

kexRenderSystem renderSystem;

//
// kexRenderSystem::kexRenderSystem
//

kexRenderSystem::kexRenderSystem(void) {
    this->viewWidth         = this->SCREEN_WIDTH;
    this->viewHeight        = this->SCREEN_HEIGHT;
    this->viewWindowX       = 0;
    this->viewWindowY       = 0;
    this->maxTextureUnits   = 1;
    this->maxTextureSize    = 64;
    this->maxAnisotropic    = 0;
    this->bWideScreen       = false;
    this->bFullScreen       = false;
}

//
// kexRenderSystem::~kexRenderSystem
//

kexRenderSystem::~kexRenderSystem(void) {
}

//
// kexRenderSystem::SetViewDimensions
//

void kexRenderSystem::SetViewDimensions(void) {
    viewWidth = sysMain.VideoWidth();
    viewHeight = sysMain.VideoHeight();

    bWideScreen = !fcmp(((float)viewWidth / (float)viewHeight), (4.0f / 3.0f));

    viewWindowX = (sysMain.VideoWidth() - viewWidth) / 2;

    if(viewWidth == sysMain.VideoWidth()) {
        viewWindowY = 0;
    }
    else {
        viewWindowY = (viewHeight) / 2;
    }
}

//
// kexRenderSystem::Init
//

void kexRenderSystem::Init(void) {
}

//
// kexRenderSystem::InitOpenGL
//

void kexRenderSystem::InitOpenGL(void) {
    gl_vendor = (const char*)dglGetString(GL_VENDOR);
    common.Printf("GL_VENDOR: %s\n", gl_vendor);
    gl_renderer = (const char*)dglGetString(GL_RENDERER);
    common.Printf("GL_RENDERER: %s\n", gl_renderer);
    gl_version = (const char*)dglGetString(GL_VERSION);
    common.Printf("GL_VERSION: %s\n", gl_version);
    dglGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    common.Printf("GL_MAX_TEXTURE_SIZE: %i\n", maxTextureSize);
    dglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);
    common.Printf("GL_MAX_TEXTURE_UNITS_ARB: %i\n", maxTextureUnits);
    
    SetViewDimensions();
    
    dglViewport(0, 0, sysMain.VideoWidth(), sysMain.VideoHeight());
    dglClearDepth(1.0f);
    dglClearStencil(0);
    
    SetState(GLSTATE_TEXTURE0, false);
    SetState(GLSTATE_CULL, true);
    SetCull(GLCULL_FRONT);
    SetDepth(GLFUNC_LEQUAL);
    SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);

    dglEnable(GL_NORMALIZE);
    dglShadeModel(GL_SMOOTH);
    dglHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    dglFogi(GL_FOG_MODE, GL_LINEAR);
    dglHint(GL_FOG_HINT, GL_NICEST);
    dglEnable(GL_SCISSOR_TEST);
    dglEnable(GL_DITHER);
    dglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    dglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    dglColorMaterial(GL_FRONT, GL_DIFFUSE);
    dglColorMaterial(GL_BACK, GL_DIFFUSE);
}

//
// kexRenderSystem::Shutdown
//

void kexRenderSystem::Shutdown(void) {
}

//
// kexRenderSystem::SetOrtho
//

void kexRenderSystem::SetOrtho(void) {
    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    dglOrtho(0, sysMain.VideoWidth(), sysMain.VideoHeight(), 0, -1, 1);
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
}

//
// kexRenderSystem::SwapBuffers
//

void kexRenderSystem::SwapBuffers(void) {
    dglFinish();
    sysMain.SwapBuffers();
}

//
// kexRenderSystem::GetScreenBuffer
//

byte *kexRenderSystem::GetScreenBuffer(int x, int y, int width, int height, bool bFlip) {
    byte* buffer;
    byte* data;
    int i;
    int pack;
    int col;

    col     = (width * 3);
    data    = (byte*)Z_Calloc(height * width * 3, PU_STATIC, 0);
    buffer  = (byte*)Z_Calloc(col, PU_STATIC, 0);

    //
    // 20120313 villsa - force pack alignment to 1
    //
    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);

    //
    // Need to vertically flip the image
    // 20120313 villsa - better method to flip image. uses one buffer instead of two
    //
    if(bFlip) {
        for(i = 0; i < height / 2; i++)
        {
            memcpy(buffer, &data[i * col], col);
            memcpy(&data[i * col], &data[(height - (i + 1)) * col], col);
            memcpy(&data[(height - (i + 1)) * col], buffer, col);
        }
    }
    
    Z_Free(buffer);
    return data;
}

//
// kexRenderSystem::ScreenToTexture
//

dtexture kexRenderSystem::ScreenToTexture(void) {
    return 0;
}

//
// kexRenderSystem::SetState
//

void kexRenderSystem::SetState(int bits, bool bEnable) {
#define TOGGLEGLBIT(flag, bit)                                  \
    if(bEnable && !(glState.glStateBits & (1 << flag))) {       \
        dglEnable(bit);                                         \
        glState.glStateBits |= (1 << flag);                     \
    }                                                           \
    else if(!bEnable && (glState.glStateBits & (1 << flag))) {  \
        dglDisable(bit);                                        \
        glState.glStateBits &= ~(1 << flag);                    \
    }
    
    switch(bits) {
    case GLSTATE_BLEND:
        TOGGLEGLBIT(GLSTATE_BLEND, GL_BLEND);
        break;
    case GLSTATE_CULL:
        TOGGLEGLBIT(GLSTATE_CULL, GL_CULL_FACE);
        break;
    case GLSTATE_TEXTURE0:
        TOGGLEGLBIT(GLSTATE_TEXTURE0, GL_TEXTURE_2D);
        break;
    case GLSTATE_TEXTURE1:
        TOGGLEGLBIT(GLSTATE_TEXTURE1, GL_TEXTURE_2D);
        break;
    case GLSTATE_TEXTURE2:
        TOGGLEGLBIT(GLSTATE_TEXTURE2, GL_TEXTURE_2D);
        break;
    case GLSTATE_TEXTURE3:
        TOGGLEGLBIT(GLSTATE_TEXTURE3, GL_TEXTURE_2D);
        break;
    case GLSTATE_ALPHATEST:
        TOGGLEGLBIT(GLSTATE_ALPHATEST, GL_ALPHA_TEST);
        break;
    case GLSTATE_TEXGEN_S:
        TOGGLEGLBIT(GLSTATE_TEXGEN_S, GL_TEXTURE_GEN_S);
        break;
    case GLSTATE_TEXGEN_T:
        TOGGLEGLBIT(GLSTATE_TEXGEN_T, GL_TEXTURE_GEN_T);
        break;
    case GLSTATE_DEPTHTEST:
        TOGGLEGLBIT(GLSTATE_DEPTHTEST, GL_DEPTH_TEST);
        break;
    case GLSTATE_LIGHTING:
        TOGGLEGLBIT(GLSTATE_LIGHTING, GL_LIGHTING);
        break;
    default:
        common.Warning("kexRenderSystem::SetState: unknown bit flag: %i\n", bits);
        break;
    }
    
#undef TOGGLEGLBIT
}

//
// kexRenderSystem::SetFunc
//

void kexRenderSystem::SetAlphaFunc(int func, float val) {
    int pFunc = (glState.alphaFunction ^ func) |
        (glState.alphaFuncThreshold != val);
        
    if(pFunc == 0)
        return;
        
    int glFunc = 0;
        
    switch(func) {
    case GLFUNC_EQUAL:
        glFunc = GL_EQUAL;
        break;
    case GLFUNC_ALWAYS:
        glFunc = GL_ALWAYS;
        break;
    case GLFUNC_LEQUAL:
        glFunc = GL_LEQUAL;
        break;
    case GLFUNC_GEQUAL:
        glFunc = GL_GEQUAL;
        break;
    case GLFUNC_NEVER:
        glFunc = GL_NEVER;
        break;
    }
    
    dglAlphaFunc(glFunc, val);
    
    glState.alphaFunction = func;
    glState.alphaFuncThreshold = val;
}

//
// kexRenderSystem::SetDepth
//

void kexRenderSystem::SetDepth(int func) {
    int pFunc = glState.depthFunction ^ func;
    
    if(pFunc == 0)
        return;
        
    int glFunc = 0;
        
    switch(func) {
    case GLFUNC_EQUAL:
        glFunc = GL_EQUAL;
        break;
    case GLFUNC_ALWAYS:
        glFunc = GL_ALWAYS;
        break;
    case GLFUNC_LEQUAL:
        glFunc = GL_LEQUAL;
        break;
    case GLFUNC_GEQUAL:
        glFunc = GL_GEQUAL;
        break;
    case GLFUNC_NEVER:
        glFunc = GL_NEVER;
        break;
    }
    
    dglDepthFunc(glFunc);
    
    glState.depthFunction = func;
}

//
// kexRenderSystem::SetBlend
//

void kexRenderSystem::SetBlend(int src, int dest) {
    int pBlend = (glState.blendSrc ^ src) | (glState.blendDest ^ dest);
    
    if(pBlend == 0)
        return;
        
    int glSrc = GL_ONE;
    int glDst = GL_ONE;
    
    switch(src) {
    case GLSRC_ZERO:
        glSrc = GL_ZERO;
        break;
    case GLSRC_ONE:
        glSrc = GL_ONE;
        break;
    case GLSRC_DST_COLOR:
        glSrc = GL_DST_COLOR;
        break;
    case GLSRC_ONE_MINUS_DST_COLOR:
        glSrc = GL_ONE_MINUS_DST_COLOR;
        break;
    case GLSRC_SRC_ALPHA:
        glSrc = GL_SRC_ALPHA;
        break;
    case GLSRC_ONE_MINUS_SRC_ALPHA:
        glSrc = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case GLSRC_DST_ALPHA:
        glSrc = GL_DST_ALPHA;
        break;
    case GLSRC_ONE_MINUS_DST_ALPHA:
        glSrc = GL_ONE_MINUS_DST_ALPHA;
        break;
    case GLSRC_ALPHA_SATURATE:
        glSrc = GL_SRC_ALPHA_SATURATE;
        break;
    }
    
    switch(dest) {
    case GLDST_ZERO:
        glDst = GL_ZERO;
        break;
    case GLDST_ONE:
        glDst = GL_ONE;
        break;
    case GLDST_SRC_COLOR:
        glDst = GL_SRC_COLOR;
        break;
    case GLDST_ONE_MINUS_SRC_COLOR:
        glDst = GL_ONE_MINUS_SRC_COLOR;
        break;
    case GLDST_SRC_ALPHA:
        glDst = GL_SRC_ALPHA;
        break;
    case GLDST_ONE_MINUS_SRC_ALPHA:
        glDst = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case GLDST_DST_ALPHA:
        glDst = GL_DST_ALPHA;
        break;
    case GLDST_ONE_MINUS_DST_ALPHA:
        glDst = GL_ONE_MINUS_DST_ALPHA;
        break;
    }
    
    dglBlendFunc(glSrc, glDst);
    
    glState.blendSrc = src;
    glState.blendDest = dest;
}

//
// kexRenderSystem::SetEnv
//

void kexRenderSystem::SetEnv(int env) {
}

//
// kexRenderSystem::SetCull
//

void kexRenderSystem::SetCull(int type) {
    int pCullType = glState.cullType ^ type;
    
    if(pCullType == 0)
        return;
        
    int cullType = 0;
    
    switch(type) {
    case GLCULL_FRONT:
        cullType = GL_FRONT;
        break;
    case GLCULL_BACK:
        cullType = GL_BACK;
        break;
    }
    
    dglCullFace(cullType);
    
    glState.cullType = type;
}

//
// kexRenderSystem::SetTextureUnit
//

void kexRenderSystem::SetTextureUnit(int unit) {
    if(unit > MAX_TEXTURE_UNITS || unit < 0)
        return;
    
    if(unit == glState.currentUnit)
        return;
        
    dglActiveTextureARB(GL_TEXTURE0_ARB + unit);
    glState.currentUnit = unit;
}
