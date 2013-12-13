// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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

#ifndef __AREAS_H__
#define __AREAS_H__

#include "common.h"
#include "keymap.h"
#include "scriptAPI/component.h"

class kexArea {
public:
                                    kexArea(void);

    void                            CreateComponent(const char *name);

    const float                     WaterPlane(void) const { return waterplane; }
    const unsigned int              TargetID(void) const { return targetID; }
    const word                      FloorSurfaceType(void) const { return fSurfaceID; }
    const word                      CeilingSurfaceType(void) const { return cSurfaceID; }
    const word                      WallSurfaceType(void) const { return wSurfaceID; }

    static void                     InitObject(void);

    kexKeyMap                       keyMap;
    kexAreaComponent                scriptComponent;

private:
    float                           waterplane;
    unsigned int                    targetID;
    char                            *triggerSound;
    word                            fSurfaceID;
    word                            cSurfaceID;
    word                            wSurfaceID;
    byte                            globalFogRGB[3];
    float                           globalFogZFar;
};

#endif
