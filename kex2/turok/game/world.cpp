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
#include "sound.h"

#define FOG_LERP_SPEED  0.025f

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
    this->mapID             = -1;
    this->nextMapID         = -1;
    this->bLoaded           = false;
    this->bReadyUnload      = false;
    this->ticks             = 0;
    this->time              = 0;
    this->deltaTime         = 0;
    this->bEnableFog        = true;
    this->fogFar            = 1024;
    this->fogNear           = this->fogFar * 0.5f;
    this->currentFogFar     = this->fogFar;
    this->currentFogNear    = this->fogNear;

    this->SetFogRGB(0, 0, 0);
    this->fogRGB[3] = 1;
    memcpy(this->currentFogRGB, this->fogRGB, sizeof(float) * 4);

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

    for(actorRover = actors.Next(); actorRover != NULL;
        actorRover = actorRover->worldLink.Next()) {
        if(actorRover->bClientOnly || actorRover->bStatic) {
            continue;
        }
        actorRover->Tick();
    }
}

//
// kexWorld::LocalTick
//

void kexWorld::LocalTick(void) {
    camera.LocalTick();

    for(actorRover = actors.Next(); actorRover != NULL;
        actorRover = actorRover->worldLink.Next()) {
            if(actorRover->bStatic) {
                continue;
            }
            
            actorRover->LocalTick();

            if(actorRover->Removing()) {
                RemoveActor(actorRover);

                if(actorRover == NULL) {
                    break;
                }
            }
    }

    fxManager.UpdateWorld(this);

    if(bEnableFog) {
        currentFogRGB[0]    = (fogRGB[0] - currentFogRGB[0]) * FOG_LERP_SPEED + currentFogRGB[0];
        currentFogRGB[1]    = (fogRGB[1] - currentFogRGB[1]) * FOG_LERP_SPEED + currentFogRGB[1];
        currentFogRGB[2]    = (fogRGB[2] - currentFogRGB[2]) * FOG_LERP_SPEED + currentFogRGB[2];
        currentFogFar       = (fogFar - currentFogFar) * FOG_LERP_SPEED + currentFogFar;
        currentFogNear      = (fogNear - currentFogNear) * FOG_LERP_SPEED + currentFogNear;
    }
}

//
// kexWorld::ConstructObject
//

kexObject *kexWorld::ConstructObject(const char *className) {
    kexObject *obj;
    kexRTTI *objType;
    
    if(!(objType = kexObject::Get(className))) {
        common.Error("kexWorld::ConstructObject: unknown class (\"%s\")\n", className);
        return NULL;
    }
        
    if(!(obj = objType->Create())) {
        common.Error("kexWorld::ConstructObject: could not spawn (\"%s\")\n", className);
        return NULL;
    }
    
    return obj;
}

//
// kexWorld::ConstructActor
//

kexWorldActor *kexWorld::ConstructActor(const char *className) {
    return static_cast<kexWorldActor*>(ConstructObject(className));
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
    actor->worldLink.Remove();

   /* Note that actorRover is guaranteed to point to us,
    * and since we're freeing our memory, we had better change that. So
    * point it to actor->prev, so the iterator will correctly move on to
    * actor->prev->next = actor->next */
    actorRover = actor->worldLink.Prev();
    Z_Free(actor);

    actor = NULL;
}

//
// kexWorld::SpawnActor
// 

kexWorldActor *kexWorld::SpawnActor(const char *className, const char *component,
                                    const kexVec3 &origin, const kexAngle &angles) {
    
    kexWorldActor *actor = ConstructActor(className);
    
    if(actor == NULL)
        return NULL;

    if(component != NULL) {
        actor->CreateComponent(component);
    }

    actor->SetOrigin(origin);
    actor->SetAngles(angles);
    
    AddActor(actor);
    return actor;
}

//
// kexWorld::SpawnActor
//

kexWorldActor *kexWorld::SpawnActor(kexStr &className, kexStr &component,
                                    kexVec3 &origin, kexAngle &angles) {
    const char *componentName;

    if(component.Length() <= 0) {
        componentName = NULL;
    }
    else {
        componentName = component.c_str();
    }
    return SpawnActor(className.c_str(), componentName, origin, angles);
}

//
// kexWorld::SpawnFX
//

