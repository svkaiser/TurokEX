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

#ifndef _SERVER_H_
#define _SERVER_H_

#include "enet/enet.h"
#include "client.h"
#include "player.h"

#define MAXCLIENTS MAXPLAYERS

typedef enum
{
    SV_STATE_UNAVAILABLE,
    SV_STATE_BUSY,
    SV_STATE_ACTIVE
} server_state_e;

typedef enum
{
    SVC_STATE_INACTIVE,
    SVC_STATE_ACTIVE
} svclient_state_e;

typedef struct
{
    player_t            *player;
    ENetPeer            *peer;
    unsigned int        client_id;
    svclient_state_e    state;
} svclient_t;

typedef struct
{
    ENetHost        *host;
    kbool           local;
    server_state_e  state;
    uint32          maxclients;
    int             time;
    float           runtime;
} server_t;

extern svclient_t svclients[MAXCLIENTS];
extern server_t server;

void SV_Shutdown(void);
void SV_CreateHost(void);
void SV_Run(int msec);
void SV_Init(void);

#endif