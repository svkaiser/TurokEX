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
#include "actor.h"
#include "game.h"

#define MAXCLIENTS  8

typedef enum
{
    SV_STATE_UNAVAILABLE,
    SV_STATE_BUSY,
    SV_STATE_ACTIVE
} server_state_e;

typedef enum
{
    SVC_STATE_INACTIVE,
    SVC_STATE_ACTIVE,
    SVC_STATE_INGAME
} svclient_state_e;

//
// svclient - server-side client controller
// may contain non-persistent player/client data.
// used to communicate data between server
// and client
//
struct svclient_s
{
    ENetPeer            *peer;
    unsigned int        client_id;
    svclient_state_e    state;
    ticcmd_t            cmd;
    netsequence_t       ns;
};

typedef struct
{
    ENetHost        *host;
    ENetEvent       netEvent;
    kbool           local;
    server_state_e  state;
    unsigned int    maxclients;
    int             time;
    int             runtime;
    int             tics;
} server_t;

extern svclient_t svclients[MAXCLIENTS];
extern server_t server;

//
// SEND
//
void SV_SendMsg(ENetEvent *sev, int type);
void SV_SendPMove(svclient_t *svcl);
void SV_SendWeaponInfo(svclient_t *svcl);
void SV_UpdateClientInfo(ENetEvent *sev, svclient_t *svc, int id);
void SV_SendAcknowledgement(ENetEvent *sev);

//
// MAIN
//
unsigned int SV_GetPlayerID(ENetPeer *peer);
char *SV_GetPeerAddress(ENetEvent *sev);
void SV_Shutdown(void);
void SV_CreateHost(void);
void SV_Run(int msec);
void SV_Init(void);

#endif