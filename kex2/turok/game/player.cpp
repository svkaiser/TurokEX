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
#include "actor_old.h"
#include "player.h"
#include "world.h"
#include "zone.h"
#include "console.h"

//
// P_RunCommand
//

void P_RunCommand(ENetEvent *sev, ENetPacket *packet)
{
    /*netPlayer_t *netplayer;
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
    packetManager.Read32(packet, (unsigned int*)&netplayer->info.netseq.ingoing);*/

    // TODO - PROCESS MOVEMENT
}

//
// P_LocalPlayerTick
//

void P_LocalPlayerTick(void)
{
    worldState_t *ws;
    gActor_t *actor;
    gActor_t *camera;
    int current;
    plane_t *plane;
    int area_idx;
    ticcmd_t *cmd;

    if(client.GetState() != CL_STATE_INGAME)
        return;

    ws = &client.LocalPlayer().worldState;
    actor = client.LocalPlayer().actor;
    camera = client.LocalPlayer().camera;
    plane = Map_IndexToPlane(actor->plane);

    area_idx = plane ? plane->area_id : -1;

    cmd = client.LocalPlayer().Cmd();
    ws->timeStamp = (float)cmd->timestamp.i;
    ws->frameTime = cmd->frametime.f;

    Map_UnlinkActorFromWorld(actor);
    Map_UnlinkActorFromWorld(camera);

    // update controller in case movement was corrected by server
    Vec_Copy3(ws->origin, actor->origin);
    Vec_Add(ws->velocity, ws->velocity, actor->velocity);

    actor->waterlevel = Map_GetWaterLevel(actor->origin,
        actor->height, plane);

    // TODO
    actor->velocity[1] = 0;

    client.LocalPlayer().PlayerEvent("onLocalTick");

    JPool_ReleaseObjects(&objPoolVector);
    JPool_ReleaseObjects(&objPoolGameActor);

    // TODO - handle actual actor movement/clipping here
    if(Actor_OnGround(actor))
    {
        G_ApplyFriction(actor->velocity, actor->friction, false);
        actor->velocity[1] = 0;
    }

    // TODO - AREA/LOCALTICK

    current = (client.LocalPlayer().NetSeq()->outgoing-1) & (NETBACKUPS-1);
    //Vec_Copy3(localPlayer.oldMoves[current], ws->origin);
    //localPlayer.oldCmds[current] = info->cmd;
    //localPlayer.latency[current] = client.GetTime();
    actor->plane = (ws->plane - gLevel.planes);

    Ang_Clamp(&ws->angles[0]);
    Ang_Clamp(&ws->angles[1]);
    Ang_Clamp(&ws->angles[2]);

    Vec_Copy3(actor->origin, ws->origin);
    Vec_Copy3(actor->angles, ws->angles);

    Map_LinkActorToWorld(actor);
    Map_LinkActorToWorld(camera);

    Actor_UpdateTransform(actor);
    Actor_UpdateTransform(camera);

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

    client.LocalPlayer().actor = pStart;
    client.LocalPlayer().SetScriptObject(pObject);

    ws = &client.LocalPlayer().worldState;

    Vec_Copy3(ws->origin, pStart->origin);
    Vec_Copy3(ws->angles, pStart->angles);
    Vec_Set3(ws->velocity, 0, 0, 0);
    Vec_Set3(ws->accel, 0, 0, 0);
    ws->actor = client.LocalPlayer().actor;

    if(pStart->plane != -1)
        ws->plane = &gLevel.planes[pStart->plane];

    // de-serialize data if it exists
    client.LocalPlayer().DeSerializeScriptObject();

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
    camera->owner = client.LocalPlayer().actor;
    strcpy(camera->name, "Camera");
    Vec_Set3(camera->scale, 1, 1, 1);
    Vec_Copy3(camera->origin, pStart->origin);
    Vec_Copy3(camera->angles, pStart->angles);
    Vec_Copy4(camera->rotation, pStart->rotation);
    Map_AddActor(&gLevel, camera);
    client.LocalPlayer().camera = camera;
}

enum {
    scplocation_id = 0,
    scplocation_end
};

static const sctokens_t playerLocationTokens[scplocation_end+1] = {
    { scplocation_id,           "id"                    },
    { -1,                       NULL                    }
};

