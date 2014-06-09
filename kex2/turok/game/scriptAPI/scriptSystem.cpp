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
#include "world.h"
#include "physics/physics.h"
#include "renderBackend.h"
#include "scriptAPI/scriptSystem.h"
#include "scriptAPI/component.h"
#include "gameManager.h"
#include "ai.h"
#include "renderUtils.h"

kexScriptManager scriptManager;

static kexHeapBlock hb_script("script", false, NULL, NULL);

//
// call
//

COMMAND(call) {
    if(command.GetArgc() < 2) {
        common.Printf("Usage: call <\"function name\">\n");
        return;
    }
    scriptManager.CallCommand(kva("void %s(void)", command.GetArgv(1)));
}

//
// callfile
//

COMMAND(callfile) {
    if(command.GetArgc() < 3) {
        common.Printf("Usage: callfile <\"file name\"> <\"function name\">\n");
        return;
    }
    scriptManager.CallExternalScript(command.GetArgv(1),
        kva("void %s(void)", command.GetArgv(2)));
}

//
// scriptmem
//

COMMAND(scriptmem) {
    common.CPrintf(RGBA(0, 255, 255, 255), "Script Memory Usage:\n");
    common.CPrintf(COLOR_YELLOW, "%ikb\n", kexHeap::Usage(hb_script) >> 10);
}

//
// statscripts
//

COMMAND(statscripts) {
    scriptManager.bDrawGCStats ^= 1;
}

//
// kexScriptManager::kexScriptManager
//

kexScriptManager::kexScriptManager(void) {
    this->engine        = NULL;
    this->ctx           = NULL;
    this->module        = NULL;
    this->bDrawGCStats  = false;
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
    return Mem_Malloc(size, hb_script);
}

//
// kexScriptManager::MemFree
//

void kexScriptManager::MemFree(void *ptr) {
    Mem_Free(ptr);
}

//
// kexScriptManager::MessageCallback
//

void kexScriptManager::MessageCallback(const asSMessageInfo *msg, void *param) {
    switch(msg->type) {
    case asMSGTYPE_INFORMATION:
        common.Printf("%s (%d, %d) : %s\n",
            msg->section,
            msg->row,
            msg->col,
            msg->message);
        break;
    default:
        common.Error("%s (%d, %d) : %s\n",
            msg->section,
            msg->row,
            msg->col,
            msg->message);
        break;
    }
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

    kexScriptObjString  ::Init();
    kexScriptObjHandle  ::Init();

    kexMath             ::InitObject();
    kexCommon           ::InitObject();
    kexKeyMap           ::InitObject();
    kexClient           ::InitObject();
    kexInputKey         ::InitObject();
    kexActor            ::InitObject();
    kexAI               ::InitObject();
    kexArea             ::InitObject();
    kexCamera           ::InitObject();
    kexLocalPlayer      ::InitObject();
    kexWorld            ::InitObject();
    kexPhysics          ::InitObject();
    kexAnimState        ::InitObject();
    kexCanvas           ::InitObject();
    kexGameManager      ::InitObject();

    kexActorComponent   ::Init();
    kexAreaComponent    ::Init();
    kexGameManager      ::Init();

    module = engine->GetModule("core", asGM_CREATE_IF_NOT_EXISTS);

    // TODO
    ProcessScript("scripts/main.txt");
    scriptBuffer += "\0";

    module->Build();

    asIScriptFunction *func = module->GetFunctionByDecl("void main(void)");

    if(func != 0) {
        ctx->Prepare(func);
        if(ctx->Execute() == asEXECUTION_EXCEPTION) {
            common.Error("%s", ctx->GetExceptionString());
        }
    }

    common.Printf("Script System Initialized\n");
}

//
// kexScriptManager::Shutdown
//

void kexScriptManager::Shutdown(void) {
    common.Printf("Shutting down scripting system\n");

    ctx->Release();
    engine->Release();

    Mem_Purge(hb_script);
}

