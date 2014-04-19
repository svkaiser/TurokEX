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
// DESCRIPTION: Actor system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "actor.h"
#include "server.h"
#include "world.h"
#include "defs.h"
#include "renderBackend.h"
#include "pickup.h"

enum {
    scactor_name = 0,
    scactor_mesh,
    scactor_bounds,
    scactor_materials,
    scactor_component,
    scactor_bCollision,
    scactor_bHidden,
    scactor_bStatic,
    scactor_bTouch,
    scactor_bOrientOnSlope,
    scactor_bNoCull,
    scactor_origin,
    scactor_scale,
    scactor_angles,
    scactor_rotation,
    scactor_radius,
    scactor_height,
    scactor_centerheight,
    scactor_viewheight,
    scactor_targetID,
    scactor_cullDistance,
    scactor_tickDistance,
    scactor_physics,
    scactor_bNoFixedTransform,
    scactor_bAllowDamage,
    scactor_impactType,
    scactor_displayType,
    scactor_end
};

static const sctokens_t mapactortokens[scactor_end+1] = {
    { scactor_name,             "name"              },
    { scactor_mesh,             "mesh"              },
    { scactor_bounds,           "bounds"            },
    { scactor_materials,        "materials"         },
    { scactor_component,        "component"         },
    { scactor_bCollision,       "bCollision"        },
    { scactor_bHidden,          "bHidden"           },
    { scactor_bStatic,          "bStatic"           },
    { scactor_bTouch,           "bTouch"            },
    { scactor_bOrientOnSlope,   "bOrientOnSlope"    },
    { scactor_bNoCull,          "bNoCull"           },
    { scactor_origin,           "origin"            },
    { scactor_scale,            "scale"             },
    { scactor_angles,           "angles"            },
    { scactor_rotation,         "rotation"          },
    { scactor_radius,           "radius"            },
    { scactor_height,           "height"            },
    { scactor_centerheight,     "centerheight"      },
    { scactor_viewheight,       "viewheight"        },
    { scactor_targetID,         "targetID"          },
    { scactor_cullDistance,     "cullDistance"      },
    { scactor_tickDistance,     "tickDistance"      },
    { scactor_physics,          "physics"           },
    { scactor_bNoFixedTransform,"bNoFixedTransform" },
    { scactor_bAllowDamage,     "bAllowDamage"      },
    { scactor_impactType,       "impactType"        },
    { scactor_displayType,      "displayType"       },
    { -1,                       NULL                }
};

DECLARE_CLASS(kexActor, kexWorldObject)

unsigned int kexActor::id = 0;

//
// kexActor::kexActor
//

kexActor::kexActor(void) {
    this->baseBBox.min.Set(-32, -32, -32);
    this->baseBBox.max.Set(32, 32, 32);

    this->worldLink.SetData(this);
    this->scriptComponent.SetOwner(this);
    this->AnimState()->SetOwner(this);

    this->bbox              = baseBBox;
    this->bTraced           = false;
    this->validcount        = 0;
    this->bNoFixedTransform = false;
    this->bNoCull           = false;
    this->materials         = NULL;
    this->definition        = NULL;
    this->cullDistance      = 4096;
    this->wireframeColor    = RGBA(192, 192, 192, 255);
}

//
// kexActor::~kexActor
//

kexActor::~kexActor(void) {
    if(scriptComponent.ScriptObject() != NULL) {
        scriptComponent.Release();
    }

    if(nodeOffsets_t) {
        Mem_Free(nodeOffsets_t);
    }

    if(nodeOffsets_r) {
        Mem_Free(nodeOffsets_r);
    }

    if(model && materials) {
        Mem_Free(materials);
        materials = NULL;
    }
}

//
// kexActor::LocalTick
//

void kexActor::LocalTick(void) {
    if(bStatic == true || IsStale()) {
        return;
    }

    UpdateTransform();

    if(scriptComponent.onLocalThink) {
        scriptComponent.CallFunction(scriptComponent.onLocalThink);
    }

    animState.Update();
    physicsRef->Think(client.GetRunTime());
}

//
// kexActor::Tick
//