DECLARE_CLASS(kexPlayerLocation, kexWorldActor)

//
// kexPlayerLocation::kexPlayerLocation
//

kexPlayerLocation::kexPlayerLocation(void) {
    this->id = 0;
}

//
// kexPlayerLocation::Parse
//

void kexPlayerLocation::Parse(kexLexer *lexer) {
    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        switch(lexer->GetIDForTokenList(playerLocationTokens, lexer->Token())) {
        case scplocation_id:
            this->id = lexer->GetNumber();
            break;
        default:
            ParseDefault(lexer);
            break;
        }
        lexer->Find();
    }
}

DECLARE_ABSTRACT_CLASS(kexPlayer, kexWorldActor)

//
// kexPlayer::kexPlayer
//

kexPlayer::kexPlayer(void) {
    ResetNetSequence();
    ResetTicCommand();
    
    jsonData        = NULL;
    name            = NULL;
    scriptObject    = NULL;
}

//
// kexPlayer::~kexPlayer
//

kexPlayer::~kexPlayer(void) {
}

//
// kexPlayer::ResetNetSequence
//

void kexPlayer::ResetNetSequence(void) {
    netseq.ingoing = 0;
    netseq.outgoing = 1;
}

//
// kexPlayer::ResetTicCommand
//

void kexPlayer::ResetTicCommand(void) {
    memset(&cmd, 0, sizeof(ticcmd_t));
}

//
// kexPlayer::Accelerate
//

void kexPlayer::Accelerate(const playerMove_t *move, int direction, int axis) {
    float time = 0;
#define LERP_ACCEL(m, v)                                        \
    time = move->accelSpeed[v] * (60.0f * cmd.frametime.f);     \
    if(time > 1) lerp = move->m[v];                             \
    else { lerp = (move->m[v] - acceleration[v]) *              \
    time + acceleration[v]; }

#define LERP_DEACCEL(v)                                         \
    time = move->deaccelSpeed[v] * (60.0f * cmd.frametime.f);   \
    if(time > 1) lerp = move->deaccelSpeed[v];                  \
    else { lerp = (0 - acceleration[v]) *                       \
    time + acceleration[v]; }
    
    float lerp = acceleration[axis];
    
    if(direction == 1) {
        LERP_ACCEL(forwardSpeed, axis);
    }
    else if(direction == -1) {
        LERP_ACCEL(backwardSpeed, axis);
    }
    else {
        LERP_DEACCEL(axis);
    }
    
    switch(axis) {
    case 0:
        acceleration.x = lerp;
        break;
    case 1:
        acceleration.y = lerp;
        break;
    case 2:
        acceleration.z = lerp;
        break;
    }
    
#undef LERP_ACCEL
#undef LERP_DEACCEL
}

DECLARE_CLASS(kexLocalPlayer, kexPlayer)

//
// kexLocalPlayer::kexLocalPlayer
//

kexLocalPlayer::kexLocalPlayer(void) {
}

//
// kexLocalPlayer::~kexLocalPlayer
//

kexLocalPlayer::~kexLocalPlayer(void) {
    if(jsonData != NULL)
        JS_free(js_context, jsonData);
}

//
// kexLocalPlayer::ProcessInput
//

bool kexLocalPlayer::ProcessInput(event_t *ev) {
    if(console.ProcessInput(ev))
        return true;

    switch(ev->type) {
    case ev_mouse:
        inputSystem.MoveMouse(ev->data2, ev->data3);
        return true;

    case ev_keydown:
    case ev_mousedown:
        inputKey.ExecuteCommand(ev->data1, false);
        return true;

    case ev_keyup:
    case ev_mouseup:
        inputKey.ExecuteCommand(ev->data1, true);
        return true;
    }

    return false;
}

//
// kexLocalPlayer::BuildCommands
//

