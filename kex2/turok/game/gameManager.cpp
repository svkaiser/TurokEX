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
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "server.h"
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
