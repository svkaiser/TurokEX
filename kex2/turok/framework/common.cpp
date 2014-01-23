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
// DESCRIPTION: Common Stuff
//
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include "system.h"
#include "filesystem.h"
#include "console.h"
#include "scriptAPI/scriptSystem.h"

int     myargc;
char**  myargv;

kexCommon common;

//
// kexCommon::Printf
//

void kexCommon::Printf(const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);

    console.Print(COLOR_WHITE, buffer);
    sysMain.Log(buffer);
}

//
// kexCommon::Printf
//

void kexCommon::Printf(const kexStr &str) {
    Printf(str.c_str());
}

//
// kexCommon::CPrintf
//

void kexCommon::CPrintf(rcolor color, const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);

    console.Print(color, buffer);
    sysMain.Log(buffer);
}

//
// kexCommon::Warning
//

void kexCommon::Warning(const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);

    console.Print(COLOR_YELLOW, buffer);
    sysMain.Log(buffer);
}

//
// kexCommon::Warning
//

void kexCommon::Warning(const kexStr &str) {
    Warning(str.c_str());
}

//
// kexCommon::DPrintf
//

void kexCommon::DPrintf(const char *string, ...) {
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
// kexCommon::Error
//

void kexCommon::Error(const char* string, ...) {
    va_list	va;

    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);
    
    fprintf(stderr, "Error - %s\n", buffer);
    fflush(stderr);

    sysMain.Log(buffer);

#ifdef _WIN32
    Sys_Error(buffer);
#endif

    exit(0);    // just in case...
}

//
// kexCommon::CheckParam
//

int kexCommon::CheckParam(const char *check) {
    for(int i = 1; i < myargc; i++) {
        if(!stricmp(check, myargv[i]))
            return i;
    }
    return 0;
}

//
// kexCommon::Shutdown
//

void kexCommon::Shutdown(void) {
    common.Printf("Purging all generic allocations\n");
    Mem_Purge(hb_static);
}

//
// kexCommon::ReadConfigFile
//

void kexCommon::ReadConfigFile(const char *file) {
    char *buffer;
    int len;

    len = fileSystem.ReadExternalTextFile(file, (byte**)(&buffer));

    if(len == -1) {
        Warning("Warning: %s not found\n", file);
        return;
    }

    command.Execute(buffer);
    Mem_Free(buffer);
}

//
// kexCommon::WriteConfigFile
//
extern kexCvar cvarBasePath;

void kexCommon::WriteConfigFile(void) {
    FILE * f = fopen(kva("%s\\config.cfg", cvarBasePath.GetValue()), "w");
    if(f) {
        inputKey.WriteBindings(f);
        cvarManager.WriteToFile(f);
        fclose(f);
    }
}

//
// kexCommon::HashFileName
//

unsigned int kexCommon::HashFileName(const char *name) {
    unsigned int hash   = 0;
    char *str           = (char*)name;
    char c;

    while((c = *str++)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash & (MAX_HASH-1);
}

//
// kexCommon::AddCvar
//

void kexCommon::AddCvar(const kexStr &name, const kexStr &value, const kexStr &desc, const int flags) {
    new kexCvar(
        Mem_Strdup(name.c_str(), hb_static),
        flags|CVF_ALLOCATED,
        Mem_Strdup(value.c_str(), hb_static),
        Mem_Strdup(desc.c_str(), hb_static));
}

//
// kexCommon::GetCvarFloat
//

float kexCommon::GetCvarFloat(const kexStr &name) {
    kexCvar *cvar = cvarManager.Get(name.c_str());

    if(cvar == NULL) {
        return 0;
    }

    return cvar->GetFloat();
}

//
// kexCommon::GetCvarBool
//

bool kexCommon::GetCvarBool(const kexStr &name) {
    kexCvar *cvar = cvarManager.Get(name.c_str());

    if(cvar == NULL) {
        return 0;
    }

    return cvar->GetBool();
}

//
// kexCommon::InitObject
//

void kexCommon::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kCommon",
        sizeof(kexCommon),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);

    scriptManager.Engine()->RegisterObjectMethod(
        "kCommon",
        "void Print(const kStr &in)",
        asMETHODPR(kexCommon, Printf, (const kexStr &str), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kCommon",
        "void Warning(const kStr &in)",
        asMETHODPR(kexCommon, Warning, (const kexStr &str), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kCommon",
        "void AddCvar(const kStr &in, const kStr &in, const kStr &in, const int)",
        asMETHODPR(kexCommon, AddCvar,
        (const kexStr &name, const kexStr &value, const kexStr &desc, const int flags), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kCommon",
        "float GetCvarFloat(const kStr &in)",
        asMETHODPR(kexCommon, GetCvarFloat, (const kexStr &name), float),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kCommon",
        "bool GetCvarBool(const kStr &in)",
        asMETHODPR(kexCommon, GetCvarBool, (const kexStr &name), bool),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterGlobalProperty("kCommon Com", &common);

    scriptManager.Engine()->RegisterEnum("CvarFlags");
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_BOOL", CVF_BOOL);
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_INT", CVF_INT);
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_FLOAT", CVF_FLOAT);
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_STRING", CVF_STRING);
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_NETWORK", CVF_NETWORK);
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_CHEAT", CVF_CHEAT);
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_CONFIG", CVF_CONFIG);
    scriptManager.Engine()->RegisterEnumValue("CvarFlags", "CVF_ALLOCATED", CVF_ALLOCATED);
}

//
// fcmp
//

kbool fcmp(float f1, float f2) {
    float precision = 0.00001f;
    if(((f1 - precision) < f2) && 
        ((f1 + precision) > f2)) {
        return true;
    }
    else {
        return false;
    }
}

//
// kva
//

// TODO - this seems really dodgy as fuck...
char *kva(char *str, ...) {
    va_list v;
    static char vastr[1024];
	
    va_start(v, str);
    vsprintf(vastr, str,v);
    va_end(v);
    
    return vastr;	
}
