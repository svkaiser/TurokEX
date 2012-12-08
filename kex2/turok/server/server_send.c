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
// DESCRIPTION: Server Send Commands
//
//-----------------------------------------------------------------------------

#include "enet/enet.h"
#include "common.h"
#include "client.h"
#include "packet.h"
#include "server.h"

//
// SV_SendMsg
//

void SV_SendMsg(ENetEvent *sev, int type)
{
    ENetPacket *packet;

    if(!(packet = Packet_New()))
        return;

    Packet_Write8(packet, sp_msg);
    Packet_Write8(packet, type);
    Packet_Send(packet, sev->peer);
}

//
// SV_UpdateClientInfo
//

void SV_UpdateClientInfo(ENetEvent *sev, svclient_t *svc, int id)
{
    ENetPacket *packet;

    if(!(packet = Packet_New()))
        return;

    Com_Printf("%s connected...\n",
        SV_GetPeerAddress(sev));

    Packet_Write8(packet, sp_clientinfo);
    Packet_Write8(packet, svc->client_id);
    Packet_Write8(packet, id);
    Packet_Send(packet, svc->peer);
}

//
// SV_SendAcknowledgement
//

void SV_SendAcknowledgement(ENetEvent *sev)
{
    ENetPacket *packet;

    if(!(packet = Packet_New()))
        return;

    Packet_Write8(packet, sp_ping);
    Packet_Send(packet, sev->peer);
}

//
// SV_SendPMove
//

void SV_SendPMove(svclient_t *svcl)
{
    ENetPacket *packet;

    if(svcl->state != SVC_STATE_INGAME)
        return;

    if(!(packet = Packet_New()))
        return;
    
    Packet_Write8(packet, sp_pmove);
    Packet_Write32(packet, server.tics);
    Packet_WriteVector(packet, svcl->pmove.origin);
    Packet_WriteVector(packet, svcl->pmove.velocity);
    Packet_Write32(packet, svcl->pmove.flags);
    Packet_Write32(packet, svcl->pmove.movetype);
    Packet_Write32(packet, svcl->pmove.plane);
    Packet_Send(packet, svcl->peer);
}
