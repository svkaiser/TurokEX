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
#include "fileSystem.h"
#include "zone.h"
#include "scriptAPI/scriptSystem.h"

kexScriptManager scriptManager;

#define BEGIN_OBJECT_DATA(c, sc, f)    \
    engine->RegisterObjectType(#sc, sizeof(c), asOBJ_VALUE | f);

#define OBJECT_FACTORY(c, sc, f)    \
    engine->RegisterStringFactory(#sc, asFUNCTION(f), asCALL_CDECL)

#define OBJECT_CONSTRUCTOR(c, sc, f, fp)    \
    engine->RegisterObjectBehaviour(#sc, asBEHAVE_CONSTRUCT, #f, asFUNCTION(fp), asCALL_CDECL_OBJLAST)

#define OBJECT_DECONSTRUCTOR(c, sc, f, fp)    \
    engine->RegisterObjectBehaviour(#sc, asBEHAVE_DESTRUCT, #f, asFUNCTION(fp), asCALL_CDECL_OBJLAST)

#define OBJECT_METHOD(c, sc, f, fp, a, ra) \
    engine->RegisterObjectMethod(#sc, #f, asMETHODPR(c, fp, a, ra), asCALL_THISCALL)

#define OBJECT_FUNCTION(c, sc, f, fp) \
    engine->RegisterObjectMethod(#sc, #f, asFUNCTION(fp), asCALL_CDECL_OBJLAST)

#define OBJECT_PROPERTY(c, sc, p, pp)   \
    engine->RegisterObjectProperty(#sc, #p, asOFFSET(c, pp))

#define END_OBJECT_DATA()

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

    engine->SetEngineProperty(asEP_DISALLOW_GLOBAL_VARS, 1);
    engine->SetEngineProperty(asEP_COMPILER_WARNINGS, 2);
    engine->SetMessageCallback(asFUNCTION(kexScriptManager::MessageCallback), 0, asCALL_CDECL);

    ctx = engine->CreateContext();

    kexComponent::Init();

    RegisterBasicTypes();
    RegisterObjects();

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
    mod->AddScriptSection("section", &data[0], size);
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
    kexScriptObjSystem::Init();
    kexScriptObjClient::Init();
    kexScriptObjInput::Init();
}

//
// kexScriptManager::RegisterBasicTypes
//

void kexScriptManager::RegisterBasicTypes(void) {

    // initialize string object
    BEGIN_OBJECT_DATA(kexStr, kStr, asOBJ_APP_CLASS_CDAK)
    OBJECT_FACTORY(kexStr, kStr, kexStr::ObjectFactory);
    OBJECT_CONSTRUCTOR(kexStr, kStr, void f(), kexStr::ObjectConstruct);
    OBJECT_CONSTRUCTOR(kexStr, kStr, void f(const kStr &in), kexStr::ObjectConstructCopy);
    OBJECT_DECONSTRUCTOR(kexStr, kStr, void f(), kexStr::ObjectDeconstruct);
    OBJECT_METHOD(kexStr, kStr, int IndexOf(const kStr &in) const, IndexOf, (const kexStr&)const, int);
    OBJECT_METHOD(kexStr, kStr, int Hash(void), Hash, (void), int);
    OBJECT_METHOD(kexStr, kStr, kStr &ToUpper(void), ToUpper, (void), kexStr&);
    OBJECT_METHOD(kexStr, kStr, kStr &ToLower(void), ToLower, (void), kexStr&);
    OBJECT_METHOD(kexStr, kStr, kStr &opAssign(const kStr &in), operator=, (const kexStr&), kexStr&);
    OBJECT_METHOD(kexStr, kStr, kStr opAdd(const kStr &in), operator+, (const kexStr&), kexStr);
    OBJECT_METHOD(kexStr, kStr, kStr opAdd(bool), operator+, (bool), kexStr);
    OBJECT_METHOD(kexStr, kStr, kStr opAdd(int), operator+, (int), kexStr);
    OBJECT_METHOD(kexStr, kStr, kStr opAdd(float), operator+, (float), kexStr);
    OBJECT_METHOD(kexStr, kStr, kStr &opAddAssign(const kStr &in), operator+=, (const kexStr&), kexStr&);
    OBJECT_METHOD(kexStr, kStr, kStr &opAddAssign(bool), operator+=, (bool), kexStr&);
    END_OBJECT_DATA()

    // initialize vector3 object
    BEGIN_OBJECT_DATA(kexVec3, kVec3, asOBJ_POD | asOBJ_APP_CLASS_CAK)
    OBJECT_CONSTRUCTOR(kexVec3, kVec3, void f(), kexVec3::ObjectConstruct1);
    OBJECT_CONSTRUCTOR(kexVec3, kVec3, void f(float, float, float), kexVec3::ObjectConstruct2);
    OBJECT_CONSTRUCTOR(kexVec3, kVec3, void f(const kVec3 &in), kexVec3::ObjectConstructCopy);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 &Normalize(void), Normalize, (void), kexVec3&);
    OBJECT_METHOD(kexVec3, kVec3, kStr ToString(void), ToString, (void)const, kexStr);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 opAdd(const kVec3 &in), operator+, (const kexVec3&), kexVec3);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 opNeg(void), operator-, (void)const, kexVec3);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 opSub(const kVec3 &in), operator-, (const kexVec3&)const, kexVec3);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 opMul(const kVec3 &in), operator*, (const kexVec3&), kexVec3);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 opMul(const float val), operator*, (const float), kexVec3);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 opDiv(const kVec3 &in), operator/, (const kexVec3&), kexVec3);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 opDiv(const float val), operator/, (const float), kexVec3);
    OBJECT_METHOD(kexVec3, kVec3, kVec3 &opAssign(const kVec3 &in), operator=, (const kexVec3&), kexVec3&);
    OBJECT_PROPERTY(kexVec3, kVec3, float x, x);
    OBJECT_PROPERTY(kexVec3, kVec3, float y, y);
    OBJECT_PROPERTY(kexVec3, kVec3, float z, z);
    END_OBJECT_DATA()

    // initialize quaternion object
    BEGIN_OBJECT_DATA(kexQuat, kQuat, asOBJ_POD | asOBJ_APP_CLASS_CAK)
    OBJECT_CONSTRUCTOR(kexQuat, kQuat, void f(), kexQuat::ObjectConstruct1);
    OBJECT_CONSTRUCTOR(kexQuat, kQuat, void f(float, float, float, float), kexQuat::ObjectConstruct2);
    OBJECT_CONSTRUCTOR(kexQuat, kQuat, void f(float, kVec3 &in), kexQuat::ObjectConstruct3);
    OBJECT_CONSTRUCTOR(kexQuat, kQuat, void f(const kQuat &in), kexQuat::ObjectConstructCopy);
    OBJECT_METHOD(kexQuat, kQuat, kQuat &Normalize(void), Normalize, (void), kexQuat&);
    OBJECT_METHOD(kexQuat, kQuat, kQuat opAdd(const kQuat &in), operator+, (const kexQuat &in), kexQuat);
    OBJECT_METHOD(kexQuat, kQuat, kQuat opSub(const kQuat &in), operator-, (const kexQuat &in), kexQuat);
    OBJECT_METHOD(kexQuat, kQuat, kQuat opMul(const kQuat &in), operator*, (const kexQuat &in), kexQuat);
    OBJECT_METHOD(kexQuat, kQuat, kQuat &opAssign(const kQuat &in), operator=, (const kexQuat&), kexQuat&);
    OBJECT_PROPERTY(kexQuat, kQuat, float x, x);
    OBJECT_PROPERTY(kexQuat, kQuat, float y, y);
    OBJECT_PROPERTY(kexQuat, kQuat, float z, z);
    OBJECT_PROPERTY(kexQuat, kQuat, float w, w);
    END_OBJECT_DATA()
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
