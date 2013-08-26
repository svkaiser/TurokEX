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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "SDL.h"
#include "shared.h"
#include "input.h"
#include "keyinput.h"

#define FIXED_WIDTH     320
#define FIXED_HEIGHT    240

class kexSystem {
public:
                    kexSystem(void);
                    ~kexSystem(void);

    void            Main(int argc, char **argv);
    void            Sleep(unsigned long usecs);
    void            Shutdown(void);
    int             GetMS(void);
    void            SpawnInternalConsole(void);
    void            ShowInternalConsole(bool show);
    void            DestroyInternalConsole(void);
    void            SwapBuffers(void);
    int             GetWindowFlags(void);

    int             VideoWidth(void) { return videoWidth; }
    int             VideoHeight(void) { return videoHeight; }
    float           VideoRatio(void) { return videoRatio; }
    bool            IsWindowed(void) { return bWindowed; }
    SDL_Window      *Window(void) { return window; }

private:
    void            InitSDL(void);
    void            InitVideo(void);
    void            MainLoop(void);

    SDL_Window      *window;
    SDL_GLContext   glContext;
    int             videoWidth;
    int             videoHeight;
    float           videoRatio;
    bool            bWindowed;
};

extern kexSystem sysMain;

#endif