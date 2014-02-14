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
// DESCRIPTION: Display Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "displayObject.h"
#include "sound.h"
#include "binFile.h"

DECLARE_ABSTRACT_CLASS(kexDisplayObject, kexGameObject)

//
// kexDisplayObject::kexDisplayObject
//

kexDisplayObject::kexDisplayObject(void) {
    this->bHidden       = false;
    this->bClientView   = false;
    
    this->attachment.SetOwner(this);
    this->scale.Set(1, 1, 1);
}

//
// kexDisplayObject::~kexDisplayObject
//

kexDisplayObject::~kexDisplayObject(void) {
}

//
// kexDisplayObject::Spawn
//

void kexDisplayObject::Spawn(void) {
    if(rotation.x == 0 && rotation.y == 0 && rotation.z == 0 && rotation.w == 0) {
        rotation = kexQuat(angles.yaw, 0, 1, 0);
    }

    rotation.Normalize();
}

//
// kexDisplayObject::Save
//

void kexDisplayObject::Save(kexBinFile *saveFile) {
    saveFile->Write8(bHidden);
    saveFile->Write8(bCulled);
    saveFile->Write8(bClientView);
    saveFile->WriteQuaternion(rotation);
    saveFile->WriteFloat(cullDistance);
    saveFile->WriteMatrix(matrix);
    saveFile->WriteVector3(scale);
}

//
// kexDisplayObject::Load
//

void kexDisplayObject::Load(kexBinFile *loadFile) {
}

//
// kexDisplayObject::EmitSound
//

void kexDisplayObject::EmitSound(const char *name) {
    if(bCulled == false) {
        soundSystem.StartSound(name, this);
    }
}

//
// kexDisplayObject::UpdateTransform
//

void kexDisplayObject::UpdateTransform(void) {
    attachment.Transform();

    matrix = kexMatrix(rotation);

    matrix.Scale(scale);
    rotMatrix = matrix;
    matrix.AddTranslation(origin);
}
