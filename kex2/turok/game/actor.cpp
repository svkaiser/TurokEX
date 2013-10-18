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
// DESCRIPTION: GameActor system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "actor.h"
#include "game.h"
#include "js.h"
#include "jsobj.h"
#include "js_shared.h"
#include "js_class.h"
#include "zone.h"
#include "parse.h"
#include "server.h"

//
// kexAttachment::AttachToActor
//

void kexAttachment::AttachToActor(kexActor *targ) {
    // If there was a attachment already, decrease its refcount
    if(actor)
        actor->RemoveRef();

    // Set new attachment and if non-NULL, increase its counter
    if((actor = targ))
        actor->AddRef();
}

//
// kexAttachment::DettachActor
//

void kexAttachment::DettachActor(void) {
    AttachToActor(NULL);
}

//
// kexAttachment::Transform
//

void kexAttachment::Transform(void) {
    if(actor != NULL) {
        // TODO
        if(bAttachRelativeAngles) {
            owner->SetOrigin(actor->GetOrigin() + attachOffset);
            owner->SetAngles(actor->GetAngles());
        }
    }
}

DECLARE_ABSTRACT_CLASS(kexActor, kexObject)

//
// kexActor::kexActor
//

kexActor::kexActor(void) {
    this->refCount      = 0;
    this->bStatic       = true;
    this->bCollision    = false;
    this->bTouch        = false;
    this->bClientOnly   = false;
    this->bHidden       = false;
    this->owner         = NULL;
    this->target        = NULL;
    this->model         = NULL;
    this->gridBound     = NULL;
    
    this->attachment.SetOwner(this);
    this->physics.SetOwner(this);

    this->scale.Set(1, 1, 1);
}

//
// kexActor::~kexActor
//

kexActor::~kexActor(void) {
}

//
// kexActor::LocalTick
//

void kexActor::LocalTick(void) {
}

//
// kexActor::Tick
//

void kexActor::Tick(void) {
}

//
// kexActor::Remove
//

void kexActor::Remove(void) {
}

//
// kexActor::AddRef
//

int kexActor::AddRef(void) {
    return ++refCount;
}

//
// kexActor::RemoveRef
//

int kexActor::RemoveRef(void) {
    return --refCount;
}

//
// kexActor::SetTarget
//

void kexActor::SetTarget(kexActor *targ) {
    // If there was a target already, decrease its refcount
    if(target)
        target->RemoveRef();

    // Set new target and if non-NULL, increase its counter
    if((target = targ))
        target->AddRef();
}

//
// kexActor::SetOwner
//

void kexActor::SetOwner(kexActor *targ) {
    // If there was a owner already, decrease its refcount
    if(owner)
        owner->RemoveRef();

    // Set new owner and if non-NULL, increase its counter
    if((owner = targ))
        owner->AddRef();
}

//
// kexActor::SetBoundingBox
//

void kexActor::SetBoundingBox(const kexVec3 &min, const kexVec3 &max) {
    bbox.min = min;
    bbox.max = max;
}

enum {
    scactor_name = 0,
    scactor_mesh,
    scactor_bounds,
    scactor_textureSwaps,
    scactor_component,
    scactor_bCollision,
    scactor_bHidden,
    scactor_bStatic,
    scactor_bTouch,
    scactor_origin,
    scactor_scale,
    scactor_angles,
    scactor_rotation,
    scactor_radius,
    scactor_height,
    scactor_centerheight,
    scactor_viewheight,
    scactor_targetID,
    scactor_modelVariant,
    scactor_cullDistance,
    scactor_tickDistance,
    scactor_physics,
    scactor_clipmesh,
    scactor_end
};

static const sctokens_t mapactortokens[scactor_end+1] = {
    { scactor_name,             "name"              },
    { scactor_mesh,             "mesh"              },
    { scactor_bounds,           "bounds"            },
    { scactor_textureSwaps,     "textureSwaps"      },
    { scactor_component,        "component"         },
    { scactor_bCollision,       "bCollision"        },
    { scactor_bHidden,          "bHidden"           },
    { scactor_bStatic,          "bStatic"           },
    { scactor_bTouch,           "bTouch"            },
    { scactor_origin,           "origin"            },
    { scactor_scale,            "scale"             },
    { scactor_angles,           "angles"            },
    { scactor_rotation,         "rotation"          },
    { scactor_radius,           "radius"            },
    { scactor_height,           "height"            },
    { scactor_centerheight,     "centerheight"      },
    { scactor_viewheight,       "viewheight"        },
    { scactor_targetID,         "targetID"          },
    { scactor_modelVariant,     "modelVariant"      },
    { scactor_cullDistance,     "cullDistance"      },
    { scactor_tickDistance,     "tickDistance"      },
    { scactor_physics,          "physics"           },
    { scactor_clipmesh,         "clipMesh"          },
    { -1,                       NULL                }
};

