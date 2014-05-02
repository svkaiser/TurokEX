// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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
// DESCRIPTION: AI Collision detection / Physics behavior
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "world.h"
#include "physics/physics_ai.h"

DECLARE_CLASS(kexAIPhysics, kexPhysics)

//
// kexAIPhysics::kexAIPhysics
//

kexAIPhysics::kexAIPhysics(void) {
    this->clipFlags = (PF_CLIPEDGES|PF_NOENTERWATER);
}

//
// kexAIPhysics::~kexAIPhysics
//

kexAIPhysics::~kexAIPhysics(void) {
}

//
// kexAIPhysics::Think
//

void kexAIPhysics::Think(const float timeDelta) {
    traceInfo_t trace;
    float       time;
    float       massAmount;
    float       currentMass;
    float       radius;
    float       height;
    kexVec3     start;
    kexVec3     gravity;

    if(!bEnabled) {
        return;
    }

    if(owner == NULL || timeDelta == 0) {
        return;
    }
    if(owner->bStatic == true) {
        return;
    }

    // don't update on first two ticks
    if(localWorld.GetTicks() <= 1) {
        return;
    }

    if(velocity.UnitSq() <= 1 && OnGround()) {
        velocity.Clear();
        CorrectSectorPosition();
        return;
    }

    currentMass = (bInWater && waterLevel >= WLT_BETWEEN) ? 0 : mass;
    start       = owner->GetOrigin();
    time        = timeDelta;
    massAmount  = (currentMass * timeDelta);
    radius      = owner->Radius();
    height      = owner->Height();
    gravity     = localWorld.GetGravity();

    trace.owner = owner;
    trace.bUseBBox = true;
    trace.localBBox.min.Set(-(radius * 0.5f), 0, -(radius * 0.5f));
    trace.localBBox.max.Set(radius * 0.5f, height, radius * 0.5f);
    trace.bbox = trace.localBBox;
    trace.bbox.min += start;
    trace.bbox.max += start;
    trace.sector = &sector;
    // resize box to account for movement
    trace.bbox *= (velocity * time);

    trace.start = start;
    trace.end = start + (gravity * mass) * mass;
    trace.dir = gravity;

    // need to determine if we're standing on the ground or not
    localWorld.Trace(&trace, clipFlags);
    if(trace.hitTri) {
        groundGeom = trace.hitTri;
        groundMesh = trace.hitMesh;
    }

    if(bInWater && waterLevel >= WLT_BETWEEN) {
        bOnGround = false;

        // slowly drift to the bottom
        if(waterLevel == WLT_UNDER && sinkVelocity != 0) {
            kexVec3 sinkVel = (-gravity * (GetWaterDepth() / timeDelta)) + velocity;
            velocity = velocity.Lerp(sinkVel, -sinkVelocity * timeDelta);
        }
    }
    else {
        bOnGround = OnGround();

        // handle freefall if not touching the ground
        if(!bOnGround) {
            velocity += (gravity * massAmount);
        }
        else {
            ImpactVelocity(velocity, groundGeom->plane.Normal(), 1.024f);
        }
    }

    trace.start = start;
    trace.end = start + (velocity * time);
    trace.dir = (trace.end - trace.start).Normalize();

    // trace through world
    localWorld.Trace(&trace, clipFlags);

    // project velocity
    if(trace.fraction != 1) {
        ImpactVelocity(velocity, trace.hitNormal, 1.024f);
    }

    if(sector) {
        groundGeom = &sector->lowerTri;
    }

    // update origin
    owner->SetOrigin(trace.hitVector - (trace.dir * 0.125f));
    owner->LinkArea();

    if(groundMesh) {
        // fudge the origin if we're slightly clipping below the floor
        trace.start = start - (gravity * (stepHeight * 0.5f));
        trace.end = start;
        localWorld.Trace(&trace, clipFlags);
    
        if(trace.fraction != 1 && !trace.hitActor) {
            start = trace.hitVector - (gravity * 1.024f);
            owner->SetOrigin(start);
            owner->LinkArea();
        }
    }

    ApplyFriction();
    CorrectSectorPosition();
}
