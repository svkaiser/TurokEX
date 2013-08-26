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
#include "system.h"
#include "packet.h"
#include "client.h"
#include "server.h"
#include "actor.h"
#include "player.h"
#include "zone.h"
#include "console.h"

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
    if(console.ProcessInput(ev))
        return true;

    switch(ev->type)
    {
    case ev_mouse:
        inputSystem.MoveMouse(ev->data2, ev->data3);
        return true;

    case ev_keydown:
        inputKey.ExecuteCommand(ev->data1, false);
        return true;

    case ev_keyup:
        inputKey.ExecuteCommand(ev->data1, true);
        return true;

    case ev_mousedown:
        inputKey.ExecuteCommand(ev->data1, false);
        return true;

    case ev_mouseup:
        inputKey.ExecuteCommand(ev->data1, true);
        return true;
    }

    return false;
}

//
// P_GetNetPlayer
//

netPlayer_t *P_GetNetPlayer(ENetPeer *peer)
{
    for(int i = 0; i < server.GetMaxClients(); i++)
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
    for(int i = 0; i < server.GetMaxClients(); i++)
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

            if(!(packet = packetManager.Create()))
                return;

            common.Printf("%s connected...\n",
                server.GetPeerAddress(sev));

            packetManager.Write8(packet, sp_clientinfo);
            packetManager.Write8(packet, info->id);
            packetManager.Send(packet, info->peer);
            return;
        }
    }

    server.SendMessage(sev, sm_full);
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

    if(client.GetState() != CL_STATE_INGAME)
        return;

    pinfo = &localPlayer.info;
    cmd = &pinfo->cmd;
    ctrl = &control;

    for(i = 0; i < MAXACTIONS; i++)
        cmd->buttons[i] = false;

    for(i = 0; i < MAXACTIONS; i++)
    {
        cmd->buttons[i] = (ctrl->actions[i]) ? true : false;
        if(cmd->buttons[i] == 1)
        {
            if(cmd->heldtime[i] < 255)
                cmd->heldtime[i]++;
            else
                cmd->heldtime[i] = 255;
        }
        else
            cmd->heldtime[i] = 0;
    }

    cmd->timestamp.i = client.GetTime();
    cmd->frametime.f = client.GetRunTime();
    cmd->mouse[0].f = ctrl->mousex;
    cmd->mouse[1].f = ctrl->mousey;

    ctrl->mousex = 0;
    ctrl->mousey = 0;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, cp_cmd);

    numactions = 0;

    for(i = 0; i < MAXACTIONS; i++)
    {
        if(cmd->buttons[i])
            numactions++;
    }

    packetManager.Write32(packet, cmd->mouse[0].i);
    packetManager.Write32(packet, cmd->mouse[1].i);
    packetManager.Write32(packet, cmd->timestamp.i);
    packetManager.Write32(packet, cmd->frametime.i);
    packetManager.Write32(packet, numactions);

    for(i = 0; i < MAXACTIONS; i++)
    {
        if(cmd->buttons[i])
        {
            packetManager.Write8(packet, i);
            packetManager.Write8(packet, cmd->heldtime[i]);
        }
    }

    packetManager.Write32(packet, pinfo->netseq.ingoing);
    packetManager.Write32(packet, pinfo->netseq.outgoing);

    pinfo->netseq.outgoing++;

    packetManager.Send(packet, pinfo->peer);
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

    netplayer = &netPlayers[server.GetClientID(sev->peer)];
    cmd = &netplayer->info.cmd;

    packetManager.Read32(packet, (unsigned int*)&cmd->mouse[0].i);
    packetManager.Read32(packet, (unsigned int*)&cmd->mouse[1].i);
    packetManager.Read32(packet, (unsigned int*)&cmd->timestamp.i);
    packetManager.Read32(packet, (unsigned int*)&cmd->frametime.i);
    packetManager.Read32(packet, (unsigned int*)&numactions);

    for(i = 0; i < numactions; i++)
    {
        int btn = 0;
        int held = 0;

        packetManager.Read8(packet, (unsigned int*)&btn);
        cmd->buttons[btn] = true;
        packetManager.Read8(packet, (unsigned int*)&held);
        cmd->heldtime[btn] = held;
    }

    packetManager.Read32(packet, (unsigned int*)&netplayer->info.netseq.acks);
    packetManager.Read32(packet, (unsigned int*)&netplayer->info.netseq.ingoing);

    // TODO - PROCESS MOVEMENT
}

//
// P_LocalPlayerEvent
//

int P_LocalPlayerEvent(const char *eventName)
{
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval rval;

    if(client.GetState() != CL_STATE_INGAME)
        return 0;

    cx = js_context;

    if(!JS_GetProperty(cx, localPlayer.playerObject, eventName, &val))
        return 0;
    if(!JS_ValueToObject(cx, val, &function))
        return 0;

    JS_CallFunctionValue(cx, localPlayer.playerObject,
        OBJECT_TO_JSVAL(function), 0, NULL, &rval);

    return rval;
}

//
// P_SaveLocalComponentData
//

void P_SaveLocalComponentData(void)
{
    jsval val;

    val = (jsval)P_LocalPlayerEvent("serialize");

    if(val == 0)
        return;

    if(JSVAL_IS_STRING(val))
    {
        JSString *str;

        if(str = JS_ValueToString(js_context, val))
        {
            // free the old string
            if(localPlayer.info.jsonData != NULL)
                JS_free(js_context, localPlayer.info.jsonData);

            localPlayer.info.jsonData = JS_EncodeString(js_context, str);
        }
    }
}

