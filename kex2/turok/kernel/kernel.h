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

#include "type.h"
#include "client.h"

//
// SYSTEM
//
void Sys_Sleep(unsigned long usecs);
void Sys_Shutdown(void);
int Sys_GetMilliseconds(void);
void Kernel_Main(int argc, char **argv);

//
// FILES
//
void KF_Shutdown(void);
void KF_Init(void);
void KF_LoadZipFile(const char *file);
int KF_OpenFileCache(const char *filename, byte **data, int tag);
int KF_ReadTextFile(const char *name, byte **buffer);

//
// KEYS
//
#define MAX_KEYS 256
extern char keycode[2][MAX_KEYS];
void Key_Init(void);

//
// INPUT
//
void IN_PollInput(void);
void IN_UpdateGrab(void);
void IN_Init(void);
void IN_CenterMouse(void);
kbool IN_Shiftdown(int c);
kbool IN_Ctrldown(int c);
kbool IN_Altdown(int c);

//
// VIDEO
//
#define FIXED_WIDTH     320
#define FIXED_HEIGHT    240

extern int video_width;
extern int video_height;
extern float video_ratio;
extern kbool video_windowed;
void V_Init(void);
void V_SetupScreen(void);

//
// CONSOLE
//
kbool Con_Responder(event_t* ev);
void Con_Printf(rcolor clr, const char *s, ...);
void Con_Ticker(void);
void Con_Draw(void);
void Con_Init(void);

#endif