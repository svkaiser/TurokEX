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

#define CL_BACKUP_CMDS  128

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
    ticcmd_t        cmd;
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
// CL_Responder
//

kbool CL_Responder(event_t *ev)
{
    switch(ev->type)
    {
    case ev_mouse:
        IN_MouseMove(ev->data1, ev->data2);
        return true;

    case ev_keydown:
        Key_ExecCmd(ev->data1, false);
        return true;

    case ev_keyup:
        Key_ExecCmd(ev->data1, true);
        return true;
    }

    return false;
}

//
// CL_WriteTiccmd
//

static void CL_WriteTiccmd(ENetPacket *packet, ticcmd_t *cmd)
{
    byte bits = 0;

#define DIFF_TICCMDS(name, bit)         \
    if(cmd->name != 0)                  \
    bits |= bit

#define WRITE_TICCMD8(name, bit)        \
    if(bits & bit)                      \
    Packet_Write8(packet, cmd->name)

#define WRITE_TICCMD16(name, bit)       \
    if(bits & bit)                      \
    Packet_Write16(packet, cmd->name)

    DIFF_TICCMDS(forwardmove, CL_TICDIFF_FORWARD);
    DIFF_TICCMDS(sidemove, CL_TICDIFF_SIDE);
    DIFF_TICCMDS(angleturn, CL_TICDIFF_TURN);
    DIFF_TICCMDS(pitch, CL_TICDIFF_PITCH);
    DIFF_TICCMDS(buttons, CL_TICDIFF_BUTTONS);

    Packet_Write8(packet, bits);

    WRITE_TICCMD8(forwardmove, CL_TICDIFF_FORWARD);
    WRITE_TICCMD8(sidemove, CL_TICDIFF_SIDE);
    WRITE_TICCMD16(angleturn, CL_TICDIFF_TURN);
    WRITE_TICCMD16(pitch, CL_TICDIFF_PITCH);
    WRITE_TICCMD8(buttons, CL_TICDIFF_BUTTONS);

    Packet_Write8(packet, cmd->msec);

#undef WRITE_TICCMD16
#undef WRITE_TICCMD8
#undef DIFF_TICCMDS
}

//
// CL_BuildTiccmd
//

static void CL_BuildTiccmd(void)
{
    ticcmd_t cmd;
    control_t *ctrl;
    int msec;
    ENetPacket *packet;

    memset(&cmd, 0, sizeof(ticcmd_t));
    ctrl = &control;

    if(ctrl->key[KEY_ATTACK])
    {
        cmd.buttons |= BT_ATTACK;
    }

    if(ctrl->key[KEY_JUMP])
    {
        cmd.buttons |= BT_JUMP;
    }

    if(ctrl->key[KEY_CENTER])
    {
        cmd.buttons |= BT_CENTER;
    }

    msec = (int)(kex_client.runtime * 1000);
    if(msec > 250)
    {
        msec = 100;
    }

    cmd.msec = msec;

    if(!(packet = Packet_New()))
    {
        return;
    }

    Packet_Write8(packet, CLIENT_PACKET_CMD);

    CL_WriteTiccmd(packet, &cmd);
    kex_client.cmd = cmd;
    Packet_Send(packet, kex_client.peer);
}

//
// CL_ProcessServerPackets
//

void CL_ProcessServerPackets(ENetPacket *packet, ENetEvent *cev)
{
    int type = 0;

    Packet_Read8(packet, &type);

    switch(type)
    {
    case SERVER_PACKET_PING:
        Com_Printf("Recieved acknowledgement from server\n");
        break;
    default:
        Com_Warning("Recieved unknown packet type: %i\n", type);
        break;
    }

    enet_packet_destroy(cev->packet);
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

        case ENET_EVENT_TYPE_RECEIVE:
            CL_ProcessServerPackets(cev.packet, &cev);
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

    CL_CheckHost();

    IN_PollInput();

    CL_ProcessEvents();

    CL_BuildTiccmd();

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
