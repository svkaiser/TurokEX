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
#include "scriptAPI/scriptSystem.h"

//
// kexComponent:: kexComponent
//

kexComponent::kexComponent(void) {
    this->obj           = NULL;
    this->type          = NULL;
    this->onThink       = NULL;
    this->onTouch       = NULL;
    this->onDamage      = NULL;
    this->onPreDraw     = NULL;
    this->onDraw        = NULL;
    this->onPostDraw    = NULL;
}

//
// kexComponent::~kexComponent
//

kexComponent::~kexComponent(void) {
    obj->Release();
}

//
// kexComponent::Init
//

void kexComponent::Init(void) {
    scriptManager.Engine()->RegisterInterface("Component");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnThink(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnTouch(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnDamage(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnPreDraw(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnDraw(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("Component", "void OnPostDraw(void)");
}

//
// kexComponent::Spawn
//

void kexComponent::Spawn(const char *className) {
    type = scriptManager.Engine()->GetObjectTypeById(mod->GetTypeIdByDecl(className));

    if(type == NULL)
        return;

    onThink     = type->GetMethodByDecl("void OnThink(void)");
    onTouch     = type->GetMethodByDecl("void OnTouch(void)");
    onDamage    = type->GetMethodByDecl("void OnDamage(void)");
    onPreDraw   = type->GetMethodByDecl("void OnPreDraw(void)");
    onDraw      = type->GetMethodByDecl("void OnDraw(void)");
    onPostDraw  = type->GetMethodByDecl("void OnPostDraw(void)");

    CallConstructor((kexStr(className) + " @" + className + "()").c_str());
}

//
// kexComponent::CallFunction
//

bool kexComponent::CallFunction(asIScriptFunction *func) {
    if(func == NULL)
        return false;

    scriptManager.Context()->Prepare(func);
    scriptManager.Context()->Execute();

    return true;
}

//
// kexComponent::CallConstructor
//

bool kexComponent::CallConstructor(const char *decl) {
    int state = scriptManager.Context()->GetState();
    bool ok = false;

    if(state == asEXECUTION_ACTIVE)
        scriptManager.Context()->PushState();

    if(CallFunction(type->GetFactoryByDecl(decl))) {
        obj = *(asIScriptObject**)scriptManager.Context()->GetAddressOfReturnValue();
        obj->AddRef();
        objHandle.Set(obj, type);
        ok = true;
    }

    if(state == asEXECUTION_ACTIVE)
        scriptManager.Context()->PopState();

    return ok;
}

//
// kexComponent::CallFunction
//

bool kexComponent::CallFunction(const char *decl, int *val) {
    int state = scriptManager.Context()->GetState();
    bool ok = false;
    if(state == asEXECUTION_ACTIVE)
        scriptManager.Context()->PushState();

    *val = 0;

    if(CallFunction(type->GetMethodByDecl(decl))) {
        *val = (int)scriptManager.Context()->GetReturnDWord();
        ok = true;
    }

    if(state == asEXECUTION_ACTIVE)
        scriptManager.Context()->PopState();

    return ok;
}
