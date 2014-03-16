// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
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
// DESCRIPTION: Collision detection / Physics behavior
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "world.h"
#include "physics/physics.h"

enum {
    scPhysics_mass = 0,
    scPhysics_friction,
    scPhysics_airFriction,
    scPhysics_bounceDamp,
    scPhysics_stepHeight,
    scPhysics_rotorSpeed,
    scPhysics_rotorFriction,
    scPhysics_bRotor,
    scPhysics_bOrientOnSlope,
    scPhysics_rotorVector,
    scPhysics_sinkVelocity,
    scPhysics_end
};

static const sctokens_t physicsTokens[scPhysics_end+1] = {
    { scPhysics_mass,           "mass"                  },
    { scPhysics_friction,       "friction"              },
    { scPhysics_airFriction,    "airFriction"           },
    { scPhysics_bounceDamp,     "bounceDamp"            },
    { scPhysics_stepHeight,     "stepHeight"            },
    { scPhysics_rotorSpeed,     "rotorSpeed"            },
    { scPhysics_rotorFriction,  "rotorFriction"         },
    { scPhysics_bRotor,         "bRotor"                },
    { scPhysics_bOrientOnSlope, "bOrientOnSlope"        },
    { scPhysics_rotorVector,    "rotorVector"           },
    { scPhysics_sinkVelocity,   "sinkVelocity"          },
    { -1,                       NULL                    }
};

DECLARE_CLASS(kexPhysics, kexObject)

//
// kexPhysics::kexPhysics
//

kexPhysics::kexPhysics(void) {
    this->mass                  = 1800;
    this->friction              = 1;
    this->airFriction           = 0;
    this->bounceDamp            = 0;
    this->stepHeight            = 48;
    this->rotorSpeed            = 0;
    this->rotorFriction         = 1;
    this->waterHeight           = 0;
    this->sinkVelocity          = 0.2f;
    this->bRotor                = false;
    this->bOrientOnSlope        = false;
    this->bOnGround             = false;
    this->bInWater              = false;
    this->bClimbing             = false;
    this->bEnabled              = true;
    this->waterLevel            = WLT_INVALID;
    this->groundGeom            = NULL;
    this->groundMesh            = NULL;
    this->sector                = NULL;
    this->clipFlags             = (PF_CLIPEDGES|PF_DROPOFF);

    this->rotorVector.Clear();
    this->velocity.Clear();
}

//
// kexPhysics::~kexPhysics
//

kexPhysics::~kexPhysics(void) {
}

//
// kexPhysics::Parse
//

void kexPhysics::Parse(kexLexer *lexer) {
    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        switch(lexer->GetIDForTokenList(physicsTokens, lexer->Token())) {
        case scPhysics_mass:
            this->mass = (float)lexer->GetFloat();
            break;
        case scPhysics_friction:
            this->friction = (float)lexer->GetFloat();
            break;
        case scPhysics_airFriction:
            this->airFriction = (float)lexer->GetFloat();
            break;
        case scPhysics_bounceDamp:
            this->bounceDamp = (float)lexer->GetFloat();
            break;
        case scPhysics_rotorSpeed:
            this->rotorSpeed = (float)lexer->GetFloat();
            break;
        case scPhysics_rotorFriction:
            this->rotorFriction = (float)lexer->GetFloat();
            break;
        case scPhysics_bRotor:
            this->bRotor = (lexer->GetNumber() > 0);
            break;
        case scPhysics_bOrientOnSlope:
            this->bRotor = (lexer->GetNumber() > 0);
            break;
        case scPhysics_rotorVector:
            this->rotorVector = lexer->GetVector3();
            break;
        case scPhysics_sinkVelocity:
            this->sinkVelocity = (float)lexer->GetFloat();
            break;
        default:
            if(lexer->TokenType() == TK_IDENIFIER) {
                parser.Error("kexPhysics::Parse: unknown token: %s\n",
                    lexer->Token());
            }
            break;
        }
        
        lexer->Find();
    }
}

//
// kexPhysics::GroundDistance
//

float kexPhysics::GroundDistance(void) {
    kexVec3 org = owner->GetOrigin();

    if(groundGeom == NULL) {
        return 0;
    }

    return (org[1] - groundGeom->GetDistance(org));
}

//
// kexPhysics::OnSteepSlope
//

bool kexPhysics::OnSteepSlope(void) {
    if(groundGeom == NULL) {
        return false;
    }
    return (groundGeom->plane.Normal().Dot(-localWorld.GetGravity()) <= ONPLANE_EPSILON);
}