//
// P_RestoreLocalComponentData
//

void P_RestoreLocalComponentData(void)
{
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval rval;

    if(localPlayer.info.jsonData == NULL)
        return;

    cx = js_context;

    if(!JS_GetProperty(cx, localPlayer.playerObject, "deSerialize", &val))
        return;
    if(!JS_ValueToObject(cx, val, &function))
        return;

    val = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, localPlayer.info.jsonData));

    JS_CallFunctionValue(cx, localPlayer.playerObject,
        OBJECT_TO_JSVAL(function), 1, &val, &rval);

    return;
}

//
// P_LocalPlayerTick
//

void P_LocalPlayerTick(void)
{
    worldState_t *ws;
    playerInfo_t *info;
    gActor_t *actor;
    int current;
    plane_t *plane;
    int area_idx;

    if(client.GetState() != CL_STATE_INGAME)
        return;

    ws = &localPlayer.worldState;
    info = &localPlayer.info;
    actor = localPlayer.actor;
    plane = Map_IndexToPlane(actor->plane);

    area_idx = plane ? plane->area_id : -1;

    ws->timeStamp = (float)info->cmd.timestamp.i;
    ws->frameTime = info->cmd.frametime.f;

    Map_UnlinkActorFromWorld(actor);
    Map_UnlinkActorFromWorld(localPlayer.camera);

    // update controller in case movement was corrected by server
    Vec_Copy3(ws->origin, actor->origin);
    Vec_Add(ws->velocity, ws->velocity, actor->velocity);

    actor->waterlevel = Map_GetWaterLevel(actor->origin,
        actor->height, plane);

    // TODO
    actor->velocity[1] = 0;

    P_LocalPlayerEvent("onLocalTick");

    JPool_ReleaseObjects(&objPoolVector);
    JPool_ReleaseObjects(&objPoolGameActor);

    // TODO - handle actual actor movement/clipping here
    if(Actor_OnGround(actor))
    {
        G_ApplyFriction(actor->velocity, actor->friction, false);
        actor->velocity[1] = 0;
    }

    // TODO - AREA/LOCALTICK

    current = (info->netseq.outgoing-1) & (NETBACKUPS-1);
    Vec_Copy3(localPlayer.oldMoves[current], ws->origin);
    localPlayer.oldCmds[current] = info->cmd;
    localPlayer.latency[current] = client.GetTime();
    actor->plane = (ws->plane - gLevel.planes);

    Ang_Clamp(&ws->angles[0]);
    Ang_Clamp(&ws->angles[1]);
    Ang_Clamp(&ws->angles[2]);

    Vec_Copy3(actor->origin, ws->origin);
    Vec_Copy3(actor->angles, ws->angles);

    Map_LinkActorToWorld(actor);
    Map_LinkActorToWorld(localPlayer.camera);

    Actor_UpdateTransform(actor);
    Actor_UpdateTransform(localPlayer.camera);

    if(actor->classFlags & AC_PLAYER)
    {
        if(ws->plane && area_idx != ws->plane->area_id)
        {
            Map_CallAreaEvent(Map_GetArea(ws->plane),
                "onEnter", NULL, 0);

            if(area_idx != -1)
            {
                Map_CallAreaEvent(&gLevel.areas[area_idx],
                "onExit", NULL, 0);
            }
        }
    }
}

//
// P_SpawnLocalPlayer
//

void P_SpawnLocalPlayer(void)
{
    jsval val;
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

        JS_ITERATOR_START(actor, val);
        JS_ITERATOR_LOOP(actor, val, "playerID");
        {
            if(!(JSVAL_IS_INT(vp)))
                continue;
                
            playerID = INT_TO_JSVAL(vp);
            pObject = component;
            break;
        }
        JS_ITERATOR_END(actor, val);

        if(playerID != -1)
        {
            pStart = actor;
            break;
        }
    }

    if(pStart == NULL)
        common.Error("P_SpawnLocalPlayer: No player start has been found");

    pStart->height = pStart->baseHeight * 0.72f;
    pStart->physics |= PF_TOUCHACTORS;

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

    // de-serialize data if it exists
    P_RestoreLocalComponentData();

    // update actor position if the world state was
    // modified in any way
    Vec_Copy3(pStart->origin, ws->origin);
    Vec_Copy3(pStart->angles, ws->angles);

    if(ws->plane != NULL)
        pStart->plane = (ws->plane-gLevel.planes);
    else
        pStart->plane = -1;

    // force-set the player class flag
    pStart->classFlags |= AC_PLAYER;

    // spawn an actor to be used for the camera
    camera = (gActor_t*)Z_Calloc(sizeof(gActor_t), PU_LEVEL, NULL);
    camera->bCollision = false;
    camera->bHidden = true;
    camera->bTouch = false;
    camera->bStatic = false;
    camera->bClientOnly = true;
    camera->plane = -1;
    camera->owner = localPlayer.actor;
    strcpy(camera->name, "Camera");
    Vec_Set3(camera->scale, 1, 1, 1);
    Vec_Copy3(camera->origin, pStart->origin);
    Vec_Copy3(camera->angles, pStart->angles);
    Vec_Copy4(camera->rotation, pStart->rotation);
    Map_AddActor(&gLevel, camera);
    localPlayer.camera = camera;

    client.player = &localPlayer;
}

