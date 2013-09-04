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
// DESCRIPTION: System Namespace Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "scriptAPI/scriptSystem.h"

//
// kexScriptObjSystem::Init
//

void kexScriptObjSystem::Init(void) {
    scriptManager.Engine()->SetDefaultNamespace("Sys");

    scriptManager.Engine()->RegisterGlobalFunction(
        "void Print(const kStr &in)",
        asFUNCTION(Printf),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterGlobalFunction(
        "void Log(const kStr &in)",
        asFUNCTION(DPrintf),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterGlobalFunction(
        "void CPrint(int, const kStr &in)",
        asFUNCTION(CPrintf),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterGlobalFunction(
        "void Warning(const kStr &in)",
        asFUNCTION(Warning),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterGlobalFunction(
        "void Error(const kStr &in)",
        asFUNCTION(Error),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterGlobalFunction(
        "const int GetMS(void)",
        asFUNCTION(GetMS),
        asCALL_CDECL);

    scriptManager.Engine()->SetDefaultNamespace("");
}

//
// kexScriptObjSystem::SPrintf
//

void kexScriptObjSystem::Printf(const kexStr &str) {
    common.Printf(str.c_str());
}

//
// kexScriptObjSystem::CPrintf
//

void kexScriptObjSystem::CPrintf(rcolor color, const kexStr &str) {
    common.CPrintf(color, str.c_str());
}

//
// kexScriptObjSystem::Warning
//

void kexScriptObjSystem::Warning(const kexStr &str) {
    common.Warning(str.c_str());
}

//
// kexScriptObjSystem::DPrintf
//

void kexScriptObjSystem::DPrintf(const kexStr &str) {
    common.DPrintf(str.c_str());
}

//
// kexScriptObjSystem::Error
//

void kexScriptObjSystem::Error(const kexStr &str) {
    common.Error(str.c_str());
}

//
// kexScriptObjSystem::GetMS
//

const int kexScriptObjSystem::GetMS(void) {
    return sysMain.GetMS();
}
