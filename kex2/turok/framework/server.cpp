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
#include "js.h"

kexCvar cvarServerAddress("sv_address", CVF_STRING|CVF_CONFIG, "localhost", "TODO");
kexCvar cvarServerPort("sv_port", CVF_INT|CVF_CONFIG, "58304", "TODO");
kexCvar cvarMaxPeers("sv_maxpeers", CVF_INT|CVF_CONFIG, "1", 1, MAX_PLAYERS, "TODO");
kexCvar cvarHostName("sv_hostname", CVF_STRING|CVF_CONFIG, "defaulthost", "TODO");
kexCvar cvarServerInWidth("sv_inwidth", CVF_INT|CVF_CONFIG, "0", "TODO");
kexCvar cvarServerOutWidth("sv_outwidth", CVF_INT|CVF_CONFIG, "0", "TODO");

kexServer server;

//
// kexServer::Destroy
//

void kexServer::Destroy(void) {
    if(GetHost()) {
        enet_host_destroy(GetHost());
        SetHost(NULL);
        SetState(SV_STATE_UNAVAILABLE);
    }
}

//
// kexServer::Shutdown
//

void kexServer::Shutdown(void) {
    Destroy();
    enet_deinitialize();
}

//
// kexServer::GetPeerAddress
//

char *kexServer::GetPeerAddress(ENetEvent *sev) {
    char ip[32];

    enet_address_get_host_ip(
        (ENetAddress*)&sev->peer->address, ip, 32);

    return kva("%s:%u", ip, sev->peer->address.port);
}

//
// kexServer::GetClientID
//

unsigned int kexServer::GetClientID(ENetPeer *peer) const {
    for(int i = 0; i < maxClients; i++) {
        if(players[i].GetState() == SVC_STATE_INACTIVE)
            continue;
        if(peer->connectID == players[i].GetID())
            return i;
    }

   return 0;
}

//
// kexServer::CreateHost
//

void kexServer::CreateHost(void) {
    ENetAddress address;

    if(common.CheckParam("-client"))
        return;

    Destroy();

    enet_address_set_host(&address, cvarServerAddress.GetValue());
    address.port = (uint16)cvarServerPort.GetInt();

    SetHost(enet_host_create(&address,
        cvarMaxPeers.GetInt(), 2,
        (uint32)cvarServerInWidth.GetInt(),
        (uint32)cvarServerOutWidth.GetInt()
        ));

    if(!GetHost()) {
        common.Error("kexServer::CreateHost: Failed");
        return;
    }

    SetState(SV_STATE_ACTIVE);
    maxClients = cvarMaxPeers.GetInt();
}

//
// kexServer::ClientCommand
//

void kexServer::ClientCommand(ENetEvent *sev, ENetPacket *packet) {
    char *cmd = packetManager.ReadString(packet);
    /*svclient_t *svcl;

    svcl = &clients[GetClientID(sev->peer)];
    common.DPrintf("client command: %s (%s)\n", cmd, GetPeerAddress(sev));*/
}

//
// kexServer::SendMessage
//

void kexServer::SendMessage(ENetEvent *sev, int type) {
    ENetPacket *packet;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, sp_msg);
    packetManager.Write8(packet, type);
    packetManager.Send(packet, sev->peer);
}

//
// kexServer::SendAcknowledgement
//

void kexServer::SendAcknowledgement(ENetEvent *sev) {
    ENetPacket *packet;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, sp_ping);
    packetManager.Send(packet, sev->peer);
}

//
// kexServer::ProcessPackets
//

