// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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
// DESCRIPTION: Trigger objects
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "gameManager.h"
#include "trigger.h"
#include "world.h"

DECLARE_CLASS(kexTrigger, kexActor)

//
// kexTrigger::kexTrigger
//

kexTrigger::kexTrigger(void) {
    this->delayTime         = 1.0f;
    this->currentTime       = 0;
    this->activateID        = 0;
    this->maxTriggers       = -1;
    this->counter           = 0;
    this->currentCounter    = 0;
    this->bTriggered        = false;
    this->bActionTrigger    = false;
}

//
// kexTrigger::~kexTrigger
//

kexTrigger::~kexTrigger(void) {
}

//
// kexTrigger::Spawn
//

void kexTrigger::Spawn(void) {
    args.GetFloat("delayTime", delayTime, 1.0f);
    args.GetInt("target", activateID);
    args.GetInt("maxTriggers", maxTriggers, -1);
    args.GetInt("counter", counter);
    args.GetBool("bActionTrigger", bActionTrigger);
    args.GetString("actionName", actionName);
    args.GetInt("actionHeldTime", actionHeldTime);
    
    if(delayTime < 0) {
        delayTime = 0;
    }
    
    if(counter < 0) {
        counter = 0;
    }
}

//
// kexTrigger::OnTrigger
//

void kexTrigger::OnTrigger(void) {
    if(bTriggered || maxTriggers == 0) {
        return;
    }
    
    if(++currentCounter < counter) {
        return;
    }
    
    currentTime = timeStamp;
    bTriggered = true;
    
    localWorld.TriggerActor(activateID);
    
    if(scriptComponent.onTrigger) {
        scriptComponent.CallFunction(scriptComponent.onTrigger);
    }
}

//
// kexTrigger::Tick
//

void kexTrigger::Tick(void) {
    if(((timeStamp - currentTime) / 1000.0f) > delayTime) {
        bTriggered = false;
        currentCounter = 0;
        if(maxTriggers > 0) {
            maxTriggers--;
        }
    }
    
    if(scriptComponent.onThink) {
        scriptComponent.CallFunction(scriptComponent.onThink);
    }
    
    if(bActionTrigger && actionName.Length() > 0) {
        kexLocalPlayer *p = &gameManager.localPlayer;
        int heldTime = p->ActionHeldTime(actionName);
        
        if(p->ActionDown(actionName) && heldTime > actionHeldTime) {
            OnTrigger();
        }
    }
}
