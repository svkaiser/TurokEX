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
// DESCRIPTION: Script system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "fileSystem.h"
#include "zone.h"
#include "world.h"
#include "scriptAPI/scriptSystem.h"

kexScriptManager scriptManager;

//
// FCmd_Call
//

static void FCmd_Call(void) {
    if(command.GetArgc() < 2) {
        common.Printf("Usage: call <\"name\">\n");
        return;
    }
    scriptManager.CallExternalScript(
        command.GetArgv(1),
        "void main(void)");
}

//
// FCmd_MemUsage
//

static void FCmd_MemUsage(void) {
    common.CPrintf(RGBA(0, 255, 255, 255), "Script Memory Usage:\n");
    common.CPrintf(COLOR_YELLOW, "%ikb\n", Z_TagUsage(PU_SCRIPT) >> 10);
}

//
// kexScriptManager::kexScriptManager
//

kexScriptManager::kexScriptManager(void) {
    this->engine    = NULL;
    this->ctx       = NULL;
    this->module    = NULL;
}

//
// kexScriptManager::~kexScriptManager
//

kexScriptManager::~kexScriptManager(void) {
}

//
// kexScriptManager::MemAlloc
//

void *kexScriptManager::MemAlloc(size_t size) {
    return Z_Calloc(size, PU_SCRIPT, 0);
}

//
// kexScriptManager::MemFree
//

void kexScriptManager::MemFree(void *ptr) {
    Z_Free(ptr);
}

//
// kexScriptManager::MessageCallback
//

void kexScriptManager::MessageCallback(const asSMessageInfo *msg, void *param) {
    const char *type;
    rcolor color = COLOR_WHITE;

    switch(msg->type) {
    case asMSGTYPE_WARNING:
        type = "WARN";
        color = COLOR_YELLOW;
        break;
    case asMSGTYPE_INFORMATION:
        type = "INFO";
        break;
    default:
        type = "ERR ";
        color = COLOR_RED;
        break;
    }

    common.CPrintf(color, "%s (%d, %d) : %s : %s\n",
        msg->section,
        msg->row,
        msg->col,
        type,
        msg->message);
}

//
// kexScriptManager::Init
//

void kexScriptManager::Init(void) {
    if(asSetGlobalMemoryFunctions(kexScriptManager::MemAlloc, kexScriptManager::MemFree) == -1) {
            common.Error("kexScriptManager::Init: Unable to register memory functions\n");
            return;
    }

    if(!(engine = asCreateScriptEngine(ANGELSCRIPT_VERSION))) {
        common.Error("kexScriptManager::Init: Unable to register script engine\n");
        return;
    }

    engine->SetEngineProperty(asEP_COMPILER_WARNINGS, 2);
    engine->SetMessageCallback(asFUNCTION(kexScriptManager::MessageCallback), 0, asCALL_CDECL);

    ctx = engine->CreateContext();

    kexComponent::Init();

    RegisterBasicTypes();
    RegisterObjects();

    // TODO - TEMP
    unsigned int size;
    char *data = NULL;

    if((size = fileSystem.ReadExternalTextFile("scripts/main.txt", (byte**)&data)) == -1) {
        common.Error("kexScriptManager::Init: could not load scripts/main.txt");
        return;
    }

    module = engine->GetModule("main", asGM_CREATE_IF_NOT_EXISTS);
    module->AddScriptSection("Section", &data[0], size);
    module->Build();

    asIScriptFunction *func = module->GetFunctionByDecl("void main(void)");

    if(func != 0) {
        ctx->Prepare(func);
        if(ctx->Execute() == asEXECUTION_EXCEPTION) {
            common.Error("%s", ctx->GetExceptionString());
        }
    }

    Z_Free(data);

    command.Add("call", FCmd_Call);
    command.Add("scriptMem", FCmd_MemUsage);
    common.Printf("Script System Initialized\n");
}

//
// kexScriptManager::Shutdown
//

void kexScriptManager::Shutdown(void) {
    ctx->Release();
    engine->Release();

    Z_FreeTags(PU_SCRIPT, PU_SCRIPT);
}

//
// kexScriptManager::CallExternalScript
//

void kexScriptManager::CallExternalScript(const char *file, const char *function) {
    unsigned int size;
    char *data = NULL;

    if((size = fileSystem.ReadExternalTextFile(file, (byte**)&data)) == -1) {
        common.Warning("No file named %s\n", file);
        return;
    }

    asIScriptModule *mod;

    mod = engine->GetModule(kexStr(file).StripExtension().StripPath(),
        asGM_CREATE_IF_NOT_EXISTS);
    mod->AddScriptSection("externalSection", &data[0], size);
    mod->Build();

    Z_Free(data);

    asIScriptFunction *func = mod->GetFunctionByDecl(function);

    if(func != 0) {
        ctx->Prepare(func);
        if(ctx->Execute() == asEXECUTION_FINISHED) {
            mod->Discard();
            return;
        }

        common.Warning("Execution of %s did not finish\n", function);
        return;
    }
    common.Warning("No function declared as %s\n", function);
}

//
// kexScriptManager::RegisterObjects
//

void kexScriptManager::RegisterObjects(void) {
    kexScriptObjHandle::Init();
    kexCommon::InitObject();
    kexClient::InitObject();
    kexInputKey::InitObject();
    kexWorldActor::InitObject();
    kexLocalPlayer::InitObject();
    kexWorld::InitObject();
}

//
// kexScriptManager::RegisterBasicTypes
//

void kexScriptManager::RegisterBasicTypes(void) {
    kexScriptObjString::Init();
    kexScriptObjVector::Init();
    kexScriptObjAngle::Init();
}