void kexActor::Tick(void) {
    if(bStatic == true) {
        return;
    }

    if(scriptComponent.onThink) {
        scriptComponent.CallFunction(scriptComponent.onThink);
    }
}

//
// kexActor::Spawn
//

void kexActor::Spawn(void) {
    physicsRef = &this->physics;
    physicsRef->SetOwner(this);
    physicsRef->bEnabled = false;

    if(definition != NULL) {
        kexStr str;

        definition->GetBool("bStatic", bStatic);
        definition->GetBool("bOrientOnSlope", bOrientOnSlope);
        definition->GetBool("bCollision", bCollision);
        definition->GetBool("bHidden", bHidden);
        definition->GetBool("bTouch", bTouch);
        definition->GetBool("bNoFixedTransform", bNoFixedTransform);
        definition->GetBool("bAllowDamage", bAllowDamage);
        definition->GetBool("bNoCull", bNoCull);
        definition->GetFloat("radius", radius, 10.24f);
        definition->GetFloat("height", baseHeight, 10.24f);
        definition->GetFloat("centerHeight", centerHeight, 5.12f);
        definition->GetFloat("viewHeight", viewHeight, 8.192f);
        definition->GetFloat("cullDistance", cullDistance, 4096);
        definition->GetFloat("tickDistance", tickDistance);
        definition->GetVector("bounds_min", baseBBox.min);
        definition->GetVector("bounds_max", baseBBox.max);
        definition->GetVector("scale", scale);
        definition->GetInt("health", health, 100);
        definition->GetInt("impactType", (int&)impactType);
        definition->GetInt("displayType", displayType);
        definition->GetString("footstepSound", footstepSound);

        definition->GetFloat("mass", physicsRef->mass, 1800);
        definition->GetFloat("friction", physicsRef->friction, 1);
        definition->GetFloat("airFriction", physicsRef->airFriction, 0);
        definition->GetFloat("bounceDamp", physicsRef->bounceDamp, 0);
        definition->GetFloat("stepHeight", physicsRef->stepHeight, 48);
        definition->GetFloat("rotorSpeed", physicsRef->rotorSpeed, 0);
        definition->GetFloat("rotorFriction", physicsRef->rotorFriction, 1);
        definition->GetFloat("sinkVelocity", physicsRef->sinkVelocity, 0.2f);
        definition->GetBool("bRotor", physicsRef->bRotor, false);
        definition->GetInt("clipFlags", (int&)physicsRef->clipFlags, (PF_CLIPEDGES|PF_DROPOFF));

        if(definition->GetString("mesh", str)) {
            SetModel(str.c_str());
        }
    }

    UpdateTransform();

    height = baseHeight;

    if(bTouch) {
        viewHeight = baseHeight * 0.5f;
    }

    kexCollisionMap *cm = &localWorld.CollisionMap();

    if(bStatic == false && cm->IsLoaded()) {
        int sectorIdx;
        
        args.GetInt("sectorIndex", sectorIdx, -1);

        if(sectorIdx <= -1) {
            physicsRef->sector = cm->PointInSector(origin);
        }
        else {
            physicsRef->sector = &cm->sectors[sectorIdx];
        }
    }

    scriptComponent.CallFunction(scriptComponent.onSpawn);
}

//
// kexActor::ParseDefault
//

