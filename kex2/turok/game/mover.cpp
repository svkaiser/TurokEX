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
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mover.h"
#include "world.h"
#include "client.h"

DECLARE_CLASS(kexMover, kexActor)

//
// kexMover::kexMover
//

kexMover::kexMover(void) {
    this->moveSpeed         = 8;
    this->moveAmount        = 8;
    this->diffHeight        = 0;
    this->destHeight        = 0;
    this->time              = 0;
    this->bMove             = false;
    this->distance          = 0;
    this->lerpTime          = 0;
    this->moveSound         = "";
    this->bStatic           = false;
    this->wireframeColor    = RGBA(255, 0, 255, 255);
}

//
// kexMover::~kexMover
//

kexMover::~kexMover(void) {
}

//
// kexMover::Spawn
//

void kexMover::Spawn(void) {
    args.GetFloat("moveSpeed", moveSpeed);
    args.GetFloat("moveAmount", moveAmount);
    args.GetString("moveSound", moveSound);
}

//
// kexMover::OnTrigger
//

void kexMover::OnTrigger(void) {
    if(scriptComponent.onTrigger) {
        scriptComponent.CallFunction(scriptComponent.onTrigger);
    }

    if(physics.sector == NULL) {
        return;
    }

    StartSound(moveSound.c_str());

    bMove       = true;
    destHeight  = origin.y;
    diffHeight  = moveAmount + origin.y;
    time        = timeStamp;
    lerpTime    = 1.0f;
    distance    = physics.sector->lowerTri.GetDistance(origin);
    targetID    = 0;
}

//
// kexMover::Tick
//

void kexMover::Tick(void) {
    if(bMove == false) {
        return;
    }

    lerpTime = 1.0f - (((timeStamp - time) / 1000.0f) * (1.0f / moveSpeed));

    if(scriptComponent.onThink) {
        scriptComponent.CallFunction(scriptComponent.onThink);
    }
}

//
// kexMover::LocalTick
//

void kexMover::LocalTick(void) {
    if(bMove == true) {
        if(lerpTime >= 0) {
            origin.y = (kexMath::Cos((1.0f - lerpTime) * M_PI) - 1.0f) * 0.5f *
                (diffHeight - destHeight) + destHeight;

            lerpTime -= (client.GetRunTime() / moveSpeed);

            localWorld.CollisionMap().RecursiveChangeHeight(
                physics.sector,
                origin.y - destHeight + distance,
                physics.sector->area->WorldID());
        }
        else {
            bMove = false;
        }
    }

    UpdateTransform();
    animState.Update();
}
