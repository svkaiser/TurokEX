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

    objHandle.Clear();
}

//
// kexComponent::~kexComponent
//

kexComponent::~kexComponent(void) {
    objHandle.Set(NULL, NULL);
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

    name = className;

    return true;
}

//
// kexComponent::PrepareFunction
//

int kexComponent::PrepareFunction(const char *decl) {
    asIScriptFunction *func;
    int state;

    if(obj == NULL) {
        return -1;
    }

    func = type->GetMethodByDecl(decl);

    if(func == NULL) {
        return -1;
    }

    state = scriptManager.Context()->GetState();

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PushState();
    }

    scriptManager.Context()->Prepare(func);
    scriptManager.Context()->SetObject(obj);

    return state;
}

//
// kexComponent::PrepareFunction
//

int kexComponent::PrepareFunction(asIScriptFunction *func) {
    int state;

    if(func == NULL || obj == NULL) {
        return -1;
    }

    state = scriptManager.Context()->GetState();

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PushState();
    }

    scriptManager.Context()->Prepare(func);
    scriptManager.Context()->SetObject(obj);

    return state;
}

//
// kexComponent::SetCallArgument
//

void kexComponent::SetCallArgument(const int arg, int val) {
    scriptManager.Context()->SetArgDWord(arg, val);
}

//
// kexComponent::SetCallArgument
//

void kexComponent::SetCallArgument(const int arg, byte val) {
    scriptManager.Context()->SetArgByte(arg, val);
}

//
// kexComponent::SetCallArgument
//

void kexComponent::SetCallArgument(const int arg, float val) {
    scriptManager.Context()->SetArgFloat(arg, val);
}

//
// kexComponent::SetCallArgument
//

void kexComponent::SetCallArgument(const int arg, bool val) {
    scriptManager.Context()->SetArgByte(arg, val);
}

//
// kexComponent::SetCallArgument
//

void kexComponent::SetCallArgument(const int arg, void *val) {
    scriptManager.Context()->SetArgObject(arg, val);
}

//
// kexComponent::ExecuteFunction
//

bool kexComponent::ExecuteFunction(int state) {
    if(scriptManager.Context()->Execute() == asEXECUTION_EXCEPTION) {
        common.Error("%s", scriptManager.Context()->GetExceptionString());

        if(state == asEXECUTION_ACTIVE) {
            scriptManager.Context()->PopState();
        }
        return false;
    }

    return true;
}

//
// kexComponent::FinishFunction
//

void kexComponent::FinishFunction(int state) {
    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }
}

//
// kexComponent::FinishFunction
//

void kexComponent::FinishFunction(int state, int *val) {
    *val = (int)scriptManager.Context()->GetReturnDWord();

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }
}

//
// kexComponent::FinishFunction
//

void kexComponent::FinishFunction(int state, byte *val) {
    *val = (byte)scriptManager.Context()->GetReturnByte();

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }
}

//
// kexComponent::FinishFunction
//

void kexComponent::FinishFunction(int state, float *val) {
    *val = scriptManager.Context()->GetReturnFloat();

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }
}

//
// kexComponent::FinishFunction
//

void kexComponent::FinishFunction(int state, bool *val) {
    *val = (scriptManager.Context()->GetReturnByte() == 1);

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }
}

//
// kexComponent::FinishFunction
//

void kexComponent::FinishFunction(int state, void **val) {
    *val = scriptManager.Context()->GetAddressOfReturnValue();

    if(state == asEXECUTION_ACTIVE) {
        scriptManager.Context()->PopState();
    }
}

//
// kexComponent::CallFunction
//

bool kexComponent::CallFunction(asIScriptFunction *func) {
    int state = PrepareFunction(func);

    if(state == -1) {
        return false;
    }

    if(!ExecuteFunction(state)) {
        return false;
    }

    FinishFunction(state);
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
// kexComponent::Deconstruct
//

void kexComponent::Deconstruct(void) {
    CallFunction(type->GetMethodByIndex(asBEHAVE_DESTRUCT));
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
    this->onTrigger     = NULL;
    this->onThink       = NULL;
    this->onLocalThink  = NULL;
    this->onSpawn       = NULL;
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
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "bool OnTouch(kActor@)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnTrigger(void)");
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
    onTouch         = type->GetMethodByDecl("bool OnTouch(kActor@)");
    onTrigger       = type->GetMethodByDecl("void OnTrigger(void)");
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
    this->onSpawn       = NULL;
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
    obj->AddRef();
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
