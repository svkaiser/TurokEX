// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
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
// DESCRIPTION: Network Player code
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "server.h"
#include "player/player.h"

//-----------------------------------------------------------------------------
//
// kexNetPlayer
//
//-----------------------------------------------------------------------------

DECLARE_CLASS(kexNetPlayer, kexPlayer)

//
// kexNetPlayer::kexNetPlayer
//

kexNetPlayer::kexNetPlayer(void) {
}

//
// kexNetPlayer::~kexNetPlayer
//

kexNetPlayer::~kexNetPlayer(void) {
}

//
// kexNetPlayer::kexNetPlayer
//

void kexNetPlayer::Tick(void) {
    if(GetState() == SVC_STATE_INGAME) {
        if(puppet != NULL) {
            kexSector *sector = puppet->Physics()->sector;
            if(sector != NULL && sector->area != NULL) {
                kexAreaComponent *ac = &sector->area->scriptComponent;
                ac->CallFunction(ac->onThink);
            }
        }
    }
}
