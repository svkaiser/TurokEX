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
#include "input.h"

kexInput inputSystem;

//
// kexInput::kexInput
//

kexInput::kexInput() {
    cursors[0]      = NULL;
    cursors[1]      = NULL;
    lastmbtn        = 0;
    bGrabbed        = false;
    bWindowFocused  = false;
    bEnabled        = true;
}

//
// kexInput::~kexInput
//

kexInput::~kexInput() {
}

//
// kexInput::Init
//

void kexInput::Init(void) {
    Uint8 data[1] = { 0x00 };
    
    SDL_PumpEvents();
    cursors[0] = SDL_GetCursor();
    cursors[1] = SDL_CreateCursor(data, data, 8, 1, 0, 0);

    CenterMouse();
    
    common.Printf("Input Initialized\n");
}

//
// kexInput::ReadMouse
//

void kexInput::ReadMouse(void) {
    int x, y;
    Uint8 btn;
    event_t ev;
    
    SDL_GetRelativeMouseState(&x, &y);
    btn = SDL_GetMouseState(&mouse_x, &mouse_y);
    
    if(x != 0 || y != 0 || btn || (lastmbtn != btn)) {
        ev.type = ev_mouse;
        ev.data1 = GetButtonState(btn);
        ev.data2 = x << 5;
        ev.data3 = (-y) << 5;
        ev.data4 = 0;
        client.PostEvent(&ev);
    }

    lastmbtn = btn;
    
    if(MouseShouldBeGrabbed()) {
        CenterMouse();
    }
}

//
// kexInput::GetButtonState
//

int kexInput::GetButtonState(Uint8 buttonstate) const {
    return 0
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_LEFT)      ? 1 : 0)
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_MIDDLE)    ? 2 : 0)
        | (buttonstate & SDL_BUTTON(SDL_BUTTON_RIGHT)     ? 4 : 0);
}

//
// kexInput::UpdateFocus
//

void kexInput::UpdateFocus(void) {
    Uint8 state;
    state = SDL_GetAppState();
    
    // We should have input (keyboard) focus and be visible 
    // (not minimised)
    bWindowFocused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);
}

//
// kexInput::IsShiftDown
//

bool kexInput::IsShiftDown(int c) const {
    return(c == SDLK_RSHIFT || c == SDLK_LSHIFT);
}

//
// kexInput::IsCtrlDown
//

bool kexInput::IsCtrlDown(int c) const {
    return(c == SDLK_RCTRL || c == SDLK_LCTRL);
}

//
// kexInput::IsAltDown
//

bool kexInput::IsAltDown(int c) const {
    return(c == SDLK_RALT || c == SDLK_LALT);
}

//
// kexInput::GetEvent
//

void kexInput::GetEvent(const SDL_Event *Event) {
    event_t event;
    
    switch(Event->type) {
    case SDL_KEYDOWN:
        event.type = ev_keydown;
        event.data1 = Event->key.keysym.sym;
        client.PostEvent(&event);
        break;
        
    case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = Event->key.keysym.sym;
        client.PostEvent(&event);
        break;
        
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
            if(!bWindowFocused)
                break;
            
            if(Event->button.button == SDL_BUTTON_WHEELUP) {
                event.type = ev_mousewheel;
                event.data1 = SDL_BUTTON_WHEELUP;
            }
            else if(Event->button.button == SDL_BUTTON_WHEELDOWN) {
                event.type = ev_mousewheel;
                event.data1 = SDL_BUTTON_WHEELDOWN;
            }
            else {
                event.type = Event->type ==
                    SDL_MOUSEBUTTONUP ? ev_mouseup : ev_mousedown;
                event.data1 = Event->button.button;
            }
            
            event.data2 = event.data3 = 0;
            client.PostEvent(&event);
        }
        break;
        
    case SDL_ACTIVEEVENT:
    case SDL_VIDEOEXPOSE:
        UpdateFocus();
        break;
        
    case SDL_QUIT:
        Sys_Shutdown();
        break;
        
    default:
        break;
    }
}

//
// kexInput::PollInput
//

void kexInput::PollInput(void) {
    SDL_Event Event;
    
    while(SDL_PollEvent(&Event)) {
        GetEvent(&Event);
    }

    ReadMouse();
}

//
// kexInput::MouseShouldBeGrabbed
//

bool kexInput::MouseShouldBeGrabbed(void) const {
    // if the window doesnt have focus, never grab it
    if(!bWindowFocused) {
        return false;
    }

    if(bEnabled) {
        return true;
    }

    if(!video_windowed) {
        return true;
    }

    return false;
}

//
// kexInput::AccelerateMouse
//

float kexInput::AccelerateMouse(int val) const {
    if(!cvarMAcceleration.GetFloat())
        return (float)val;
    
    if(val < 0)
        return -AccelerateMouse(-val);
    
    return (float)(pow((double)val, (double)(cvarMAcceleration.GetFloat() / 200.0f + 1.0f)));
}

//
// kexInput::MoveMouse
//

void kexInput::MoveMouse(int x, int y) {
    control_t *ctrl;
    
    ctrl = &control;

    ctrl->mousex += ((AccelerateMouse(x) * cvarMSensitivityX.GetFloat()) / 128.0f);
    ctrl->mousey += ((AccelerateMouse(y) * cvarMSensitivityY.GetFloat()) / 128.0f);
}

//
// kexInput::ActivateMouse
//

void kexInput::ActivateMouse(void) {
    SDL_SetCursor(cursors[1]);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_ShowCursor(0);
}

//
// kexInput::DeactivateMouse
//

void kexInput::DeactivateMouse(void) {
    SDL_SetCursor(cursors[0]);
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(1);
}

//
// kexInput::UpdateGrab
//

void kexInput::UpdateGrab(void) {
    bool grab;
    
    grab = MouseShouldBeGrabbed();
    if (grab && !bGrabbed) {
        ActivateMouse();
    }
    
    if (!grab && bGrabbed) {
        DeactivateMouse();
    }
    
    bGrabbed = grab;
}

//
// kexInput::CenterMouse
// Warp the mouse back to the middle of the screen
//

void kexInput::CenterMouse(void) {
    // Warp the the screen center
    SDL_WarpMouse((unsigned short)(video_width/2), (unsigned short)(video_height/2));
    
    // Clear any relative movement caused by warping
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}
