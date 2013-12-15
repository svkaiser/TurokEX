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
//
// DESCRIPTION: Script Component Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "scriptAPI/component.h"

//-----------------------------------------------------------------------------
//
// kexComponent
//
//-----------------------------------------------------------------------------

//
// kexComponent:: kexComponent
//

kexComponent::kexComponent(void) {
    this->obj           = NULL;
    this->type          = NULL;
    this->mod           = scriptManager.Module();
    this->onThink       = NULL;
    this->onLocalThink  = NULL;
    this->onSpawn       = NULL;
}

//
// kexComponent::~kexComponent
//

kexComponent::~kexComponent(void) {
    if(obj && !sysMain.IsShuttingDown()) {
        obj->Release();
    }
    else {
        objHandle.Clear();
    }
}

//
// kexComponent::Spawn
//

bool kexComponent::Spawn(const char *className) {
    mod = scriptManager.Module();

    if(mod == NULL) {
        common.Error("kexComponent::Spawn: attempted to spawn %s while no script is loaded", className);
        return false;
    }

    type = scriptManager.Engine()->GetObjectTypeById(mod->GetTypeIdByDecl(className));

    if(type == NULL) {
        common.Warning("kexComponent::Spawn: %s not found\n", className);
        return false;
    }

    return true;
}

//
// kexComponent::CallFunction
//

bool kexComponent::CallFunction(asIScriptFunction *func) {
    if(func == NULL) {
        return false;
    }

    int state = scriptManager.Context()->GetState();

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PushState();
    }

    scriptManager.Context()->Prepare(func);
    scriptManager.Context()->SetObject(obj);
    if(scriptManager.Context()->Execute() == asEXECUTION_EXCEPTION) {
        common.Error("%s", scriptManager.Context()->GetExceptionString());
        return false;
    }

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }

    return true;
}

//
// kexComponent::CallConstructor
//

bool kexComponent::CallConstructor(const char *decl) {
    int state = scriptManager.Context()->GetState();
    bool ok = false;

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PushState();
    }

    scriptManager.Context()->Prepare(type->GetFactoryByDecl(decl));
    scriptManager.Context()->SetArgObject(0, objHandle.owner);

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
// kexComponent::CallFunction
//

bool kexComponent::CallFunction(const char *decl, int *val) {
    int state = scriptManager.Context()->GetState();
    bool ok = false;
    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PushState();
    }

    *val = 0;

    if(CallFunction(type->GetMethodByDecl(decl))) {
        *val = (int)scriptManager.Context()->GetReturnDWord();
        ok = true;
    }

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }

    return ok;
}

//-----------------------------------------------------------------------------
//
// kexActorComponent
//
//-----------------------------------------------------------------------------

//
// kexActorComponent:: kexActorComponent
//

kexActorComponent::kexActorComponent(void) {
    this->onTouch       = NULL;
    this->onDamage      = NULL;
}

//
// kexActorComponent::~kexActorComponent
//

kexActorComponent::~kexActorComponent(void) {
}

//
// kexActorComponent::Init
//

void kexActorComponent::Init(void) {
    scriptManager.Engine()->RegisterInterface("Component");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnThink(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnLocalThink(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnSpawn(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnTouch(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnDamage(void)");
}

//
// kexActorComponent::Construct
//

void kexActorComponent::Construct(const char *className) {
    if(!Spawn(className)) {
        return;
    }

    CallConstructor((kexStr(className) + " @" + className + "(kActor@)").c_str());

    onThink         = type->GetMethodByDecl("void OnThink(void)");
    onLocalThink    = type->GetMethodByDecl("void OnLocalThink(void)");
    onSpawn         = type->GetMethodByDecl("void OnSpawn(void)");
    onTouch         = type->GetMethodByDecl("void OnTouch(void)");
    onDamage        = type->GetMethodByDecl("void OnDamage(void)");
}

//-----------------------------------------------------------------------------
//
// kexAreaComponent
//
//-----------------------------------------------------------------------------

//
// kexAreaComponent:: kexAreaComponent
//

kexAreaComponent::kexAreaComponent(void) {
    this->onThink       = NULL;
    this->onLocalThink  = NULL;
    this->onEnter       = NULL;
    this->onExit        = NULL;
}

//
// kexAreaComponent::~kexAreaComponent
//

kexAreaComponent::~kexAreaComponent(void) {
}

//
// kexAreaComponent::Init
//

void kexAreaComponent::Init(void) {
    scriptManager.Engine()->RegisterInterface("AreaComponent");
    scriptManager.Engine()->RegisterInterfaceMethod("AreaComponent", "void OnThink(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("AreaComponent", "void OnLocalThink(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("AreaComponent", "void OnSpawn(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("AreaComponent", "void OnEnter(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("AreaComponent", "void OnExit(void)");
}

//
// kexAreaComponent::CallConstructor
//

bool kexAreaComponent::CallConstructor(const char *decl) {
    int state = scriptManager.Context()->GetState();
    bool ok = false;

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PushState();
    }

    scriptManager.Context()->Prepare(type->GetFactoryByDecl(decl));
    scriptManager.Context()->SetArgObject(0, objHandle.owner);

    if(scriptManager.Context()->Execute() == asEXECUTION_EXCEPTION) {
        common.Error("%s", scriptManager.Context()->GetExceptionString());
        return false;
    }

    obj = *(asIScriptObject**)scriptManager.Context()->GetAddressOfReturnValue();
    objHandle.Set(obj, type);
    ok = true;

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }

    return ok;
}

//
// kexAreaComponent::Construct
//

void kexAreaComponent::Construct(const char *className) {
    if(!Spawn(className)) {
        return;
    }

    CallConstructor((kexStr(className) + " @" + className + "(kArea@)").c_str());

    onThink         = type->GetMethodByDecl("void OnThink(void)");
    onLocalThink    = type->GetMethodByDecl("void OnLocalThink(void)");
    onSpawn         = type->GetMethodByDecl("void OnSpawn(void)");
    onEnter         = type->GetMethodByDecl("void OnEnter(void)");
    onExit          = type->GetMethodByDecl("void OnExit(void)");
}
