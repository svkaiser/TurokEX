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

#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "common.h"
#include "keymap.h"
#include "scriptAPI/component.h"

//-----------------------------------------------------------------------------
//
// kexGameManager
//
//-----------------------------------------------------------------------------

class kexGameManager : public kexComponent {
public:
                            kexGameManager(void);
                            ~kexGameManager(void);

    virtual void            Construct(const char *className);
    virtual bool            CallConstructor(const char *decl);

    void                    InitGame(void);
    void                    Shutdown(void);
    void                    OnShutdown(void);
    void                    SetTitle(void);

    kexKeyMap               *GameDef(void) { return gameDef; }

    static void             Init(void);

private:
    asIScriptFunction       *onTick;
    asIScriptFunction       *onLocalTick;
    asIScriptFunction       *onSpawn;
    asIScriptFunction       *onShutdown;

    kexKeyMap               *gameDef;
};

extern kexGameManager       gameManager;

#endif