DECLARE_CLASS(kexWorldActor, kexActor)

unsigned int kexWorldActor::id = 0;

//
// kexWorldActor::kexWorldActor
//

kexWorldActor::kexWorldActor(void) {
    this->baseBBox.min.Set(-32, -32, -32);
    this->baseBBox.max.Set(32, 32, 32);

    this->worldLink.SetData(this);
    this->scriptComponent.SetOwner(this);
    this->clipMesh.SetOwner(this);

    this->radius            = 30.72f;
    this->baseHeight        = 30.72f;
    this->viewHeight        = 16.384f;
    this->centerHeight      = 10.24f;
    this->bbox              = baseBBox;
    this->bTraced           = false;
}

//
// kexWorldActor::~kexWorldActor
//

kexWorldActor::~kexWorldActor(void) {
}

//
// kexWorldActor::LocalTick
//

void kexWorldActor::LocalTick(void) {
    animState.Update();
}

//
// kexWorldActor::Tick
//

void kexWorldActor::Tick(void) {
}

//
// kexWorldActor::Spawn
//

void kexWorldActor::Spawn(void) {
    rotation.Normalize();
    UpdateTransform();

    timeStamp = (float)server.GetRunTime();
    height = bStatic ? baseHeight : 0;

    if(bTouch) {
        bCollision = false;
        viewHeight = baseHeight * 0.5f;
    }

    clipMesh.CreateShape();
    clipMesh.Transform();

    scriptComponent.CallFunction(scriptComponent.onSpawn);
}

//
// kexWorldActor::Remove
//

void kexWorldActor::Remove(void) {
}

//
// kexWorldActor::ParseDefault
//

void kexWorldActor::ParseDefault(kexLexer *lexer) {
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
    case scactor_textureSwaps:
        if(model == NULL)
            parser.Error("kexWorldActor::ParseDefault: attempted to parse \"textureSwaps\" token while model is null\n");

        // texture swap block
        lexer->ExpectNextToken(TK_LBRACK);
        for(unsigned int j = 0; j < (int)model->numNodes; j++) {
            modelNode_t *node = &model->nodes[j];

            // node block
            lexer->ExpectNextToken(TK_LBRACK);
            for(unsigned int k = 0; k < node->numSurfaceGroups; k++) {
                surfaceGroup_t *group = &node->surfaceGroups[k];

                // mesh block
                lexer->ExpectNextToken(TK_LBRACK);
                for(unsigned int l = 0; l < group->numSurfaces; l++) {
                    // parse sections
                    lexer->GetString();
                    textureSwaps[j][k][l] = Z_Strdup(lexer->StringToken(), PU_ACTOR, NULL);
                }
                // end mesh block
                lexer->ExpectNextToken(TK_RBRACK);
            }
            // end node block
            lexer->ExpectNextToken(TK_RBRACK);
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
    case scactor_modelVariant:
        variant = lexer->GetNumber();
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
    case scactor_clipmesh:
        clipMesh.Parse(lexer);
        break;
    default:
        if(lexer->TokenType() == TK_IDENIFIER) {
            parser.Error("kexWorldActor::ParseDefault: unknown token: %s\n",
                lexer->Token());
        }
        break;
    }
}

//
// kexWorldActor::Parse
//

void kexWorldActor::Parse(kexLexer *lexer) {
    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        ParseDefault(lexer);
        lexer->Find();
    }
}

//
// kexWorldActor::UpdateTransform
//

void kexWorldActor::UpdateTransform(void) {
    if(physics.bRotor) {
        angles.yaw      += (physics.rotorVector.y * physics.rotorSpeed * timeStamp);
        angles.pitch    += (physics.rotorVector.x * physics.rotorSpeed * timeStamp);
        angles.roll     += (physics.rotorVector.z * physics.rotorSpeed * timeStamp);
    }

    if(!bStatic || physics.bRotor) {
        angles.Clamp180();
        rotation =
            kexQuat(angles.pitch, kexVec3::vecRight) *
            (kexQuat(angles.yaw, kexVec3::vecUp) *
            kexQuat(angles.roll, kexVec3::vecForward));
    }

    if(!AlignToSurface())
        matrix = kexMatrix(rotation);

    rotMatrix = matrix;
    matrix.Scale(scale);
    matrix.AddTranslation(origin);

    if(!bStatic) {
        bbox = (baseBBox | rotMatrix);
    }
}

