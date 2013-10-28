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

#ifndef __RENDERSYS_H__
#define __RENDERSYS_H__

#include "textureObject.h"
#include "renderFont.h"
#include "cachefilelist.h"

typedef enum {
    GLFUNC_LEQUAL   = 0,
    GLFUNC_GEQUAL,
    GLFUNC_EQUAL,
    GLFUNC_ALWAYS,
    GLFUNC_NEVER,
} glFunctions_t;

typedef enum {
    GLCULL_FRONT    = 0,
    GLCULL_BACK
} glCullType_e;

typedef enum {
    GLSRC_ZERO      = 0,
    GLSRC_ONE,
    GLSRC_DST_COLOR,
    GLSRC_ONE_MINUS_DST_COLOR,
    GLSRC_SRC_ALPHA,
    GLSRC_ONE_MINUS_SRC_ALPHA,
    GLSRC_DST_ALPHA,
    GLSRC_ONE_MINUS_DST_ALPHA,
    GLSRC_ALPHA_SATURATE,
} glSrcBlend_e;

typedef enum {
    GLDST_ZERO      = 0,
    GLDST_ONE,
    GLDST_SRC_COLOR,
    GLDST_ONE_MINUS_SRC_COLOR,
    GLDST_SRC_ALPHA,
    GLDST_ONE_MINUS_SRC_ALPHA,
    GLDST_DST_ALPHA,
    GLDST_ONE_MINUS_DST_ALPHA,
} glDstBlend_e;

class kexRenderSystem {
public:
                                    kexRenderSystem(void);
                                    ~kexRenderSystem(void);

    void                            Init(void);
    void                            Shutdown(void);
    void                            SetOrtho(void);
    void                            SwapBuffers(void);
    void                            SetState(int bits, bool bEnable);
    void                            SetAlphaFunc(int func, float val);
    void                            SetDepth(int func);
    void                            SetBlend(int src, int dest);
    void                            SetEnv(int env);
    void                            SetCull(int type);
    void                            SetTextureUnit(int unit);
    void                            SetViewDimensions(void);
    kexFont                         *CacheFont(const char *name);
    kexTexture                      *CacheTexture(const char *name, texClampMode_t clampMode,
                                        texFilterMode_t filterMode = TF_LINEAR);

    const int                       ViewWidth(void) const { return viewWidth; }
    const int                       ViewHeight(void) const { return viewHeight; }
    const int                       WindowX(void) const { return viewWindowX; }
    const int                       WindowY(void) const { return viewWindowY; }
    const int                       MaxTextureUnits(void) const { return maxTextureUnits; }
    const int                       MaxTextureSize(void) const { return maxTextureSize; }
    const float                     MaxAnisotropic(void) const { return maxAnisotropic; }
    const bool                      IsWideScreen(void) const { return bWideScreen; }
    const bool                      IsFullScreen(void) const { return bFullScreen; }

    static const int                SCREEN_WIDTH        = 320;
    static const int                SCREEN_HEIGHT       = 240;
    static const int                MAX_TEXTURE_UNITS   = 4;

    kexTexture                      defaultTexture;
    kexTexture                      whiteTexture;
    kexTexture                      blackTexture;

    kexFont                         consoleFont;

    typedef struct {
        dtexture                    currentTexture;
        int                         environment;
    } texUnit_t;

    typedef struct {
        int                         glStateBits;
        int                         depthFunction;
        int                         blendSrc;
        int                         blendDest;
        int                         cullType;
        int                         alphaFunction;
        float                       alphaFuncThreshold;
        int                         currentUnit;
        texUnit_t                   textureUnits[MAX_TEXTURE_UNITS];
    } glState_t;

    glState_t                       glState;

private:
    int                             GetOGLVersion(const char* version);

    int                             viewWidth;
    int                             viewHeight;
    int                             viewWindowX;
    int                             viewWindowY;
    int                             maxTextureUnits;
    int                             maxTextureSize;
    float                           maxAnisotropic;
    bool                            bWideScreen;
    bool                            bFullScreen;

    kexFileCacheList<kexTexture>    textureList;
    kexFileCacheList<kexFont>       fontList;

    const char                      *gl_vendor;
    const char                      *gl_renderer;
    const char                      *gl_version;
};

extern kexRenderSystem renderSystem;

#endif