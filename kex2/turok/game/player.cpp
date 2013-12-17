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

#include "enet/enet.h"
#include "common.h"
#include "system.h"
#include "packet.h"
#include "client.h"
#include "server.h"
#include "player.h"
#include "world.h"
#include "console.h"

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
}

//
// kexPlayer::~kexPlayer
//

kexPlayer::~kexPlayer(void) {
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
    CreateComponent(puppet->playerComponent.c_str());
    
    puppet->AddRef();
    puppet->SetOwner(static_cast<kexActor*>(this));
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
}

//
// kexPlayer::PuppetToActor
//

kexActor *kexPlayer::PuppetToActor(void) {
    puppet->AddRef();
    return static_cast<kexActor*>(puppet);
}

DECLARE_CLASS(kexLocalPlayer, kexPlayer)

//
// kexLocalPlayer::kexLocalPlayer
//

kexLocalPlayer::kexLocalPlayer(void) {
}

//
// kexLocalPlayer::~kexLocalPlayer
//

kexLocalPlayer::~kexLocalPlayer(void) {
}

//
// kexLocalPlayer::ProcessInput
//

bool kexLocalPlayer::ProcessInput(event_t *ev) {
    if(console.ProcessInput(ev))
        return true;

    switch(ev->type) {
    case ev_mouse:
        inputSystem.MoveMouse(ev->data2, ev->data3);
        return true;

    case ev_keydown:
    case ev_mousedown:
        inputKey.ExecuteCommand(ev->data1, false);
        return true;

    case ev_keyup:
    case ev_mouseup:
        inputKey.ExecuteCommand(ev->data1, true);
        return true;
    }

    return false;
}

//
// kexLocalPlayer::BuildCommands
//

void kexLocalPlayer::BuildCommands(void) {
    ENetPacket *packet;
    ticcmd_t *buildCmd;
    control_t *ctrl;
    int numactions;
    int i;

    if(client.GetState() != CL_STATE_INGAME)
        return;

    buildCmd = &cmd;
    ctrl = inputKey.Controls();

    for(i = 0; i < MAXACTIONS; i++) {
        buildCmd->buttons[i] = false;
    }

    for(i = 0; i < MAXACTIONS; i++) {
        buildCmd->buttons[i] = (ctrl->actions[i]) ? true : false;
        if(buildCmd->buttons[i] == 1) {
            if(buildCmd->heldtime[i] < 255) {
                buildCmd->heldtime[i]++;
            }
            else {
                buildCmd->heldtime[i] = 255;
            }
        }
        else {
            buildCmd->heldtime[i] = 0;
        }
    }

    buildCmd->timestamp.i = client.GetTime();
    buildCmd->frametime.f = client.GetRunTime();
    buildCmd->mouse[0].f = ctrl->mousex;
    buildCmd->mouse[1].f = ctrl->mousey;

    ctrl->mousex = 0;
    ctrl->mousey = 0;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, cp_cmd);

    numactions = 0;

    for(i = 0; i < MAXACTIONS; i++) {
        if(buildCmd->buttons[i]) {
            numactions++;
        }
    }

    packetManager.Write32(packet, buildCmd->mouse[0].i);
    packetManager.Write32(packet, buildCmd->mouse[1].i);
    packetManager.Write32(packet, buildCmd->timestamp.i);
    packetManager.Write32(packet, buildCmd->frametime.i);
    packetManager.Write32(packet, numactions);

    for(i = 0; i < MAXACTIONS; i++) {
        if(buildCmd->buttons[i]) {
            packetManager.Write8(packet, i);
            packetManager.Write8(packet, buildCmd->heldtime[i]);
        }
    }

    packetManager.Write32(packet, netseq.ingoing);
    packetManager.Write32(packet, netseq.outgoing);
    netseq.outgoing++;

    packetManager.Send(packet, peer);
}

//
// kexLocalPlayer::ActionDown
//

bool kexLocalPlayer::ActionDown(const kexStr &str) {
    int action = inputKey.FindAction(str.c_str());

    return (action != -1 && cmd.buttons[action]);
}

//
// kexLocalPlayer::ActionHeldTime
//

int kexLocalPlayer::ActionHeldTime(const kexStr &str) {
    int action = inputKey.FindAction(str.c_str());
    int heldtime = 0;

    if(action != -1) {
        heldtime = (cmd.heldtime[action] - 1);
        if(heldtime < 0)
            heldtime = 0;
    }

    return heldtime;
}
//
// kexLocalPlayer::LocalTick
//

