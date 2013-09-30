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
#include "clipmesh.h"
#include "script.h"

typedef enum {
    WLT_INVALID  = 0,
    WLT_OVER     = 1,
    WLT_BETWEEN  = 2,
    WLT_UNDER    = 3
} waterLevelType_t;

class kexActor;
class kexClipMesh;

class kexPhysics {
public:
                            kexPhysics(void);
                            ~kexPhysics(void);

    void                    Parse(kexLexer *lexer);
    float                   GroundDistance(void);
    bool                    OnGround(void);
    void                    ImpactVelocity(kexVec3 &normal, const float force);
    void                    ApplyFriction(void);
    void                    ProjectOnCrease(const kexVec3 &n1, const kexVec3 &n2);
    void                    Think(const float timeDelta);

    kexVec3                 &GetVelocity(void) { return velocity; }
    void                    SetVelocity(const kexVec3 &vel) { velocity = vel; }
    kexActor                *GetOwner(void) { return owner; }
    void                    SetOwner(kexActor *actor) { owner = actor; }

    static void             InitObject(void);

    kexVec3                 velocity;
    bool                    bRotor;
    bool                    bOrientOnSlope;
    float                   friction;
    float                   airFriction;
    float                   mass;
    float                   bounceDamp;
    float                   rotorSpeed;
    float                   rotorFriction;
    kexVec3                 rotorVector;
    waterLevelType_t        waterLevel;

    typedef struct {
        kexClipMesh         *hitMesh;
        kexTri              *hitTri;
        kexActor            *hitActor;
        kexVec3             hitNormal;
        kexVec3             hitVector;
        float               fraction;
    } traceInfo_t;

    traceInfo_t             traceInfo;

private:
    kexActor                *owner;
};

#endif
