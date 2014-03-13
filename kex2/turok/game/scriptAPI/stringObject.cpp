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
// DESCRIPTION: String Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kstring.h"
#include "scriptAPI/scriptSystem.h"

//
// kexScriptObjString::Init
//

void kexScriptObjString::Init(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kStr",
        sizeof(kexStr),
        asOBJ_VALUE | asOBJ_APP_CLASS_CA);

    scriptManager.Engine()->RegisterStringFactory(
        "kStr",
        asFUNCTION(kexStr::ObjectFactory),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kStr",
        asBEHAVE_CONSTRUCT,
        "void f()",
        asFUNCTION(kexStr::ObjectConstruct),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kStr",
        asBEHAVE_CONSTRUCT,
        "void f(const kStr &in)",
        asFUNCTION(kexStr::ObjectConstructCopy),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kStr",
        asBEHAVE_DESTRUCT,
        "void f()",
        asFUNCTION(kexStr::ObjectDeconstruct),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "int IndexOf(const kStr &in) const",
        asMETHODPR(kexStr, IndexOf, (const kexStr&)const, int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "int Hash(void)",
        asMETHODPR(kexStr, Hash, (void), int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "int Atoi(void)",
        asMETHODPR(kexStr, Atoi, (void), int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr &ToUpper(void)",
        asMETHODPR(kexStr, ToUpper, (void), kexStr&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr &ToLower(void)",
        asMETHODPR(kexStr, ToLower, (void), kexStr&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr &opAssign(const kStr &in)",
        asMETHODPR(kexStr, operator=, (const kexStr&), kexStr&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr opAdd(const kStr &in)",
        asMETHODPR(kexStr, operator+, (const kexStr&), kexStr),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "int8 opIndex(const int)",
        asMETHODPR(kexStr, operator[], (const int) const, const char),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr opAdd(bool)",
        asMETHODPR(kexStr, operator+, (bool), kexStr),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr opAdd(int)",
        asMETHODPR(kexStr, operator+, (int), kexStr),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr opAdd(float)",
        asMETHODPR(kexStr, operator+, (float), kexStr),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr &opAddAssign(const kStr &in)",
        asMETHODPR(kexStr, operator+=, (const kexStr&), kexStr&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kStr",
        "kStr &opAddAssign(bool)",
        asMETHODPR(kexStr, operator+=, (bool), kexStr&),
        asCALL_THISCALL);
}
