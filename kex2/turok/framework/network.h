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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "enet/enet.h"

class kexNetwork {
public:
    virtual void        Init(void);
    virtual void        Destroy(void);
    virtual void        Shutdown(void);
    virtual void        ProcessPackets(const ENetPacket *packet);
    virtual void        CreateHost(void);
    virtual void        CheckMessages(void);
    virtual void        Run(const int msec);
    virtual void        OnConnect(void);
    virtual void        OnDisconnect(void);
    virtual void        OnRecieve(void);
    
    d_inline void       SetHost(ENetHost *_host) { host = _host; }
    d_inline void       SetTime(int _time) { time = _time; }
    d_inline void       SetState(int _state) { state = _state; }
    d_inline void       SetEvent(ENetEvent _event) { netEvent = _event; }
    d_inline void       SetRunTime(float _runTime) { runTime = _runTime; }
    d_inline void       SetTicks(int _ticks) { ticks = _ticks; }

    d_inline ENetHost   *GetHost(void) { return host; }
    d_inline int        GetTime(void) { return time; }
    d_inline int        GetState(void) { return state; }
    d_inline ENetEvent  *GetEvent(void) { return &netEvent; }
    d_inline float      GetRunTime(void) { return runTime; }
    d_inline int        GetTicks(void) { return ticks; }

    d_inline void       UpdateTicks(void) { ticks++; }
    d_inline void       DestroyPacket(void) { enet_packet_destroy(netEvent.packet); }
    d_inline void       DestroyHost(void) { enet_host_destroy(host); host = NULL; }

private:
    ENetHost            *host;
    ENetEvent           netEvent;
    int                 state;
    int                 time;
    float               runTime;
    int                 ticks;
};

#endif
