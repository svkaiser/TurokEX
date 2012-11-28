// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
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
// DESCRIPTION: Kernel System
//
//-----------------------------------------------------------------------------

#include "SDL.h"

#include "common.h"
#include "zone.h"
#include "kernel.h"
#include "server.h"
#include "client.h"
#include "render.h"
#include "gl.h"
#include "menu.h"
#include "script.h"
#include "game.h"
#include "js.h"
#include "type.h"

CVAR(developer, 0);
CVAR(fixedtime, 0);

//
// Sys_Sleep
//

void Sys_Sleep(unsigned long usecs)
{
    SDL_Delay(usecs);
}

//
// Sys_Shutdown
//

void Sys_Shutdown(void)
{
    Com_WriteConfigFile();

    Tex_Shutdown();
    R_Shutdown();
    G_Shutdown();
    KF_Shutdown();
    SV_Shutdown();
    Com_Shutdown();
    J_Shutdown();

#ifdef _WIN32
    Sys_DestroyConsole();
#endif

    SDL_Quit();

    exit(0);
}

//
// Sys_GetMilliseconds
//

int Sys_GetMilliseconds(void)
{
    uint32 ticks;
    static int basetime = 0;
    
    ticks = SDL_GetTicks();
    
    if(basetime == 0)
        basetime = ticks;
    
    return ticks - basetime;
}

//
// Sys_Init
//

static void Sys_Init(void)
{
    uint32 f = SDL_INIT_VIDEO;
    
#ifdef _DEBUG
    f |= SDL_INIT_NOPARACHUTE;
#endif
    
    putenv("SDL_VIDEO_CENTERED=1");

    if(SDL_Init(f) < 0)
    {
        Com_Error("Failed to initialize SDL");
        return;
    }
    
    SDL_WM_SetCaption(kva("Kex Engine - Version Date: %s", __DATE__),
        "Kex Engine");

    Z_Init();
    Con_Init();
}

//
// FCmd_Quit
//

static void FCmd_Quit(void)
{
    Sys_Shutdown();
}

//
// Kernel_Run
//

void Kernel_Run(void)
{
    int msec;
    int prevmsec;
    int nextmsec;

    SV_CreateHost();
    CL_Connect("localhost");

    prevmsec = Sys_GetMilliseconds();

#ifdef _WIN32
    Sys_ShowConsole(0);
#endif

    while(1)
    {
        do
        {
            nextmsec = Sys_GetMilliseconds();
            msec = nextmsec - prevmsec;
        }
        while(msec < 1);

        if((int)fixedtime.value)
        {
            msec = (int)fixedtime.value;
        }

        SV_Run(msec);
        CL_Run(msec);

        prevmsec = nextmsec;

        Z_FreeAlloca();
    }
}

//
// Kernel_Main
//

void Kernel_Main(int argc, char **argv)
{
    myargc = argc;
    myargv = argv;

#ifdef _WIN32
    Sys_SpawnConsole();
#endif

    Sys_Init();
    Com_Printf("Kex Kernel Initialized\n");
    Cmd_Init();
    Com_Printf("Command System Initialized\n");
    Cvar_Init();
    Com_Printf("Cvar System Initialized\n");
    SC_Init();
    Com_Printf("Lexer System Initialized\n");
    Key_Init();
    Com_Printf("Key System Initialized\n");
    KF_Init();
    Com_Printf("File System Initialized\n");
    SV_Init();
    Com_Printf("Server Initialized\n");
    CL_Init();
    Com_Printf("Client Initialized\n");
    Menu_Init();
    Com_Printf("Menu System Initialized\n");
    V_Init();
    Com_Printf("Video Initialized\n");
    R_Init();
    Com_Printf("Renderer Initialized\n");

    GL_Register();

    Cvar_Register(&fixedtime);
    Cvar_Register(&developer);
    Cmd_AddCommand("quit", FCmd_Quit);

    Com_ReadConfigFile("config.cfg");
    Com_ReadConfigFile("autoexec.cfg");

    KF_LoadZipFile("game.kpf");

    G_Init();
    Com_Printf("Game Initialized\n");
    GL_Init();
    Com_Printf("OpenGL Initialized\n");
    J_Init();
    Com_Printf("Javascript API Initialized\n");

    Com_Printf("Running kernel...\n");
    Kernel_Run();
}