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

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "SDL.h"
#include "shared.h"
#include "input.h"
#include "keyinput.h"

//
// SYSTEM
//
void Sys_Sleep(unsigned long usecs);
void Sys_Shutdown(void);
int Sys_GetMilliseconds(void);
void Kernel_Main(int argc, char **argv);

//
// VIDEO
//
#define FIXED_WIDTH     320
#define FIXED_HEIGHT    240

extern int video_width;
extern int video_height;
extern float video_ratio;
extern kbool video_windowed;
void V_SetupScreen(void);

#endif