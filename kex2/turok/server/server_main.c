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
// DESCRIPTION: Main Server Code
//
//-----------------------------------------------------------------------------

#include "enet/enet.h"
#include "common.h"
#include "client.h"
#include "packet.h"

CVAR(sv_address, localhost);
CVAR(sv_port, 58304);
CVAR(sv_maxpeers, 1);
CVAR(sv_hostname, defaulthost);
CVAR(sv_inwidth, 0);
CVAR(sv_outwidth, 0);

typedef enum
{
    SV_STATE_UNAVAILABLE,
    SV_STATE_BUSY,
    SV_STATE_ACTIVE
} server_state_e;

typedef struct
{
    ENetHost        *host;
    kbool           local;
    server_state_e  state;
    uint32          maxclients;
    int             time;
    float           runtime;
} kex_server_t;

kex_server_t kex_server;

//
// SV_DestroyHost
//

void SV_DestroyHost(void)
{
    if(kex_server.host)
    {
        enet_host_destroy(kex_server.host);
        kex_server.host = NULL;
        kex_server.state = SV_STATE_UNAVAILABLE;
    }
}

//
// SV_Shutdown
//

void SV_Shutdown(void)
{
    SV_DestroyHost();
    enet_deinitialize();
}

//
// SV_GetPeerAddress
//

static char *SV_GetPeerAddress(ENetEvent *sev)
{
    char ip[32];

    enet_address_get_host_ip(
        (ENetAddress*)&sev->peer->address, ip, 32);

    return kva("%s:%u", ip, sev->peer->address.port);
}

//
// SV_CreateHost
//

void SV_CreateHost(void)
{
    ENetAddress address;

    if(Com_CheckParam("-client"))
    {
        return;
    }

    SV_DestroyHost();

    enet_address_set_host(&address, sv_address.string);
    address.port = (uint16)sv_port.value;

    kex_server.host =  enet_host_create(&address,
        (int)sv_maxpeers.value, 2,
        (uint32)sv_inwidth.value,
        (uint32)sv_outwidth.value
        );

    if(!kex_server.host)
    {
        Com_Error("SV_CreateHost: Failed");
        return;
    }

    kex_server.state = SV_STATE_ACTIVE;
    kex_server.maxclients = (int)sv_maxpeers.value;
}

//
// SV_ReadTiccmd
//

static void SV_ReadTiccmd(ENetPacket *packet, ticcmd_t *cmd)
{
    int bits = 0;
    int tmp = 0;

    memset(cmd, 0, sizeof(ticcmd_t));

#define READ_TICCMD8(name, bit)             \
    if(bits & bit)                          \
    {                                       \
        Packet_Read8(packet, &tmp);         \
        cmd->name = tmp;                    \
    }

#define READ_TICCMD16(name, bit)            \
    if(bits & bit)                          \
    {                                       \
        Packet_Read16(packet, &tmp);        \
        cmd->name = tmp;                    \
    }

    Packet_Read8(packet, &bits);

    READ_TICCMD8(forwardmove, CL_TICDIFF_FORWARD);
    READ_TICCMD8(sidemove, CL_TICDIFF_SIDE);
    READ_TICCMD16(angleturn, CL_TICDIFF_TURN);
    READ_TICCMD16(pitch, CL_TICDIFF_PITCH);
    READ_TICCMD8(buttons, CL_TICDIFF_BUTTONS);

    Packet_Read8(packet, &tmp);
    cmd->msec = tmp;

#undef READ_TICCMD16
#undef READ_TICCMD8
}

//
// SV_SendAcknowledgement
//

static void SV_SendAcknowledgement(ENetEvent *sev)
{
    ENetPacket *packet;

    if(!(packet = Packet_New()))
    {
        return;
    }

    Packet_Write8(packet, SERVER_PACKET_PING);
    Packet_Send(packet, sev->peer);
}

//
// SV_ProcessClientPackets
//

void SV_ProcessClientPackets(ENetPacket *packet, ENetEvent *sev)
{
    int type = 0;

    Packet_Read8(packet, &type);

    switch(type)
    {
    case CLIENT_PACKET_PING:
        Com_Printf("Recieved ping from %s (channel %i)\n",
            SV_GetPeerAddress(sev), sev->channelID);
        SV_SendAcknowledgement(sev);
        break;

    case CLIENT_PACKET_SAY:
        Com_Printf("%s: %s\n", SV_GetPeerAddress(sev), Packet_ReadString(packet));
        break;

    case CLIENT_PACKET_CMD:
        {
            ticcmd_t cmd;

            SV_ReadTiccmd(packet, &cmd);
        }
        break;

    default:
        Com_Warning("Recieved unknown packet type: %i\n", type);
        break;
    }

    enet_packet_destroy(sev->packet);
}

//
// SV_Run
//

void SV_Run(int msec)
{
    ENetEvent sev;

    if(kex_server.state != SV_STATE_ACTIVE)
    {
        return;
    }

    while(enet_host_service(kex_server.host, &sev, 0) > 0)
    {
        switch(sev.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                {
                    Com_Printf("%s connected...\n",
                        SV_GetPeerAddress(&sev));
                }
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                {
                    Com_Printf("%s disconnected...\n",
                        SV_GetPeerAddress(&sev));
                }
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                SV_ProcessClientPackets(sev.packet, &sev);
                break;
        }
    }
}

//
// SV_Init
//

void SV_Init(void)
{
    if(enet_initialize() != 0)
    {
        Com_Error("SV_Init: Failed to initialize enet");
        return;
    }

    kex_server.host = NULL;
    kex_server.local = (Com_CheckParam("-server") == 0);
    kex_server.maxclients = 0;
    kex_server.time = 0;
    kex_server.state = SV_STATE_UNAVAILABLE;

    Cvar_Register(&sv_address);
    Cvar_Register(&sv_port);
    Cvar_Register(&sv_maxpeers);
    Cvar_Register(&sv_hostname);
    Cvar_Register(&sv_inwidth);
    Cvar_Register(&sv_outwidth);
}
