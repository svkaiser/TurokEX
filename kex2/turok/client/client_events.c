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
#include "menu.h"

event_t events[MAXEVENTS];
int     eventhead = 0;
int     eventtail = 0;


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
    }
}


