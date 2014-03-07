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
#include "object.h"
#include "script.h"
#include "mathlib.h"
#include "client.h"
#include "server.h"
#include "world.h"
#include "sound.h"
#include "renderSystem.h"
#include "gameManager.h"
#include "defs.h"

#define FOG_LERP_SPEED      0.025f

#define NODE_MAX_CONTENTS   16
#define NODE_MAX_SIZE       128

kexHeapBlock kexWorld::hb_world("world", false, NULL, NULL);

kexWorld localWorld;

enum {
    scmap_title = 0,
    scmap_mapID,
    scmap_glight_origin,
    scmap_glight_color,
    scmap_glight_ambience,
    scmap_glight_modelamb,
    scmap_actor,
    scmap_staticActors,
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
    { scmap_staticActors,       "staticActors"          },
    { -1,                       NULL                    }
};

//
// kexWorld::kexWorld
//

kexWorld::kexWorld(void) {
    this->validcount        = 0;
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
    kexActor *next;

    if(bLoaded == false) {
        return;
    }

    ticks++;
    time = (float)(ticks * SERVER_RUNTIME);
    deltaTime = (float)server.GetElaspedTime();

    // don't update on first two ticks
    if(ticks <= 1) {
        return;
    }

    for(actorRover = actors.Next(); actorRover != NULL; actorRover = next) {
        next = actorRover->worldLink.Next();

        if(actorRover->bClientOnly || actorRover->bStatic) {
            continue;
        }

        actorRover->OldTimeStamp() = actorRover->TimeStamp();
        actorRover->FracTime() = 0;
        actorRover->TimeStamp() = deltaTime;
        actorRover->Tick();
    }
}

//
// kexWorld::LocalTick
//

