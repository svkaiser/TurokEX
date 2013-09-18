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

enum {
    scmap_title = 0,
    scmap_mapID,
    scmap_glight_origin,
    scmap_glight_color,
    scmap_glight_ambience,
    scmap_glight_modelamb,
    scmap_actor,
    scmap_gridbounds,
    scmap_end
};

static const sctokens_t maptokens[scmap_end+1] = {
    { scmap_title,              "title"                 },
    { scmap_mapID,              "mapID"                 },
    { scmap_glight_origin,      "global_light_position" },
    { scmap_glight_color,       "global_light_color"    },
    { scmap_glight_ambience,    "global_light_ambience" },
    { scmap_glight_modelamb,    "global_model_ambience" },
    { scmap_actor,              "actor"                 },
    { scmap_gridbounds,         "gridbounds"            },
    { -1,                       NULL                    }
};

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
// kexWorld::ConstructActor
//

kexWorldActor *kexWorld::ConstructActor(const char *className) {
    kexObject *obj;
    kexRTTI *objType;
    
    if(!(objType = kexObject::Get(className)))
        return NULL;
        
    if(!(obj = objType->Create()))
        return NULL;
    
    return static_cast<kexWorldActor*>(obj);
}

//
// kexWorld::AddActor
//

void kexWorld::AddActor(kexWorldActor *actor) {
    actors.Add(actor->worldLink);
    actor->CallSpawn();
}

//
// kexWorld::RemoveActor
//

void kexWorld::RemoveActor(kexWorldActor *actor) {
    if(actor->RefCount() > 0)
        return;
        
    actor->worldLink.Remove();
    actor->Remove();
}

//
// kexWorld:SpawnActor
//

kexWorldActor *kexWorld::SpawnActor(const char *className, const kexVec3 &origin, const kexAngle &angles) {
    
    kexWorldActor *actor = ConstructActor(className);
    
    if(actor == NULL)
        return NULL;
        
    actor->SetOrigin(origin);
    actor->SetAngles(angles);
    
    AddActor(actor);
    return actor;
}

//
// kexWorld::Load
//

void kexWorld::Load(const char *mapFile) {
    if(client.GetState() < CL_STATE_READY || bLoaded)
        return;
        
    kexRand::SetSeed(-470403613);
    bLoaded = false;
    bReadyUnload = false;
    
    worldLightOrigin.Set(0, 0, 0, 0);
    worldLightColor.Set(1, 1, 1, 1);
    worldLightAmbience.Set(1, 1, 1, 1);
    worldLightModelAmbience.Set(1, 1, 1, 1);
    
    kexLexer *lexer;
    kexWorldActor *actor;
    
    if(!(lexer = parser.Open(mapFile)))
        return;

    // begin parsing
    while(lexer->CheckState()) {
        lexer->Find();
        
        switch(lexer->TokenType()) {
        case TK_NONE:
        case TK_EOF:
            break;
        case TK_IDENIFIER:
            switch(lexer->GetIDForTokenList(maptokens, lexer->Token())) {
            case scmap_title:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->GetString();
                title = lexer->StringToken();
                break;

            case scmap_mapID:
                lexer->AssignFromTokenList(maptokens, (unsigned int*)&mapID,
                    scmap_mapID, false);
                break;

            case scmap_glight_origin:
                lexer->AssignVectorFromTokenList(maptokens,
                    worldLightOrigin.ToVec3(), scmap_glight_origin, false);
                break;

            case scmap_glight_color:
                lexer->AssignVectorFromTokenList(maptokens,
                    worldLightColor.ToVec3(), scmap_glight_color, false);
                break;

            case scmap_glight_ambience:
                lexer->AssignVectorFromTokenList(maptokens,
                    worldLightAmbience.ToVec3(), scmap_glight_ambience, false);
                break;

            case scmap_glight_modelamb:
                lexer->AssignVectorFromTokenList(maptokens,
                    worldLightModelAmbience.ToVec3(), scmap_glight_modelamb, false);
                break;
                
            case scmap_actor:
                lexer->GetString();
                
                actor = ConstructActor(lexer->StringToken());
                actor->Parse(lexer);
                
                AddActor(actor);
                break;
            }
            break;
        }
    }

    parser.Close();
    
    nextMapID = -1;
    bLoaded = true;
    
    P_SpawnLocalPlayer();
    client.SetState(CL_STATE_INGAME);
}

//
// kexWorld::Unload
//

void kexWorld::Unload(void) {
}
