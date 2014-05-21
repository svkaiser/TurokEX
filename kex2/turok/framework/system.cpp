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
#include "system.h"
#include "server.h"
#include "client.h"
#include "sound.h"
#include "script.h"
#include "type.h"
#include "fx.h"
#include "filesystem.h"
#include "console.h"
#include "renderBackend.h"
#include "world.h"
#include "scriptAPI/scriptSystem.h"
#include "gameManager.h"
#include "gui.h"

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

static char buffer[4096];

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
    gameManager.OnShutdown();

    localWorld.Unload();

    soundSystem.Shutdown();
    
    Mem_Purge(kexClipMesh::hb_clipMesh);
    Mem_Purge(kexCollisionMap::hb_collisionMap);

    renderBackend.Shutdown();
    modelManager.Shutdown();
    gameManager.Shutdown();
    fileSystem.Shutdown();
    client.Shutdown();
    server.Shutdown();
    cvarManager.Shutdown();
    common.Shutdown();

    Mem_Purge(hb_object);

    DestroyInternalConsole();

    if(glContext) {
        SDL_GL_DeleteContext(glContext);
        glContext = NULL;
    }

    if(window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    common.Printf("Shutting down\n");

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
    
    if(basetime == 0) {
        basetime = ticks;
    }
    
    return ticks - basetime;
}

//
// kexSystem::Init
//

void kexSystem::Init(void) {
    uint32 f = SDL_INIT_VIDEO;
    
#ifdef _DEBUG
    f |= SDL_INIT_NOPARACHUTE;
#endif

    if(SDL_Init(f) < 0) {
        common.Error("Failed to initialize SDL");
        return;
    }
    
    SDL_ShowCursor(0);

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
// kexSystem::GetWindowTitle
//

const char *kexSystem::GetWindowTitle(void) {
    return SDL_GetWindowTitle(window);
}

//
// kexSystem::SetWindowTitle
//

void kexSystem::SetWindowTitle(const char *string) {
    SDL_SetWindowTitle(window, string);
}

//
// kexSystem::SetWindowGrab
//

void kexSystem::SetWindowGrab(const bool bEnable) {
    SDL_SetWindowGrab(window, (SDL_bool)bEnable);
}

//
// kexSystem::WarpMouseToCenter
//

void kexSystem::WarpMouseToCenter(void) {
    SDL_WarpMouseInWindow(window,
                          (unsigned short)(sysMain.VideoWidth()/2),
                          (unsigned short)(sysMain.VideoHeight()/2));
}

//
// kexSystemBase::SwapLE16
//

short kexSystem::SwapLE16(const short val) {
    return SDL_SwapLE16(val);
}

//
// kexSystemBase::SwapBE16
//

short kexSystem::SwapBE16(const short val) {
    return SDL_SwapBE16(val);
}

//
// kexSystemBase::SwapLE32
//

int kexSystem::SwapLE32(const int val) {
    return SDL_SwapLE32(val);
}

//
// kexSystemBase::SwapBE32
//

int kexSystem::SwapBE32(const int val) {
    return SDL_SwapBE32(val);
}

//
// kexSystem::CheckParam
//

int kexSystem::CheckParam(const char *check) {
    for(int i = 1; i < argc; i++) {
        if(!kexStr::Compare(check, argv[i])) {
            return i;
        }
    }
    return 0;
}

//
// kexSystem::Printf
//

void kexSystem::Printf(const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);
    
    console.Print(COLOR_WHITE, buffer);
    Log(buffer);
}

//
// kexSystem::CPrintf
//

void kexSystem::CPrintf(rcolor color, const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);
    
    console.Print(color, buffer);
    Log(buffer);
}

//
// kexSystem::Warning
//

void kexSystem::Warning(const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);
    
    console.Print(COLOR_YELLOW, buffer);
    Log(buffer);
}

//
// kexSystem::DPrintf
//

void kexSystem::DPrintf(const char *string, ...) {
    if(cvarDeveloper.GetBool()) {
        static char buffer[1024];
        va_list	va;
        
        va_start(va, string);
        vsprintf(buffer, string, va);
        va_end(va);
        
        CPrintf(RGBA(0xE0, 0xE0, 0xE0, 0xff), buffer);
    }
}

//
// kexSystem::Error
//

void kexSystem::Error(const char* string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);
    
    fprintf(stderr, "Error - %s\n", buffer);
    fflush(stderr);
    
    Log(buffer);
    
#ifdef _WIN32
    Sys_Error(buffer);
#else
    const SDL_MessageBoxButtonData buttons[1] = {
        {
            SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
            0,
            "OK"
        }
    };
    SDL_MessageBoxData data = {
        SDL_MESSAGEBOX_ERROR,
        NULL,
        "Error",
        buffer,
        2,
        buttons,
        NULL
    };
    
    int button = -1;
    SDL_ShowMessageBox(&data, &button);
#endif
    
    exit(0);    // just in case...
}

//
// kexSystem::GetBaseDirectory
//

const char *kexSystem::GetBaseDirectory(void) {
    static const char dummyDirectory[] = {"."};
    // cache multiple requests
    if(!basePath) {
        size_t len = strlen(*argv);
        char *p = (basePath = (char*)Mem_Malloc(len + 1, hb_static)) + len - 1;
        
        strcpy(basePath, *argv);
        while (p > basePath && *p!='/' && *p!='\\') {
            *p--=0;
        }
        
        if(*p=='/' || *p=='\\') {
            *p--=0;
        }
        
        if(strlen(basePath) < 2) {
            Mem_Free(basePath);
            basePath = (char*)Mem_Malloc(1024, hb_static);
            if(!getcwd(basePath, 1024)) {
                strcpy(basePath, dummyDirectory);
            }
        }
    }
    
    return basePath;
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
    
    if(sysMain.CheckParam("-window")) {
        bWindowed = true;
    }

    if(sysMain.CheckParam("-fullscreen")) {
        bWindowed = false;
    }
    
    newwidth = newheight = 0;
    
    p = sysMain.CheckParam("-width");
    if(p && p < argc - 1) {
        newwidth = atoi(argv[p+1]);
    }

    p = sysMain.CheckParam("-height");
    if(p && p < argc - 1) {
        newheight = atoi(argv[p+1]);
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

    if((glContext = SDL_GL_CreateContext(window)) == NULL) {
        common.Error("kexSystem::InitVideo: Failed to create opengl context");
    }

    gameManager.SetTitle();
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

        Mem_GC();
    }
}

//
// kexSystem::Main
//

void kexSystem::Main(int argc, char **argv) {
    this->argc = argc;
    this->argv = argv;

    f_stdout = freopen("stdout.txt", "wt", stdout);
    f_stderr = freopen("stderr.txt", "wt", stderr);

    SpawnInternalConsole();

    Init();

    command.Init();
    kexHeap::Init();
    cvarManager.Init();
    kexObject::Init();
    console.Init();
    inputSystem.Init();
    inputKey.Init();
    fileSystem.Init();
    soundSystem.Init();
    server.Init();
    client.Init();

    command.Add("quit", FCmd_Quit);
    cvarManager.InitCustomCvars();

    gameManager.InitGame();
    inputKey.InitActions();

    common.ReadConfigFile("config.cfg");
    common.ReadConfigFile("autoexec.cfg");

    InitVideo();

    renderBackend.Init();
    fxManager.Init();
    guiManager.Init();

    common.Printf("Running kernel...\n");
    gameManager.SpawnGame();
    MainLoop();
}