//
// kexPhysics::OnGround
//

bool kexPhysics::OnGround(void) {
    if(groundGeom == NULL) {
        return false;
    }
    if(OnSteepSlope()) {
        return false;
    }
    return GroundDistance() <= ONPLANE_EPSILON;
}

//
// kexPhysics::GroundNormal
//

kexVec3 kexPhysics::GroundNormal(void) {
    if(groundGeom == NULL) {
        return -localWorld.GetGravity();
    }

    return groundGeom->plane.Normal();
}

//
// kexPhysics::CorrectSectorPosition
//

bool kexPhysics::CorrectSectorPosition(void) {
    bool ok = false;

    if(sector == NULL) {
        return false;
    }

    kexVec3 org = owner->GetOrigin();
    float dist = (org[1] - sector->lowerTri.GetDistance(org));

    if(dist < 0) {
        // correct position
        owner->GetOrigin()[1] = org[1] - dist;
        groundGeom = &sector->lowerTri;
        velocity.Clear();
        ok = true;
    }

    if(sector->flags & CLF_CHECKHEIGHT) {
        dist = (sector->upperTri.GetDistance(org) - owner->GetViewHeight());

        if(dist < org[1]) {
            // correct position
            owner->GetOrigin()[1] = dist;
            groundGeom = &sector->lowerTri;
            velocity.Clear();
            ok = true;
        }
    }

    // check if in water sector
    CheckWater((owner->GetViewHeight() + owner->GetCenterHeight()) * 0.5f);
    return ok;
}

//
// kexPhysics::ImpactVelocity
//

void kexPhysics::ImpactVelocity(kexVec3 &vel, kexVec3 &normal, const float force) {
    kexVec3 dir = vel;
    float d = vel.Unit();
    float bounce = force;

    if(bounceDamp != 0) {
        if(d >= 1.05f) {
            bounce = (force + bounceDamp);

            if(d < 16.8f && force < 1.0f)
                bounce = 0.2f;
        }
    }

    vel = dir - (normal * (dir.Dot(normal) * bounce));

    if(d != 0) {
        vel *= (dir.Unit() / d);
    }
}

//
// kexPhysics::ApplyFriction
//

void kexPhysics::ApplyFriction(void) {
    float speed;

    speed = velocity.Unit();

    if(speed < VELOCITY_EPSILON) {
        velocity.x = 0;
        velocity.z = 0;

        if(bInWater && waterLevel == WLT_UNDER) {
            velocity.y = 0;
        }
    }
    else {
        float clipspeed = speed - (speed * friction);

        if(clipspeed < 0) {
            clipspeed = 0;
        }

        clipspeed /= speed;

        // de-accelerate velocity
        velocity.x = velocity.x * clipspeed;
        velocity.z = velocity.z * clipspeed;

        if(bInWater && waterLevel == WLT_UNDER) {
            velocity.y = velocity.y * clipspeed;
        }
    }

    if(!bInWater) {
        float yFriction = 0;

        if(airFriction == 0) {
            float dist;

            if(groundGeom == NULL) {
                return;
            }

            dist = GroundDistance();

            // apply vertical friction only if we're rubbing up against the floor
            if((dist > ONPLANE_EPSILON || dist < -ONPLANE_EPSILON) ||
                velocity.Dot(groundGeom->plane.Normal()) <= 0) {
                return;
            }

            yFriction = friction;
        }
        else {
            yFriction = airFriction;
        }

        speed = velocity.y;

        if(speed < VELOCITY_EPSILON) {
            velocity.y = 0;
        }
        else {
            float clipspeed = speed - (speed * yFriction);

            if(clipspeed < 0) {
                clipspeed = 0;
            }

            clipspeed /= speed;

            // de-accelerate velocity
            velocity.y = velocity.y * clipspeed;
        }
    }
}

//
// kexPhysics::ClimbOnSurface
//

