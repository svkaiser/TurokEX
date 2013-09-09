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
// DESCRIPTION: Client Namespace Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "scriptAPI/scriptSystem.h"

//
// kexScriptObjClient::Init
//

void kexScriptObjClient::Init(void) {
    scriptManager.Engine()->SetDefaultNamespace("Client");

    scriptManager.Engine()->RegisterGlobalFunction(
        "bool IsLocal(void)",
        asFUNCTION(IsLocal),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterGlobalFunction(
        "int GetState(void)",
        asFUNCTION(GetState),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterEnum("EnumClientState");
    scriptManager.Engine()->RegisterEnumValue(
        "EnumClientState",
        "STATE_UNITIALIZED",
        CL_STATE_UNINITIALIZED);

    scriptManager.Engine()->RegisterEnumValue(
        "EnumClientState",
        "STATE_CONNECTING",
        CL_STATE_CONNECTING);

    scriptManager.Engine()->RegisterEnumValue(
        "EnumClientState",
        "STATE_CONNECTED",
        CL_STATE_CONNECTED);

    scriptManager.Engine()->RegisterEnumValue(
        "EnumClientState",
        "STATE_DISCONNECTED",
        CL_STATE_DISCONNECTED);

    scriptManager.Engine()->RegisterEnumValue(
        "EnumClientState",
        "STATE_READY",
        CL_STATE_READY);

    scriptManager.Engine()->RegisterEnumValue(
        "EnumClientState",
        "STATE_INGAME",
        CL_STATE_INGAME);

    scriptManager.Engine()->RegisterEnumValue(
        "EnumClientState",
        "STATE_CHANGINGLEVEL",
        CL_STATE_CHANGINGLEVEL);

    scriptManager.Engine()->SetDefaultNamespace("");
}

//
// kexScriptObjClient::IsLocal
//

bool kexScriptObjClient::IsLocal(void) {
    return client.IsLocal();
}

//
// kexScriptObjClient::GetState
//

int kexScriptObjClient::GetState(void) {
    return client.GetState();
}
