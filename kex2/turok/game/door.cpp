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
// DESCRIPTION: Door objects
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "door.h"
#include "world.h"

DECLARE_CLASS(kexDoor, kexActor)

//
// kexDoor::kexDoor
//

kexDoor::kexDoor(void) {
    this->delayTime         = 0;
    this->currentTime       = 0;
    this->bTriggered        = false;
    this->idleAnim          = 0;
    this->openAnim          = 0;
    this->closeAnim         = 0;
    this->wireframeColor    = RGBA(255, 0, 255, 255);
}

//
// kexDoor::~kexDoor
//

kexDoor::~kexDoor(void) {
}

//
// kexDoor::Spawn
//

void kexDoor::Spawn(void) {
    args.GetFloat("delayTime", delayTime);
    args.GetInt("idleAnim", idleAnim);
    args.GetInt("openAnim", openAnim);
    args.GetInt("closeAnim", closeAnim);

    animState.Set(idleAnim, 4, 0);
}

//
// kexDoor::OnTrigger
//

void kexDoor::OnTrigger(void) {
    if(bTriggered) {
        return;
    }

    animState.Blend(openAnim, 4, 4, 0);

    currentTime = timeStamp;
    bTriggered = true;

    if(scriptComponent.onTrigger) {
        scriptComponent.CallFunction(scriptComponent.onTrigger);
    }
}

//
// kexDoor::Tick
//

void kexDoor::Tick(void) {
    if(animState.track.anim != NULL) {
        if(animState.track.anim->animID == openAnim) {
            if(delayTime > 0 && animState.flags & ANF_STOPPED) {
                if(((timeStamp - currentTime) / 1000.0f) > delayTime) {
                    animState.Blend(closeAnim, 4, 4, 0);
                }
            }
        }
        else {
            if(bTriggered) {
                bTriggered = false;
            }
            if(animState.flags & ANF_STOPPED) {
                animState.Set(idleAnim, 4, 0);
            }
        }
    }

    if(scriptComponent.onThink) {
        scriptComponent.CallFunction(scriptComponent.onThink);
    }
}