void kexServer::ProcessPackets(const ENetPacket *packet) {
    unsigned int type = 0;
    ENetEvent *netEvent = GetEvent();

    packetManager.Read8((ENetPacket*)packet, &type);

    switch(type)
    {
    case cp_ping:
        common.Printf("Recieved ping from %s (channel %i)\n",
            GetPeerAddress(netEvent), netEvent->channelID);
        SendAcknowledgement(netEvent);
        break;

    case cp_say:
        common.Printf("%s: %s\n", GetPeerAddress(netEvent),
            packetManager.ReadString((ENetPacket*)packet));
        break;

    case cp_cmd:
        P_RunCommand(netEvent, (ENetPacket*)packet);
        break;

    case cp_msgserver:
        ClientCommand(netEvent, (ENetPacket*)packet);
        break;

    case cp_changeweapon:
        break;

    default:
        common.Warning("Recieved unknown packet type: %i\n", type);
        break;
    }

    DestroyPacket();
}

//
// kexServer::SendClientMessages
//

void kexServer::SendClientMessages(void) {
    for(int i = 0; i < maxClients; i++) {
        /*if(players[i].State() != SVC_STATE_INACTIVE) {
            SendMoveData(&clients[i]);
        }*/
    }
}

//
// kexServer::OnConnect
//

void kexServer::OnConnect(void) {
    ENetEvent *sev = GetEvent();
    for(int i = 0; i < GetMaxClients(); i++) {
        if(players[i].GetState() == SVC_STATE_INACTIVE) {
            kexNetPlayer *player = &players[i];
            ENetPacket *packet;

            player->SetState(SVC_STATE_ACTIVE);
            player->SetPeer(sev->peer);
            player->SetID(sev->peer->connectID);
            player->ResetNetSequence();
            player->ResetTicCommand();

            if(!(packet = packetManager.Create()))
                return;

            common.Printf("%s connected...\n",
                GetPeerAddress(sev));

            packetManager.Write8(packet, sp_clientinfo);
            packetManager.Write8(packet, player->GetID());
            packetManager.Send(packet, player->GetPeer());
            return;
        }
    }

    SendMessage(sev, sm_full);
}

//
// kexServer::OnDisconnect
//

void kexServer::OnDisconnect(void) {
    common.Printf("%s disconnected...\n",
        GetPeerAddress(GetEvent()));
}

//
// kexServer::SendMoveData
//

/*void kexServer::SendMoveData(svclient_t *svcl) {
    ENetPacket *packet;

    if(svcl->state != SVC_STATE_INGAME)
        return;

    if(!(packet = packetManager.Create()))
        return;
    
    packetManager.Write8(packet, sp_pmove);
    packetManager.Write32(packet, server.tics);
    packetManager.WriteVector(packet, svcl->pmove.origin);
    packetManager.WriteVector(packet, svcl->pmove.velocity);
    packetManager.Write32(packet, svcl->pmove.flags);
    packetManager.Write32(packet, svcl->pmove.movetype);
    packetManager.Write32(packet, svcl->pmove.plane);
    packetManager.Write32(packet, svcl->ns.ingoing);
    packetManager.Write32(packet, svcl->ns.outgoing);
    svcl->ns.outgoing++;

    packetManager.Send(packet, svcl->peer);
}*/

//
// kexServer::Run
//

void kexServer::Run(int msec) {
    if(GetState() != SV_STATE_ACTIVE)
        return;

    SetRunTime(GetRunTime() + msec);
    server.elaspedTime += msec;

    CheckMessages();

    if(GetRunTime() < GetTime()) {
        if(GetTime() - GetRunTime() > SERVER_RUNTIME) {
            SetRunTime((float)GetTime() - SERVER_RUNTIME);
        }
        return;
    }

    SetTicks(GetTicks() + 1);
    SetTime(GetTicks() * SERVER_RUNTIME);

    G_Ticker();

    if(GetTime() < GetRunTime()) {
        SetRunTime((float)GetTime());
    }
}

//
// kexServer::Init
//

void kexServer::Init(void) {
    if(enet_initialize() != 0) {
        common.Error("kexServer::Init: Failed to initialize enet");
        return;
    }

    bLocal = (common.CheckParam("-server") == 0);
    maxClients = 0;

    SetHost(NULL);
    SetTime(0);
    SetRunTime(0);
    SetState(SV_STATE_UNAVAILABLE);
}