void kexWorld::LocalTick(void) {
    float cfrac;
    kexActor *next;

    if(bLoaded == false) {
        return;
    }

    camera.LocalTick();

    if(bEnableFog) {
        currentFogRGB[0]    = (fogRGB[0] - currentFogRGB[0]) * FOG_LERP_SPEED + currentFogRGB[0];
        currentFogRGB[1]    = (fogRGB[1] - currentFogRGB[1]) * FOG_LERP_SPEED + currentFogRGB[1];
        currentFogRGB[2]    = (fogRGB[2] - currentFogRGB[2]) * FOG_LERP_SPEED + currentFogRGB[2];
        currentFogFar       = (fogFar - currentFogFar) * FOG_LERP_SPEED + currentFogFar;
        currentFogNear      = (fogNear - currentFogNear) * FOG_LERP_SPEED + currentFogNear;

        if(camera.IsClampZFarToFog()) {
            camera.ZFar() = currentFogFar;
        }
    }

    // don't update on first two ticks
    if(ticks <= 1) {
        return;
    }

    for(actorRover = actors.Next(); actorRover != NULL; actorRover = next) {
        next = actorRover->worldLink.Next();
        actorRover->LocalTick();

        if(actorRover == NULL) {
            common.Warning("kexWorld::LocalTick: actorRover went null for some odd reason\n");
            continue;
        }

        cfrac = -(actorRover->TimeStamp() -
            client.GetTime()) * (SERVER_RUNTIME / 10000.0f);

        if(cfrac > 1) {
            cfrac = 1;
        }
        if(cfrac < 0) {
            cfrac = 0;
        }

        actorRover->FracTime() = cfrac;

        if(actorRover->Removing()) {
            RemoveActor(actorRover);
        }
    }

    fxManager.UpdateWorld(this);
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

kexActor *kexWorld::ConstructActor(const char *className) {
    return static_cast<kexActor*>(ConstructObject(className));
}

//
// kexWorld::AddActor
//

void kexWorld::AddActor(kexActor *actor) {
    actor->worldLink.Add(actors);

    if(actor->GetName().Length() <= 0) {
        actor->SetName(kexStr(kva("%s_%i",
            actor->ClassName(), kexActor::id)));
    }

    actor->CallSpawn();
}

//
// kexWorld::RemoveActor
//

void kexWorld::RemoveActor(kexActor *actor) {
    actor->SetOwner(NULL);
    actor->SetTarget(NULL);
    actor->worldLink.Remove();
    actor->UnlinkArea();
    delete actor;
}

//
// kexWorld::TeleportActor
//

void kexWorld::TeleportActor(kexActor *actor, const kexVec3 &position,
                             const kexAngle &angle, int sectorID) {

    kexSector *sector = NULL;

    if(sectorID != -1 && sectorID >= 0 && sectorID < collisionMap.numSectors) {
        sector = &collisionMap.sectors[sectorID];
    }
    else {
        sector = collisionMap.PointInSector(position);
    }

    actor->UnlinkArea();

    actor->Physics()->sector = sector;
    actor->SetOrigin(position);
    actor->SetAngles(angle);

    actor->LinkArea();

    if(actor->Physics()->sector) {
        if(actor->Physics()->sector->area) {
            actor->Physics()->sector->area->Enter();
        }
    }
}

//
// kexWorld::SpawnActor
// 

kexActor *kexWorld::SpawnActor(const char *className, const char *component,
                               const kexVec3 &origin, const kexAngle &angles) {
    
    kexActor *actor = ConstructActor(className);
    
    if(actor == NULL) {
        return NULL;
    }

    if(component != NULL) {
        actor->CreateComponent(component);
    }

    actor->SetOrigin(origin);
    actor->SetAngles(angles);
    actor->LinkArea();
    
    AddActor(actor);
    return actor;
}

//
// kexWorld::SpawnActor
//

kexActor *kexWorld::SpawnActor(kexStr &className, kexStr &component,
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
// kexWorld::SpawnActor
//

kexActor *kexWorld::SpawnActor(const char *definition,
                               const kexVec3 &origin, const kexAngle &angles) {
    kexKeyMap *def;
    kexStr className;
    kexStr componentName;
    kexActor *actor;
    
    if(!(def = defManager.FindDefEntry(definition))) {
        return NULL;
    }

    def->GetString("classname", className);
    def->GetString("component", componentName);

    if(className.Length() <= 0) {
        return NULL;
    }

    actor = ConstructActor(className);
    
    if(actor == NULL) {
        return NULL;
    }

    if(componentName.Length() > 0) {
        actor->CreateComponent(componentName.c_str());
    }

    actor->definition = def;
    actor->SetOrigin(origin);
    actor->SetAngles(angles);
    actor->LinkArea();
    
    AddActor(actor);
    return actor;
}

//
// kexWorld::SpawnActor
//

kexActor *kexWorld::SpawnActor(kexStr &definition, kexVec3 &origin, kexAngle &angles) {
    return SpawnActor(definition.c_str(), origin, angles);
}

//
// kexWorld::SpawnFX
//

kexFx *kexWorld::SpawnFX(const char *name, kexGameObject *source, kexVec3 &velocity,
                         kexVec3 &origin, kexQuat &rotation, kexFx *parentFx) {
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

            if(parentFx) {
                fx->SetParent(parentFx);
            }

            fx->worldLink.Add(fxList);
            fx->CallSpawn();
        }
    }

    return fx;
}

//
// kexWorld::SpawnFX
//

void kexWorld::SpawnFX(const kexStr &str, kexGameObject *source, kexVec3 &velocity,
                         kexVec3 &origin, kexQuat &rotation) {
    SpawnFX(str.c_str(), source, velocity, origin, rotation);
}

//
// kexLocalPlayer::SpawnLocalPlayer
//