void kexActor::ParseDefault(kexLexer *lexer) {
    kexStr keyName;

    switch(lexer->GetIDForTokenList(mapactortokens, lexer->Token())) {
    case scactor_name:
        lexer->GetString();
        name = lexer->StringToken();
        break;
    case scactor_mesh:
        lexer->GetString();
        SetModel(lexer->StringToken());
        break;
    case scactor_origin:
        origin = lexer->GetVector3();
        break;
    case scactor_angles:
        angles = lexer->GetVector3();
        rotation = angles.ToQuat();
        break;
    case scactor_scale:
        scale = lexer->GetVector3();
        break;
    case scactor_rotation:
        rotation = lexer->GetVector4();
        break;
    case scactor_bounds:
        baseBBox.min = lexer->GetVector3();
        baseBBox.max = lexer->GetVector3();
        bbox = baseBBox;
        break;
    case scactor_materials:
        if(model == NULL) {
            parser.Error("kexActor::ParseDefault: attempted to parse \"materials\" token while model is null\n");
        }

        AllocateMaterials();

        // texture swap block
        lexer->ExpectNextToken(TK_LBRACK);

        for(unsigned int l = 0; l < model->nodes[0].numSurfaces; l++) {
            char *str;

            // parse sections
            lexer->GetString();
            str = lexer->StringToken();

            if(str[0] != '-') {
                materials[l] = renderBackend.CacheMaterial(lexer->StringToken());
            }
            else {
                materials[l] = NULL;
            }
        }

        // end texture swap block
        lexer->ExpectNextToken(TK_RBRACK);
        break;
    case scactor_component:
        lexer->GetString();
        CreateComponent(lexer->StringToken());
        break;
    case scactor_bCollision:
        bCollision = (lexer->GetNumber() > 0);
        break;
    case scactor_bHidden:
        bHidden = (lexer->GetNumber() > 0);
        break;
    case scactor_bStatic:
        bStatic = (lexer->GetNumber() > 0);
        break;
    case scactor_bTouch:
        bTouch = (lexer->GetNumber() > 0);
        break;
    case scactor_bOrientOnSlope:
        bOrientOnSlope = (lexer->GetNumber() > 0);
        break;
    case scactor_bNoCull:
        bNoCull = (lexer->GetNumber() > 0);
        break;
    case scactor_bNoFixedTransform:
        bNoFixedTransform = (lexer->GetNumber() > 0);
        break;
    case scactor_bAllowDamage:
        bAllowDamage = (lexer->GetNumber() > 0);
        break;
    case scactor_radius:
        radius = (float)lexer->GetFloat();
        break;
    case scactor_height:
        baseHeight = (float)lexer->GetFloat();
        break;
    case scactor_centerheight:
        centerHeight = (float)lexer->GetFloat();
        break;
    case scactor_viewheight:
        viewHeight = (float)lexer->GetFloat();
        break;
    case scactor_targetID:
        targetID = lexer->GetNumber();
        break;
    case scactor_impactType:
        impactType = (impactType_t)lexer->GetNumber();
        break;
    case scactor_displayType:
        displayType = lexer->GetNumber();
        break;
    case scactor_cullDistance:
        cullDistance = (float)lexer->GetFloat();
        break;
    case scactor_tickDistance:
        tickDistance = (float)lexer->GetFloat();
        break;
    case scactor_physics:
        physics.Parse(lexer);
        break;
    default:
        if(lexer->TokenType() != TK_IDENIFIER) {
            parser.Error("kexActor::ParseDefault: unknown token: %s\n",
                lexer->Token());
        }
        keyName = (char*)lexer->Token();
        lexer->Find();
        args.Add(keyName.c_str(), lexer->Token());
        break;
    }
}

//
// kexActor::Parse
//

void kexActor::Parse(kexLexer *lexer) {
    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        ParseDefault(lexer);
        lexer->Find();
    }
}

//
// kexActor::UpdateTransform
//

void kexActor::UpdateTransform(void) {
    attachment.Transform();

    if(physics.bRotor) {
        float delta = client.GetRunTime();

        angles.yaw      += (physics.rotorVector.y * physics.rotorSpeed * delta);
        angles.pitch    += (physics.rotorVector.x * physics.rotorSpeed * delta);
        angles.roll     += (physics.rotorVector.z * physics.rotorSpeed * delta);
    }

    if(!bStatic || physics.bRotor) {
        angles.Clamp180();
        rotation = angles.ToQuat();
    }

    if(!AlignToSurface()) {
        matrix = kexMatrix(rotation);
    }

    matrix.Scale(scale);
    rotMatrix = matrix;

    matrix.AddTranslation(origin);

    if(!InstanceOf(&kexPickup::info)) {
        bbox = (baseBBox | rotMatrix);
    }
    else {
        // pickups will have a fixed bounding box size regardless of rotation
        bbox = baseBBox;
    }

    bbox.min += origin;
    bbox.max += origin;
}

