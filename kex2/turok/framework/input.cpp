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
#include "system.h"
#include "client.h"
#include "input.h"
#include "console.h"

class kexInputSystemLocal : public kexInputSystem {
public:
                            kexInputSystemLocal(void);

    virtual void            Init(void);
    virtual void            PumpInputEvents(void);
    virtual void            PollInput(void);

    virtual unsigned int    MouseGetState(int *x, int *y);
    virtual unsigned int    MouseGetRelativeState(int *x, int *y);
    virtual void            MouseCenter(void);
    virtual void            MouseRead(void);
    virtual void            MouseMove(const int x, const int y);
    virtual void            MouseActivate(const bool bToggle);
    virtual void            MouseUpdateGrab(void);
    virtual void            MouseUpdateFocus(void);

private:
    void                    GetEvent(const SDL_Event *Event);
    bool                    MouseShouldBeGrabbed(void) const;
    float                   AccelerateMouse(int val) const;

    bool                    bWindowFocused;
    Uint8                   lastmbtn;
};

kexInputSystemLocal inputSystemLocal;
kexInputSystem *inputSystem = &inputSystemLocal;

//
// kexInputSystemLocal::kexInputSystemLocal
//

kexInputSystemLocal::kexInputSystemLocal() {
    lastmbtn        = 0;
    bWindowFocused  = false;
}

//
// kexInputSystemLocal::PumpInputEvents
//

void kexInputSystemLocal::PumpInputEvents(void) {
    SDL_PumpEvents();
}

//
// kexInputSystemLocal::Init
//

void kexInputSystemLocal::Init(void) {
    PumpInputEvents();
    MouseCenter();
    
    common.Printf("Input Initialized\n");
}

//
// kexInputSystemLocal::MouseGetState
//

unsigned int kexInputSystemLocal::MouseGetState(int *x, int *y) {
    return SDL_GetMouseState(x, y);
}

//
// kexInputSystemLocal::MouseGetRelativeState
//

unsigned int kexInputSystemLocal::MouseGetRelativeState(int *x, int *y) {
    return SDL_GetRelativeMouseState(x, y);
}

//
// kexInputSystemLocal::MouseRead
//

void kexInputSystemLocal::MouseRead(void) {
    int x, y;
    Uint8 btn;
    event_t ev;

    if(sysMain.IsWindowed() && console.IsActive()) {
        return;
    }
    
    MouseGetRelativeState(&x, &y);
    btn = MouseGetState(&mouse_x, &mouse_y);
    
    if(x != 0 || y != 0 || btn || (lastmbtn != btn)) {
        ev.type = ev_mouse;
        ev.data1 = btn;
        ev.data2 = x << 5;
        ev.data3 = (-y) << 5;
        ev.data4 = 0;
        client.PostEvent(&ev);
    }

    lastmbtn = btn;
    
    if(MouseShouldBeGrabbed()) {
        MouseCenter();
    }
}

//
// kexInputSystemLocal::MouseUpdateFocus
//

void kexInputSystemLocal::MouseUpdateFocus(void) {
    Uint32 flags;
    flags = sysMain.GetWindowFlags();
    
    // We should have input (keyboard) focus and be visible 
    // (not minimised)
    bWindowFocused = (flags & SDL_WINDOW_INPUT_FOCUS) || (flags & SDL_WINDOW_MOUSE_FOCUS);
}

//
// kexInputSystemLocal::GetEvent
//

void kexInputSystemLocal::GetEvent(const SDL_Event *Event) {
    event_t event;
    
    switch(Event->type) {
        case SDL_KEYDOWN:
            if(Event->key.repeat) {
                break;
            }
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
        case SDL_MOUSEBUTTONUP:
            if(!bWindowFocused) {
                break;
            }
            event.type = Event->type ==
                SDL_MOUSEBUTTONUP ? ev_mouseup : ev_mousedown;
            event.data1 = Event->button.button;
            event.data2 = event.data3 = 0;
            client.PostEvent(&event);
            break;

        case SDL_MOUSEWHEEL:
            if(!bWindowFocused) {
                break;
            }
            event.type = ev_mousewheel;
            event.data1 = Event->wheel.y > 0 ? KM_BUTTON_SCROLL_UP : KM_BUTTON_SCROLL_DOWN;
            event.data2 = event.data3 = 0;
            client.PostEvent(&event);
            break;
    
        case SDL_WINDOWEVENT:
            switch(Event->window.event) {
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                case SDL_WINDOWEVENT_ENTER:
                    MouseUpdateFocus();
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    bWindowFocused = false;
                    bMouseGrabbed = false;
                    break;
            }
            break;
        
        case SDL_QUIT:
            sysMain.Shutdown();
            break;
        
        default:
            break;
    }
}

//
// kexInputSystemLocal::PollInput
//

void kexInputSystemLocal::PollInput(void) {
    SDL_Event Event;
    
    while(SDL_PollEvent(&Event)) {
        GetEvent(&Event);
    }

    MouseRead();
}

//
// kexInputSystemLocal::MouseShouldBeGrabbed
//

bool kexInputSystemLocal::MouseShouldBeGrabbed(void) const {
    // if the window doesnt have focus, never grab it
    if(!bWindowFocused) {
        return false;
    }

    if(sysMain.IsWindowed()) {
        if(console.IsActive()) {
            return false;
        }
    }

    return bEnabled;
}

//
// kexInputSystemLocal::AccelerateMouse
//

float kexInputSystemLocal::AccelerateMouse(int val) const {
    if(!cvarMAcceleration.GetFloat()) {
        return (float)val;
    }
    
    if(val < 0) {
        return -AccelerateMouse(-val);
    }
    
    return kexMath::Pow((float)val, (cvarMAcceleration.GetFloat() / 200.0f + 1.0f));
}

//
// kexInputSystemLocal::MouseMove
//

void kexInputSystemLocal::MouseMove(const int x, const int y) {
    control_t *ctrl;
    
    ctrl = inputKey.Controls();

    ctrl->mousex += ((AccelerateMouse(x) * cvarMSensitivityX.GetFloat()) / 128.0f);
    ctrl->mousey += ((AccelerateMouse(y) * cvarMSensitivityY.GetFloat()) / 128.0f);
}

//
// kexInputSystemLocal::ActivateMouse
//

void kexInputSystemLocal::MouseActivate(const bool bToggle) {
    if(bToggle) {
        SDL_ShowCursor(0);
        SDL_SetRelativeMouseMode(SDL_TRUE);
        sysMain.SetWindowGrab(SDL_TRUE);
    }
    else {
        SDL_ShowCursor(1);
        SDL_SetRelativeMouseMode(SDL_FALSE);
        sysMain.SetWindowGrab(SDL_FALSE);
    }
}

//
// kexInputSystemLocal::MouseUpdateGrab
//

void kexInputSystemLocal::MouseUpdateGrab(void) {
    bool grab;
    
    grab = MouseShouldBeGrabbed();
    if(grab && !bMouseGrabbed) {
        MouseActivate(true);
    }
    
    if (!grab && bMouseGrabbed) {
        MouseActivate(false);
    }
    
    bMouseGrabbed = grab;
}

//
// kexInputSystemLocal::MouseCenter
// Warp the mouse back to the middle of the screen
//

void kexInputSystemLocal::MouseCenter(void) {
    // Warp the the screen center
    sysMain.WarpMouseToCenter();
    
    // Clear any relative movement caused by warping
    PumpInputEvents();
    MouseGetRelativeState(NULL, NULL);
}
