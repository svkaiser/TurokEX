// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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

#ifndef __GUI_H__
#define __GUI_H__

typedef enum {
    GUIS_DISABLED   = 0,
    GUIS_READY,
    GUIS_FADEIN,
    GUIS_FADEOUT
} guiStatus_t;

typedef enum {
    GBS_NONE        = 0,
    GBS_DISABLED,
    GBS_HOVER,
    GBS_DOWN,
    GBS_PRESSED,
    GBS_RELEASED
} guiButtonState_t;

typedef struct guiButton_s {
    kexContainer *container;
    guiButtonState_t state;
    kexStr name;
    kexLinklist<struct guiButton_s> link;
} guiButton_t;

class kexGui {
    friend class kexGuiManager;
public:
                                kexGui(void);
                                ~kexGui(void);
    
    void                        Draw(void);
    void                        CheckEvents(void);
    void                        ExecuteButtonEvent(guiButton_t *button, const guiButtonState_t btnState);
    
private:
    kexCanvas                   canvas;
    kexGui                      *prevGui;
    kexLinklist<kexGui>         link;
    kexLinklist<guiButton_t>    buttons;
    kexStr                      name;
    guiStatus_t                 status;
};

#include "tinyxml2.h"

class kexGuiManager {
public:
                                kexGuiManager(void);
                                ~kexGuiManager(void);
    
    void                        Init(void);
    kexGui                      *LoadGui(const char *guiFile);
    kexGui                      *FindGuiByName(const char *name);
    void                        DrawGuis(void);
    void                        DeleteGuis(void);
    bool                        ProcessInput(const event_t *ev);
    
    void                        Toggle(const bool bToggle) { bEnabled = bToggle; }
    
    kexLinklist<kexGui>         guis;
    float                       cursor_x;
    float                       cursor_y;
    kexMaterial                 *cursorMaterial;
    
    bool                        bDebugButtons;
    
private:
    void                        DrawCursor(void);
    void                        ParseNode(tinyxml2::XMLElement *element,
                                          kexGui *gui,
                                          kexContainer *container);
    void                        ParseSimpleProperties(tinyxml2::XMLNode *node,
                                                      kexCanvasObject *object);
    
    bool                        bEnabled;
};

extern kexGuiManager guiManager;

#endif
