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
// DESCRIPTION: Main System
//
//-----------------------------------------------------------------------------

#include <time.h>

#include "SDL.h"

#include "common.h"
#include "zone.h"
#include "system.h"
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
#include "renderWorld.h"
#include "scriptAPI/scriptSystem.h"

kexCvar cvarDeveloper("developer", CVF_BOOL|CVF_CONFIG, "0", "Developer mode");
kexCvar cvarFixedTime("fixedtime", CVF_INT|CVF_CONFIG, "0", "TODO");
kexCvar cvarVidWidth("v_width", CVF_INT|CVF_CONFIG, "640", "TODO");
kexCvar cvarVidHeight("v_height", CVF_INT|CVF_CONFIG, "480", "TODO");
kexCvar cvarVidWindowed("v_windowed", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarVidVSync("v_vsync", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarVidDepthSize("v_depthsize", CVF_INT|CVF_CONFIG, "24", "TODO");
kexCvar cvarVidStencilSize("v_stencilsize", CVF_INT|CVF_CONFIG, "8", "TODO");
kexCvar cvarVidBuffSize("v_buffersize", CVF_INT|CVF_CONFIG, "32", "TODO");

kexSystem sysMain;

//
// FCmd_ShowWinConsole
//

static void FCmd_ShowWinConsole(void) {
    if(command.GetArgc() < 1) {
        common.Printf("1 = show, 0 = hide");
        return;
    }

    sysMain.ShowInternalConsole((atoi(command.GetArgv(1)) != 0));
}

//
// FCmd_Quit
//

static void FCmd_Quit(void) {
    sysMain.Shutdown();
}

//
// kexSystem::kexSystem
//

kexSystem::kexSystem(void) {
    bShuttingDown = false;
}

//
// kexSystem::~kexSystem
//

kexSystem::~kexSystem(void) {
}

//
// kexSystem::Sleep
//

void kexSystem::Sleep(unsigned long usecs) {
    SDL_Delay(usecs);
}

//
// kexSystem::SpawnInternalConsole
//

void kexSystem::SpawnInternalConsole(void) {
#ifdef _WIN32
    Sys_SpawnConsole();
#endif
}

//
// kexSystem::ShowInternalConsole
//

void kexSystem::ShowInternalConsole(bool show) {
#ifdef _WIN32
    Sys_ShowConsole(show);
#endif
}

//
// kexSystem::DestroyInternalConsole
//

void kexSystem::DestroyInternalConsole(void) {
#ifdef _WIN32
    Sys_DestroyConsole();
#endif
}

//
// kexSystem::Shutdown
//

void kexSystem::Shutdown(void) {
    common.WriteConfigFile();

    bShuttingDown = true;

    J_Shutdown();
    Snd_Shutdown();
    FX_Shutdown();
    Tex_Shutdown();
    R_Shutdown();
    G_Shutdown();

    scriptManager.Shutdown();
    fileSystem.Shutdown();
    client.Shutdown();
    server.Shutdown();
    cvarManager.Shutdown();
    common.Shutdown();

    DestroyInternalConsole();

    if(glContext) {
        SDL_GL_DeleteContext(glContext);
        glContext = NULL;
    }

    if(window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();

    fclose(f_stdout);
    fclose(f_stderr);

    exit(0);
}

//
// kexSystem::GetMS
//

int kexSystem::GetMS(void) {
    uint32 ticks;
    static int basetime = 0;
    
    ticks = SDL_GetTicks();
    
    if(basetime == 0)
        basetime = ticks;
    
    return ticks - basetime;
}

//
// kexSystem::InitSDL
//

void kexSystem::InitSDL(void) {
    uint32 f = SDL_INIT_VIDEO;
    
#ifdef _DEBUG
    f |= SDL_INIT_NOPARACHUTE;
#endif

    if(SDL_Init(f) < 0) {
        common.Error("Failed to initialize SDL");
        return;
    }
    
    SDL_ShowCursor(0);

    Z_Init();

    command.Add("showconsole", FCmd_ShowWinConsole);
    common.Printf("SDL Initialized\n");
}

//
// kexSystem::Log
//

void kexSystem::Log(const char *fmt, ...) {
#define MAX_LOGMESSAGE_LENGTH   3584
    va_list list;
    time_t copy;
    static char buffer[64];
    struct tm *local;
    char logMessage[MAX_LOGMESSAGE_LENGTH];

    SDL_memset(logMessage, 0, MAX_LOGMESSAGE_LENGTH);
    va_start(list, fmt);
    SDL_vsnprintf(logMessage, MAX_LOGMESSAGE_LENGTH - 1, fmt, list);
    va_end(list);

#ifdef _WIN32
    Sys_Printf(logMessage);
#endif

    SDL_memset(buffer, 0, sizeof(buffer));
    copy = time(0);
    local = localtime(&copy);
    strftime(buffer, sizeof(buffer), "%X", local);

    printf("%s: %s", buffer, logMessage);
}

//
// kexSystem::SwapBuffers
//

void kexSystem::SwapBuffers(void) {
    SDL_GL_SwapWindow(window);
}

//
// kexSystem::GetWindowFlags
//

int kexSystem::GetWindowFlags(void) {
    return SDL_GetWindowFlags(window);
}

//
// kexSystem::InitVideo
//

void kexSystem::InitVideo(void) {
    int newwidth;
    int newheight;
    int p;
    uint32 flags = 0;
    
    bWindowed = cvarVidWindowed.GetBool();
    videoWidth = cvarVidWidth.GetInt();
    videoHeight = cvarVidHeight.GetInt();
    videoRatio = (float)videoWidth / (float)videoHeight;
    
    if(common.CheckParam("-window")) {
        bWindowed = true;
    }

    if(common.CheckParam("-fullscreen")) {
        bWindowed = false;
    }
    
    newwidth = newheight = 0;
    
    p = common.CheckParam("-width");
    if(p && p < myargc - 1) {
        newwidth = atoi(myargv[p+1]);
    }

    p = common.CheckParam("-height");
    if(p && p < myargc - 1) {
        newheight = atoi(myargv[p+1]);
    }
    
    if(newwidth && newheight) {
        videoWidth = newwidth;
        videoHeight = newheight;
    }

    if(cvarVidDepthSize.GetInt() != 8 &&
        cvarVidDepthSize.GetInt() != 16 &&
        cvarVidDepthSize.GetInt() != 24) {
        cvarVidDepthSize.Set(24);
    }

    // TODO
    if(cvarVidStencilSize.GetInt() != 8/* &&
        cvarVidStencilSize.GetInt() != 16 &&
        cvarVidStencilSize.GetInt() != 24*/) {
        cvarVidStencilSize.Set(8);
    }
    
    if(cvarVidBuffSize.GetInt() != 8 &&
        cvarVidBuffSize.GetInt() != 16 &&
        cvarVidBuffSize.GetInt() != 24
        && cvarVidBuffSize.GetInt() != 32) {
        cvarVidBuffSize.Set(32);
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, cvarVidBuffSize.GetInt());
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, cvarVidDepthSize.GetInt());
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, cvarVidStencilSize.GetInt());
    SDL_GL_SetSwapInterval(cvarVidVSync.GetBool());
    
    flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
    
    if(!bWindowed)
        flags |= SDL_WINDOW_FULLSCREEN;

    window = SDL_CreateWindow(
        kva("Kex Engine - Version Date: %s", __DATE__),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        sysMain.VideoWidth(), sysMain.VideoHeight(), flags);
    
    if(window == NULL) {
        common.Error("kexSystem::InitVideo: Failed to create window");
    }

    if((glContext = SDL_GL_CreateContext(window)) == NULL)
        common.Error("kexSystem::InitVideo: Failed to create opengl context");

    common.Printf("Video Initialized\n");
}

//
// kexSystem::MainLoop
//

void kexSystem::MainLoop(void) {
    int msec;
    int prevmsec;
    int nextmsec;

    server.CreateHost();
    client.Connect("localhost");

    prevmsec = GetMS();

    ShowInternalConsole(false);

    while(1) {
        do {
            nextmsec = GetMS();
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
// kexSystem::Main
//

void kexSystem::Main(int argc, char **argv) {
    myargc = argc;
    myargv = argv;

    f_stdout = freopen("stdout.txt", "wt", stdout);
    f_stderr = freopen("stderr.txt", "wt", stderr);

    SpawnInternalConsole();

    InitSDL();

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

    command.Add("quit", FCmd_Quit);

    G_Init();
    common.Printf("Game Initialized\n");

    common.ReadConfigFile("config.cfg");
    common.ReadConfigFile("autoexec.cfg");

    scriptManager.Init();

    InitVideo();

    GL_Init();
    common.Printf("OpenGL Initialized\n");

    R_Init();
    common.Printf("Renderer Initialized\n");

    kexRenderWorld::Init();

    common.Printf("Running kernel...\n");
    MainLoop();
}