//
// kexActor::AllocateMaterials
//

void kexActor::AllocateMaterials(void) {
    modelNode_t *node;

    node = &model->nodes[0];

    if(node->numSurfaces == 0) {
        return;
    }

    // allocate data for material swap array
    materials = (kexMaterial**)Mem_Calloc(sizeof(kexMaterial*) *
        node->numSurfaces, hb_static);
}

//
// kexActor::SetModel
//

void kexActor::SetModel(const char* modelFile) {
    if(modelFile) {
        model = modelManager.LoadModel(modelFile);
    }

    if(model) {
        unsigned int i;
        kexAnim_t *anim;
        kexModel_t *m;

        m = model;

        // set initial animation
        // TODO - rename anim00 to something better
        if((anim = kexAnimState::GetAnim(m, "anim00"))) {
            animState.Set(anim, 4, ANF_LOOP);
        }

        if(m->numNodes > 0) {
            // allocate node translation offset data
            nodeOffsets_t = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * m->numNodes, hb_static);

            // allocate node rotation offset data
            nodeOffsets_r = (kexQuat*)Mem_Calloc(sizeof(kexQuat) * m->numNodes, hb_static);

            // set default rotation offsets
            for(i = 0; i < m->numNodes; i++)
                nodeOffsets_r[i].Set(0, 0, 0, 1);
        }
    }
}

//
// kexActor::SetRotationOffset
//

void kexActor::SetRotationOffset(const int node, const float angle,
                                 const float x, const float y, const float z) {
     float an;

     if(node < 0 || node >= (int)model->numNodes) {
         return;
     }

     an = angle;
     kexAngle::Clamp(&an);

     nodeOffsets_r[node] = kexQuat(an, x, y, z);
}

//
// kexActor::SetModel
//

void kexActor::SetModel(const kexStr &modelFile) {
    SetModel(modelFile.c_str());
}

//
// kexActor::PlayFootStepSound
//

void kexActor::PlayFootStepSound(void) {
    if(footstepSound.Length() <= 0) {
        return;
    }

    StartSound(footstepSound);
}

//
// kexActor::ToLocalOrigin
//

kexVec3 kexActor::ToLocalOrigin(const float x, const float y, const float z) {
    kexMatrix mtx(DEG2RAD(-90), 1);
    mtx.Scale(-1, 1, 1);
    
    return ((kexVec3(x, y, z) | mtx) | matrix);
}

//
// kexActor::ToLocalOrigin
//

kexVec3 kexActor::ToLocalOrigin(const kexVec3 &org) {
    return ToLocalOrigin(org.x, org.y, org.z);
}

//
// kexActor::SpawnFX
//

void kexActor::SpawnFX(const char *fxName, const float x, const float y, const float z) {
    if(bStatic || bCulled)
        return;

    kexVec3 org = ToLocalOrigin(x, y, z);
    org.y += viewHeight;


    localWorld.SpawnFX(fxName, this, kexVec3::vecZero, org, rotation);
}

//
// kexActor::SpawnFX
//

void kexActor::SpawnFX(const kexStr &str, const float x, const float y, const float z) {
    SpawnFX(str.c_str(), x, y, z);
}

//
// kexActor::CreateComponent
//

void kexActor::CreateComponent(const char *name) {
    // TODO
    scriptComponent.Construct(name);
}

//
// kexActor::OnTouch
//

void kexActor::OnTouch(kexActor *instigator) {
    if(scriptComponent.onTouch) {
        scriptComponent.CallFunction(scriptComponent.onTouch);
    }
}

//
// kexActor::OnTrigger
//

void kexActor::OnTrigger(void) {
    if(scriptComponent.onTrigger) {
        scriptComponent.CallFunction(scriptComponent.onTrigger);
    }

    targetID = 0;
}

//
// kexActor::OnDamage
//

