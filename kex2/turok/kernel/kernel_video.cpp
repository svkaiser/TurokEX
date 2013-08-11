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

kexCvar cvarVidWidth("v_width", CVF_INT|CVF_CONFIG, "640", "TODO");
kexCvar cvarVidHeight("v_height", CVF_INT|CVF_CONFIG, "480", "TODO");
kexCvar cvarVidWindowed("v_windowed", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarVidVSync("v_vsync", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarVidDepthSize("v_depthsize", CVF_INT|CVF_CONFIG, "24", "TODO");
kexCvar cvarVidStencilSize("v_stencilsize", CVF_INT|CVF_CONFIG, "24", "TODO");
kexCvar cvarVidBuffSize("v_buffersize", CVF_INT|CVF_CONFIG, "32", "TODO");

//
// V_SetupScreen
//

void V_SetupScreen(void)
{
    int newwidth;
    int newheight;
    int p;
    
    video_windowed = cvarVidWindowed.GetBool();
    video_width = cvarVidWidth.GetInt();
    video_height = cvarVidHeight.GetInt();
    video_ratio = (float)video_width / (float)video_height;
    
    if(common.CheckParam("-window"))
    {
        video_windowed = true;
    }

    if(common.CheckParam("-fullscreen"))
    {
        video_windowed = false;
    }
    
    newwidth = newheight = 0;
    
    p = common.CheckParam("-width");
    if(p && p < myargc - 1)
    {
        newwidth = atoi(myargv[p+1]);
    }

    p = common.CheckParam("-height");
    if(p && p < myargc - 1)
    {
        newheight = atoi(myargv[p+1]);
    }
    
    if(newwidth && newheight)
    {
        video_width = newwidth;
        video_height = newheight;
    }

    if(cvarVidDepthSize.GetInt() != 8 &&
        cvarVidDepthSize.GetInt() != 16 &&
        cvarVidDepthSize.GetInt() != 24)
    {
        cvarVidDepthSize.Set(24);
    }

    if(cvarVidStencilSize.GetInt() != 8 &&
        cvarVidStencilSize.GetInt() != 16 &&
        cvarVidStencilSize.GetInt() != 24)
    {
        cvarVidStencilSize.Set(24);
    }
    
    if(cvarVidBuffSize.GetInt() != 8 &&
        cvarVidBuffSize.GetInt() != 16 &&
        cvarVidBuffSize.GetInt() != 24
        && cvarVidBuffSize.GetInt() != 32)
    {
        cvarVidBuffSize.Set(32);
    }
}
