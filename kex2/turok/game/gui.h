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
    GUIS_READY  = 0,
    GUIS_FADEIN,
    GUIS_FADEOUT
} guiStatus_t;

class kexGui {
    friend class kexGuiManager;
public:
                                kexGui(void);
                                ~kexGui(void);
    
    void                        Draw(void);
    
private:
    kexCanvas                   canvas;
    kexGui                      *prevGui;
    kexLinklist<kexGui>         link;
    kexStr                      name;
    guiStatus_t                 status;
};

#include "tinyxml2.h"

class kexGuiManager {
public:
                                kexGuiManager(void);
                                ~kexGuiManager(void);
    
    kexGui                      *LoadGui(const char *guiFile);
    kexGui                      *FindGuiByName(const char *name);
    void                        DrawGuis(void);
    
    kexLinklist<kexGui>         guis;
    
private:
    void                        ParseNode(tinyxml2::XMLElement *element,
                                          kexGui *gui,
                                          kexContainer *container);
    void                        ParseSimpleProperties(tinyxml2::XMLNode *node,
                                                      kexCanvasObject *object);
};

extern kexGuiManager guiManager;

#endif
