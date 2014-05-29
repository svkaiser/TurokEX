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
#include "SDL_endian.h"
#include "systemBase.h"
#include "input.h"
#include "keyinput.h"

// Defines for checking the endianness of the system.

#if SDL_BYTEORDER == SYS_LIL_ENDIAN
    #define SYS_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SYS_BIG_ENDIAN
    #define SYS_BIG_ENDIAN
#endif

//
// SYSTEM
//
#ifdef _WIN32
void Sys_ShowConsole(kbool show);
void Sys_SpawnConsole(void);
void Sys_DestroyConsole(void);
void Sys_UpdateConsole(void);
void Sys_Printf(const char *string);
void Sys_Error(const char *string);
#endif

class kexSystem : public kexSystemBase {
public:
                            kexSystem(void);
                            ~kexSystem(void);

    virtual void            Main(int argc, char **argv);
    virtual void            Init(void);
    virtual void            Sleep(unsigned long usecs);
    virtual void            Shutdown(void);
    virtual int             GetMS(void);
    virtual int             GetTicks(void);
    virtual void            SpawnInternalConsole(void);
    virtual void            ShowInternalConsole(bool show);
    virtual void            DestroyInternalConsole(void);
    virtual void            SwapBuffers(void);
    virtual int             GetWindowFlags(void);
    virtual const char      *GetWindowTitle(void);
    virtual void            SetWindowTitle(const char *string);
    virtual void            SetWindowGrab(const bool bEnable);
    virtual void            WarpMouseToCenter(void);
    virtual short           SwapLE16(const short val);
    virtual short           SwapBE16(const short val);
    virtual int             SwapLE32(const int val);
    virtual int             SwapBE32(const int val);
    virtual void            *GetProcAddress(const char *proc);
    virtual int             CheckParam(const char *check);
    virtual const char      *GetBaseDirectory(void);
    virtual void            Log(const char *fmt, ...);
    virtual void            Printf(const char *string, ...);
    virtual void            CPrintf(rcolor color, const char *string, ...);
    virtual void            Warning(const char *string, ...);
    virtual void            DPrintf(const char *string, ...);
    virtual void            Error(const char *string, ...);
    
    virtual void            *Window(void) { return window; }

private:
    void                    InitVideo(void);
    void                    MainLoop(void);

    SDL_Window              *window;
    SDL_GLContext           glContext;
};

extern kexSystem sysMain;

#endif