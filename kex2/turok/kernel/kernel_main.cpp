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
#include "sound.h"
#include "gl.h"
#include "script.h"
#include "game.h"
#include "js.h"
#include "type.h"
#include "fx.h"
#include "debug.h"
#include "filesystem.h"
#include "console.h"

kexCvar cvarDeveloper("developer", CVF_BOOL|CVF_CONFIG, "0", "Developer mode");
kexCvar cvarFixedTime("fixedtime", CVF_INT|CVF_CONFIG, "0", "TODO");

//
// Sys_Sleep
//

void Sys_Sleep(unsigned long usecs) {
    SDL_Delay(usecs);
}

//
// Sys_Shutdown
//

void Sys_Shutdown(void) {
    common.WriteConfigFile();

    J_Shutdown();
    Snd_Shutdown();
    FX_Shutdown();
    Tex_Shutdown();
    R_Shutdown();
    G_Shutdown();
    fileSystem.Shutdown();
    client.Shutdown();
    server.Shutdown();
    cvarManager.Shutdown();
    common.Shutdown();

#ifdef _WIN32
    Sys_DestroyConsole();
#endif

    SDL_Quit();

    exit(0);
}

//
// Sys_GetMilliseconds
//

int Sys_GetMilliseconds(void) {
    uint32 ticks;
    static int basetime = 0;
    
    ticks = SDL_GetTicks();
    
    if(basetime == 0)
        basetime = ticks;
    
    return ticks - basetime;
}

//
// FCmd_ShowWinConsole
//

static void FCmd_ShowWinConsole(void) {
    if(command.GetArgc() < 1) {
        common.Printf("1 = show, 0 = hide");
        return;
    }

#ifdef _WIN32
    Sys_ShowConsole(atoi(command.GetArgv(1)));
#endif
}

//
// Sys_Init
//

static void Sys_Init(void) {
    uint32 f = SDL_INIT_VIDEO;
    
#ifdef _DEBUG
    f |= SDL_INIT_NOPARACHUTE;
#endif
    
    putenv("SDL_VIDEO_CENTERED=1");

    if(SDL_Init(f) < 0) {
        common.Error("Failed to initialize SDL");
        return;
    }
    
    SDL_ShowCursor(0);
    SDL_WM_SetCaption(kva("Kex Engine - Version Date: %s", __DATE__),
        "Kex Engine");

    Z_Init();

    command.Add("showconsole", FCmd_ShowWinConsole);
}

//
// FCmd_Quit
//

static void FCmd_Quit(void) {
    Sys_Shutdown();
}

//
// Kernel_Run
//

void Kernel_Run(void) {
    int msec;
    int prevmsec;
    int nextmsec;

    server.CreateHost();
    client.Connect("localhost");

    prevmsec = Sys_GetMilliseconds();

#ifdef _WIN32
    Sys_ShowConsole(0);
#endif

    while(1) {
        do {
            nextmsec = Sys_GetMilliseconds();
            msec = nextmsec - prevmsec;
        }
        while(msec < 1);

        if(cvarFixedTime.GetInt())
            msec = cvarFixedTime.GetInt();

        server.Run(msec);
        client.Run(msec);

        prevmsec = nextmsec;

        Z_FreeAlloca();
        J_GarbageCollect();
    }
}

//
// Kernel_Main
//

void Kernel_Main(int argc, char **argv) {
    myargc = argc;
    myargv = argv;

#ifdef _WIN32
    Sys_SpawnConsole();
#endif

    Sys_Init();
    common.Printf("Kex Kernel Initialized\n");

    command.Init();
    cvarManager.Init();
    kexObject::Init();
    console.Init();
    inputSystem.Init();
    inputKey.Init();

    Debug_Init();
    common.Printf("Debug System Initialized\n");

    fileSystem.Init();

    Snd_Init();
    common.Printf("Sound System Initialized (%s)\n", Snd_GetDeviceName());
    J_Init();
    common.Printf("Javascript API Initialized\n");

    server.Init();
    client.Init();

    R_Init();
    common.Printf("Renderer Initialized\n");

    command.Add("quit", FCmd_Quit);

    G_Init();
    common.Printf("Game Initialized\n");

    common.ReadConfigFile("config.cfg");
    common.ReadConfigFile("autoexec.cfg");

    GL_Init();
    common.Printf("OpenGL Initialized\n");

    common.Printf("Running kernel...\n");
    Kernel_Run();
}
