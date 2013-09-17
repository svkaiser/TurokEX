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
//
// DESCRIPTION: World system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "mathlib.h"
#include "client.h"
#include "server.h"
#include "js.h"
#include "jsobj.h"
#include "js_parse.h"
#include "js_shared.h"
#include "world.h"

kexWorld localWorld;

//
// kexWorld::kexWorld
//

kexWorld::kexWorld(void) {
    this->mapID         = -1;
    this->nextMapID     = -1;
    this->bLoaded       = false;
    this->bReadyUnload  = false;
    this->ticks         = 0;
    this->time          = 0;
    this->deltaTime     = 0;
}

//
// kexWorld::~kexWorld
//

kexWorld::~kexWorld(void) {
}

//
// kexWorld::Init
//

void kexWorld::Init(void) {
}

//
// kexWorld::GetMapFileFromID
//

const char *kexWorld::GetMapFileFromID(const int id) {
    return NULL;
}

//
// kexWorld::Tick
//

void kexWorld::Tick(void) {
    if(bLoaded == false)
    {
        if(nextMapID >= 0)
            Load(GetMapFileFromID(nextMapID));

        return;
    }

    ticks++;
    time = (float)(ticks * SERVER_RUNTIME);
    deltaTime = (float)server.GetElaspedTime();

    if(bReadyUnload)
        Unload();
}

//
// kexWorld::LocalTick
//

void kexWorld::LocalTick(void) {
}

//
// kexWorld::Load
//

void kexWorld::Load(const char *mapFile) {
}

//
// kexWorld::Unload
//

void kexWorld::Unload(void) {
}
