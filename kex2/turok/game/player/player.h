// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "enet/enet.h"
#include "common.h"
#include "actor.h"

typedef enum {
    PS_STATE_INACTIVE,
    PS_STATE_ACTIVE,
    PS_STATE_INGAME
} player_state_e;

//-----------------------------------------------------------------------------
//
// kexPlayerMove
//
//-----------------------------------------------------------------------------

class kexPlayerMove {
public:
    kexVec3                 accelSpeed;
    kexVec3                 deaccelSpeed;
    kexVec3                 forwardSpeed;
    kexVec3                 backwardSpeed;
    kexVec3                 *accelRef;
    
    void                    Accelerate(int direction, int axis, const float deltaTime);
};

//-----------------------------------------------------------------------------
//
// kexPlayerPuppet
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexPlayerPuppet, kexActor);
public:
                            kexPlayerPuppet(void);
                            ~kexPlayerPuppet(void);

    virtual void            Parse(kexLexer *lexer);
    virtual void            LocalTick(void);

    const int               GetID(void) const { return id; }
    kexStr                  playerComponent;

private:
    int                     id;
END_CLASS();

//-----------------------------------------------------------------------------
//
// kexPlayer
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexPlayer, kexActor);
public:
                            kexPlayer(void);
                            ~kexPlayer(void);

    void                    ResetTicCommand(void);
    void                    PossessPuppet(kexPlayerPuppet *puppetActor);
    void                    UnpossessPuppet(void);
    void                    ToggleClipping(void);
    bool                    ActionDown(const kexStr &str);
    int                     ActionHeldTime(const kexStr &str);

    kexPlayerMove           &GroundMove(void) { return groundMove; }
    kexPlayerMove           &AirMove(void) { return airMove; }
    kexPlayerMove           &SwimMove(void) { return swimMove; }
    kexPlayerMove           &ClimbMove(void) { return climbMove; }
    kexPlayerMove           &CrawlMove(void) { return crawlMove; }
    kexPlayerMove           &FlyMove(void) { return flyMove; }
    kexPlayerMove           &NoClipMove(void) { return noClipMove; }
    kexPlayerMove           &GetCurrentMove(void) { return *currentMove; }
    void                    SetCurrentMove(kexPlayerMove &move) { currentMove = &move; }

    kexVec3                 &GetAcceleration(void) { return acceleration; }
    void                    SetAcceleration(const kexVec3 &accel) { acceleration = accel; }
    bool                    GetAllowCrawl(void) { return bAllowCrawl; }
    void                    SetAllowCrawl(const bool b) { bAllowCrawl = b; }
    float                   GetCrawlHeight(void) { return crawlHeight; }
    void                    SetCrawlHeight(const float f) { crawlHeight = f; }
    float                   GetMoveTime(void) { return moveTime; }
    void                    SetMoveTime(const float t) { moveTime = t; }
    bool                    &NoClip(void) { return bNoClip; }
    byte                    &State(void) { return state; }
    
    ticcmd_t                *Cmd(void) { return &cmd; }
    kexPlayerPuppet         *Puppet(void) { return puppet; }
    kexActor                *PuppetToActor(void);
    ENetPeer                *GetPeer(void) { return peer; }
    void                    SetPeer(ENetPeer *_peer) { peer = _peer; }
    int                     GetID(void) const { return id; }
    void                    SetID(const int _id) { id = _id; }

protected:
    kexPlayerPuppet         *puppet;
    kexVec3                 acceleration;
    kexPlayerMove           *currentMove;
    kexPlayerMove           groundMove;
    kexPlayerMove           airMove;
    kexPlayerMove           swimMove;
    kexPlayerMove           climbMove;
    kexPlayerMove           crawlMove;
    kexPlayerMove           flyMove;
    kexPlayerMove           noClipMove;
    ticcmd_t                cmd;
    bool                    bAllowCrawl;
    float                   crawlHeight;
    float                   moveTime;
    int                     id;
    char                    *name;
    ENetPeer                *peer;
    float                   frameTime;
    float                   timeStamp;
    bool                    bNoClip;
    byte                    state;
END_CLASS();

#include "player/player_client.h"

#endif
