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
// DESCRIPTION: Base Console Functions
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kernel.h"
#include "js.h"

CVAR_EXTERNAL(developer);

char con_lastOutputBuffer[512];

//
// Con_GetLastBuffer
//

char *Con_GetLastBuffer(void)
{
    return con_lastOutputBuffer;
}

//
// Con_Printf
//

void Con_Printf(rcolor clr, const char *s)
{
    char *src;

    if(developer.value)
    {
        memset(con_lastOutputBuffer, 0, 512);
        strcpy(con_lastOutputBuffer, kva("%f : %s", (Sys_GetMilliseconds() / 1000.0f), s));
    }
    
    if(clr != COLOR_WHITE)
    {
        char *buf[2];
        
        buf[0] = (char*)s;
        buf[1] = kva("%i,%i,%i",
            clr & 0xff,
            (clr >> 8) & 0xff,
            (clr >> 16) & 0xff);

        J_CallClassFunction(JS_EV_SYS, "event_OutputText", buf, 2);
        return;
    }
    
    src = (char*)s;
    J_CallClassFunction(JS_EV_SYS, "event_OutputText", &src, 1);
}