void kexLocalPlayer::BuildCommands(void) {
    ENetPacket *packet;
    ticcmd_t *buildCmd;
    control_t *ctrl;
    int numactions;
    int i;

    if(client.GetState() != CL_STATE_INGAME)
        return;

    buildCmd = &cmd;
    ctrl = inputKey.Controls();

    for(i = 0; i < MAXACTIONS; i++) {
        buildCmd->buttons[i] = false;
    }

    for(i = 0; i < MAXACTIONS; i++) {
        buildCmd->buttons[i] = (ctrl->actions[i]) ? true : false;
        if(buildCmd->buttons[i] == 1) {
            if(buildCmd->heldtime[i] < 255) {
                buildCmd->heldtime[i]++;
            }
            else {
                buildCmd->heldtime[i] = 255;
            }
        }
        else {
            buildCmd->heldtime[i] = 0;
        }
    }

    buildCmd->timestamp.i = client.GetTime();
    buildCmd->frametime.f = client.GetRunTime();
    buildCmd->mouse[0].f = ctrl->mousex;
    buildCmd->mouse[1].f = ctrl->mousey;

    ctrl->mousex = 0;
    ctrl->mousey = 0;

    if(!(packet = packetManager.Create()))
        return;

    packetManager.Write8(packet, cp_cmd);

    numactions = 0;

    for(i = 0; i < MAXACTIONS; i++) {
        if(buildCmd->buttons[i]) {
            numactions++;
        }
    }

    packetManager.Write32(packet, buildCmd->mouse[0].i);
    packetManager.Write32(packet, buildCmd->mouse[1].i);
    packetManager.Write32(packet, buildCmd->timestamp.i);
    packetManager.Write32(packet, buildCmd->frametime.i);
    packetManager.Write32(packet, numactions);

    for(i = 0; i < MAXACTIONS; i++) {
        if(buildCmd->buttons[i]) {
            packetManager.Write8(packet, i);
            packetManager.Write8(packet, buildCmd->heldtime[i]);
        }
    }

    packetManager.Write32(packet, netseq.ingoing);
    packetManager.Write32(packet, netseq.outgoing);
    netseq.outgoing++;

    packetManager.Send(packet, peer);
}

//
// kexLocalPlayer::PlayerEvent
//

int kexLocalPlayer::PlayerEvent(const char *eventName) {
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval rval;

    if(client.GetState() != CL_STATE_INGAME)
        return 0;

    cx = js_context;

    if(!JS_GetProperty(cx, scriptObject, eventName, &val))
        return 0;
    if(!JS_ValueToObject(cx, val, &function))
        return 0;
    if(!function || !JS_ObjectIsFunction(cx, function))
        return 0;

    JS_CallFunctionValue(cx, scriptObject,
        OBJECT_TO_JSVAL(function), 0, NULL, &rval);

    return rval;
}

//
// kexLocalPlayer::SerializeScriptObject
//

void kexLocalPlayer::SerializeScriptObject(void) {
    jsval val = (jsval)PlayerEvent("serialize");

    if(val == 0)
        return;

    if(JSVAL_IS_STRING(val)) {
        JSString *str;

        if(str = JS_ValueToString(js_context, val)) {
            // free the old string
            if(jsonData != NULL)
                JS_free(js_context, jsonData);

            jsonData = JS_EncodeString(js_context, str);
        }
    }
}

//
// kexLocalPlayer::DeSerializeScriptObject
//

void kexLocalPlayer::DeSerializeScriptObject(void) {
    gObject_t *function;
    JSContext *cx;
    jsval val;
    jsval rval;

    if(jsonData == NULL)
        return;

    cx = js_context;

    if(!JS_GetProperty(cx, scriptObject, "deSerialize", &val))
        return;
    if(!JS_ValueToObject(cx, val, &function))
        return;

    val = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, jsonData));

    JS_CallFunctionValue(cx, scriptObject,
        OBJECT_TO_JSVAL(function), 1, &val, &rval);
}

//
// kexLocalPlayer::LocalTick
//

void kexLocalPlayer::LocalTick(void) {
    if(client.GetState() != CL_STATE_INGAME)
        return;
}

DECLARE_CLASS(kexNetPlayer, kexPlayer)

//
// kexNetPlayer::kexNetPlayer
//

kexNetPlayer::kexNetPlayer(void) {
}

//
// kexNetPlayer::~kexNetPlayer
//

kexNetPlayer::~kexNetPlayer(void) {
}

//
// kexNetPlayer::kexNetPlayer
//

void kexNetPlayer::Tick(void) {
}