kexFx *kexWorld::SpawnFX(const char *name, kexActor *source, kexVec3 &velocity,
                         kexVec3 &origin, kexQuat &rotation) {
    kexFx *fx = NULL;
    fxfile_t *fxfile;
    fxinfo_t *info;

    if(!(fxfile = fxManager.LoadKFX(name))) {
        return NULL;
    }

    for(unsigned int i = 0; i < fxfile->numfx; i++) {
        bool ok = true;
        int instances;
        int spawnDice;

        info = &fxfile->info[i];

        instances = info->instances.value;
        spawnDice = instances + FX_RAND_VALUE((int)info->instances.rand);

        if(spawnDice <= 0) {
            continue;
        }

        // allow only one FX instance to be spawned per actor
        if(source && info->bActorInstance) {
            for(fxRover = fxList.Next(); fxRover != NULL; fxRover = fxRover->worldLink.Next()) {
                if(!fxRover->GetOwner()) {
                    continue;
                }

                if(fxRover->GetOwner() == source && fxRover->fxInfo == fxfile->info) {
                    fxRover->bForcedRestart = true;
                    ok = false;
                    break;
                }
            }
        }

        if(!ok) {
            continue;
        }

        if(instances <= 0) {
            instances = 1;
        }

        for(int j = 0; j < instances; j++) {
            if(!(fx = static_cast<kexFx*>(ConstructObject("kexFx")))) {
                continue;
            }

            fx->SetOrigin(origin);
            fx->SetRotation(rotation);
            fx->SetVelocityOffset(velocity);

            fx->fxFile = fxfile;
            fx->fxInfo = info;

            if(source) {
                fx->SetOwner(source);
            }

            fxList.Add(fx->worldLink);
            fx->CallSpawn();
        }
    }

    return fx;
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

void kexWorld::Trace(traceInfo_t *trace) {
    kexBBox box;

    trace->fraction = 1.0f;
    trace->hitActor = NULL;
    trace->hitTri = NULL;
    trace->hitMesh = NULL;
    trace->hitVector.Clear();
    trace->hitNormal.Clear();

    for(unsigned int i = 0; i < gridBounds.Length(); i++) {
        gridBound_t *grid = gridBounds[i];

        grid->bTraced = false;

        box = grid->box;

        if(trace->bUseBBox) {
            box = box + trace->localBBox.Radius();
        }

        if(box.LineIntersect(trace->start, trace->end)) {
            grid->bTraced = true;

            for(kexWorldActor *actor = grid->staticActors.Next();
                actor != NULL; actor = actor->worldLink.Next()) {
                    if(!actor->bCollision)
                        continue;

                    box.min = actor->BoundingBox().min + actor->GetOrigin();
                    box.max = actor->BoundingBox().max + actor->GetOrigin();

                    box = box + (box.Radius() * 0.5f);

                    if(box.LineIntersect(trace->start, trace->end)) {
                        actor->bTraced = true;

                        // do simple sphere intersection test if no collision
                        // mesh is present
                        if(actor->ClipMesh().GetType() == CMT_NONE) {
                            actor->Trace(trace);
                        }
                        else {
                            actor->ClipMesh().Trace(trace);
                        }
                    }
            }
        }
    }
}

//
// kexWorld::PlaySound
//

void kexWorld::PlaySound(const char *name) {
    soundSystem.PlaySound(name, NULL);
}

//
// kexWorld::PlaySound
//

void kexWorld::PlaySound(const kexStr &name) {
    PlaySound(name.c_str());
}

//
// kexWorld::SetFogRGB
//

void kexWorld::SetFogRGB(float r, float g, float b) {
    fogRGB[0] = r; fogRGB[1] = g; fogRGB[2] = b;
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

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "void SetFogRGB(float, float, float)",
        asMETHODPR(kexWorld, SetFogRGB, (float, float, float), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "float GetFogNear(void)",
        asMETHODPR(kexWorld, GetFogNear, (void), float),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "void SetFogNear(float)",
        asMETHODPR(kexWorld, SetFogNear, (float), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "float GetFogFar(void)",
        asMETHODPR(kexWorld, GetFogFar, (void), float),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "void SetFogFar(float)",
        asMETHODPR(kexWorld, SetFogFar, (float), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "bool FogEnabled(void)",
        asMETHODPR(kexWorld, FogEnabled, (void), bool),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "void ToggleFog(bool)",
        asMETHODPR(kexWorld, ToggleFog, (bool), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "const int GetTicks(void) const",
        asMETHODPR(kexWorld, GetTicks, (void)const, const int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "void PlaySound(const kStr &in)",
        asMETHODPR(kexWorld, PlaySound, (const kexStr &), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "kActor @SpawnActor(kStr &in, kStr &in, kVec3 &in, kAngle &in)",
        asMETHODPR(kexWorld, SpawnActor,
        (kexStr &className, kexStr &component,
        kexVec3 &origin, kexAngle &angles), kexWorldActor*),
        asCALL_THISCALL);
        
    scriptManager.Engine()->RegisterGlobalProperty(
        "kWorld LocalWorld",
        &localWorld);
}
