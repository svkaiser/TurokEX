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
// DESCRIPTION: Player code
//
//-----------------------------------------------------------------------------

#include "enet/enet.h"
#include "js.h"
#include "jsobj.h"
#include "js_shared.h"
#include "common.h"
#include "kernel.h"
#include "packet.h"
#include "client.h"
#include "server.h"
#include "actor.h"
#include "controller.h"
#include "player.h"
#include "zone.h"

localPlayer_t localPlayer;
netPlayer_t netPlayers[MAX_PLAYERS];

//
// P_ResetNetSeq
//

void P_ResetNetSeq(playerInfo_t *info)
{
    info->netseq.ingoing = 0;
    info->netseq.outgoing = 1;
}

//
// P_Responder
//

kbool P_Responder(event_t *ev)
{
    if(Con_ProcessConsoleInput(ev))
        return true;

    switch(ev->type)
    {
    case ev_mouse:
        IN_MouseMove(ev->data2, ev->data3);
        return true;

    case ev_keydown:
        Key_ExecCmd(ev->data1, false);
        return true;

    case ev_keyup:
        Key_ExecCmd(ev->data1, true);
        return true;

    case ev_mousedown:
        Key_ExecCmd(ev->data1, false);
        return true;

    case ev_mouseup:
        Key_ExecCmd(ev->data1, true);
        return true;
    }

    return false;
}

//
// P_GetNetPlayer
//

netPlayer_t *P_GetNetPlayer(ENetPeer *peer)
{
    unsigned int i;
    
    for(i = 0; i < server.maxclients; i++)
    {
        if(netPlayers[i].state == SVC_STATE_INACTIVE)
            continue;

        if(peer->connectID == netPlayers[i].info.id)
            return &netPlayers[i];
    }

   return &netPlayers[0];
}

//
// P_NewPlayerConnected
//

void P_NewPlayerConnected(ENetEvent *sev)
{
    unsigned int i;

    for(i = 0; i < server.maxclients; i++)
    {
        if(netPlayers[i].state == SVC_STATE_INACTIVE)
        {
            playerInfo_t *info = &netPlayers[i].info;
            ENetPacket *packet;

            netPlayers[i].state = SVC_STATE_ACTIVE;
            info->peer = sev->peer;
            info->id = sev->peer->connectID;
            P_ResetNetSeq(info);
            memset(&info->cmd, 0, sizeof(ticcmd_t));

            if(!(packet = Packet_New()))
                return;

            Com_Printf("%s connected...\n",
                SV_GetPeerAddress(sev));

            Packet_Write8(packet, sp_clientinfo);
            Packet_Write8(packet, info->id);
            Packet_Send(packet, info->peer);
            return;
        }
    }

    SV_SendMsg(sev, sm_full);
}

//
// P_BuildCommands
//

void P_BuildCommands(void)
{
    ENetPacket *packet;
    playerInfo_t *pinfo;
    ticcmd_t *cmd;
    control_t *ctrl;
    int numactions;
    int i;

    if(client.state != CL_STATE_INGAME)
        return;

    pinfo = &localPlayer.info;
    cmd = &pinfo->cmd;
    ctrl = &control;

    for(i = 0; i < MAXACTIONS; i++)
        cmd->buttons[i] = false;

    for(i = 0; i < MAXACTIONS; i++)
    {
        cmd->buttons[i] = (ctrl->actions[i]) ? true : false;
        if(cmd->buttons[i] == true)
        {
            if(cmd->heldtime[i] < 255)
                cmd->heldtime[i]++;
            else
                cmd->heldtime[i] = 255;
        }
        else
            cmd->heldtime[i] = 0;
    }

    cmd->timestamp.i = client.time;
    cmd->frametime.f = client.runtime;
    cmd->mouse[0].f = ctrl->mousex;
    cmd->mouse[1].f = ctrl->mousey;

    ctrl->mousex = 0;
    ctrl->mousey = 0;

    if(!(packet = Packet_New()))
        return;

    Packet_Write8(packet, cp_cmd);

    numactions = 0;

    for(i = 0; i < MAXACTIONS; i++)
    {
        if(cmd->buttons[i])
            numactions++;
    }

    Packet_Write32(packet, cmd->mouse[0].i);
    Packet_Write32(packet, cmd->mouse[1].i);
    Packet_Write32(packet, cmd->timestamp.i);
    Packet_Write32(packet, cmd->frametime.i);
    Packet_Write32(packet, numactions);

    for(i = 0; i < MAXACTIONS; i++)
    {
        if(cmd->buttons[i])
        {
            Packet_Write8(packet, i);
            Packet_Write8(packet, cmd->heldtime[i]);
        }
    }

    Packet_Write32(packet, pinfo->netseq.ingoing);
    Packet_Write32(packet, pinfo->netseq.outgoing);

    pinfo->netseq.outgoing++;

    Packet_Send(packet, pinfo->peer);
}

//
// P_RunCommand
//

