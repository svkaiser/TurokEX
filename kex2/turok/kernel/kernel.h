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
#define MAX_KEYS SDLK_LAST

#define CKF_GAMEPAD     0x1
#define CKF_UP          0x4000
#define CKF_COUNTMASK   0x00ff

#define MAXACTIONS  256

typedef struct
{
    float       mousex;
    float       mousey;
    float       joyx;
    float       joyy;
    int         actions[MAXACTIONS];
    int         flags;
} control_t;

extern control_t control;
void Key_ExecCmd(char key, kbool keyup);
void Key_WriteBindings(FILE *file);
void Key_AddAction(byte id, char *name);
int Key_FindAction(char *name);
void Key_ClearControls(void);
void Key_Init(void);

//
// INPUT
//

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

void IN_PollInput(void);
void IN_UpdateGrab(void);
void IN_Init(void);
void IN_MouseMove(int x, int y);
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
void Con_Printf(rcolor clr, const char *s);
char *Con_GetLastBuffer(void);
void Con_Ticker(void);
void Con_Drawer(void);
kbool Con_ProcessConsoleInput(event_t *ev);

#endif