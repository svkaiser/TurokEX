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
// DESCRIPTION: Attachments
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "displayObject.h"
#include "attachment.h"

//
// kexAttachment::AttachToActor
//

void kexAttachment::AttachToObject(kexDisplayObject *targ) {
    // If there was a attachment already, decrease its refcount
    if(obj) {
        obj->RemoveRef();
    }

    // Set new attachment and if non-NULL, increase its counter
    if((obj = targ)) {
        obj->AddRef();
    }
}

//
// kexAttachment::DettachObject
//

void kexAttachment::DettachObject(void) {
    AttachToObject(NULL);
}

//
// kexAttachment::Transform
//

void kexAttachment::Transform(void) {
    if(obj != NULL) {
        owner->SetOrigin((obj->GetOrigin() + sourceOffset) +
            ((attachOffset - sourceOffset) * obj->GetRotation()));

        if(bAttachRelativeAngles) {
            owner->SetAngles(obj->GetAngles());
        }
    }
}
