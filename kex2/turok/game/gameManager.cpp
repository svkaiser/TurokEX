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
// DESCRIPTION: Game Management System. Handles global definitions
//              and non in-game inputs (can be used for menus, etc)
//              as well as save/loading states.
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "defs.h"
#include "gameManager.h"

kexGameManager gameManager;

//
// kexGameManager:: kexGameManager
//

kexGameManager::kexGameManager(void) {
    this->onTick        = NULL;
    this->onLocalTick   = NULL;
    this->onSpawn       = NULL;
    this->onShutdown    = NULL;
    this->gameDef       = NULL;
}

//
// kexGameManager::~kexGameManager
//

kexGameManager::~kexGameManager(void) {
}

//
// kexGameManager::Init
//

void kexGameManager::Init(void) {
    scriptManager.Engine()->RegisterInterface("KexGame");

    // register protocols
    scriptManager.Engine()->RegisterInterfaceMethod("KexGame", "void OnTick(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("KexGame", "void OnLocalTick(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("KexGame", "void OnSpawn(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("KexGame", "void OnShutdown(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("KexGame", "bool OnInput(int, int, int, int)");
}

//
// kexGameManager::InitObject
//

void kexGameManager::InitObject(void) {
    kexScriptManager::RegisterDataObject<kexCanvas>("kGame");
    
    scriptManager.Engine()->RegisterObjectMethod(
        "kGame",
        "kCanvas &MenuCanvas(void)",
        asMETHODPR(kexGameManager, MenuCanvas, (void), kexCanvas&),
        asCALL_THISCALL);
    
    scriptManager.Engine()->RegisterObjectMethod(
        "kGame",
        "kKeyMapMem @GameDef(void)",
        asMETHODPR(kexGameManager, GameDef, (void), kexKeyMap*),
        asCALL_THISCALL);
    
    scriptManager.Engine()->RegisterGlobalProperty("kGame Game", &gameManager);
}

//
// kexGameManager::Shutdown
//

void kexGameManager::Shutdown(void) {
    Release();
    objHandle.Set(NULL, NULL);
}

//
// kexGameManager::OnShutdown
//

void kexGameManager::OnShutdown(void) {
    CallFunction(onShutdown);
}

//
// kexGameManager::Construct
//

void kexGameManager::Construct(const char *className) {
    if(!Spawn(className)) {
        return;
    }

    CallConstructor((kexStr(className) + " @" + className + "(void)").c_str());

    onTick      = type->GetMethodByDecl("void OnTick(void)");
    onLocalTick = type->GetMethodByDecl("void OnLocalTick(void)");
    onSpawn     = type->GetMethodByDecl("void OnSpawn(void)");
    onShutdown  = type->GetMethodByDecl("void OnShutdown(void)");
    onInput     = type->GetMethodByDecl("bool OnInput(int, int, int, int)");
}

//
// kexGameManager::CallConstructor
//

bool kexGameManager::CallConstructor(const char *decl) {
    int state = scriptManager.Context()->GetState();
    bool ok = false;

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PushState();
    }

    scriptManager.Context()->Prepare(type->GetFactoryByDecl(decl));

    if(scriptManager.Context()->Execute() == asEXECUTION_EXCEPTION) {
        common.Error("%s", scriptManager.Context()->GetExceptionString());
        return false;
    }

    obj = *(asIScriptObject**)scriptManager.Context()->GetAddressOfReturnValue();
    obj->AddRef();
    objHandle.Set(obj, type);
    ok = true;

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }

    return ok;
}

//
// kexGameManager::InitGame
//
// Should only be called once on startup
//

void kexGameManager::InitGame(void) {
    kexStr gameClass("DefaultGame");

    // load default game info
    if(gameDef = defManager.FindDefEntry("defs/game.def@default")) {
        gameDef->GetString("gameClass", gameClass);
    }

    Construct(gameClass.c_str());
    CallFunction(onSpawn);
}

//
// kexGameManager::SetTitle
//

void kexGameManager::SetTitle(void) {
    kexStr str;

    if(gameDef == NULL) {
        return;
    }

    if(gameDef->GetString("gameName", str)) {
        kexStr title = SDL_GetWindowTitle(sysMain.Window());
        title = title + "  (" + str + ")";

        SDL_SetWindowTitle(sysMain.Window(), title.c_str());
    }
}

//
// kexGameManager::ProcessInput
//

bool kexGameManager::ProcessInput(const event_t *ev) {
    int state = PrepareFunction(onInput);
    bool ok = false;
    
    if(state == -1) {
        return false;
    }
    
    SetCallArgument(0, ev->type);
    SetCallArgument(1, ev->data1);
    SetCallArgument(2, ev->data2);
    SetCallArgument(3, ev->data3);
    
    if(!ExecuteFunction(state)) {
        return false;
    }
    
    FinishFunction(state, &ok);
    return ok;
}
