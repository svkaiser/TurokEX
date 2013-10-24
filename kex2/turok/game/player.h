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

#include "common.h"
#include "actor.h"

void P_RunCommand(ENetEvent *sev, ENetPacket *packet);

typedef struct {
    int                     ingoing;
    int                     outgoing;
    int                     acks;
} netSequence_t;

class kexPlayerMove {
public:
    kexVec3                 accelSpeed;
    kexVec3                 deaccelSpeed;
    kexVec3                 forwardSpeed;
    kexVec3                 backwardSpeed;
    kexVec3                 *accelRef;
    
    void                    Accelerate(int direction, int axis, const float deltaTime);
};

BEGIN_EXTENDED_CLASS(kexPlayerPuppet, kexWorldActor);
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

BEGIN_EXTENDED_CLASS(kexPlayer, kexWorldActor);
public:
                            kexPlayer(void);
                            ~kexPlayer(void);

    void                    ResetNetSequence(void);
    void                    ResetTicCommand(void);
    void                    PossessPuppet(kexPlayerPuppet *puppetActor);
    void                    UnpossessPuppet(void);

    kexPlayerMove           &GroundMove(void) { return groundMove; }
    kexPlayerMove           &AirMove(void) { return airMove; }
    kexPlayerMove           &SwimMove(void) { return swimMove; }
    kexPlayerMove           &ClimbMove(void) { return climbMove; }
    kexPlayerMove           &CrawlMove(void) { return crawlMove; }
    kexPlayerMove           &FlyMove(void) { return flyMove; }
    kexPlayerMove           &NoClipMove(void) { return noClipMove; }

    kexVec3                 &GetAcceleration(void) { return acceleration; }
    void                    SetAcceleration(const kexVec3 &accel) { acceleration = accel; }
    bool                    GetAllowCrawl(void) { return bAllowCrawl; }
    void                    SetAllowCrawl(const bool b) { bAllowCrawl = b; }
    float                   GetCrawlHeight(void) { return crawlHeight; }
    void                    SetCrawlHeight(const float f) { crawlHeight = f; }
    float                   GetMoveTime(void) { return moveTime; }
    void                    SetMoveTime(const float t) { moveTime = t; }
    
    ticcmd_t                *Cmd(void) { return &cmd; }
    kexPlayerPuppet         *Puppet(void) { return puppet; }
    kexWorldActor           *PuppetToActor(void);
    ENetPeer                *GetPeer(void) { return peer; }
    void                    SetPeer(ENetPeer *_peer) { peer = _peer; }
    int                     GetID(void) const { return id; }
    void                    SetID(const int _id) { id = _id; }
    netSequence_t           *NetSeq(void) { return &netseq; }

    // TODO - REMOVE
    worldState_t            worldState;
    
protected:
    kexPlayerPuppet         *puppet;
    kexVec3                 acceleration;
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
    netSequence_t           netseq;
    int                     id;
    char                    *name;
    ENetPeer                *peer;
    char                    *jsonData;
    float                   frameTime;
    float                   timeStamp;
END_CLASS();

BEGIN_EXTENDED_CLASS(kexLocalPlayer, kexPlayer);
public:
                            kexLocalPlayer(void);
                            ~kexLocalPlayer(void);

    virtual void            LocalTick(void);

    bool                    ProcessInput(event_t *ev);
    void                    BuildCommands(void);
    bool                    ActionDown(const kexStr &str);
    int                     ActionHeldTime(const kexStr &str);

    //kexActor               *Camera(void) { return camera; }
    kexVec3                 &MoveDiff(void) { return moveDiff; }
    kexWorldActor           *ToWorldActor(void) { return static_cast<kexWorldActor*>(this); }

    // TODO
    gActor_t                *actor;
    gActor_t                *camera;

    static void             InitObject(void);

private:
    //kexActor              *camera;
    int                     latency[NETBACKUPS];
    kexVec3                 moveDiff;
    kexVec3                 oldMoves[NETBACKUPS];
    ticcmd_t                oldCmds[NETBACKUPS];
END_CLASS();

BEGIN_EXTENDED_CLASS(kexNetPlayer, kexPlayer);
public:
                            kexNetPlayer(void);
                            ~kexNetPlayer(void);

    virtual void            Tick(void);
    int                     GetState(void) const { return state; }
    void                    SetState(const int s) { state = s; }

    // TODO
    gActor_t                *actor;

private:
    int                     state;
END_CLASS();

#endif
