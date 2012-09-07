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
// DESCRIPTION: Input System
//
//-----------------------------------------------------------------------------

#include <math.h>

#include "SDL.h"
#include "common.h"
#include "kernel.h"
#include "client.h"

static SDL_Cursor* cursors[2] = { NULL, NULL };
static kbool window_focused;
static float mouse_accelfactor;
static int mouse_x = 0;
static int mouse_y = 0;

CVAR_EXTERNAL(cl_msensitivityx);
CVAR_EXTERNAL(cl_msensitivityy);
CVAR_EXTERNAL(cl_macceleration);

static void IN_ReadMouse(void);

//
// IN_SDLtoDoomMouseState
//

static int IN_SDLtoDoomMouseState(Uint8 buttonstate)
{
    return 0
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_LEFT)      ? 1 : 0)
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_MIDDLE)    ? 2 : 0)
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_RIGHT)     ? 4 : 0);
}

//
// I_UpdateFocus
//

static void IN_UpdateFocus(void)
{
    Uint8 state;
    state = SDL_GetAppState();
    
    // We should have input (keyboard) focus and be visible 
    // (not minimised)
    window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);
}

//
// IN_Shiftdown
//

kbool IN_Shiftdown(int c)
{
    return(c == SDLK_RSHIFT || c == SDLK_LSHIFT);
}

//
// IN_Ctrldown
//

kbool IN_Ctrldown(int c)
{
    return(c == SDLK_RCTRL || c == SDLK_LCTRL);
}

//
// IN_Altdown
//

kbool IN_Altdown(int c)
{
    return(c == SDLK_RALT || c == SDLK_LALT);
}

//
// IN_GetEvent
//

static void IN_GetEvent(SDL_Event *Event)
{
    event_t event;
    
    switch(Event->type)
    {
    case SDL_KEYDOWN:
        event.type = ev_keydown;
        event.data1 = Event->key.keysym.sym;
        CL_PostEvent(&event);
        break;
        
    case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = Event->key.keysym.sym;
        CL_PostEvent(&event);
        break;
        
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        {
            if(!window_focused)
                break;
            
            if(Event->button.button == SDL_BUTTON_WHEELUP)
            {
                event.type = ev_mousewheel;
                event.data1 = SDL_BUTTON_WHEELUP;
            }
            else if(Event->button.button == SDL_BUTTON_WHEELDOWN)
            {
                event.type = ev_mousewheel;
                event.data1 = SDL_BUTTON_WHEELDOWN;
            }
            else
            {
                event.type = Event->type ==
                    SDL_MOUSEBUTTONUP ? ev_mouseup : ev_mousedown;
                event.data1 = IN_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
            }
            
            event.data2 = event.data3 = 0;
            CL_PostEvent(&event);
        }
        break;
        
    case SDL_ACTIVEEVENT:
    case SDL_VIDEOEXPOSE:
        IN_UpdateFocus();
        break;
        
    case SDL_QUIT:
        Sys_Shutdown();
        break;
        
    default:
        break;
    }
}

//
// IN_PollInput
//

void IN_PollInput(void)
{
    SDL_Event Event;
    
    while(SDL_PollEvent(&Event))
    {
        IN_GetEvent(&Event);
    }

    IN_ReadMouse();
}

//
// IN_MouseShouldBeGrabbed
//

static kbool IN_MouseShouldBeGrabbed(void)
{
    // if the window doesnt have focus, never grab it
    if(!window_focused)
    {
        return false;
    }

    if(!video_windowed)
    {
        return true;
    }

    return false;
}

//
// IN_MouseAccelChange
//

void IN_MouseAccelChange(void)
{
    mouse_accelfactor = cl_macceleration.value / 200.0f + 1.0f;
}

//
// IN_MouseAccel
//

int IN_MouseAccel(int val)
{
    if(!cl_macceleration.value)
        return val;
    
    if(val < 0)
        return -IN_MouseAccel(-val);
    
    return (int)(pow((double)val, (double)mouse_accelfactor));
}

//
// IN_MouseMove
//

void IN_MouseMove(int x, int y)
{
    control_t *ctrl;
    
    ctrl = &control;

    ctrl->mousex += ((IN_MouseAccel(x) * (int)cl_msensitivityx.value) / 128);
    ctrl->mousey += ((IN_MouseAccel(y) * (int)cl_msensitivityy.value) / 128);
}

//
// IN_ActivateMouse
//

static void IN_ActivateMouse(void)
{
    SDL_SetCursor(cursors[1]);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(1);
}

//
// IN_DeactivateMouse
//

static void IN_DeactivateMouse(void)
{
    SDL_SetCursor(cursors[0]);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(0);
}

//
// IN_UpdateGrab
//

void IN_UpdateGrab(void)
{
    static kbool currently_grabbed = false;
    kbool grab;
    
    grab = IN_MouseShouldBeGrabbed();
    if (grab && !currently_grabbed)
    {
        IN_ActivateMouse();
    }
    
    if (!grab && currently_grabbed)
    {
        IN_DeactivateMouse();
    }
    
    currently_grabbed = grab;
}

// IN_CenterMouse
// Warp the mouse back to the middle of the screen
//

void IN_CenterMouse(void)
{
    // Warp the the screen center
    SDL_WarpMouse((unsigned short)(video_width/2), (unsigned short)(video_height/2));
    
    // Clear any relative movement caused by warping
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}

//
// IN_ReadMouse
//

static void IN_ReadMouse(void)
{
    int x, y;
    Uint8 btn;
    event_t ev;
    static Uint8 lastmbtn = 0;
    
    SDL_GetRelativeMouseState(&x, &y);
    btn = SDL_GetMouseState(&mouse_x, &mouse_y);
    
    if(x != 0 || y != 0 || btn || (lastmbtn != btn)) 
    {
        ev.type = ev_mouse;
        ev.data1 = IN_SDLtoDoomMouseState(btn);
        ev.data2 = x << 5;
        ev.data3 = (-y) << 5;
        ev.data4 = 0;
        CL_PostEvent(&ev);
    }

    lastmbtn = btn;
    
    if(IN_MouseShouldBeGrabbed())
    {
        IN_CenterMouse();
    }
}

//
// IN_Init
//

void IN_Init(void)
{
    Uint8 data[1] = { 0x00 };
    
    SDL_PumpEvents();
    cursors[0] = SDL_GetCursor();
    cursors[1] = SDL_CreateCursor(data, data, 8, 1, 0, 0);

    IN_CenterMouse();
}
