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
#include "renderBackend.h"
#include "gameManager.h"
#include "defs.h"
#include "worldModel.h"
#include "renderWorld.h"

#define FOG_LERP_SPEED      0.025f

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
    this->currentFogRGB.Set(0, 0, 0, 1);

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
        kexStr tmp(kva("%s_%i", actor->ClassName(), kexActor::id));
        actor->SetName(tmp);
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

            fx->worldLink.AddBefore(fxList);
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
// kexWorld::TraverseAreaNodes
//

void kexWorld::TraverseAreaNodes(traceInfo_t *trace, kexSDNodeObj<kexWorldObject> *areaNode) {
    float min, max;
    kexBBox box;
    float r;

    if(trace->bUseBBox) {
        r = trace->localBBox.Radius();
    }
    else {
        r = 16.384f;
    }

    for(kexWorldObject *obj = areaNode->objects.Next();
        obj != NULL;
        obj = obj->areaLink.link.Next()) {
            if(obj == trace->owner || (!obj->bCollision && !obj->bTouch)) {
                continue;
            }

            box = obj->Bounds();

            if(!obj->bTouch) {
                box += r;
            }

            if(trace->bUseBBox) {
                if(!box.IntersectingBox(trace->bbox)) {
                    continue;
                }
            }
            else if(!box.LineIntersect(trace->start, trace->end)) {
                continue;
            }

            if(trace->owner && trace->owner->bCanPickup && obj->bTouch) {
                obj->OnTouch(trace->owner);
                continue;
            }

            if(obj->InstanceOf(&kexWorldModel::info)) {
                kexWorldModel *wm = static_cast<kexWorldModel*>(obj);

                if(wm->ClipMesh().GetType() != CMT_NONE) {
                    wm->ClipMesh().Trace(trace);
                    continue;
                }
            }

            obj->Trace(trace);
    }

    if(areaNode->axis == -1) {
        return;
    }

    if(trace->bUseBBox) {
        max = trace->bbox.max[areaNode->axis];
        min = trace->bbox.min[areaNode->axis];
    }
    else if(trace->owner) {
        max = trace->owner->Bounds().max[areaNode->axis]+1;
        min = trace->owner->Bounds().min[areaNode->axis]-1;
    }
    else {
        max = trace->start[areaNode->axis];
        min = trace->start[areaNode->axis];
    }

    if(min > areaNode->dist || areaNode->axis == 1) {
        TraverseAreaNodes(trace, areaNode->children[NODE_FRONT]);
    }

    if(max < areaNode->dist || areaNode->axis == 1) {
        TraverseAreaNodes(trace, areaNode->children[NODE_BACK]);
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

    if(!(clipFlags & (PF_NOCLIPACTORS|PF_NOCLIPSTATICS))) {
        TraverseAreaNodes(trace, areaNodes.nodes);
    }

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
// kexWorld::BuildAreaNodes
//

void kexWorld::BuildAreaNodes(void) {
    areaNodes.Init(8);

    for(kexWorldModel *wm = staticActors.Next(); wm != NULL;
        wm = wm->worldLink.Next()) {
            if(wm->bStatic == false || wm->bCollision == false) {
                continue;
            }

            areaNodes.AddBoxToRoot(wm->Bounds());
    }

    areaNodes.BuildNodes();

    for(actorRover = actors.Next(); actorRover != NULL;
        actorRover = actorRover->worldLink.Next()) {
            actorRover->LinkArea();
    }

    for(kexWorldModel *wm = staticActors.Next();
        wm != NULL; wm = wm->worldLink.Next()) {
            if(wm->bCollision == false) {
                continue;
            }

            wm->LinkArea();
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
    kexWorldModel *wm;
    kexStr file(mapFile);

    renderBackend.DrawLoadingScreen("Loading Collision...");
    collisionMap.Load((file + ".kclm").c_str());
    
    if(!(lexer = parser.Open((file + ".kmap").c_str()))) {
        return false;
    }

    renderBackend.DrawLoadingScreen("Loading Objects...");

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
                while(lexer->TokenType() != TK_RBRACK) {
                    switch(lexer->GetIDForTokenList(maptokens, lexer->Token())) {
                    case scmap_actor:
                        lexer->GetString();
                        wm = static_cast<kexWorldModel*>(ConstructObject("kexWorldModel"));
                        wm->Parse(lexer);
                        wm->worldLink.Add(staticActors);
                        wm->CallSpawn();
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

    BuildAreaNodes();
    renderWorld.BuildNodes();
    
    SpawnLocalPlayer();
    return true;
}

//
// kexWorld::Unload
//

void kexWorld::Unload(void) {
    kexLocalPlayer *localPlayer;
    kexActor *actor;
    kexWorldModel *wm;
    kexActor *next;
    kexWorldModel *nextWm;
    kexFx *nextFx;

    if(bLoaded == false) {
        // nothing is currently loaded
        return;
    }

    common.Printf("Unloading %s\n", title.c_str());
    
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
    for(wm = staticActors.Next(); wm != NULL; wm = nextWm) {
        nextWm = wm->worldLink.Next();

        wm->worldLink.Remove();
        delete wm;
    }

    fxList.Clear();
    actors.Clear();
    staticActors.Clear();
    
    Mem_Purge(hb_world);

    areaNodes.Destroy();
    renderWorld.renderNodes.Destroy();
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
