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

#include "enet/enet.h"
#include "common.h"
#include "keymap.h"
#include "renderBackend.h"
#include "client.h"
#include "scriptAPI/component.h"
#include "player/player.h"

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
    void                    SetMainGui(const kexStr &name);
    void                    FadeMainGui(const bool fadeIn, const float speed);
    bool                    GuisAreActive(void);
    void                    ClearGuis(const float fadeSpeed);
    void                    SpawnGame(void);
    void                    StartGame(const char *mapname);
    bool                    ProcessInput(const event_t *ev);
    void                    OnTick(void);
    void                    OnLocalTick(void);
    void                    ClientEvent(const int type, const ENetPacket *packet);
    void                    ServerEvent(const int type, const ENetPacket *packet);
    int                     GetPlayerID(ENetPeer *peer) const;
    bool                    ConnectPlayer(ENetEvent *sev);
    void                    NotifyMapChange(ENetEvent *sev, const int mapID);
    void                    ClientRequestMapChange(const int mapID);
    void                    PrintDebugStats(void);

    kexKeyMap               *GameDef(void) { return gameDef; }
    kexCanvas               &MenuCanvas(void) { return menuCanvas; }

    static void             Init(void);
    static void             InitObject(void);
    
    kexPlayer               players[MAX_PLAYERS];
    kexLocalPlayer          localPlayer;

    bool                    bPrintStats;

private:
    void                    HandleMapChangeRequest(const ENetPacket *packet);
    void                    PrepareMapChange(const ENetPacket *packet);
    void                    SetupClientInfo(const ENetPacket *packet);
    
    asIScriptFunction       *onTick;
    asIScriptFunction       *onLocalTick;
    asIScriptFunction       *onSpawn;
    asIScriptFunction       *onNewGame;
    asIScriptFunction       *onShutdown;
    asIScriptFunction       *onInput;

    kexKeyMap               *gameDef;
    kexCanvas               menuCanvas;

    int                     gameTimeMS;
};

extern kexGameManager       gameManager;

#endif
