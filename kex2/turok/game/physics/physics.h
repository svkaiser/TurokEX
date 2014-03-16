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

#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "common.h"
#include "mathlib.h"
#include "collisionMap.h"
#include "script.h"

#define ONPLANE_EPSILON     0.512f
#define VELOCITY_EPSILON    0.0001f

class kexWorldObject;
class kexClipMesh;

typedef enum {
    IT_DEFAULT = 0,
    IT_WATER,
    IT_METAL,
    IT_STONE,
    IT_FLESH_HUMAN,
    IT_FLESH_CREATURE,
    IT_FLESH_UNDEAD,
    IT_LAVA,
    IT_SLIME,
    IT_FORCEFIELD,
    IT_NUMIMPACTTYPES
} impactType_t;

typedef struct {
    kexVec3                 start;
    kexVec3                 end;
    kexVec3                 dir;
    kexClipMesh             *hitMesh;
    kexTri                  *hitTri;
    kexWorldObject          *hitActor;
    kexVec3                 hitNormal;
    kexVec3                 hitVector;
    float                   fraction;
    bool                    bUseBBox;
    kexBBox                 bbox;
    kexBBox                 localBBox;
    kexWorldObject          *owner;
    kexSector               **sector;
} traceInfo_t;

#include "clipmesh.h"

BEGIN_EXTENDED_CLASS(kexPhysics, kexObject);
public:
                            kexPhysics(void);
                            ~kexPhysics(void);

    void                    Parse(kexLexer *lexer);
    float                   GroundDistance(void);
    bool                    OnGround(void);
    bool                    OnSteepSlope(void);
    void                    ImpactVelocity(kexVec3 &vel, kexVec3 &normal, const float force);
    void                    ApplyFriction(void);
    void                    ClimbOnSurface(kexVec3 &start, const kexVec3 &end, kexTri *tri);
    float                   GetWaterDepth(void);
    void                    CheckWater(float height);
    kexVec3                 GroundNormal(void);
    bool                    CorrectSectorPosition(void);

    virtual void            Think(const float timeDelta);

    kexVec3                 &GetVelocity(void) { return velocity; }
    void                    SetVelocity(const kexVec3 &vel) { velocity = vel; }
    kexWorldObject          *GetOwner(void) { return owner; }
    void                    SetOwner(kexWorldObject *actor) { owner = actor; }

    static void             InitObject(void);

    kexVec3                 velocity;
    bool                    bRotor;
    bool                    bOrientOnSlope;
    float                   friction;
    float                   airFriction;
    float                   mass;
    float                   bounceDamp;
    float                   stepHeight;
    float                   rotorSpeed;
    float                   rotorFriction;
    float                   sinkVelocity;
    kexVec3                 rotorVector;
    waterLevelType_t        waterLevel;
    float                   waterHeight;
    kexTri                  *groundGeom;
    kexClipMesh             *groundMesh;
    kexSector               *sector;
    bool                    bOnGround;
    bool                    bInWater;
    bool                    bClimbing;
    bool                    bEnabled;
    unsigned int            clipFlags;

protected:
    kexWorldObject          *owner;
END_CLASS();

#endif
