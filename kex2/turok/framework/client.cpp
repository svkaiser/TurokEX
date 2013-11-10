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
#include "system.h"
#include "sound.h"
#include "fx.h"
#include "network.h"
#include "console.h"
#include "world.h"
#include "renderSystem.h"
#include "renderWorld.h"

kexCvar cvarClientName("cl_name", CVF_STRING|CVF_CONFIG, "Player", "Name for client player");
kexCvar cvarClientFOV("cl_fov", CVF_FLOAT|CVF_CONFIG, "74.0", "Client's field of view");
kexCvar cvarPaused("cl_paused", CVF_BOOL|CVF_CHEAT|CVF_CONFIG, "0", "Pause the game");
kexCvar cvarMSensitivityX("cl_msensitivityx", CVF_FLOAT|CVF_CONFIG, "5", "Mouse-X sensitivity");
kexCvar cvarMSensitivityY("cl_msensitivityy", CVF_FLOAT|CVF_CONFIG, "5", "Mouse-Y sensitivity");
kexCvar cvarMAcceleration("cl_macceleration", CVF_FLOAT|CVF_CONFIG, "0", "Mouse acceleration");
kexCvar cvarInvertLook("cl_mlookinvert", CVF_BOOL|CVF_CONFIG, "0", "Invert mouse-look");
kexCvar cvarClientPort("cl_port", CVF_INT|CVF_CONFIG, "58304", "Client's forwarding port");
kexCvar cvarClientFPS("cl_maxfps", CVF_INT|CVF_CONFIG, "60", 1, 60, "Game render FPS");

kexClient client;

//
// kexClient::Destroy
//

void kexClient::Destroy(void) {
    if(GetHost()) {
        if(GetPeer() != NULL) {
            enet_peer_disconnect(GetPeer(), 0);
            enet_peer_reset(GetPeer());
        }

        DestroyHost();
        SetState(CL_STATE_DISCONNECTED);
    }
}

//
// kexClient::Shutdown
//

void kexClient::Shutdown(void) {
    Destroy();
}

//
// kexClient::Connect
//

void kexClient::Connect(const char *address) {
    ENetAddress addr;
    char ip[32];

    enet_address_set_host(&addr, address);
    addr.port = cvarClientPort.GetInt();
    SetPeer(enet_host_connect(GetHost(), &addr, 2, 0));
    playerClient.SetPeer(GetPeer());
    enet_address_get_host_ip(&addr, ip, 32);
    
    common.Printf("Connecting to %s:%u...\n", ip, addr.port);

    if(GetPeer() == NULL) {
        common.Warning("No available peers for initiating an ENet connection.\n");
        return;
    }

    SetState(CL_STATE_CONNECTING);
}

//
// kexClient::PrepareMapChange
//

void kexClient::PrepareMapChange(const ENetPacket *packet) {
    // TEMP
    unsigned int mapID;

    packetManager.Read8((ENetPacket*)packet, &mapID);
    client.SetState(CL_STATE_CHANGINGLEVEL);

    localWorld.Load(kva("maps/map%02d/map%02d.kmap", mapID, mapID));
    client.SetState(CL_STATE_INGAME);
    localWorld.SpawnLocalPlayer();
}

//
// kexClient::ProcessPackets
//

void kexClient::ProcessPackets(const ENetPacket *packet) {
    unsigned int type = 0;

    packetManager.Read8((ENetPacket*)packet, &type);

    switch(type) {
    case sp_ping:
        common.Printf("Recieved acknowledgement from server\n");
        break;

    case sp_clientinfo:
        packetManager.Read8((ENetPacket*)packet, &id);
        playerClient.SetID(id);
        SetState(CL_STATE_READY);
        common.DPrintf("CL_ReadClientInfo: ID is %i\n", id);
        break;

    case sp_changemap:
        PrepareMapChange(packet);
        break;

    default:
        common.Warning("Recieved unknown packet type: %i\n", type);
        break;
    }

    DestroyPacket();
}

//
// kexClient::OnConnect
//

void kexClient::OnConnect(void) {
    SetState(CL_STATE_CONNECTED);
    playerClient.ResetNetSequence();
}

//
// kexClient::OnDisconnect
//

void kexClient::OnDisconnect(void) {
    SetState(CL_STATE_DISCONNECTED);
}

//
// kexClient::Run
//

void kexClient::Run(const int msec) {

    curtime += msec;

    if(curtime < (1000 / cvarClientFPS.GetInt()))
        return;

    SetRunTime((float)curtime / 1000.0f);
    SetTime(GetTime() + curtime);

    fps = (int)(1000.0f / (GetRunTime() * 1000.0f));
    curtime = 0;

    // check for new packets
    CheckMessages();

    // check for new inputs
    inputSystem.PollInput();

    // handle input events
    ProcessEvents();

    // prep and send input information to server
    playerClient.BuildCommands();

    // run client-side ticks
    playerClient.LocalTick();
    console.Tick();
    localWorld.LocalTick();

    // draw
    renderWorld.RenderScene();
    renderSystem.SetOrtho();
    console.Draw();

    // finish frame
    inputSystem.UpdateGrab();
    renderSystem.SwapBuffers();

    // update all sound sources
    soundSystem.UpdateListener();

    UpdateTicks();
}

