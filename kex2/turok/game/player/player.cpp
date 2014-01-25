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
// DESCRIPTION: Player code
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "player/player.h"
#include "world.h"

//-----------------------------------------------------------------------------
//
// kexPlayerMove
//
//-----------------------------------------------------------------------------

//
// kexPlayerMove::Accelerate
//

void kexPlayerMove::Accelerate(int direction, int axis, const float deltaTime) {
    float time = 0;
    float lerp = 0;

    switch(axis) {
    case 0:
        lerp = accelRef->x;
        break;
    case 1:
        lerp = accelRef->y;
        break;
    case 2:
        lerp = accelRef->z;
        break;
    }
    
    if(direction == 1) {
        time = accelSpeed[axis] * (60.0f * deltaTime);
        if(time > 1) {
            lerp = forwardSpeed[axis];
        }
        else {
            lerp = (forwardSpeed[axis] - lerp) * time + lerp;
        }
    }
    else if(direction == -1) {
        time = accelSpeed[axis] * (60.0f * deltaTime);
        if(time > 1) {
            lerp = backwardSpeed[axis];
        }
        else {
            lerp = (backwardSpeed[axis] - lerp) * time + lerp;
        }
    }
    else {
        time = deaccelSpeed[axis] * (60.0f * deltaTime);
        if(time > 1) {
            lerp = 0;
        }
        else {
            lerp = (0 - lerp) * time + lerp;
        }
    }
    
    switch(axis) {
    case 0:
        accelRef->x = lerp;
        break;
    case 1:
        accelRef->y = lerp;
        break;
    case 2:
        accelRef->z = lerp;
        break;
    }
}

//
// P_RunCommand
//

void P_RunCommand(ENetEvent *sev, ENetPacket *packet)
{
    /*netPlayer_t *netplayer;
    ticcmd_t *cmd;
    int numactions;
    int i;

    netplayer = &netPlayers[server.GetClientID(sev->peer)];
    cmd = &netplayer->info.cmd;

    packetManager.Read32(packet, (unsigned int*)&cmd->mouse[0].i);
    packetManager.Read32(packet, (unsigned int*)&cmd->mouse[1].i);
    packetManager.Read32(packet, (unsigned int*)&cmd->timestamp.i);
    packetManager.Read32(packet, (unsigned int*)&cmd->frametime.i);
    packetManager.Read32(packet, (unsigned int*)&numactions);

    for(i = 0; i < numactions; i++)
    {
        int btn = 0;
        int held = 0;

        packetManager.Read8(packet, (unsigned int*)&btn);
        cmd->buttons[btn] = true;
        packetManager.Read8(packet, (unsigned int*)&held);
        cmd->heldtime[btn] = held;
    }

    packetManager.Read32(packet, (unsigned int*)&netplayer->info.netseq.acks);
    packetManager.Read32(packet, (unsigned int*)&netplayer->info.netseq.ingoing);*/

    // TODO - PROCESS MOVEMENT
}

enum {
    scplocation_id = 0,
    scplocation_component,
    scplocation_end
};

static const sctokens_t playerLocationTokens[scplocation_end+1] = {
    { scplocation_id,           "id"                    },
    { scplocation_component,    "component"             },
    { -1,                       NULL                    }
};

//-----------------------------------------------------------------------------
//
// kexPlayerPuppet
//
//-----------------------------------------------------------------------------

DECLARE_CLASS(kexPlayerPuppet, kexActor)

//
// kexPlayerPuppet::kexPlayerPuppet
//

kexPlayerPuppet::kexPlayerPuppet(void) {
    this->bCollision = true;
    this->id = 0;
}

//
// kexPlayerPuppet::Parse
//

void kexPlayerPuppet::Parse(kexLexer *lexer) {
    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        switch(lexer->GetIDForTokenList(playerLocationTokens, lexer->Token())) {
        case scplocation_id:
            this->id = lexer->GetNumber();
            break;
        case scplocation_component:
            // instead of creating a component object for this actor, store the
            // name of the component which will be used to initialize the actual
            // component for the player later on
            lexer->GetString();
            this->playerComponent = lexer->StringToken();
            break;
        default:
            ParseDefault(lexer);
            break;
        }
        
        lexer->Find();
    }
}

//
// kexPlayerPuppet::LocalTick
//

void kexPlayerPuppet::LocalTick(void) {
}

//-----------------------------------------------------------------------------
//
// kexPlayer
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_CLASS(kexPlayer, kexActor)

//
// kexPlayer::kexPlayer
//

kexPlayer::kexPlayer(void) {
    this->bStatic = false;

    ResetNetSequence();
    ResetTicCommand();

    acceleration.Clear();
    
    this->jsonData              = NULL;
    this->name                  = NULL;
    this->groundMove.accelRef   = &this->acceleration;
    this->swimMove.accelRef     = &this->acceleration;
    this->climbMove.accelRef    = &this->acceleration;
    this->airMove.accelRef      = &this->acceleration;
    this->crawlMove.accelRef    = &this->acceleration;
    this->flyMove.accelRef      = &this->acceleration;
    this->noClipMove.accelRef   = &this->acceleration;
    this->bNoClip               = false;

    SetCurrentMove(groundMove);
}

//
// kexPlayer::~kexPlayer
//

kexPlayer::~kexPlayer(void) {
    scriptComponent.Handle().Clear();
    scriptComponent.Clear();
}

//
// kexPlayer::ResetNetSequence
//

void kexPlayer::ResetNetSequence(void) {
    netseq.ingoing = 0;
    netseq.outgoing = 1;
}

//
// kexPlayer::ResetTicCommand
//

void kexPlayer::ResetTicCommand(void) {
    memset(&cmd, 0, sizeof(ticcmd_t));
}

//
// kexPlayer::PossessPuppet
//

void kexPlayer::PossessPuppet(kexPlayerPuppet *puppetActor) {
    if(puppetActor == NULL) {
        return;
    }
    
    puppet = puppetActor;
    // TODO - what if we possess another puppet mid-game?
    if(scriptComponent.ScriptObject() == NULL) {
        CreateComponent(puppet->playerComponent.c_str());
    }
    
    puppet->AddRef();
    puppet->SetOwner(static_cast<kexActor*>(this));

    if(puppet->Physics()->sector) {
        puppet->Physics()->sector->area->Enter();
    }
}

//
// kexPlayer::UnpossessPuppet
//

void kexPlayer::UnpossessPuppet(void) {
    if(puppet == NULL) {
        return;
    }
    
    puppet->SetOwner(NULL);
    puppet->RemoveRef();
    
    this->puppet = NULL;
    // TODO - destroy component
    if(sysMain.IsShuttingDown()) {
        scriptComponent.Release();
        scriptComponent.~kexActorComponent();
    }
}

//
// kexPlayer::PuppetToActor
//

kexActor *kexPlayer::PuppetToActor(void) {
    puppet->AddRef();
    return static_cast<kexActor*>(puppet);
}

//
// kexPlayer::ToggleClipping
//

void kexPlayer::ToggleClipping(void) {
    bool bToggle = bNoClip;

    bNoClip ^= 1;

    if(bToggle == true && bNoClip == false) {
        if(localWorld.CollisionMap().IsLoaded()) {
            puppet->Physics()->sector = localWorld.CollisionMap().PointInSector(puppet->GetOrigin());
        }
        puppet->LinkArea();
    }
    else {
        puppet->UnlinkArea();
    }
}
