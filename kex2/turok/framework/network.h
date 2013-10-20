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
    virtual void        Init(void) = 0;
    virtual void        Destroy(void);
    virtual void        Shutdown(void);
    virtual void        ProcessPackets(const ENetPacket *packet) = 0;
    virtual void        CreateHost(void) = 0;
    virtual void        CheckMessages(void);
    virtual void        Run(const int msec) = 0;
    virtual void        OnConnect(void) = 0;
    virtual void        OnDisconnect(void) = 0;
    virtual void        OnRecieve(void);
    
    void                SetHost(ENetHost *_host) { host = _host; }
    void                SetTime(int _time) { time = _time; }
    void                SetState(int _state) { state = _state; }
    void                SetEvent(ENetEvent _event) { netEvent = _event; }
    void                SetRunTime(float _runTime) { runTime = _runTime; }
    void                SetTicks(int _ticks) { ticks = _ticks; }

    ENetHost            *GetHost(void) { return host; }
    int                 GetTime(void) { return time; }
    int                 GetState(void) { return state; }
    ENetEvent           *GetEvent(void) { return &netEvent; }
    float               GetRunTime(void) { return runTime; }
    int                 GetTicks(void) { return ticks; }

    void                UpdateTicks(void) { ticks++; }
    void                DestroyPacket(void) { enet_packet_destroy(netEvent.packet); }
    void                DestroyHost(void) { enet_host_destroy(host); host = NULL; }

private:
    ENetHost            *host;
    ENetEvent           netEvent;
    int                 state;
    int                 time;
    float               runTime;
    int                 ticks;
};

#endif