void kexActor::OnDamage(kexWorldObject *instigator, int damage, kexKeyMap *damageDef) {
    int state;
    kexActorComponent *ac;

    if(InstanceOf(&kexPlayerPuppet::info)) {
        ac = &static_cast<kexActor*>(owner)->scriptComponent;
    }
    else {
        ac = &scriptComponent;
    }

    state = ac->PrepareFunction("void OnDamage(kActor@, int, kKeyMapMem@)");
    if(state == -1) {
        return;
    }

    ac->SetCallArgument(0, static_cast<kexActor*>(instigator));
    ac->SetCallArgument(1, damage);
    ac->SetCallArgument(2, damageDef);

    if(!ac->ExecuteFunction(state)) {
        return;
    }

    ac->FinishFunction(state);
}

//
// kexActor::OnDeath
//

void kexActor::OnDeath(kexWorldObject *instigator, kexKeyMap *damageDef) {
    int state;
    kexActorComponent *ac;

    if(InstanceOf(&kexPlayerPuppet::info)) {
        ac = &static_cast<kexActor*>(owner)->scriptComponent;
    }
    else {
        ac = &scriptComponent;
    }

    state = ac->PrepareFunction("void OnDeath(kActor@, kKeyMapMem@)");
    if(state == -1) {
        return;
    }

    ac->SetCallArgument(0, static_cast<kexActor*>(instigator));
    ac->SetCallArgument(1, damageDef);

    if(!ac->ExecuteFunction(state)) {
        return;
    }

    ac->FinishFunction(state);
}

//
// kexActor::InitObject
//

void kexActor::InitObject(void) {
    kexScriptManager::RegisterRefObjectNoCount<kexActor>("kActor");
    kexScriptManager::RegisterDataObject<kexAttachment>("kAttachment");

    kexActor::RegisterBaseProperties<kexActor>("kActor");

    scriptManager.RegisterMethod("kAttachment", "void Transform(void)",
        asMETHODPR(kexAttachment, Transform, (void), void));
    scriptManager.RegisterMethod("kAttachment", "void AttachToActor(kActor@)",
        asMETHODPR(kexAttachment, AttachToObject, (kexDisplayObject *targ), void));
    scriptManager.RegisterMethod("kAttachment", "void DettachActor(void)",
        asMETHODPR(kexAttachment, DettachObject, (void), void));
    scriptManager.RegisterMethod("kAttachment", "kVec3 &GetAttachOffset(void)",
        asMETHODPR(kexAttachment, GetAttachOffset, (void), kexVec3&));
    scriptManager.RegisterMethod("kAttachment", "kVec3 &GetSourceOffset(void)",
        asMETHODPR(kexAttachment, GetSourceOffset, (void), kexVec3&));
    scriptManager.RegisterMethod("kAttachment", "void SetAttachOffset(const kVec3 &in)",
        asMETHODPR(kexAttachment, SetAttachOffset, (const kexVec3 &vec), void));
    scriptManager.RegisterMethod("kAttachment", "void SetSourceOffset(const kVec3 &in)",
        asMETHODPR(kexAttachment, SetSourceOffset, (const kexVec3 &vec), void));
    scriptManager.RegisterMethod("kAttachment", "kActor @GetOwner(void)",
        asMETHODPR(kexAttachment, GetOwner, (void), kexDisplayObject*));
    scriptManager.RegisterMethod("kAttachment", "void SetOwner(kActor@)",
        asMETHODPR(kexAttachment, SetOwner, (kexDisplayObject *o), void));
    scriptManager.RegisterMethod("kAttachment", "kActor @GetAttachedActor(void)",
        asMETHODPR(kexAttachment, GetAttachedObject, (void), kexDisplayObject*));

    scriptManager.Engine()->RegisterObjectProperty("kAttachment", "bool bAttachRelativeAngles",
        asOFFSET(kexAttachment, bAttachRelativeAngles));

    scriptManager.Engine()->RegisterEnum("EnumObjectDisplayType");
    scriptManager.Engine()->RegisterEnumValue("EnumObjectDisplayType", "ODT_NORMAL", ODT_NORMAL);
    scriptManager.Engine()->RegisterEnumValue("EnumObjectDisplayType", "ODT_CLIENTVIEW", ODT_CLIENTVIEW);
    scriptManager.Engine()->RegisterEnumValue("EnumObjectDisplayType", "ODT_FOREGROUND", ODT_FOREGROUND);
}
