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

typedef struct
{
    netsequence_t   netseq;
    char            *name;
    ticcmd_t        cmd;
    ENetPeer        *peer;
    int             id;
    char            *jsonData;
} playerInfo_t;

typedef struct
{
    playerInfo_t    info;
    worldState_t    worldState;
    gActor_t        *actor;
    gActor_t        *camera;
    int             latency[NETBACKUPS];
    vec3_t          moveDiff;
    vec3_t          oldMoves[NETBACKUPS];
    ticcmd_t        oldCmds[NETBACKUPS];
    gObject_t       *playerObject;
} localPlayer_t;

typedef struct
{
    playerInfo_t    info;
    worldState_t    worldState;
    gActor_t        *actor;
    gObject_t       *playerObject;
    int             state;
} netPlayer_t;

extern localPlayer_t localPlayer;
extern netPlayer_t netPlayers[MAX_PLAYERS];

void P_ResetNetSeq(playerInfo_t *info);
void P_RunCommand(ENetEvent *sev, ENetPacket *packet);
void P_NewPlayerConnected(ENetEvent *sev);
netPlayer_t *P_GetNetPlayer(ENetPeer *peer);
void P_SpawnLocalPlayer(void);
void P_BuildCommands(void);
int P_LocalPlayerEvent(const char *eventName);
void P_SaveLocalComponentData(void);
void P_LocalPlayerTick(void);
kbool P_Responder(event_t *ev);

#endif
