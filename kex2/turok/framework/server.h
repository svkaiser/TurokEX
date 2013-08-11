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
#include "player.h"

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

#define SERVER_RUNTIME  16

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
    int                 state;
    ticcmd_t            cmd;
    netsequence_t       ns;
};

class kexServer : public kexNetwork {
public:
    virtual void        Init(void);
    virtual void        Destroy(void);
    virtual void        Shutdown(void);
    virtual void        ProcessPackets(const ENetPacket *packet);
    virtual void        CreateHost(void);
    virtual void        Run(const int msec);
    virtual void        OnConnect(void);
    virtual void        OnDisconnect(void);

    d_inline bool       IsLocal(void) { return bLocal; }
    d_inline int        GetElaspedTime(void) { return elaspedTime; }
    d_inline void       SetElaspedTime(int _time) { elaspedTime = _time; }
    d_inline int        GetMaxClients(void) { return maxClients; }
    d_inline void       SetMaxClients(int _max) { maxClients = _max; }

    char                *GetPeerAddress(ENetEvent *sev);
    void                SendMoveData(svclient_t *svcl);
    void                SendMessage(ENetEvent *sev, int type);
    unsigned int        GetClientID(ENetPeer *peer) const;

private:
    void                SendClientMessages(void);
    void                AddClient(ENetEvent *sev);
    void                ClientCommand(ENetEvent *sev, ENetPacket *packet);
    void                SendAcknowledgement(ENetEvent *sev);

    bool                bLocal;
    int                 maxClients;
    int                 elaspedTime;
    svclient_t          clients[MAX_PLAYERS];
};

extern kexServer server;


#endif