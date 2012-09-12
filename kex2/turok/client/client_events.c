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
// DESCRIPTION: Event Handling From Inputs
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kernel.h"
#include "client.h"
#include "packet.h"
#include "menu.h"

event_t events[MAXEVENTS];
int     eventhead = 0;
int     eventtail = 0;

//
// CL_Responder
//

kbool CL_Responder(event_t *ev)
{
    if(client.state != CL_STATE_READY)
    {
        return false;
    }

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
    }

    return false;
}

//
// CL_WriteTiccmd
//

void CL_WriteTiccmd(ENetPacket *packet, ticcmd_t *cmd)
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

    DIFF_TICCMDS(angle[0], CL_TICDIFF_TURN1);
    DIFF_TICCMDS(angle[1], CL_TICDIFF_TURN2);
    DIFF_TICCMDS(buttons, CL_TICDIFF_BUTTONS);

    Packet_Write8(packet, bits);

    WRITE_TICCMD16(angle[0], CL_TICDIFF_TURN1);
    WRITE_TICCMD16(angle[1], CL_TICDIFF_TURN2);
    WRITE_TICCMD16(buttons, CL_TICDIFF_BUTTONS);

    Packet_Write8(packet, cmd->msec);

#undef WRITE_TICCMD16
#undef WRITE_TICCMD8
#undef DIFF_TICCMDS
}

//
// CL_BuildTiccmd
//

void CL_BuildTiccmd(void)
{
    ticcmd_t cmd;
    control_t *ctrl;
    int msec;
    ENetPacket *packet;

    if(client.state != CL_STATE_READY)
    {
        return;
    }

    memset(&cmd, 0, sizeof(ticcmd_t));
    ctrl = &control;

#define SET_KEYFLAG(kname, button)  \
    if(ctrl->key[kname])            \
    {                               \
        cmd.buttons |= button;      \
    }

    SET_KEYFLAG(KEY_ATTACK, BT_ATTACK);
    SET_KEYFLAG(KEY_JUMP, BT_JUMP);
    SET_KEYFLAG(KEY_CENTER, BT_CENTER);
    SET_KEYFLAG(KEY_FORWARD, BT_FORWARD);
    SET_KEYFLAG(KEY_BACK, BT_BACKWARD);
    SET_KEYFLAG(KEY_STRAFELEFT, BT_STRAFELEFT);
    SET_KEYFLAG(KEY_STRAFERIGHT, BT_STRAFERIGHT);

    if(ctrl->flags & CKF_NEXTWEAPON)
    {
        cmd.buttons |= BT_NEXTWEAP;
    }

    if(ctrl->flags & CKF_PREVWEAPON)
    {
        cmd.buttons |= BT_PREVWEAP;
    }

    cmd.angle[0] -= ANGLETOSHORT(ctrl->mousex);
    cmd.angle[1] -= ANGLETOSHORT(ctrl->mousey);

    ctrl->mousex = 0;
    ctrl->mousey = 0;

#undef SET_KEYFLAG

    msec = (int)(client.runtime * 1000);
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
    client.cmd = cmd;
    Packet_Send(packet, client.peer);
}

//
// CL_PostEvent
// Called by the I/O functions when input is detected
//

void CL_PostEvent(event_t *ev)
{
    events[eventhead] = *ev;
    eventhead = (++eventhead)&(MAXEVENTS-1);
}

//
// CL_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//

void CL_ProcessEvents(void)
{
    event_t *ev;
    
    for(; eventtail != eventhead; eventtail = (++eventtail)&(MAXEVENTS-1))
    {
        ev = &events[eventtail];

        if(Con_Responder(ev))
        {
            continue;
        }

        if(Menu_Responder(ev))
        {
            continue;
        }

        CL_Responder(ev);
    }
}