//
// kexScriptManager::HasScriptFile
//

bool kexScriptManager::HasScriptFile(const char *file) {
    kexStr fileName(file);
    
    fileName.StripExtension().StripPath();
    
    for(unsigned int i = 0; i < scriptFiles.Length(); i++) {
        if(!strcmp(scriptFiles[i].c_str(), fileName.c_str())) {
            return true;
        }
    }
    
    return false;
}

//
// kexScriptManager::ProcessScript
//

void kexScriptManager::ProcessScript(const char *file) {
    kexLexer *lexer;
    kexStr scrBuffer;
    
    if(!(lexer = parser.Open(file))) {
        common.Error("kexScriptManager::Init: could not load %s", file);
        return;
    }
    
    while(lexer->CheckState()) {
        char ch = lexer->GetChar();
        
        if(ch == '#') {
            lexer->Find();
            if(lexer->Matches("include")) {
                lexer->GetString();
                char *nfile = lexer->StringToken();
                
                if(!HasScriptFile(nfile)) {
                    ProcessScript(nfile);
                    scriptFiles.Push(kexStr(nfile).StripExtension().StripPath());
                }
                continue;
            }
            else {
                parser.Error("kexScriptManager::ProcessScript: unknown token: %s\n",
                    lexer->Token());
            }
        }
        
        scriptBuffer += ch;
        scrBuffer += ch;
    }

    module->AddScriptSection(kexStr(file).StripExtension().StripPath(),
        scrBuffer.c_str(), scrBuffer.Length());
    
    parser.Close();
}

//
// kexScriptManager::RegisterMethod
//

void kexScriptManager::RegisterMethod(const char *name, const char *decl,
                                      const asSFuncPtr &funcPointer) {
    engine->RegisterObjectMethod(name, decl, funcPointer, asCALL_THISCALL);
}

//
// kexScriptManager::CallExternalScript
//

void kexScriptManager::CallExternalScript(const char *file, const char *function) {
    unsigned int size;
    char *data = NULL;

    if((size = fileSystem.OpenExternalFile(file, (byte**)&data)) == -1) {
        common.Warning("No file named %s\n", file);
        return;
    }

    asIScriptModule *mod;

    mod = engine->GetModule(kexStr(file).StripExtension().StripPath(),
        asGM_CREATE_IF_NOT_EXISTS);
    mod->AddScriptSection("externalSection", &data[0], size);
    mod->Build();

    Mem_Free(data);

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
// kexScriptManager::CallCommand
//

void kexScriptManager::CallCommand(const char *decl) {
    asIScriptFunction *func = module->GetFunctionByDecl(decl);

    if(func != 0) {
        int state = ctx->GetState();

        if(state == asEXECUTION_ACTIVE)
            ctx->PushState();

        ctx->Prepare(func);
        if(ctx->Execute() == asEXECUTION_EXCEPTION) {
            common.Error("%s", ctx->GetExceptionString());
        }

        if(state == asEXECUTION_ACTIVE)
            ctx->PopState();
    }
}

//
// kexScriptManager::DrawGCStats
//

void kexScriptManager::DrawGCStats(void) {
    unsigned int data[5];

    if(scriptManager.bDrawGCStats == false) {
        return;
    }

    scriptManager.Engine()->GetGCStatistics(
        &data[0],
        &data[1],
        &data[2],
        &data[3],
        &data[4]);
    
    kexRenderUtils::PrintStatsText("CurrentSize:", ": %i", data[0]);
    kexRenderUtils::PrintStatsText("Total Destroyed:", ": %i", data[1]);
    kexRenderUtils::PrintStatsText("Total Detected:", ": %i", data[2]);
    kexRenderUtils::PrintStatsText("New Objects:", ": %i", data[3]);
    kexRenderUtils::PrintStatsText("Total New Destroyed:", ": %i", data[4]);
    kexRenderUtils::AddDebugLineSpacing();
}
