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
// DESCRIPTION: Pickup objects
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "pickup.h"

DECLARE_CLASS(kexPickup, kexActor)

//
// kexPickup::kexPickup
//

kexPickup::kexPickup(void) {
    this->pickupSound   = "";
    this->bRespawn      = false;
    this->respawnTime   = 4;
}

//
// kexPickup::~kexPickup
//

kexPickup::~kexPickup(void) {
}

//
// kexPickup::OnTouch
//

void kexPickup::OnTouch(kexWorldObject *instigator) {
    if(IsStale()) {
        return;
    }
    if(instigator->bCanPickup == false) {
        return;
    }
    if(scriptComponent.onTouch) {
        bool ok = false;
        int state;

        state = scriptComponent.PrepareFunction(scriptComponent.onTouch);
        if(state == -1) {
            return;
        }

        scriptComponent.SetCallArgument(0, static_cast<kexActor*>(instigator));

        if(!scriptComponent.ExecuteFunction(state)) {
            return;
        }

        scriptComponent.FinishFunction(state, &ok);

        if(ok == false) {
            return;
        }
    }

    if(instigator && pickupSound.Length() > 0) {
        instigator->StartSound(pickupSound.c_str());
    }

    if(!bRespawn) {
        Remove();
    }
}

//
// kexPickup::Spawn
//

void kexPickup::Spawn(void) {
    bStatic = false;
    bTouch = true;
    bCanPickup = false;
    bNoFixedTransform = true;

    args.GetString("pickupSound", pickupSound);
    args.GetBool("bRespawn", bRespawn);
    args.GetFloat("respawnTime", respawnTime);
}