void kexLocalPlayer::LocalTick(void) {
    kexSector *exitSector;
    kexSector *enterSector;

    if(client.GetState() != CL_STATE_INGAME) {
        return;
    }

    timeStamp = (float)cmd.timestamp.i;
    frameTime = cmd.frametime.f;

    int current = (netseq.outgoing-1) & (NETBACKUPS-1);

    oldMoves[current] = puppet->GetOrigin();
    latency[current] = client.GetTime();

    scriptComponent.CallFunction(scriptComponent.onLocalThink);

    exitSector = puppet->Physics()->sector;

    puppet->Physics()->Think(frameTime);
    puppet->UpdateTransform();

    enterSector = puppet->Physics()->sector;

    localWorld.CollisionMap().PlayerCrossAreas(enterSector, exitSector);

    // area local thinker
    if(enterSector != NULL && enterSector->area != NULL) {
        enterSector->area->scriptComponent.
            CallFunction(enterSector->area->scriptComponent.onLocalThink);
    }

    angles          = puppet->GetAngles();
    origin          = puppet->GetOrigin();
    viewHeight      = puppet->GetViewHeight();
    centerHeight    = puppet->GetCenterHeight();
    rotation        = puppet->GetRotation();

    angles.Clamp180();
}

//
// kexLocalPlayer::InitObject
//

void kexLocalPlayer::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kLocalPlayer",
        sizeof(kexLocalPlayer),
        asOBJ_REF);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kLocalPlayer",
        asBEHAVE_ADDREF,
        "void f()",
        asMETHOD(kexLocalPlayer, AddRef),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kLocalPlayer",
        asBEHAVE_RELEASE,
        "void f()",
        asMETHOD(kexLocalPlayer, RemoveRef),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kActor",
        asBEHAVE_REF_CAST,
        "kLocalPlayer@ f()",
        asFUNCTION((kexScriptManager::RefCast<kexLocalPlayer, kexActor>)),
        asCALL_CDECL_OBJLAST);

    scriptManager.Engine()->RegisterObjectType(
        "kPlayerMove",
        sizeof(kexPlayerMove),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);

    kexActor::RegisterBaseProperties<kexLocalPlayer>("kLocalPlayer");

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kLocalPlayer",                             \
        str,                                        \
        asMETHODPR(kexLocalPlayer, a, b, c),        \
        asCALL_THISCALL)

    OBJMETHOD("bool ActionDown(const kStr &in)", ActionDown, (const kexStr&), bool);
    OBJMETHOD("int ActionHeldTime(const kStr &in)", ActionHeldTime, (const kexStr&), int);
    OBJMETHOD("kActor @ToActor(void)", ToWorldActor, (void), kexActor*);
    OBJMETHOD("kActor @Puppet(void)", PuppetToActor, (void), kexActor*);
    OBJMETHOD("kVec3 &GetAcceleration(void)", GetAcceleration, (void), kexVec3&);
    OBJMETHOD("void SetAcceleration(const kVec3 &in)", SetAcceleration, (const kexVec3&), void);
    OBJMETHOD("float GetCrawlHeight(void)", GetCrawlHeight, (void), float);
    OBJMETHOD("void SetCrawlHeight(const float)", SetCrawlHeight, (const float f), void);
    OBJMETHOD("kPlayerMove &GroundMove(void)", GroundMove, (void), kexPlayerMove&);
    OBJMETHOD("kPlayerMove &AirMove(void)", AirMove, (void), kexPlayerMove&);
    OBJMETHOD("kPlayerMove &SwimMove(void)", SwimMove, (void), kexPlayerMove&);
    OBJMETHOD("kPlayerMove &CrawlMove(void)", CrawlMove, (void), kexPlayerMove&);
    OBJMETHOD("kPlayerMove &FlyMove(void)", FlyMove, (void), kexPlayerMove&);
    OBJMETHOD("kPlayerMove &NoClipMove(void)", NoClipMove, (void), kexPlayerMove&);

#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kLocalPlayer",                             \
        str,                                        \
        asOFFSET(kexLocalPlayer, p))

    OBJPROPERTY("float cmdMouseX", cmd.mouse[0].f);
    OBJPROPERTY("float cmdMouseY", cmd.mouse[1].f);
    OBJPROPERTY("float deltaTime", cmd.frametime.f);
    OBJPROPERTY("bool bAllowCrawl", bAllowCrawl);

#undef OBJMETHOD
#undef OBJPROPERTY

    scriptManager.Engine()->RegisterGlobalProperty(
        "kLocalPlayer LocalPlayer",
        client.GetLocalPlayerRef());

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kPlayerMove",                              \
        str,                                        \
        asMETHODPR(kexPlayerMove, a, b, c),         \
        asCALL_THISCALL)

    OBJMETHOD("void Accelerate(int, int, const float)",
        Accelerate, (int, int, const float), void);
        
#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kPlayerMove",                              \
        str,                                        \
        asOFFSET(kexPlayerMove, p))

    OBJPROPERTY("kVec3 accelSpeed", accelSpeed);
    OBJPROPERTY("kVec3 deaccelSpeed", deaccelSpeed);
    OBJPROPERTY("kVec3 forwardSpeed", forwardSpeed);
    OBJPROPERTY("kVec3 backwardSpeed", backwardSpeed);

#undef OBJMETHOD
#undef OBJPROPERTY
}

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
}
