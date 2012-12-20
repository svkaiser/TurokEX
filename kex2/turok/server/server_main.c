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
#include "server.h"
#include "game.h"
#include "type.h"

CVAR(sv_address, localhost);
CVAR(sv_port, 58304);

CVAR_CMD(sv_maxpeers, 1)
{
    cvar->value = (float)(int)cvar->value;

    if(cvar->value <= 0)
    {
        cvar->value = 1;
    }
    else if(cvar->value > MAXCLIENTS)
    {
        cvar->value = (float)MAXCLIENTS;
    }
}

CVAR(sv_hostname, defaulthost);
CVAR(sv_inwidth, 0);
CVAR(sv_outwidth, 0);

svclient_t svclients[MAXCLIENTS];
server_t server;

//
// SV_DestroyHost
//

void SV_DestroyHost(void)
{
    if(server.host)
    {
        enet_host_destroy(server.host);
        server.host = NULL;
        server.state = SV_STATE_UNAVAILABLE;
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

char *SV_GetPeerAddress(ENetEvent *sev)
{
    char ip[32];

    enet_address_get_host_ip(
        (ENetAddress*)&sev->peer->address, ip, 32);

    return kva("%s:%u", ip, sev->peer->address.port);
}

//
// SV_ResetAllClients
//

static void SV_ResetAllClients(void)
{
    memset(svclients, 0, sizeof(svclient_t));
}

//
// SV_GetPlayerID
//

unsigned int SV_GetPlayerID(ENetPeer *peer)
{
    unsigned int i;
    
    for(i = 0; i < server.maxclients; i++)
    {
        if(svclients[i].state == SVC_STATE_INACTIVE)
            continue;

        if(peer->connectID == svclients[i].client_id)
            return i;
    }

   return 0;
}

//
// SV_AddClient
//

static void SV_AddClient(ENetEvent *sev)
{
    unsigned int i;

    for(i = 0; i < server.maxclients; i++)
    {
        if(svclients[i].state == SVC_STATE_INACTIVE)
        {
            svclients[i].state = SVC_STATE_ACTIVE;
            svclients[i].peer = sev->peer;
            svclients[i].client_id = sev->peer->connectID;
            svclients[i].ns.ingoing = 0;
            svclients[i].ns.outgoing = 1;
            memset(&svclients[i].cmd, 0, sizeof(ticcmd_t));

            SV_UpdateClientInfo(sev, &svclients[i], i);
            return;
        }
    }

    SV_SendMsg(sev, sm_full);
}

//
// SV_CreateHost
//

void SV_CreateHost(void)
{
    ENetAddress address;

    if(Com_CheckParam("-client"))
        return;

    SV_DestroyHost();

    enet_address_set_host(&address, sv_address.string);
    address.port = (uint16)sv_port.value;

    server.host =  enet_host_create(&address,
        (int)sv_maxpeers.value, 2,
        (uint32)sv_inwidth.value,
        (uint32)sv_outwidth.value
        );

    if(!server.host)
    {
        Com_Error("SV_CreateHost: Failed");
        return;
    }

    server.state = SV_STATE_ACTIVE;
    server.maxclients = (int)sv_maxpeers.value;
}

//
// SV_ReadTiccmd
//

static void SV_ReadTiccmd(ENetEvent *sev, ENetPacket *packet)
{
    int bits = 0;
    int tmp = 0;
    int i;
    ticcmd_t cmd;
    svclient_t *svcl;

    memset(&cmd, 0, sizeof(ticcmd_t));

#define READ_TICCMD8(name, bit)             \
    if(bits & bit)                          \
    {                                       \
        Packet_Read8(packet, &tmp);         \
        cmd.name = tmp;                     \
    }

#define READ_TICCMD16(name, bit)            \
    if(bits & bit)                          \
    {                                       \
        Packet_Read16(packet, &tmp);        \
        cmd.name = tmp;                     \
    }

#define READ_TICCMD32(name, bit)            \
    if(bits & bit)                          \
    {                                       \
        Packet_Read32(packet, &tmp);        \
        cmd.name = tmp;                     \
    }

    Packet_Read8(packet, &bits);

    READ_TICCMD32(angle[0].i, CL_TICDIFF_TURN1);
    READ_TICCMD32(angle[1].i, CL_TICDIFF_TURN2);
    READ_TICCMD16(buttons, CL_TICDIFF_BUTTONS);

    Packet_Read32(packet, &tmp);
    cmd.msec.i = tmp;

    for(i = 0; i < NUM_CTRLKEYS; i++)
    {
        Packet_Read8(packet, &tmp);
        cmd.heldtime[i] = tmp;
    }

    svcl = &svclients[SV_GetPlayerID(sev->peer)];
    memcpy(&svcl->cmd, &cmd, sizeof(ticcmd_t));

    Packet_Read32(packet, &svcl->ns.acks);
    Packet_Read32(packet, &svcl->ns.ingoing);

#undef READ_TICCMD16
#undef READ_TICCMD8
}

//
// SV_ClientCommand
//

static void SV_ClientCommand(ENetEvent *sev, ENetPacket *packet)
{
    char *cmd = Packet_ReadString(packet);
    svclient_t *svcl;

    svcl = &svclients[SV_GetPlayerID(sev->peer)];
    Com_DPrintf("client command: %s (%s)\n", cmd, SV_GetPeerAddress(sev));

    if(!strcmp(cmd, "noclip"))
    {
        G_NoClip(svcl);
    }
    else if(!strcmp(cmd, "giveall"))
    {
        G_GiveAll(svcl);
    }
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
    case cp_ping:
        Com_Printf("Recieved ping from %s (channel %i)\n",
            SV_GetPeerAddress(sev), sev->channelID);
        SV_SendAcknowledgement(sev);
        break;

    case cp_say:
        Com_Printf("%s: %s\n", SV_GetPeerAddress(sev), Packet_ReadString(packet));
        break;

    case cp_cmd:
        SV_ReadTiccmd(sev, packet);
        Pred_ServerMovement();
        break;

    case cp_msgserver:
        SV_ClientCommand(sev, packet);
        break;

    case cp_changeweapon:
        G_SwitchWeapon(sev, packet);
        break;

    default:
        Com_Warning("Recieved unknown packet type: %i\n", type);
        break;
    }

    enet_packet_destroy(sev->packet);
}

//
// SV_SendClientMessages
//

static void SV_SendClientMessages(void)
{
    unsigned int i;

    for(i = 0; i < server.maxclients; i++)
    {
        if(svclients[i].state != SVC_STATE_INACTIVE)
        {
            SV_SendPMove(&svclients[i]);
        }
    }
}

//
// SV_ReadPackets
//

static void SV_ReadPackets(void)
{
    ENetEvent sev;

    while(enet_host_service(server.host, &sev, 0) > 0)
    {
        switch(sev.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                SV_AddClient(&sev);
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
// SV_Ticker
//

static void SV_Ticker(void)
{
    if(server.runtime < server.time)
    {
        if(server.time - server.runtime > 100)
            server.runtime = server.time - 100;

        return;
    }

    server.tics++;
    server.time = server.tics * 100;

    G_Ticker();
}

//
// SV_Run
//

void SV_Run(int msec)
{
    if(server.state != SV_STATE_ACTIVE)
        return;

    server.runtime += msec;

    SV_ReadPackets();

    SV_SendClientMessages();

    SV_Ticker();
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

    server.host = NULL;
    server.local = (Com_CheckParam("-server") == 0);
    server.maxclients = 0;
    server.time = 0;
    server.runtime = 0;
    server.state = SV_STATE_UNAVAILABLE;

    SV_ResetAllClients();

    Cvar_Register(&sv_address);
    Cvar_Register(&sv_port);
    Cvar_Register(&sv_maxpeers);
    Cvar_Register(&sv_hostname);
    Cvar_Register(&sv_inwidth);
    Cvar_Register(&sv_outwidth);
}
