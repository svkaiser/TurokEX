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
// DESCRIPTION: Video Backend
//
//-----------------------------------------------------------------------------

#include "SDL.h"
#include "common.h"

int video_width;
int video_height;
float video_ratio;
kbool video_windowed;
kbool window_focused;

CVAR(v_width, 640);
CVAR(v_height, 480);
CVAR(v_windowed, 1);
CVAR(v_vsync, 1);
CVAR(v_depthsize, 24);
CVAR(v_buffersize, 32);

//
// V_SetupScreen
//

void V_SetupScreen(void)
{
    int newwidth;
    int newheight;
    int p;
    
    video_windowed = (int)v_windowed.value;
    video_width = (int)v_width.value;
    video_height = (int)v_height.value;
    video_ratio = (float)video_width / (float)video_height;
    
    if(Com_CheckParam("-window"))
    {
        video_windowed = true;
    }

    if(Com_CheckParam("-fullscreen"))
    {
        video_windowed = false;
    }
    
    newwidth = newheight = 0;
    
    p = Com_CheckParam("-width");
    if(p && p < myargc - 1)
    {
        newwidth = atoi(myargv[p+1]);
    }

    p = Com_CheckParam("-height");
    if(p && p < myargc - 1)
    {
        newheight = atoi(myargv[p+1]);
    }
    
    if(newwidth && newheight)
    {
        video_width = newwidth;
        video_height = newheight;
    }

    if(v_depthsize.value != 8 &&
        v_depthsize.value != 16 &&
        v_depthsize.value != 24)
    {
        Cvar_SetValue(v_depthsize.name, 24);
    }
    
    if(v_buffersize.value != 8 &&
        v_buffersize.value != 16 &&
        v_buffersize.value != 24
        && v_buffersize.value != 32)
    {
        Cvar_SetValue(v_buffersize.name, 32);
    }
}

//
// V_Init
//

void V_Init(void)
{
    Cvar_Register(&v_width);
    Cvar_Register(&v_height);
    Cvar_Register(&v_windowed);
    Cvar_Register(&v_vsync);
    Cvar_Register(&v_depthsize);
    Cvar_Register(&v_buffersize);
}
