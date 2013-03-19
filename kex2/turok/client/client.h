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

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "enet/enet.h"
#include "kernel.h"
#include "actor.h"
#include "game.h"

#define MAXEVENTS 64

extern event_t  events[MAXEVENTS];
extern int      eventhead;
extern int      eventtail;

typedef enum
{
    CL_STATE_UNINITIALIZED,
    CL_STATE_CONNECTING,
    CL_STATE_CONNECTED,
    CL_STATE_DISCONNECTED,
    CL_STATE_READY
} client_state_e;

typedef struct
{
    ENetHost        *host;
    client_state_e  state;
    ENetPeer        *peer;
    ENetEvent       netEvent;
    int             client_id;
    int             time;
    float           runtime;
    kbool           local;
    int             tics;
} client_t;

extern client_t client;

kbool CL_Responder(event_t *ev);
void CL_WriteTiccmd(ENetPacket *packet, ticcmd_t *cmd);
void CL_BuildTiccmd(void);
void CL_PostEvent(event_t *ev);
event_t *CL_GetEvent(void);
void CL_ProcessEvents(void);
kbool CL_Responder(event_t *ev);
void CL_Connect(const char *address);
void CL_Run(int msec);
void CL_Init(void);

#endif