void kexWorld::SpawnLocalPlayer(void) {
    for(kexActor *actor = actors.Next();
        actor != NULL; actor = actor->worldLink.Next()) {
        
        // find a kexPlayerPuppet and see if its not occupied
        if(actor->GetOwner() != NULL || !actor->InstanceOf(&kexPlayerPuppet::info)) {
            continue;
        }

        kexLocalPlayer *localPlayer = &gameManager.localPlayer;
        
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
// kexWorld::TraverseWorldNodes
//

void kexWorld::TraverseWorldNodes(worldNode_t *node, traceInfo_t *trace) {
    if(node->bLeaf) {
        kexBBox box;
        kexActor *actor;
        float r = 16.384f;

        if(trace->bUseBBox) {
            r = trace->localBBox.Radius();
        }

        for(unsigned i = 0; i < node->actors.Length(); i++) {
            actor = node->actors[i];

            if(actor->validcount == validcount) {
                // already checked this one
                continue;
            }

            box = actor->Bounds() + r;
            actor->validcount = validcount;

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

        return;
    }

    kexVec3 normal = node->plane.Normal();

    float d1 = normal.Dot(trace->start) - node->plane.d;
    float d2 = normal.Dot(trace->end) - node->plane.d;

    if((d1 >= 0 || d2 >= 0) && node->children[0]) {
        TraverseWorldNodes(node->children[0], trace);
    }
    
    if((d1 < 0 || d2 < 0) && node->children[1]) {
        TraverseWorldNodes(node->children[1], trace);
    }
}

//
// kexWorld::Trace
//

void kexWorld::Trace(traceInfo_t *trace, const int clipFlags) {
    trace->fraction = 1.0f;
    trace->hitActor = NULL;
    trace->hitTri = NULL;
    trace->hitMesh = NULL;
    trace->hitVector = trace->end;
    trace->hitNormal.Clear();

    if(trace->owner && trace->owner->areaNode) {
        for(kexWorldObject *obj = trace->owner->areaNode->objects.Next();
            obj != NULL;
            obj = obj->areaLink.Next()) {
                if(obj == trace->owner || !obj->bCollision) {
                    continue;
                }
                obj->Trace(trace);
        }
    }

    TraverseWorldNodes(&worldNode, trace);

    if(trace->hitTri) {
        trace->hitTri->bTraced = true;
    }

    if(collisionMap.IsLoaded() == true) {
        cMapTraceResult_t cmResult;

        collisionMap.Trace(&cmResult, trace->start, trace->end,
            *trace->sector,
            clipFlags,
            (trace->owner && trace->bUseBBox) ? trace->owner->BaseHeight() : 0);

        if(cmResult.sector) {
            *trace->sector = cmResult.sector;
            cmResult.sector->bTraced = true;
        }

        if(cmResult.fraction < 1 && cmResult.fraction < trace->fraction) {
            trace->fraction = cmResult.fraction;
            trace->hitNormal = cmResult.normal;
            trace->hitVector = cmResult.position;

            trace->hitActor = NULL;
            trace->hitMesh = NULL;

            if(cmResult.sector != NULL) {
                trace->hitTri = &cmResult.sector->lowerTri;
            }

            if(cmResult.contactSector) {
                trace->hitTri = &cmResult.contactSector->lowerTri;
                cmResult.contactSector->bTraced = true;
            }
            else if(cmResult.bClippedEdge) {
                trace->hitTri = NULL;
            }
        }
    }

    validcount++;
}

//
// kexWorld::StartSound
//

void kexWorld::StartSound(const char *name) {
    soundSystem.StartSound(name, NULL);
}

//
// kexWorld::StartSound
//

void kexWorld::StartSound(const kexStr &name) {
    StartSound(name.c_str());
}

//
// kexWorld::SetFogRGB
//

void kexWorld::SetFogRGB(float r, float g, float b) {
    fogRGB[0] = r; fogRGB[1] = g; fogRGB[2] = b;
}

//
// kexWorld::TriggerActor
//

void kexWorld::TriggerActor(const int targetID) {
    for(actorRover = actors.Next(); actorRover != NULL;
        actorRover = actorRover->worldLink.Next()) {
            if(actorRover->bStatic) {
                continue;
            }
            if(actorRover->Removing()) {
                continue;
            }
            if(actorRover->TargetID() == targetID) {
                actorRover->OnTrigger();
            }
    }
}

//
// kexWorld::Load
//

bool kexWorld::Load(const char *mapFile) {
    if(client.GetState() < CL_STATE_READY || bLoaded) {
        return false;
    }

    kexRand::SetSeed(-470403613);
    bLoaded = false;
    bReadyUnload = false;
    ticks = 0;
    
    worldLightOrigin.Set(0, 0, 0, 0);
    worldLightColor.Set(1, 1, 1, 1);
    worldLightAmbience.Set(1, 1, 1, 1);
    worldLightModelAmbience.Set(1, 1, 1, 1);
    
    kexLexer *lexer;
    kexActor *actor;
    kexStr file(mapFile);

    renderSystem.DrawLoadingScreen("Loading Collision...");
    collisionMap.Load((file + ".kclm").c_str());
    
    if(!(lexer = parser.Open((file + ".kmap").c_str()))) {
        return false;
    }

    renderSystem.DrawLoadingScreen("Loading Objects...");

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
            case scmap_staticActors:
                // read into nested block
                lexer->ExpectNextToken(TK_LBRACK);
                lexer->Find();
                renderSystem.DrawLoadingScreen("Loading Static Meshes...");
                while(lexer->TokenType() != TK_RBRACK) {
                    switch(lexer->GetIDForTokenList(maptokens, lexer->Token())) {
                    case scmap_actor:
                        lexer->GetString();
                        actor = ConstructActor(lexer->StringToken());
                        actor->Parse(lexer);
                        actor->worldLink.Add(staticActors);
                        actor->CallSpawn();
                        break;
                    default:
                        if(lexer->TokenType() == TK_IDENIFIER) {
                            parser.Error("kexWorld::Load: unknown token: %s\n",
                                lexer->Token());
                        }
                        break;
                    }
                    lexer->Find();
                }

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

    BuildWorldNodes();
    BuildAreaNodes();

    SpawnLocalPlayer();
    return true;
}

//
// kexWorld::Unload
//

void kexWorld::Unload(void) {
    kexLocalPlayer *localPlayer;
    kexActor *actor;
    kexActor *next;
    kexFx *nextFx;

    if(bLoaded == false) {
        // nothing is currently loaded
        return;
    }

    common.Printf("Unloading %s\n", title);
    
    bLoaded = false;
    
    soundSystem.StopAll();
    
    localPlayer = &gameManager.localPlayer;
    localPlayer->UnpossessPuppet();

    camera.Attachment().DettachObject();
    camera.SetTarget(NULL);
    camera.SetOwner(NULL);

    if(collisionMap.IsLoaded()) {
        collisionMap.Unload();
    }
    
    // remove all fx
    for(fxRover = fxList.Next(); fxRover != NULL; fxRover = nextFx) {
        nextFx = fxRover->worldLink.Prev();

        fxRover->SetParent(NULL);
        fxRover->worldLink.Remove();
        fxRover->UnlinkArea();

        delete fxRover;
    }

    // remove all actors
    for(actor = actors.Next(); actor != NULL; actor = next) {
        next = actor->worldLink.Next();
        RemoveActor(actor);
    }
    
    // remove all static actors
    for(actor = staticActors.Next(); actor != NULL; actor = next) {
        next = actor->worldLink.Next();

        actor->worldLink.Remove();
        delete actor;
    }

    fxList.Clear();
    actors.Clear();
    staticActors.Clear();
    
    Mem_Purge(hb_world);

    worldNode.actors.Empty();
    worldNode.children[0] = NULL;
    worldNode.children[1] = NULL;
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
        asMETHODPR(kexWorld, StartSound, (const kexStr &), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "const int MapID(void) const",
        asMETHODPR(kexWorld, MapID, (void) const, const int),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "bool IsLoaded(void) const",
        asMETHODPR(kexWorld, IsLoaded, (void) const, bool),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "kActor @SpawnActor(kStr &in, kStr &in, kVec3 &in, kAngle &in)",
        asMETHODPR(kexWorld, SpawnActor,
        (kexStr &className, kexStr &component,
        kexVec3 &origin, kexAngle &angles), kexActor*),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "kActor @SpawnActor(kStr &in, kVec3 &in, kAngle &in)",
        asMETHODPR(kexWorld, SpawnActor,
        (kexStr &definition, kexVec3 &origin, kexAngle &angles), kexActor*),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "void TeleportActor(kActor@, const kVec3 &in, const kAngle &in, int sectorID = -1)",
        asMETHODPR(kexWorld, TeleportActor,
        (kexActor *actor, const kexVec3 &position, const kexAngle &angle, int sectorID), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kWorld",
        "void SpawnFX(kStr &in, kActor@, kVec3 &in, kVec3 &in, kQuat &in)",
        asMETHODPR(kexWorld, SpawnFX,
        (const kexStr &str, kexGameObject *source, kexVec3 &velocity,
        kexVec3 &origin, kexQuat &rotation), void),
        asCALL_THISCALL);
        
    scriptManager.Engine()->RegisterGlobalProperty(
        "kWorld LocalWorld",
        &localWorld);
}

//
// kexWorld::SetupChildWorldNode
//

bool kexWorld::SetupChildWorldNode(worldNode_t *parent, worldNode_t *child,
                                    float *splitX, float *splitZ,
                                    nodeSide_t side, int split) {
    float rx;
    float rz;
    float r;
    kexBBox *box = &child->bounds;
    kexBBox abox;

    // resize the bounding box for the child node
    switch(side) {
        case NODE_FRONT:
            if(split == 0) {
                box->min.Set(parent->bounds.min.x, 0, parent->bounds.min.z);
                box->max.Set(parent->bounds.max.x, 0, splitZ[1]);
            }
            else {
                box->min.Set(parent->bounds.min.x, 0, parent->bounds.min.z);
                box->max.Set(splitX[1], 0, parent->bounds.max.z);
            }
            break;
        case NODE_BACK:
            if(split == 0) {
                box->min.Set(parent->bounds.min.x, 0, splitZ[0]);
                box->max.Set(parent->bounds.max.x, 0, parent->bounds.max.z);
            }
            else {
                box->min.Set(splitX[0], 0, parent->bounds.min.z);
                box->max.Set(parent->bounds.max.x, 0, parent->bounds.max.z);
            }
            break;
    }

    rx = box->max.x - ((box->max.x + box->min.x) * 0.5f);
    rz = box->max.z - ((box->max.z + box->min.z) * 0.5f);
    r = kexMath::Sqrt(rx*rx+rz*rz);

    // test all actors bounding boxes with this node
    for(kexActor *actor = staticActors.Next(); actor != NULL;
        actor = actor->worldLink.Next()) {
            if(actor->bStatic == false || actor->bCollision == false) {
                continue;
            }
            
            abox = actor->Bounds();

            // expand a little bit
            abox += (((abox.max - abox.Center()).Unit()) * 0.5f);
            
            if(abox.max.x > child->bounds.max.x && abox.min.x < child->bounds.min.x &&
                abox.max.z > child->bounds.max.z && abox.min.z < child->bounds.min.z) {
                    // node box completely inside actor box
                    child->actors.Push(actor);
                    continue;
            }

            if(!(abox.max.x < child->bounds.min.x || abox.max.z < child->bounds.min.z ||
                abox.min.x > child->bounds.max.x || abox.min.z > child->bounds.max.z)) {
                    // node box intersecting with actor box
                    child->actors.Push(actor);
            }
    }

    // determine if we need to break this grid volume down even more
    if(child->actors.Length() > NODE_MAX_CONTENTS && r >= NODE_MAX_SIZE) {
        child->actors.Empty();
        return false;
    }
    else {
        child->bLeaf = true;
    }
    
    return true;
}

//
// kexWorld::AddWorldNode
//

void kexWorld::AddWorldNode(worldNode_t *node, int split) {
    float mx = (node->bounds.max.x + node->bounds.min.x) * 0.5f;
    float mz = (node->bounds.max.z + node->bounds.min.z) * 0.5f;
    float splitX[2];
    float splitZ[2];
    worldNode_t *frontNode;
    worldNode_t *backNode;
    kexPlane *plane;

    if(split == 0) {
        splitX[0] = node->bounds.min.x;
        splitZ[0] = mz;
        splitX[1] = node->bounds.max.x;
        splitZ[1] = mz;
    }
    else {
        splitX[0] = mx;
        splitZ[0] = node->bounds.min.z;
        splitX[1] = mx;
        splitZ[1] = node->bounds.max.z;
    }

    plane = &node->plane;

    // splitting planes are either vertical or horizontal
    plane->a = (split == 0 ? 0 : -1.0f);
    plane->b = 0;
    plane->c = (split == 0 ? -1.0f : 0);
    plane->d = (plane->a * splitX[0] + plane->c * splitZ[0]);

    frontNode = (worldNode_t*)Mem_Calloc(sizeof(worldNode_t), hb_world);
    node->children[NODE_FRONT] = frontNode;

    if(!SetupChildWorldNode(node, frontNode, splitX, splitZ, NODE_FRONT, split)) {
        AddWorldNode(frontNode, split ^ 1);
    }

    backNode = (worldNode_t*)Mem_Calloc(sizeof(worldNode_t), hb_world);
    node->children[NODE_BACK] = backNode;

    if(!SetupChildWorldNode(node, backNode, splitX, splitZ, NODE_BACK, split)) {
        AddWorldNode(backNode, split ^ 1);
    }
}

//
// kexWorld::BuildWorldNodes
//
// Recursively split a group of bounding boxes
// into grid volumes
//

void kexWorld::BuildWorldNodes(void) {
    kexActor *actor;

    float maxX = -M_INFINITY;
    float maxZ = -M_INFINITY;
    float minX =  M_INFINITY;
    float minZ =  M_INFINITY;

    kexBBox box;
    int count = 0;

    // setup the root node bounding box
    for(actor = staticActors.Next(); actor != NULL;
        actor = actor->worldLink.Next()) {
            if(actor->bStatic == false || actor->bCollision == false) {
                continue;
            }

            box = actor->Bounds();
        
            if(box.min[0] < minX) minX = box.min[0];
            if(box.min[2] < minZ) minZ = box.min[2];
            if(box.max[0] > maxX) maxX = box.max[0];
            if(box.max[2] > maxZ) maxZ = box.max[2];

            count++;
    }

    if(count <= 0) {
        return;
    }

    worldNode.bounds.min.Set(minX, 0, minZ);
    worldNode.bounds.max.Set(maxX, 0, maxZ);

    AddWorldNode(&worldNode, 0);
}

//
// kexWorld::AddAreaNode
//

areaNode_t *kexWorld::AddAreaNode(int depth, kexBBox &box) {
    areaNode_t *node = &areaNodes[numAreaNodes++];

    node->objects.Clear();
    node->bounds = box;

    if(depth == MAX_AREA_DEPTH || ((box.max - box.min) * 0.5f).Unit() <= 512) {
        node->axis = -1;
        node->children[0] = NULL;
        node->children[1] = NULL;
    }
    else {
        kexVec3 size = box.max - box.min;
        kexBBox box1, box2;

        node->axis = (size.x > size.z) ? 0 : 2;
        node->dist = (box.max[node->axis] + box.min[node->axis]) * 0.5f;

        box1 = box;
        box2 = box;

        box1.max[node->axis] = node->dist;
        box2.min[node->axis] = node->dist;

        node->children[0] = AddAreaNode(depth+1, box2);
        node->children[1] = AddAreaNode(depth+1, box1);
    }

    return node;
}

//
// kexWorld::BuildAreaNodes
//

void kexWorld::BuildAreaNodes(void) {
    float maxX = -M_INFINITY;
    float maxZ = -M_INFINITY;
    float minX =  M_INFINITY;
    float minZ =  M_INFINITY;
    kexBBox box;

    numAreaNodes = 0;
    memset(areaNodes, 0, sizeof(areaNodes));

    if(collisionMap.IsLoaded()) {
        for(int i = 0; i < collisionMap.numSectors; i++) {
            kexSector *s;

            s = &collisionMap.sectors[i];

            box = s->lowerTri.bounds;

            if(box.min[0] < minX) minX = box.min[0];
            if(box.min[2] < minZ) minZ = box.min[2];
            if(box.max[0] > maxX) maxX = box.max[0];
            if(box.max[2] > maxZ) maxZ = box.max[2];
        }
    }
    else {
        for(kexActor *actor = staticActors.Next(); actor != NULL;
            actor = actor->worldLink.Next()) {
                if(actor->bStatic == false || actor->bCollision == false) {
                    continue;
                }

                box = actor->Bounds();
            
                if(box.min[0] < minX) minX = box.min[0];
                if(box.min[2] < minZ) minZ = box.min[2];
                if(box.max[0] > maxX) maxX = box.max[0];
                if(box.max[2] > maxZ) maxZ = box.max[2];
        }
    }

    box.min.Set(minX, 0, minZ);
    box.max.Set(maxX, 0, maxZ);

    AddAreaNode(0, box);

    for(actorRover = actors.Next(); actorRover != NULL;
        actorRover = actorRover->worldLink.Next()) {
            actorRover->LinkArea();
    }
}
