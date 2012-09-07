// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _CLIENT_H_
#define _CLIENT_H_

// Input event types.
typedef enum
{
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_mousedown,
    ev_mouseup,
    ev_mousewheel,
    ev_gamepad
} evtype_t;

// Event structure.
typedef struct
{
    evtype_t    type;
    int         data1;  // keys / mouse/joystick buttons
    int         data2;  // mouse/joystick x move
    int         data3;  // mouse/joystick y move
    int         data4;  // misc data
} event_t;

typedef struct
{
    char    forwardmove;    // *2048 for move
    char    sidemove;       // *2048 for move
    short   angleturn;      // <<16 for angle delta
	short   pitch;
    byte    buttons;
} ticcmd_t;

#define MAXEVENTS 64

extern event_t  events[MAXEVENTS];
extern int      eventhead;
extern int      eventtail;

void CL_PostEvent(event_t *ev);
void CL_ProcessEvents(void);
kbool CL_Responder(event_t *ev);
int CL_Random(void);
void CL_Connect(const char *address);
void CL_Run(int msec);
void CL_Init(void);

#endif