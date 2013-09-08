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

class kexRenderSystem {
public:

                        kexRenderSystem(void);
                        ~kexRenderSystem(void);
                    
    void                Init(void);
    void                InitOpenGL(void);
    void                Shutdown(void);
    void                SetOrtho(void);
    void                SwapBuffers(void);
    byte                *GetScreenBuffer(int x, int y, int width, int height, bool bFlip);
    dtexture            ScreenToTexture(void);
    void                SetState(int bits, bool bEnable);
    void                SetFunc(int type, int func, float val);
    void                SetEnv(int env);
    void                SetCull(int type);
    void                SetTextureUnit(int unit);
    
    const int           ViewWidth(void) const { return viewWidth; }
    const int           ViewHeight(void) const { return viewHeight; }
    const int           WindowX(void) const { return viewWindowX; }
    const int           WindowY(void) const { return viewWindowY; }
    const int           MaxTextureUnits(void) const { return maxTextureUnits; }
    const int           MaxTextureSize(void) const { return maxTextureSize; }
    const float         MaxAnisotropic(void) const { return maxAnisotropic; }
    const bool          IsWideScreen(void) const { return bWideScreen; }
    const bool          IsFullScreen(void) const { return bFullScreen; }

    static const int    SCREEN_WIDTH    = 320;
    static const int    SCREEN_HEIGHT   = 240;

private:
    int                 viewWidth;
    int                 viewHeight;
    int                 viewWindowX;
    int                 viewWindowY;
    int                 maxTextureUnits;
    int                 maxTextureSize;
    float               maxAnisotropic;
    bool                bWideScreen;
    bool                bFullScreen;
    
    static const int    MAX_TEXTURE_UNITS = 4;
    
    typedef struct {
        dtexture        currentTexture;
        int             environment;
    } texUnit_t;
    
    typedef struct {
        int             glStateBits;
        int             depthFunction;
        int             blendFunction;
        int             alphaFunction;
        float           alphaFuncThreshold;
        int             currentUnit;
        texUnit_t       textureUnits[MAX_TEXTURE_UNITS];
    } glState_t;

    glState_t           glState;
};

extern kexRenderSystem renderSystem;

#endif
