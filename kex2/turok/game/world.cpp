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
    scmap_gridbound,
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
    { scmap_gridbound,          "gridbound"             },
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

    this->gravity.Set(0, -1, 0);
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
    if(bLoaded == false) {
        if(nextMapID >= 0) {
            Load(GetMapFileFromID(nextMapID));
        }
        return;
    }

    ticks++;
    time = (float)(ticks * SERVER_RUNTIME);
    deltaTime = (float)server.GetElaspedTime();

    if(bReadyUnload) {
        Unload();
        return;
    }
}

//
// kexWorld::LocalTick
//

void kexWorld::LocalTick(void) {
    camera.LocalTick();
}

//
// kexWorld::ConstructActor
//

kexWorldActor *kexWorld::ConstructActor(const char *className) {
    kexObject *obj;
    kexRTTI *objType;
    
    if(!(objType = kexObject::Get(className))) {
        common.Error("kexWorld::ConstructActor: unknown class (\"%s\")\n", className);
        return NULL;
    }
        
    if(!(obj = objType->Create())) {
        common.Error("kexWorld::ConstructActor: could not spawn (\"%s\")\n", className);
        return NULL;
    }
    
    return static_cast<kexWorldActor*>(obj);
}

//
// kexWorld::AddActor
//

void kexWorld::AddActor(kexWorldActor *actor) {
    actors.Add(actor->worldLink);

    if(actor->GetName().Length() <= 0) {
        actor->SetName(kexStr(kva("%s_%i",
            actor->GetClassName(), kexWorldActor::id)));
    }

    actor->CallSpawn();
    kexWorldActor::id++;
}

//
// kexWorld::RemoveActor
//

void kexWorld::RemoveActor(kexWorldActor *actor) {
    if(actor->RefCount() > 0)
        return;

    kexWorldActor *next = actorRover->worldLink.Next();
        
    actor->worldLink.Remove();

   /* Note that actorRover is guaranteed to point to us,
    * and since we're freeing our memory, we had better change that. So
    * point it to actor->prev, so the iterator will correctly move on to
    * actor->prev->next = actor->next */
    actorRover = actor->worldLink.Prev();
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
// kexLocalPlayer::SpawnLocalPlayer
//

void kexWorld::SpawnLocalPlayer(void) {
    for(kexWorldActor *actor = actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
        
        // find a kexPlayerPuppet and see if its not occupied
        if(actor->GetOwner() != NULL || !actor->InstanceOf(&kexPlayerPuppet::info))
            continue;

        kexLocalPlayer *localPlayer = &client.LocalPlayer();
        
        // set client's position
        localPlayer->SetOrigin(actor->GetOrigin());
        localPlayer->SetAngles(actor->GetAngles());
        localPlayer->GetAcceleration().Clear();
        
        // take control of this actor
        localPlayer->PossessPuppet(static_cast<kexPlayerPuppet*>(actor));
        localPlayer->CallSpawn();
        return;
    }
    
    common.Error("kexLocalPlayer::SetPlayerActor: no player locations found\n");
}

//
// kexWorld::Trace
//

void kexWorld::Trace(kexPhysics *physics,
                     const kexVec3 &start,
                     const kexVec3 &end,
                     const kexVec3 &dir) {
    for(unsigned int i = 0; i < gridBounds.Length(); i++) {
        gridBound_t *grid = gridBounds[i];

        grid->bTraced = false;

        if(grid->box.LineIntersect(start, end)) {
            grid->bTraced = true;

            for(kexWorldActor *actor = grid->staticActors.Next();
                actor != NULL; actor = actor->worldLink.Next()) {
                    if(actor->bHidden || !actor->bCollision)
                        continue;

                    kexBBox box;

                    box.min = actor->BoundingBox().min + actor->GetOrigin();
                    box.max = actor->BoundingBox().max + actor->GetOrigin();

                    if(box.LineIntersect(start, end)) {
                        actor->bTraced = true;

                        // do simple sphere intersection test if no collision
                        // mesh is present
                        if(actor->ClipMesh().GetType() == CMT_NONE) {
                            actor->Trace(physics, start, end, dir);
                        }
                        else {
                            actor->ClipMesh().Trace(physics, start, end, dir);
                        }
                    }
            }
        }
    }
}

//
// kexWorld::ParseGridBound
//

void kexWorld::ParseGridBound(kexLexer *lexer) {
    gridBound_t *grid = new gridBound_t;

    grid->box.min = lexer->GetVector3();
    grid->box.max = lexer->GetVector3();

    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    kexWorldActor *actor;

    while(lexer->TokenType() != TK_RBRACK) {
        switch(lexer->GetIDForTokenList(maptokens, lexer->Token())) {
        case scmap_actor:
            lexer->GetString();
            actor = ConstructActor(lexer->StringToken());
            actor->Parse(lexer);
            grid->staticActors.Add(actor->worldLink);
            actor->CallSpawn();
            break;
        default:
            if(lexer->TokenType() == TK_IDENIFIER) {
                parser.Error("kexWorld::ParseGridBound: unknown token: %s\n",
                    lexer->Token());
            }
            break;
        }
        lexer->Find();
    }

    gridBounds.Push(grid);
}

//
// kexWorld::LinkGridBounds
//

void kexWorld::LinkGridBounds(void) {
    for(unsigned int i = 0; i < gridBounds.Length(); i++) {
        gridBound_t *grid = gridBounds[i];

        for(unsigned int j = 0; j < gridBounds.Length(); j++) {
            gridBound_t *nGrid = gridBounds[j];

            if(grid == nGrid) {
                continue;
            }

            if(grid->box.IntersectingBox(nGrid->box)) {
                grid->linkedBounds.Push(nGrid);
            }
        }
    }
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
            case scmap_gridbound:
                ParseGridBound(lexer);
                break;
            default:
                if(lexer->TokenType() == TK_IDENIFIER) {
                    parser.Error("kexWorld::Load: unknown token: %s\n",
                        lexer->Token());
                }
                break;
            }
            break;
        }
    }

    parser.Close();
    
    nextMapID = -1;
    bLoaded = true;

    LinkGridBounds();
}

//
// kexWorld::Unload
//

void kexWorld::Unload(void) {
}

//
// kexWorld::InitObject
//

void kexWorld::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kWorld",
        sizeof(kexWorld),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
        
    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "kCamera @Camera(void)",
        asMETHODPR(kexWorld, Camera, (void), kexCamera*),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "float DeltaTime(void)",
        asMETHODPR(kexWorld, DeltaTime, (void), float),
        asCALL_THISCALL);
        
    scriptManager.Engine()->RegisterGlobalProperty(
        "kWorld LocalWorld",
        &localWorld);
}