void kexPhysics::ClimbOnSurface(kexVec3 &start, const kexVec3 &end, kexTri *tri) {
    kexVec3 dir;
    float dist;
    float lenxz;
    float leny;
    float y1;
    float y2;

    if((end - *tri->point[0]).Dot(tri->plane.Normal()) > 0) {
        return;
    }

    y1 = tri->GetDistance(start);
    y2 = tri->GetDistance(end);

    dir.Set(end[0] - start[0], y2 - y1, end[2] - start[2]);
    lenxz = dir.ToVec2().UnitSq();
    leny = dir[1]*dir[1]+lenxz;

    if(leny == 0) {
        start[1] = y1;
        return;
    }

    dist = kexMath::Sqrt(lenxz / leny);

    start[0] = (end[0] - start[0]) * dist + start[0];
    start[1] = (y2 - y1) * dist + y1;
    start[2] = (end[2] - start[2]) * dist + start[2];
}

//
// kexPhysics::CheckWater
//

void kexPhysics::CheckWater(float height) {
    if(owner && sector && sector->area) {
        waterLevel = sector->area->GetWaterLevel(owner->GetOrigin(), height);
        waterHeight = sector->area->WaterPlane();
        bInWater = (waterLevel > WLT_OVER);
    }
}

//
// kexPhysics::GetWaterDepth
//

float kexPhysics::GetWaterDepth(void) {
    float dist;
    float sink;

    if(groundGeom == NULL) {
        return 0;
    }

    dist = GroundDistance();

    if(dist <= ONPLANE_EPSILON) {
        dist = 0;
    }

    sink = dist;

    if(dist * 0.125f >= 2) {
        dist = dist * 0.125f;
    }
    else {
        dist = 2;
    }

    return dist * 4;
}

//
// kexPhysics::Think
//

void kexPhysics::Think(const float timeDelta) {
    if(!bEnabled) {
        return;
    }
    // correct position
    if(sector) {
        kexVec3 org = owner->GetOrigin();
        float dist = (org[1] - sector->lowerTri.GetDistance(org));
        
        if(dist < 0) {
            owner->GetOrigin()[1] = org[1] - dist;
        }
        
        groundGeom = &sector->lowerTri;
    }
}

//
// kexPhysics::InitObject
//

void kexPhysics::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kPhysics",
        sizeof(kexPhysics),
        asOBJ_REF | asOBJ_NOCOUNT);

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kPhysics",                                 \
        str,                                        \
        asMETHODPR(kexPhysics, a, b, c),            \
        asCALL_THISCALL)

    OBJMETHOD("kActor @GetOwner(void)", GetOwner, (void), kexWorldObject*);
    OBJMETHOD("void SetOwner(kActor@)", SetOwner, (kexWorldObject *o), void);
    OBJMETHOD("kVec3 &GetVelocity(void)", GetVelocity, (void), kexVec3&);
    OBJMETHOD("void SetVelocity(const kVec3 &in)", SetVelocity, (const kexVec3 &vel), void);
    OBJMETHOD("bool OnGround(void)", OnGround, (void), bool);
    OBJMETHOD("bool OnSteepSlope(void)", OnSteepSlope, (void), bool);
    OBJMETHOD("float GroundDistance(void)", GroundDistance, (void), float);
    OBJMETHOD("float GetWaterDepth(void)", GetWaterDepth, (void), float);
    OBJMETHOD("kVec3 GroundNormal(void)", GroundNormal, (void), kexVec3);

#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kPhysics",                                 \
        str,                                        \
        asOFFSET(kexPhysics, p))

    OBJPROPERTY("bool bRotor", bRotor);
    OBJPROPERTY("bool bOrientOnSlope", bOrientOnSlope);
    OBJPROPERTY("float friction", friction);
    OBJPROPERTY("float airFriction", airFriction);
    OBJPROPERTY("float mass", mass);
    OBJPROPERTY("float bounceDamp", bounceDamp);
    OBJPROPERTY("float stepHeight", stepHeight);
    OBJPROPERTY("float rotorSpeed", rotorSpeed);
    OBJPROPERTY("float rotorFriction", rotorFriction);
    OBJPROPERTY("kVec3 rotorVector", rotorVector);
    OBJPROPERTY("bool bInWater", bInWater);
    OBJPROPERTY("bool bClimbing", bClimbing);
    OBJPROPERTY("int waterLevel", waterLevel);
    OBJPROPERTY("float waterHeight", waterHeight);
    OBJPROPERTY("float sinkVelocity", sinkVelocity);
    OBJPROPERTY("uint clipFlags", clipFlags);

#undef OBJMETHOD
#undef OBJPROPERTY

    scriptManager.Engine()->RegisterObjectMethod(
        "kActor",
        "kPhysics @Physics(void)",
        asMETHODPR(kexActor, Physics, (void), kexPhysics*),
        asCALL_THISCALL);
}