//
// kexWorldActor::SetModel
//

void kexWorldActor::SetModel(const char* modelFile) {
    if(modelFile)
        model = modelManager.LoadModel(modelFile);

    if(model) {
        unsigned int i;
        unsigned int j;
        kexAnim_t *anim;
        kexModel_t *m;

        m = model;

        // set initial animation
        // TODO - rename anim00 to something better
        if(anim = kexAnimState::GetAnim(m, "anim00")) {
            animState.Set(anim, 4, ANF_LOOP);
        }

        // allocate node translation offset data
        nodeOffsets_t = (kexVec3*)Z_Realloc(
            nodeOffsets_t,
            sizeof(kexVec3) * m->numNodes,
            PU_ACTOR,
            0);

        // allocate node rotation offset data
        nodeOffsets_r = (kexQuat*)Z_Realloc(
            nodeOffsets_r,
            sizeof(kexQuat) * m->numNodes,
            PU_ACTOR,
            0);

        // set default rotation offsets
        for(i = 0; i < m->numNodes; i++)
            nodeOffsets_r[i].Set(0, 0, 0, 1);

        // allocate data for texture swap array
        textureSwaps = (char****)Z_Calloc(sizeof(char***) *
            m->numNodes, PU_ACTOR, NULL);

        for(j = 0; j < m->numNodes; j++) {
            unsigned int k;
            modelNode_t *node;

            node = &m->nodes[j];

            textureSwaps[j] = (char***)Z_Calloc(sizeof(char**) *
                node->numSurfaceGroups, PU_ACTOR, NULL);

            for(k = 0; k < node->numSurfaceGroups; k++) {
                surfaceGroup_t *group;

                group = &node->surfaceGroups[k];

                if(group->numSurfaces == 0)
                    continue;

                textureSwaps[j][k] = (char**)Z_Calloc(sizeof(char*) *
                    group->numSurfaces, PU_ACTOR, NULL);
            }
        }
    }
}

//
// kexWorldActor::SetModel
//

void kexWorldActor::SetModel(const kexStr &modelFile) {
    SetModel(modelFile.c_str());
}

//
// kexWorldActor::ToLocalOrigin
//

kexVec3 kexWorldActor::ToLocalOrigin(const float x, const float y, const float z) {
    kexMatrix mtx(DEG2RAD(-90), 1);
    mtx.Scale(-1, 1, 1);
    
    return ((kexVec3(x, y, z) | mtx) | matrix);
}

//
// kexWorldActor::ToLocalOrigin
//

kexVec3 kexWorldActor::ToLocalOrigin(const kexVec3 &org) {
    return ToLocalOrigin(org.x, org.y, org.z);
}

//
// kexWorldActor::SpawnFX
//

void kexWorldActor::SpawnFX(const char *fxName, const float x, const float y, const float z) {
    if(bStatic || bCulled)
        return;
        
    //TODO
}

//
// kexWorldActor::Event
//

bool kexWorldActor::Event(const char *function, long *args, unsigned int nargs) {
    return false;
}

//
// kexWorldActor::CreateComponent
//

void kexWorldActor::CreateComponent(const char *name) {
    // TODO
    scriptComponent.Spawn(name);

    jsval val;
    JSContext *cx;
    gObject_t *cObject;

    cx = js_context;

    // get prototype
    if(!JS_GetProperty(cx, js_gobject, name, &val))
        return;
    if(!JS_ValueToObject(cx, val, &cObject))
        return;
    if(cObject == NULL)
        return;

    // construct class object
    if(!(component = classObj.create(cObject)))
        return;

    JS_AddRoot(cx, &component);
}

//
// kexWorldActor::Trace
//

bool kexWorldActor::Trace(traceInfo_t *trace) {
    kexVec3 org = (origin - trace->start);

    if(trace->dir.Dot(org) <= 0) {
        return false;
    }

    float len = trace->dir.Unit();

    if(len == 0) {
        return false;
    }

    kexVec3 nDir    = (trace->dir * (1.0f / len));
    float cp        = nDir.Dot(org);
    kexVec3 cDist   = (org - (nDir * cp));
    float rd        = radius * radius - cDist.UnitSq();

    if(rd <= 0) {
        return false;
    }

    float frac = (cp - (float)sqrt(rd)) * (1.0f / len);

    if(frac <= 1.0f && frac < trace->fraction) {
        if(frac < 0) {
            frac = 0;
        }
        trace->hitActor = this;
        trace->fraction = frac;
        trace->hitVector = trace->start - (trace->dir * frac);
        trace->hitNormal = (trace->start - origin);
        trace->hitNormal.Normalize();
        return true;
    }

    return false;
}