void P_RunCommand(ENetEvent *sev, ENetPacket *packet)
{
    netPlayer_t *netplayer;
    ticcmd_t *cmd;
    int numactions;
    int i;

    netplayer = &netPlayers[SV_GetPlayerID(sev->peer)];
    cmd = &netplayer->info.cmd;

    Packet_Read32(packet, &cmd->mouse[0].i);
    Packet_Read32(packet, &cmd->mouse[1].i);
    Packet_Read32(packet, &cmd->timestamp.i);
    Packet_Read32(packet, &cmd->frametime.i);

    Packet_Read32(packet, &numactions);

    for(i = 0; i < numactions; i++)
    {
        int btn = 0;
        int held = 0;

        Packet_Read8(packet, &btn);
        cmd->buttons[btn] = true;
        Packet_Read8(packet, &held);
        cmd->heldtime[btn] = held;
    }

    Packet_Read32(packet, &netplayer->info.netseq.acks);
    Packet_Read32(packet, &netplayer->info.netseq.ingoing);

    // TODO - PROCESS MOVEMENT
}

//
// P_LocalPlayerEvent
//

void P_LocalPlayerEvent(const char *eventName)
{
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval rval;

    if(client.state != CL_STATE_INGAME)
        return;

    cx = js_context;

    if(!JS_GetProperty(cx, localPlayer.playerObject, eventName, &val))
        return;
    if(!JS_ValueToObject(cx, val, &function))
        return;

    JS_CallFunctionValue(cx, localPlayer.playerObject,
        OBJECT_TO_JSVAL(function), 0, NULL, &rval);
}

//
// P_LocalPlayerTick
//

void P_LocalPlayerTick(void)
{
    worldState_t *ws;
    playerInfo_t *info;
    int current;

    if(client.state != CL_STATE_INGAME)
        return;

    ws = &localPlayer.worldState;
    info = &localPlayer.info;

    ws->timeStamp = (float)info->cmd.timestamp.i;
    ws->frameTime = info->cmd.frametime.f;

    // update controller in case movement was corrected by server
    Vec_Copy3(ws->origin, localPlayer.actor->origin);

    P_LocalPlayerEvent("onLocalTick");

    // TODO - AREA/LOCALTICK

    current = (info->netseq.outgoing-1) & (NETBACKUPS-1);
    Vec_Copy3(localPlayer.oldMoves[current], ws->origin);
    localPlayer.oldCmds[current] = info->cmd;
    localPlayer.latency[current] = client.time;
    localPlayer.actor->plane = (ws->plane - gLevel.planes);

    Vec_Copy3(localPlayer.actor->origin, ws->origin);
    Vec_Copy3(localPlayer.actor->angles, ws->angles);
    Actor_UpdateTransform(localPlayer.actor);
}

//
// P_SpawnLocalPlayer
//

void P_SpawnLocalPlayer(void)
{
    jsid id;
    jsval val;
    JSScopeProperty *sprop;
    int playerID;
    gActor_t *camera;
    gActor_t *pStart;
    gObject_t *pObject;
    worldState_t *ws;

    playerID = -1;
    pStart = NULL;
    pObject = NULL;

    for(gLevel.actorRover = gLevel.actorRoot.next;
        gLevel.actorRover != &gLevel.actorRoot;
        gLevel.actorRover = gLevel.actorRover->next)
    {
        gActor_t *actor = gLevel.actorRover;

        if(actor->components == NULL)
            continue;

        JS_GetReservedSlot(js_context, actor->iterator, 0, &val);
        sprop = (JSScopeProperty*)JS_GetPrivate(js_context, actor->iterator);

        while(JS_NextProperty(js_context, actor->iterator, &id))
        {
            jsval vp;
            kbool found;
            gObject_t *obj;
            gObject_t *component;

            if(id == JSVAL_VOID)
                break;

            if(!JS_GetMethodById(js_context, actor->components, id, &obj, &vp))
                continue;
            if(!JS_ValueToObject(js_context, vp, &component))
                continue;
            if(component == NULL)
                continue;
            if(!JS_HasProperty(js_context, component, "playerID", &found))
                continue;
            if(!found)
                continue;
            if(!JS_GetProperty(js_context, component, "playerID", &vp))
                continue;
            if(!(JSVAL_IS_INT(vp)))
                continue;
            
            playerID = INT_TO_JSVAL(vp);
            pObject = component;
            break;
        }

        JS_SetReservedSlot(js_context, actor->iterator, 0, val);
        JS_SetPrivate(js_context, actor->iterator, sprop);

        if(playerID != -1)
        {
            pStart = actor;
            break;
        }
    }

    if(pStart == NULL)
        Com_Error("P_SpawnLocalPlayer: No player start has been found");

    client.playerActor = pStart;
    localPlayer.actor = pStart;
    localPlayer.playerObject = pObject;

    ws = &localPlayer.worldState;

    Vec_Copy3(ws->origin, pStart->origin);
    Vec_Copy3(ws->angles, pStart->angles);
    Vec_Set3(ws->velocity, 0, 0, 0);
    Vec_Set3(ws->accel, 0, 0, 0);
    ws->actor = localPlayer.actor;

    if(pStart->plane != -1)
        ws->plane = &gLevel.planes[pStart->plane];

    // spawn an actor to be used for the camera
    camera = (gActor_t*)Z_Calloc(sizeof(gActor_t), PU_ACTOR, NULL);
    camera->bCollision = false;
    camera->bHidden = true;
    camera->bTouch = false;
    camera->bClientOnly = true;
    camera->plane = -1;
    strcpy(camera->name, "Camera");
    Vec_Copy3(camera->origin, pStart->origin);
    Vec_Copy3(camera->angles, pStart->angles);
    Vec_Copy4(camera->rotation, pStart->rotation);
    Map_AddActor(&gLevel, camera);
    localPlayer.camera = camera;

    client.player = &localPlayer;
}
