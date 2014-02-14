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
// DESCRIPTION: Game Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "gameObject.h"
#include "sound.h"
#include "server.h"
#include "binFile.h"

DECLARE_ABSTRACT_CLASS(kexGameObject, kexObject)

unsigned int kexGameObject::id = 0;

//
// kexGameObject::kexGameObject
//

kexGameObject::kexGameObject(void) {
    this->refCount      = 0;
    this->bClientOnly   = false;
    this->owner         = NULL;
    this->target        = NULL;
    this->bStale        = false;
    this->targetID      = 0;
    this->fracTime      = 0;
    this->timeStamp     = 0;
    this->oldTimeStamp  = 0;
    this->objID         = 0;
}

//
// kexGameObject::~kexGameObject
//

kexGameObject::~kexGameObject(void) {
    SetTarget(NULL);
    SetOwner(NULL);
}

//
// kexGameObject::AddRef
//

int kexGameObject::AddRef(void) {
    return ++refCount;
}

//
// kexGameObject::RemoveRef
//

int kexGameObject::RemoveRef(void) {
    return --refCount;
}

//
// kexGameObject::Remove
//

void kexGameObject::Remove(void) {
    bStale = true;
}

//
// kexGameObject::Removing
//

const bool kexGameObject::Removing(void) const {
    return (bStale && RefCount() <= 0);
}

//
// kexGameObject::SetTarget
//

void kexGameObject::SetTarget(kexGameObject *targ) {
    // If there was a target already, decrease its refcount
    if(target) {
        target->RemoveRef();
    }

    // Set new target and if non-NULL, increase its counter
    if((target = targ)) {
        target->AddRef();
    }
}

//
// kexGameObject::SetOwner
//

void kexGameObject::SetOwner(kexGameObject *targ) {
    // If there was a owner already, decrease its refcount
    if(owner) {
        owner->RemoveRef();
    }

    // Set new owner and if non-NULL, increase its counter
    if((owner = targ)) {
        owner->AddRef();
    }
}

//
// kexGameObject::Spawn
//

void kexGameObject::Spawn(void) {
    timeStamp = (float)server.GetRunTime();
    objID = ++kexGameObject::id;
}

//
// kexGameObject::Save
//

void kexGameObject::Save(kexBinFile *saveFile) {
    saveFile->WriteString(ClassString());
    saveFile->Write32(objID);
    saveFile->WriteVector3(origin);
    saveFile->WriteFloat(angles.yaw);
    saveFile->WriteFloat(angles.pitch);
    saveFile->WriteFloat(angles.roll);
    saveFile->Write8(bClientOnly);
    saveFile->Write32(targetID);
    saveFile->WriteFloat(timeStamp);
    saveFile->WriteFloat(oldTimeStamp);
    saveFile->WriteFloat(tickDistance);
    saveFile->WriteFloat(fracTime);
    saveFile->Write8(bStale);
    
    saveFile->Write8(owner ? 1 : 0);
    if(owner) {
        saveFile->Write32(owner->objID);
    }
    
    saveFile->Write8(target ? 1 : 0);
    if(target) {
        saveFile->Write32(target->objID);
    }
}

//
// kexGameObject::Load
//

void kexGameObject::Load(kexBinFile *loadFile) {
    loadFile->ReadString();

    objID           = loadFile->Read32();
    origin          = loadFile->ReadVector3();
    angles.yaw      = loadFile->ReadFloat();
    angles.pitch    = loadFile->ReadFloat();
    angles.roll     = loadFile->ReadFloat();
    bClientOnly     = (loadFile->Read8() != 0);
    targetID        = loadFile->Read32();
    timeStamp       = loadFile->ReadFloat();
    oldTimeStamp    = loadFile->ReadFloat();
    tickDistance    = loadFile->ReadFloat();
    fracTime        = loadFile->ReadFloat();
    bStale          = (loadFile->Read8() != 0);
    
    if(loadFile->Read8()) {
        // TODO
        loadFile->Read32();
    }
    
    if(loadFile->Read8()) {
        // TODO
        loadFile->Read32();
    }
}

//
// kexGameObject::EmitSound
//

void kexGameObject::EmitSound(const char *name) {
    soundSystem.StartSound(name, this);
}

//
// kexGameObject::StartSound
//

void kexGameObject::StartSound(const char *name) {
    EmitSound(name);
}

//
// kexGameObject::StartSound
//

void kexGameObject::StartSound(const kexStr &name) {
    StartSound(name.c_str());
}
