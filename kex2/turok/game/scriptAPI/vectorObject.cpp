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
// DESCRIPTION: Vector Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "scriptAPI/scriptSystem.h"

//
// kexScriptObjVector::Init
//

void kexScriptObjVector::Init(void) {

    scriptManager.Engine()->RegisterObjectType(
        "kVec3",
        sizeof(kexVec3),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kVec3",
        asBEHAVE_CONSTRUCT,
        "void f()",
        asFUNCTION(kexVec3::ObjectConstruct1),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kVec3",
        asBEHAVE_CONSTRUCT,
        "void f(float, float, float)",
        asFUNCTION(kexVec3::ObjectConstruct2),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kVec3",
        asBEHAVE_CONSTRUCT,
        "void f(const kVec3 &in)",
        asFUNCTION(kexVec3::ObjectConstructCopy),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 &Normalize(void)",
        asMETHODPR(kexVec3, Normalize, (void), kexVec3&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kStr ToString(void)",
        asMETHODPR(kexVec3, ToString, (void)const, kexStr),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 opAdd(const kVec3 &in)",
        asMETHODPR(kexVec3, operator+, (const kexVec3&), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 opNeg(void)",
        asMETHODPR(kexVec3, operator-, (void)const, kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 opSub(const kVec3 &in)",
        asMETHODPR(kexVec3, operator-, (const kexVec3&)const, kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 opMul(const kVec3 &in)",
        asMETHODPR(kexVec3, operator*, (const kexVec3&), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 opMul(const float val)",
        asMETHODPR(kexVec3, operator*, (const float), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 opDiv(const kVec3 &in)",
        asMETHODPR(kexVec3, operator/, (const kexVec3&), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 opDiv(const float val)",
        asMETHODPR(kexVec3, operator/, (const float), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kVec3",
        "kVec3 &opAssign(const kVec3 &in)",
        asMETHODPR(kexVec3, operator=, (const kexVec3&), kexVec3&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty("kVec3", "float x", asOFFSET(kexVec3, x));
    scriptManager.Engine()->RegisterObjectProperty("kVec3", "float y", asOFFSET(kexVec3, y));
    scriptManager.Engine()->RegisterObjectProperty("kVec3", "float z", asOFFSET(kexVec3, z));

    //
    // might as well do quaternions here...
    //
    scriptManager.Engine()->RegisterObjectType(
        "kQuat",
        sizeof(kexQuat),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kQuat",
        asBEHAVE_CONSTRUCT,
        "void f()",
        asFUNCTION(kexQuat::ObjectConstruct1),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kQuat",
        asBEHAVE_CONSTRUCT,
        "void f(float, float, float, float)",
        asFUNCTION(kexQuat::ObjectConstruct2),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kQuat",
        asBEHAVE_CONSTRUCT,
        "void f(float, kVec3 &in)",
        asFUNCTION(kexQuat::ObjectConstruct3),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kQuat",
        asBEHAVE_CONSTRUCT,
        "void f(const kQuat &in)",
        asFUNCTION(kexQuat::ObjectConstructCopy),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectMethod(
        "kQuat",
        "kQuat &Normalize(void)",
        asMETHODPR(kexQuat, Normalize, (void), kexQuat&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kQuat",
        "kQuat opAdd(const kQuat &in)",
        asMETHODPR(kexQuat, operator+, (const kexQuat &in), kexQuat),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kQuat",
        "kQuat opSub(const kQuat &in)",
        asMETHODPR(kexQuat, operator-, (const kexQuat &in), kexQuat),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kQuat",
        "kQuat opMul(const kQuat &in)",
        asMETHODPR(kexQuat, operator*, (const kexQuat &in), kexQuat),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kQuat",
        "kQuat &opAssign(const kQuat &in)",
        asMETHODPR(kexQuat, operator=, (const kexQuat&), kexQuat&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float x", asOFFSET(kexQuat, x));
    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float y", asOFFSET(kexQuat, y));
    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float z", asOFFSET(kexQuat, z));
    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float w", asOFFSET(kexQuat, w));
}