//
// kexWorldActor::ToJSVal
//

bool kexWorldActor::ToJSVal(long *val) {
    gObject_t *aObject;
    
    *val = JSVAL_NULL;

    if(!(aObject = JPool_GetFree(&objPoolGameActor, &GameActor_class)) ||
        !(JS_SetPrivate(js_context, aObject, this))) {
        return false;
    }

    *val = (jsval)OBJECT_TO_JSVAL(aObject);
    return true;
}

//
// kexWorldActor::OnTouch
//

void kexWorldActor::OnTouch(kexWorldActor *instigator) {
    jsval val;

    if(bStatic || !bTouch || !instigator->ToJSVal(&val))
        return;

    Event("onTouch", &val, 1);
}

//
// kexWorldActor::Think
//

void kexWorldActor::Think(void) {
}

//
// kexWorldActor::AlignToSurface
//

bool kexWorldActor::AlignToSurface(void) {
    return false;
}

//
// kexWorldActor::InitObject
//

void kexWorldActor::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kActor",
        sizeof(kexWorldActor),
        asOBJ_REF);
        
    scriptManager.Engine()->RegisterObjectType(
        "kAttachment",
        sizeof(kexAttachment),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kActor",
        asBEHAVE_ADDREF,
        "void f()",
        asMETHOD(kexWorldActor, AddRef),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kActor",
        asBEHAVE_RELEASE,
        "void f()",
        asMETHOD(kexWorldActor, RemoveRef),
        asCALL_THISCALL);

    kexActor::RegisterBaseProperties<kexWorldActor>("kActor");

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kActor",                                   \
        str,                                        \
        asMETHODPR(kexWorldActor, a, b, c),         \
        asCALL_THISCALL)

    OBJMETHOD("void SetModel(const kStr &in)", SetModel, (const kexStr &modelFile), void);

#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kActor",                                   \
        str,                                        \
        asOFFSET(kexWorldActor, p))

    OBJPROPERTY("ref @obj", scriptComponent.Handle());
    OBJPROPERTY("int health", health);
    OBJPROPERTY("float radius", radius);
    OBJPROPERTY("float height", height);
    OBJPROPERTY("float baseHeight", baseHeight);
    OBJPROPERTY("float centerHeight", centerHeight);
    OBJPROPERTY("float viewHeight", viewHeight);
    OBJPROPERTY("kQuat lerpRotation", lerpRotation);

#undef OBJMETHOD
#undef OBJPROPERTY

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kAttachment",                              \
        str,                                        \
        asMETHODPR(kexAttachment, a, b, c),         \
        asCALL_THISCALL)

    OBJMETHOD("void Transform(void)", Transform, (void), void);
    OBJMETHOD("void AttachToActor(kActor@)", AttachToActor, (kexActor *targ), void);
    OBJMETHOD("void DettachActor(void)", DettachActor, (void), void);
    OBJMETHOD("kVec3 &GetAttachOffset(void)", GetAttachOffset, (void), kexVec3&);
    OBJMETHOD("void SetAttachOffset(const kVec3 &in)", SetAttachOffset, (const kexVec3 &vec), void);
    OBJMETHOD("kActor @GetOwner(void)", GetOwner, (void), kexActor*);
    OBJMETHOD("void SetOwner(kActor@)", SetOwner, (kexActor *o), void);
    OBJMETHOD("kActor @GetAttachedActor(void)", GetAttachedActor, (void), kexActor*);

#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kAttachment",                              \
        str,                                        \
        asOFFSET(kexAttachment, p))

    OBJPROPERTY("bool bAttachRelativeAngles", bAttachRelativeAngles);

#undef OBJMETHOD
#undef OBJPROPERTY
}

DECLARE_CLASS(kexViewActor, kexWorldActor)

//
// kexViewActor::kexViewActor
//

kexViewActor::kexViewActor(void) {
    this->bClientOnly = true;
    this->bCollision = false;
    this->bTouch = false;
}

//
// kexViewActor::~kexViewActor
//

kexViewActor::~kexViewActor(void) {
}
