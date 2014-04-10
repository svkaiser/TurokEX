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
// DESCRIPTION: Player Collision detection / Physics behavior
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "world.h"
#include "physics/physics_player.h"

#define TRYMOVE_COUNT   5

DECLARE_CLASS(kexPlayerPhysics, kexPhysics)

//
// kexPlayerPhysics::kexPlayerPhysics
//

kexPlayerPhysics::kexPlayerPhysics(void) {
}

//
// kexPlayerPhysics::~kexPlayerPhysics
//

kexPlayerPhysics::~kexPlayerPhysics(void) {
}

//
// kexPlayerPhysics::Think
//

void kexPlayerPhysics::Think(const float timeDelta) {
    traceInfo_t trace;
    float       slope;
    int         moves;
    int         hits;
    float       d;
    float       time;
    float       currentMass;
    float       massAmount;
    float       radius;
    float       height;
    float       stepFraction;
    bool        bCanStep;
    kexVec3     start;
    kexVec3     end;
    kexVec3     direction;
    kexVec3     vel;
    kexVec3     normals[TRYMOVE_COUNT];
    kexVec3     slideNormal;
    kexVec3     gravity;
    kexVec3     cDir;

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
    moves       = 0;
    time        = timeDelta;
    massAmount  = (currentMass * timeDelta);
    radius      = owner->Radius();
    height      = owner->BaseHeight();
    bCanStep    = true;
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
    trace.bbox |= (velocity * time);

    // handle interactions with touchable objects
    if(owner->bCanPickup && owner->areaLink.node) {
        for(kexWorldObject *obj = owner->areaLink.node->objects.Next();
            obj != NULL;
            obj = obj->areaLink.link.Next()) {
                if(obj == owner || !obj->bTouch) {
                    continue;
                }
                if(obj->Bounds().IntersectingBox(trace.bbox)) {
                    obj->OnTouch(owner);
                }
        }
    }

    trace.start = start;
    trace.end = start + (gravity * mass) * mass;
    trace.dir = gravity;

    // need to determine if we're standing on the ground or not
    localWorld.Trace(&trace, clipFlags);
    if(trace.hitTri) {
        groundGeom = trace.hitTri;
        groundMesh = trace.hitMesh;
    }

    if(trace.hitActor && trace.hitActor->bTouch) {
        trace.hitActor->OnTouch(owner);
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
            vel = velocity;

            // project along the ground plane
            if(velocity.Dot(groundGeom->plane.Normal()) <= 1.024f) {
                float oldDist;
                float newDist;

                ImpactVelocity(velocity, groundGeom->plane.Normal(), 1.024f);

                if(vel.Dot(velocity) > 0) {
                    oldDist = vel.UnitSq();
                    if(oldDist > 1) {
                        newDist = velocity.UnitSq();
                        if(oldDist != newDist && newDist > 1) {
                            velocity *= kexMath::Sqrt(oldDist / newDist);
                        }
                    }
                }

                normals[moves++] = trace.hitNormal;
            }
        }
    }

    bClimbing = false;

    if(sector && sector->flags & CLF_CLIMB) {
        bCanStep = false;
    }

    // fudge the origin if we're slightly clipping below the floor
    trace.start = start - (gravity * (stepHeight * 0.5f));
    trace.end = start;
    localWorld.Trace(&trace, clipFlags);

    if(trace.fraction != 1 && !trace.hitActor) {
        start = trace.hitVector - (gravity * 1.024f);
        owner->SetOrigin(start);
        owner->LinkArea();
    }

    for(int i = 0; i < TRYMOVE_COUNT; i++) {
        start = owner->GetOrigin();
        end = start + (velocity * time);

        direction = (end - start);
        direction.Normalize();

        trace.start = start;
        trace.end = end;
        trace.dir = direction;

        // trace through world
        localWorld.Trace(&trace, clipFlags);
        time -= (time * trace.fraction);

        if(sector) {
            groundGeom = &sector->lowerTri;
        }

        if(trace.fraction >= 1) {
            // went the entire distance
            owner->SetOrigin(end);
            owner->LinkArea();
            break;
        }

        // check if climbing up along a surface
        if(sector && sector->flags & CLF_CLIMB) {
            owner->SetOrigin(trace.hitVector);
            ClimbOnSurface(owner->GetOrigin(), end, &sector->lowerTri);
            bClimbing = true;

            // nudge origin away from plane
            owner->GetOrigin() -= (direction * 0.0035f);
            owner->LinkArea();

            velocity.Clear();
            break;
        }
        else {
            owner->SetOrigin(trace.hitVector);
            if(!trace.hitActor) {
                // update origin and nudge origin away from plane
                owner->GetOrigin() -= (direction * 0.125f);
            }
            owner->LinkArea();
        }

        // test if walking on steep slopes
        slope = trace.hitNormal.Dot(gravity);

        if(slope >= 0) {
            bCanStep = false;
        }

        if(trace.hitTri && !trace.hitActor) {
            if(slope < 0 && slope >= -0.5f) {
                if(trace.hitTri == groundGeom) {
                    // remove vertical movement
                    slideNormal = (trace.hitNormal + (trace.hitNormal * gravity));
                    vel = (velocity + (velocity * gravity));

                    ImpactVelocity(vel, slideNormal, 1.024f);

                    // continue sliding down the slope
                    vel = (-gravity * velocity) + vel;
                    if(vel.Dot(velocity) <= 0) {
                        velocity.Clear();
                        break;
                    }

                    velocity = vel;
                    bCanStep = false;
                }
                else if(trace.hitMesh != NULL) {
                    // trying to move from ground to steep slope will be
                    // treated as a solid wall
                    cDir = trace.hitVector + (trace.hitNormal.Cross(-gravity).Normalize());

                    cDir += (cDir * gravity);
                    cDir -= gravity;

                    trace.hitVector += (trace.hitVector * gravity);
                    trace.hitVector -= gravity;

                    trace.hitNormal = trace.hitVector.Cross(cDir);
                    trace.hitNormal += (trace.hitNormal * gravity);
                    trace.hitNormal.Normalize();

                    bCanStep = false;
                }
            }
        }

        if(moves >= TRYMOVE_COUNT) {
            break;
        }

        normals[moves++] = trace.hitNormal;
        
        if(trace.hitTri && !trace.hitActor) {
            // handle stepping
            if(bCanStep && slope >= -0.5f) {
                trace.start = owner->GetOrigin();
                trace.end = trace.start + (-gravity * stepHeight);
                trace.dir = -gravity;

                // trace up
                localWorld.Trace(&trace, clipFlags);

                trace.start = trace.hitVector;
                trace.end = trace.start + (velocity * time);
                trace.dir = direction;

                // see if we can trace over the step
                localWorld.Trace(&trace, clipFlags);
                stepFraction = trace.fraction;

                trace.start = trace.hitVector;
                trace.end = trace.start + (gravity * stepHeight);
                trace.dir = gravity;

                // test the ground
                localWorld.Trace(&trace, clipFlags);
                slope = trace.hitNormal.Dot(-gravity);

                if(((trace.hitTri != groundGeom && slope > 0.5f) || trace.fraction >= 1)) {
                    // don't try to step up against a wall
                    if(!(stepFraction < 0.99f && slope <= 0.5f)) {
                        owner->SetOrigin(trace.hitVector - (gravity * 0.125f));
                        owner->LinkArea();

                        if(stepFraction >= 1) {
                            break;
                        }

                        time -= (time * trace.fraction);
                    }
                }
            }
        }

        // try all interacted normals
        for(hits = 0; hits < moves; hits++) {
            if(velocity.Dot(normals[hits]) >= 0) {
                continue;
            }

            vel = velocity;
            ImpactVelocity(vel, normals[hits], 1.024f);

            // try bumping against another plane
            for(int j = 0; j < moves; j++) {
                if(j == hits || vel.Dot(normals[j]) >= 0) {
                    continue;
                }

                // bump into second plane
                ImpactVelocity(vel, normals[j], 1.024f);

                if(vel.Dot(normals[hits]) >= 0) {
                    continue;
                }

                // slide along the crease between two planes
                cDir = normals[hits].Cross(normals[j]).Normalize();
                d = cDir.Dot(velocity);
                vel = cDir * d;

                // see if it bumps into a third plane
                for(int k = 0; k < moves; k++) {
                    if(k != j && k != hits && vel.Dot(normals[k]) < 0) {
                        // force a dead stop
                        velocity.Clear();
                        return;
                    }
                }
            }

            velocity = vel;
            break;
        }
    }

    ApplyFriction();
    CorrectSectorPosition();
}
