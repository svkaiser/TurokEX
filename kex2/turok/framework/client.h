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
#include "system.h"
#include "player.h"
#include "network.h"

#define MAXEVENTS 64

extern kexCvar cvarClientFOV;
extern kexCvar cvarMSensitivityX;
extern kexCvar cvarMSensitivityY;
extern kexCvar cvarMAcceleration;
extern kexCvar cvarClientFPS;

typedef enum
{
    CL_STATE_UNINITIALIZED,
    CL_STATE_CONNECTING,
    CL_STATE_CONNECTED,
    CL_STATE_DISCONNECTED,
    CL_STATE_READY,
    CL_STATE_INGAME,
    CL_STATE_CHANGINGLEVEL
} client_state_e;

class kexClient : public kexNetwork {
public:
    virtual void        Init(void);
    virtual void        Destroy(void);
    virtual void        Shutdown(void);
    virtual void        ProcessPackets(const ENetPacket *packet);
    virtual void        CreateHost(void);
    virtual void        Run(const int msec);
    virtual void        OnConnect(void);
    virtual void        OnDisconnect(void);

    void                Connect(const char *address);
    void                MessageServer(char *string);
    void                PostEvent(event_t *ev);
    event_t             *GetEvent(void);
    void                ProcessEvents(void);

    void                SetPeer(ENetPeer *_peer) { peer = _peer; }
    ENetPeer            *GetPeer(void) { return peer; }
    bool                IsLocal(void) { return bLocal; }
    kexLocalPlayer      &LocalPlayer(void) { return playerClient; }
    kexLocalPlayer      *GetLocalPlayerRef(void) { return &playerClient; }

    unsigned int        id;
    int                 curtime;
    int                 fps;

    static void         InitObject(void);

private:
    void                PrepareMapChange(const ENetPacket *packet);

    bool                bLocal;
    ENetPeer            *peer;
    kexLocalPlayer      playerClient;
    event_t             events[MAXEVENTS];
    int                 eventhead;
    int                 eventtail;
};

extern kexClient client;

#endif