//
// kexClient::MessageServer
//

void kexClient::MessageServer(char *string) {
    ENetPacket *packet;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, cp_msgserver);
    packetManager.WriteString(packet, string);
    packetManager.Send(packet, GetPeer());
}

//
// kexClient::CreateHost
//

void kexClient::CreateHost(void) {
    SetHost(enet_host_create(NULL, 1, 2, 0, 0));

    if(!GetHost()) {
        common.Error("kexClient::Init: failed to create client");
    }
}

//
// kexClient::PostEvent
// Called by the I/O functions when input is detected
//

void kexClient::PostEvent(event_t *ev) {
    events[eventhead] = *ev;
    eventhead = (++eventhead)&(MAXEVENTS-1);
}

//
// kexClient::GetEvent
//

event_t *kexClient::GetEvent(void) {
    event_t *ev;

    if(eventtail == eventhead)
        return NULL;

    ev = &events[eventtail];
    eventtail = (++eventtail)&(MAXEVENTS-1);

    return ev;
}

//
// kexClient::ProcessEvents
// Send all the events of the given timestamp down the responder chain
//

void kexClient::ProcessEvents(void) {
    event_t *ev;
    event_t *oldev = NULL; // TEMP
    
    while((ev = GetEvent()) != NULL) {
        if(ev == oldev)
            return;

        if(console.ProcessInput(ev))
            continue;

        // TODO - TEMP
        eventtail = (--eventtail)&(MAXEVENTS-1);
        playerClient.ProcessInput(GetEvent());

        oldev = ev;
    }
}

//
// kexClient::InitObject
//

void kexClient::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kClient",
        sizeof(kexClient),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);

    scriptManager.Engine()->RegisterObjectMethod(
        "kClient",
        "int GetState(void)",
        asMETHODPR(kexClient, GetState, (void), int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kClient",
        "bool IsLocal(void)",
        asMETHODPR(kexClient, IsLocal, (void), bool),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kClient",
        "float GetRunTime(void)",
        asMETHODPR(kexClient, GetRunTime, (void), float),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kClient",
        "int GetTicks(void)",
        asMETHODPR(kexClient, GetTicks, (void), int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kClient",
        "int GetTime(void)",
        asMETHODPR(kexClient, GetTime, (void), int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty(
        "kClient",
        "int id",
        asOFFSET(kexClient, id));

    scriptManager.Engine()->RegisterGlobalProperty("kClient Client", &client);

    scriptManager.Engine()->RegisterEnum("EnumClientState");
    scriptManager.Engine()->RegisterEnumValue("EnumClientState", "STATE_UNITIALIZED", CL_STATE_UNINITIALIZED);
    scriptManager.Engine()->RegisterEnumValue("EnumClientState", "STATE_CONNECTING", CL_STATE_CONNECTING);
    scriptManager.Engine()->RegisterEnumValue("EnumClientState", "STATE_CONNECTED", CL_STATE_CONNECTED);
    scriptManager.Engine()->RegisterEnumValue("EnumClientState", "STATE_DISCONNECTED", CL_STATE_DISCONNECTED);
    scriptManager.Engine()->RegisterEnumValue("EnumClientState", "STATE_READY", CL_STATE_READY);
    scriptManager.Engine()->RegisterEnumValue("EnumClientState", "STATE_INGAME", CL_STATE_INGAME);
    scriptManager.Engine()->RegisterEnumValue("EnumClientState", "STATE_CHANGINGLEVEL", CL_STATE_CHANGINGLEVEL);
}

//
// FCmd_Ping
//

static void FCmd_Ping(void) {
    ENetPacket *packet;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, cp_ping);
    packetManager.Send(packet, client.GetPeer());
}

//
// FCmd_Say
//

static void FCmd_Say(void) {
    ENetPacket *packet;

    if(command.GetArgc() < 2)
        return;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, cp_say);
    packetManager.WriteString(packet, command.GetArgv(1));
    packetManager.Send(packet, client.GetPeer());
}

//
// FCmd_MsgServer
//

static void FCmd_MsgServer(void) {
    if(command.GetArgc() < 2)
        return;

    client.MessageServer(command.GetArgv(1));
}

//
// kexClient::Init
//

void kexClient::Init(void) {
    Destroy();
    CreateHost();

    curtime = 0;
    fps = 0;
    id = -1;
    bLocal = (common.CheckParam("-client") == 0);
    
    SetTime(0);
    SetTicks(0);
    SetPeer(NULL);
    SetState(CL_STATE_UNINITIALIZED);

    command.Add("ping", FCmd_Ping);
    command.Add("say", FCmd_Say);
    command.Add("msgserver", FCmd_MsgServer);

    common.Printf("Client Initialized\n");
}
