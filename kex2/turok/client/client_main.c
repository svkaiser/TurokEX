// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
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
//
// DESCRIPTION: Main Client Code
//
//-----------------------------------------------------------------------------

#include "enet/enet.h"
#include "common.h"
#include "packet.h"
#include "client.h"
#include "kernel.h"
#include "render.h"
#include "menu.h"
#include "gl.h"

static kbool bDebugTime;

CVAR(cl_name, player);
CVAR(cl_fov, 74);
CVAR(cl_autorun, 0);
CVAR(cl_paused, 0);
CVAR(cl_msensitivityx, 5);
CVAR(cl_msensitivityy, 5);
CVAR(cl_macceleration, 0);
CVAR(cl_mlook, 0);
CVAR(cl_mlookinvert, 0);
CVAR(cl_port, 58304);

CVAR_CMD(cl_maxfps, 60)
{
    if(cvar->value <= 0)
    {
        cvar->value = 1;
    }
}

typedef enum
{
    CL_STATE_UNINITIALIZED,
    CL_STATE_CONNECTING,
    CL_STATE_CONNECTED,
    CL_STATE_DISCONNECTED
} client_state_e;

typedef struct
{
    ENetHost        *host;
    client_state_e  state;
    ENetPeer        *peer;
    int             client_id;
    int             time;
    float           runtime;
    kbool           local;
    int             tics;
} kex_client_t;

kex_client_t kex_client;

//
// CL_DestroyClient
//

void CL_DestroyClient(void)
{
    if(kex_client.host)
    {
        if(kex_client.state == CL_STATE_CONNECTED &&
            kex_client.peer != NULL)
        {
            enet_peer_disconnect(kex_client.peer, 0);
        }

        enet_host_destroy(kex_client.host);
        kex_client.host = NULL;
    }
}

//
// CL_Shutdown
//

void CL_Shutdown(void)
{
    CL_DestroyClient();
}

//
// CL_Connect
//

void CL_Connect(const char *address)
{
    ENetAddress addr;
    char ip[32];

    enet_address_set_host(&addr, address);
    addr.port = (int)cl_port.value;
    kex_client.peer = enet_host_connect(kex_client.host, &addr, 2, 0);
    enet_address_get_host_ip(&addr, ip, 32);
    Com_Printf("Connecting to %s:%u...\n", ip, addr.port);

    if(kex_client.peer == NULL)
    {
        Com_Warning("No available peers for initiating an ENet connection.\n");
    }
    else
    {
        kex_client.state = CL_STATE_CONNECTING;
    }
}

//
// CL_CheckHost
//

static void CL_CheckHost(void)
{
    ENetEvent cev;

    while(enet_host_service(kex_client.host, &cev, 0) > 0)
    {
        switch(cev.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            Com_Printf("connected to host\n");
            kex_client.state = CL_STATE_CONNECTED;
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            Com_Printf("disconnected from host\n");
            kex_client.state = CL_STATE_DISCONNECTED;
            break;
        }
    }
}

//
// CL_Ticker
//

static void CL_Ticker(void)
{
    Menu_Ticker();
    Con_Ticker();
    kex_client.tics++;
}

//
// CL_DrawDebug
//

static void CL_DrawDebug(void)
{
    if(bDebugTime)
    {
        Draw_Text(0, 16,  COLOR_GREEN, 1, "-------------------");
        Draw_Text(0, 32,  COLOR_GREEN, 1, "   client debug");
        Draw_Text(0, 48,  COLOR_GREEN, 1, "-------------------");
        Draw_Text(0, 64,  COLOR_GREEN, 1, "runtime: %f", kex_client.runtime);
        Draw_Text(0, 80,  COLOR_GREEN, 1, "time: %i", kex_client.time);
        Draw_Text(0, 96,  COLOR_GREEN, 1, "tics: %i", kex_client.tics);
        Draw_Text(0, 112, COLOR_GREEN, 1, "max msecs: %f", (1000.0f / cl_maxfps.value));
    }
}

//
// CL_Drawer
//

static void CL_Drawer(void)
{
    R_DrawFrame();
    CL_DrawDebug();
    Menu_Drawer();
    Con_Draw();
    R_FinishFrame();
}

//
// CL_Run
//

static int curtime = 0;

void CL_Run(int msec)
{
    curtime += msec;

    if(curtime < (1000 / (int)cl_maxfps.value))
    {
        return;
    }

    kex_client.runtime = (float)curtime / 1000.0f;
    kex_client.time += curtime;

    curtime = 0;

    IN_PollInput();

    CL_ProcessEvents();
    CL_CheckHost();
    CL_Drawer();
    CL_Ticker();
}

//
// CL_Random
//

int CL_Random(void)
{
    return 0;
}

//
// FCmd_ShowClientTime
//

static void FCmd_ShowClientTime(void)
{
    bDebugTime ^= 1;
}

//
// FCmd_Ping
//

static void FCmd_Ping(void)
{
    ENetPacket *packet;

    if(!(packet = Packet_New()))
    {
        return;
    }

    Packet_Write8(packet, CLIENT_PACKET_PING);
    Packet_Send(packet, kex_client.peer);
}

//
// FCmd_Say
//

static void FCmd_Say(void)
{
    ENetPacket *packet;

    if(Cmd_GetArgc() < 2)
    {
        return;
    }

    if(!(packet = Packet_New()))
    {
        return;
    }

    Packet_Write8(packet, CLIENT_PACKET_SAY);
    Packet_WriteString(packet, Cmd_GetArgv(1));
    Packet_Send(packet, kex_client.peer);
}

//
// CL_Init
//

void CL_Init(void)
{
    CL_DestroyClient();

    kex_client.host = enet_host_create(NULL, 1, 2, 0, 0);

    if(!kex_client.host)
    {
        Com_Error("CL_Init: failed to create client");
        return;
    }

    bDebugTime = false;

    kex_client.client_id = 0;
    kex_client.time = 0;
    kex_client.tics = 0;
    kex_client.peer = NULL;
    kex_client.state = CL_STATE_UNINITIALIZED;
    kex_client.local = (Com_CheckParam("-client") == 0);

    Cvar_Register(&cl_name);
    Cvar_Register(&cl_fov);
    Cvar_Register(&cl_autorun);
    Cvar_Register(&cl_maxfps);
    Cvar_Register(&cl_paused);
    Cvar_Register(&cl_msensitivityx);
    Cvar_Register(&cl_msensitivityy);
    Cvar_Register(&cl_macceleration);
    Cvar_Register(&cl_mlook);
    Cvar_Register(&cl_mlookinvert);
    Cvar_Register(&cl_port);

    Cmd_AddCommand("debugclienttime", FCmd_ShowClientTime);
    Cmd_AddCommand("ping", FCmd_Ping);
    Cmd_AddCommand("say", FCmd_Say);
}
