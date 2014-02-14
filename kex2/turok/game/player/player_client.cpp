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
// DESCRIPTION: Client Player code
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "system.h"
#include "packet.h"
#include "client.h"
#include "server.h"
#include "player/player.h"
#include "world.h"
#include "console.h"

//-----------------------------------------------------------------------------
//
// kexLocalPlayer
//
//-----------------------------------------------------------------------------

DECLARE_CLASS(kexLocalPlayer, kexPlayer)

//
// kexLocalPlayer::kexLocalPlayer
//

kexLocalPlayer::kexLocalPlayer(void) {
    this->clientTarget = &client;
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

    if(client.GetState() != CL_STATE_INGAME) {
        return;
    }

    if(bLocked == true) {
        return;
    }

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

    if(!(packet = packetManager.Create())) {
        return;
    }

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

    packetManager.Send(packet, peer);
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
    
    scriptComponent.CallFunction(scriptComponent.onLocalThink);
    
    exitSector = puppet->Physics()->sector;

    if(bLocked == false) {
        if(bNoClip == false) {
            puppet->Physics()->Think(frameTime);
        }
        else {
            puppet->GetOrigin() += (puppet->Physics()->velocity * frameTime);
            puppet->Physics()->velocity.Clear();
        }
    }

    puppet->UpdateTransform();
    enterSector = puppet->Physics()->sector;

    angles          = puppet->GetAngles();
    origin          = puppet->GetOrigin();
    viewHeight      = puppet->GetViewHeight();
    centerHeight    = puppet->GetCenterHeight();
    rotation        = puppet->GetRotation();

    angles.Clamp180();

    localWorld.CollisionMap().PlayerCrossAreas(enterSector, exitSector);

    if(puppet->Physics()->waterLevel > WLT_OVER) {
        SetCurrentMove(swimMove);
    }
    else {
        SetCurrentMove(groundMove);
    }

    // area local thinker
    if(enterSector != NULL && enterSector->area != NULL) {
        enterSector->area->scriptComponent.
            CallFunction(enterSector->area->scriptComponent.onLocalThink);
    }
}

//
// kexLocalPlayer::InitObject
//

void kexLocalPlayer::InitObject(void) {
    kexScriptManager::RegisterRefObjectNoCount<kexLocalPlayer>("kLocalPlayer");
    scriptManager.Engine()->RegisterObjectBehaviour(
        "kActor",
        asBEHAVE_REF_CAST,
        "kLocalPlayer@ f()",
        asFUNCTION((kexScriptManager::RefCast<kexLocalPlayer, kexActor>)),
        asCALL_CDECL_OBJLAST);

    kexActor::RegisterBaseProperties<kexLocalPlayer>("kLocalPlayer");
    kexScriptManager::RegisterDataObject<kexPlayerMove>("kPlayerMove");

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
    OBJMETHOD("kPlayerMove &GetCurrentMove(void)", GetCurrentMove, (void), kexPlayerMove&);
    OBJMETHOD("void SetCurrentMove(kPlayerMove &in)", SetCurrentMove, (kexPlayerMove&), void);
    OBJMETHOD("void Lock(void)", Lock, (void), void);
    OBJMETHOD("void Unlock(void)", Unlock, (void), void);

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
