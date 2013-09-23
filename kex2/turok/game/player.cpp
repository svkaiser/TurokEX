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

enum {
    scplocation_id = 0,
    scplocation_component,
    scplocation_end
};

static const sctokens_t playerLocationTokens[scplocation_end+1] = {
    { scplocation_id,           "id"                    },
    { scplocation_component,    "component"             },
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
        case scplocation_component:
            // instead of creating a component object for this actor, store the
            // name of the component which will be used to initialize the actual
            // component for the player later on
            lexer->GetString();
            this->playerComponent = lexer->StringToken();
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

    if(!JS_GetProperty(cx, component, eventName, &val))
        return 0;
    if(!JS_ValueToObject(cx, val, &function))
        return 0;
    if(!function || !JS_ObjectIsFunction(cx, function))
        return 0;

    JS_CallFunctionValue(cx, component,
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

    if(!JS_GetProperty(cx, component, "deSerialize", &val))
        return;
    if(!JS_ValueToObject(cx, val, &function))
        return;

    val = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, jsonData));

    JS_CallFunctionValue(cx, component,
        OBJECT_TO_JSVAL(function), 1, &val, &rval);
}

//
// kexLocalPlayer::ActionDown
//

bool kexLocalPlayer::ActionDown(const kexStr &str) {
    int action = inputKey.FindAction(str.c_str());

    return (action != -1 && cmd.buttons[action]);
}

//
// kexLocalPlayer::LocalTick
//

void kexLocalPlayer::LocalTick(void) {
    if(client.GetState() != CL_STATE_INGAME)
        return;

    scriptComponent.CallFunction(scriptComponent.onThink);
}

//
// kexLocalPlayer::ToWorldActor
//

kexWorldActor *kexLocalPlayer::ToWorldActor(void) {
    return static_cast<kexWorldActor*>(this);
}

//
// kexLocalPlayer::InitObject
//

void kexLocalPlayer::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kLocalPlayer",
        sizeof(kexLocalPlayer),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);

    scriptManager.Engine()->RegisterObjectMethod(
        "kLocalPlayer",
        "bool ActionDown(const kStr &in)",
        asMETHODPR(kexLocalPlayer, ActionDown, (const kexStr&), bool),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kLocalPlayer",
        "kAngle &GetAngles(void)",
        asMETHODPR(kexLocalPlayer, GetAngles, (void), kexAngle&),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kLocalPlayer",
        "void SetAngles(const kAngle &in)",
        asMETHODPR(kexLocalPlayer, SetAngles, (const kexAngle &an), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty(
        "kLocalPlayer",
        "float cmdMouseX",
        asOFFSET(kexLocalPlayer, cmd.mouse[0].f));

    scriptManager.Engine()->RegisterObjectProperty(
        "kLocalPlayer",
        "float cmdMouseY",
        asOFFSET(kexLocalPlayer, cmd.mouse[1].f));

    scriptManager.Engine()->RegisterObjectProperty(
        "kLocalPlayer",
        "ref @obj",
        asOFFSET(kexLocalPlayer, scriptComponent.Handle()));

    scriptManager.Engine()->RegisterObjectMethod(
        "kLocalPlayer",
        "kActor @ToActor(void)",
        asMETHODPR(kexLocalPlayer, ToWorldActor, (void), kexWorldActor*),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterGlobalProperty(
        "kLocalPlayer LocalPlayer",
        client.GetLocalPlayerRef());
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
