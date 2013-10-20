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
// DESCRIPTION: Angle Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "scriptAPI/scriptSystem.h"

//
// kexScriptObjAngle::Init
//

void kexScriptObjAngle::Init(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kAngle",
        sizeof(kexAngle),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kAngle",
        asBEHAVE_CONSTRUCT,
        "void f()",
        asFUNCTION(kexAngle::ObjectConstruct1),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kAngle",
        asBEHAVE_CONSTRUCT,
        "void f(float, float, float)",
        asFUNCTION(kexAngle::ObjectConstruct2),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kAngle",
        asBEHAVE_CONSTRUCT,
        "void f(const kVec3 &in)",
        asFUNCTION(kexAngle::ObjectConstruct3),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kAngle",
        asBEHAVE_CONSTRUCT,
        "void f(const kAngle &in)",
        asFUNCTION(kexAngle::ObjectConstructCopy),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectMethod(
        "kAngle",
        "kAngle &Clamp180(void)",
        asMETHODPR(kexAngle, Clamp180, (void), kexAngle&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kAngle",
        "kAngle &Clamp180Invert(void)",
        asMETHODPR(kexAngle, Clamp180Invert, (void), kexAngle&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kAngle",
        "kAngle &Clamp180InvertSum(const kAngle &in)",
        asMETHODPR(kexAngle, Clamp180InvertSum, (const kexAngle&), kexAngle&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kAngle",
        "kVec3 ToForwardAxis(void)",
        asMETHODPR(kexAngle, ToForwardAxis, (void), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kAngle",
        "kVec3 ToUpAxis(void)",
        asMETHODPR(kexAngle, ToUpAxis, (void), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kAngle",
        "kVec3 ToRightAxis(void)",
        asMETHODPR(kexAngle, ToRightAxis, (void), kexVec3),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty("kAngle", "float yaw", asOFFSET(kexAngle, yaw));
    scriptManager.Engine()->RegisterObjectProperty("kAngle", "float pitch", asOFFSET(kexAngle, pitch));
    scriptManager.Engine()->RegisterObjectProperty("kAngle", "float roll", asOFFSET(kexAngle, roll));
}
