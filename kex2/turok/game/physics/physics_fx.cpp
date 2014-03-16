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
// DESCRIPTION: FX Collision detection / Physics behavior
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "fx.h"
#include "world.h"
#include "physics/physics_fx.h"

DECLARE_CLASS(kexFxPhysics, kexPhysics)

//
// kexFxPhysics::kexFxPhysics
//

kexFxPhysics::kexFxPhysics(void) {
}

//
// kexFxPhysics::~kexFxPhysics
//

kexFxPhysics::~kexFxPhysics(void) {
}

//
// kexFxPhysics::ImpactObject
//

void kexFxPhysics::ImpactObject(kexFx *fx, kexWorldObject *obj, kexVec3 &normal) {
    fxinfo_t *fxinfo = fx->fxInfo;
    kexFx *nfx;
    short iType;

    switch(fxinfo->ontouch) {
        case VFX_BOUNCE:
            if(fxinfo->bStopAnimOnImpact) {
                fx->bAnimate = false;
            }
            ImpactVelocity(velocity, normal, 1.05f);
            break;
        case VFX_DESTROY:
            owner->GetOrigin() += (normal * 1.024f);
            iType = obj->GetImpactType();

            if(iType != -1) {
                nfx = fx->Event(&fxinfo->onImpact[iType], obj);
                if(nfx != fx && nfx->fxInfo->drawtype == VFX_DRAWSURFACE) {
                    nfx->SetRotation(normal.ToQuat());
                }
            }

            fx->Remove();
            fx->SetParent(NULL);
            break;
        default:
            break;
    }
}

//
// kexFxPhysics::ImpactSurface
//

void kexFxPhysics::ImpactSurface(kexFx *fx, kexTri *geom, kexVec3 &normal) {
    fxinfo_t *fxinfo = fx->fxInfo;
    kexArea *area = sector->area;
    kexFx *nfx;
    short iType;

    switch(fxinfo->onplane) {
        case VFX_BOUNCE:
            if(fxinfo->bStopAnimOnImpact) {
                fx->bAnimate = false;
            }
            ImpactVelocity(velocity, normal, 1.05f);
            ApplyFriction();
            break;
        case VFX_DESTROY:
            fx->GetOrigin() += (normal * 1.024f);

            if(geom == NULL) {
                iType = area->WallSurfaceType();
            }
            else {
                iType = area->FloorSurfaceType();
            }

            if(iType != -1) {
                nfx = fx->Event(&fxinfo->onImpact[iType], NULL);
                if(nfx != fx && nfx->fxInfo->drawtype == VFX_DRAWSURFACE) {
                    nfx->SetRotation(normal.ToQuat());
                }
            }
            fx->Remove();
            fx->SetParent(NULL);
            break;
        default:
            break;
    }
}

//
// kexFxPhysics::Think
//

void kexFxPhysics::Think(const float timeDelta) {
    kexVec3 move;
    kexFx *fx;
    float moveAmount;
    fxinfo_t *fxinfo;
    traceInfo_t trace;
    kexVec3 start;
    int oldWL;

    if(!bEnabled) {
        return;
    }

    if(owner == NULL) {
        return;
    }

    velocity += (localWorld.GetGravity() * (mass * timeDelta));

    move = velocity * timeDelta;
    moveAmount = move.UnitSq();

    if(moveAmount < 0.01f) {
        return;
    }

    fx = static_cast<kexFx*>(owner);
    start = owner->GetOrigin();

    if(owner->bCollision == false) {
        owner->SetOrigin(start + move);
        return;
    }

    if(sector == NULL) {
        kexWorldObject *source = static_cast<kexWorldObject*>(owner->GetOwner());
        kexFx *parent = fx->GetParent();

        if(parent != NULL && parent->Physics()->sector) {
            sector = parent->Physics()->sector;
        }
        else if(source != NULL && source->Physics()->sector) {
            sector = source->Physics()->sector;
        }
        else {
            sector = localWorld.CollisionMap().PointInSector(owner->GetOrigin());
        }
    }

    if(sector) {
        CheckWater(0);
    }

    oldWL = waterLevel;
    fxinfo = fx->fxInfo;

    if(moveAmount < 0.001f || fxinfo->onplane == VFX_DEFAULT) {
        owner->SetOrigin(start + move);
    }
    else {
        if(fxinfo->bLinkArea && CorrectSectorPosition()) {
            ImpactSurface(fx, &sector->lowerTri, sector->lowerTri.plane.Normal());
            return;
        }
        trace.owner = owner;
        trace.sector = &sector;
        trace.bUseBBox = false;
        trace.start = start;
        trace.end = start + move;
        trace.dir = (trace.end - start).Normalize();

        if(fxinfo->bLinkArea) {
            trace.bUseBBox = true;
            trace.localBBox.min.Set(-2, -2, -2);
            trace.localBBox.max.Set(2, 2, 2);
            trace.bbox = trace.localBBox;
            trace.bbox.min += start;
            trace.bbox.max += start;
            // resize box to account for movement
            trace.bbox |= (velocity * timeDelta);
        }

        localWorld.Trace(&trace);

        if(trace.fraction >= 1) {
            owner->SetOrigin(start + move);

            if(fxinfo->bLinkArea) {
                owner->LinkArea();
            }
        }
        else {
            owner->SetOrigin(trace.hitVector - (trace.dir * 0.125f));

            if(fxinfo->bLinkArea) {
                owner->LinkArea();
            }

            if(trace.hitActor != NULL) {
                ImpactObject(fx, trace.hitActor, trace.hitNormal);
                return;
            }

            if(trace.hitTri != NULL) {
                groundGeom = trace.hitTri;
            }

            ImpactSurface(fx, trace.hitTri, trace.hitNormal);
        }
    }

    if(sector != NULL) {
        // check if in water sector
        CheckWater(0);
        if((oldWL == WLT_OVER && waterLevel >= WLT_BETWEEN) ||
            (oldWL >= WLT_BETWEEN && waterLevel == WLT_OVER)) {
                float tmpY = owner->GetOrigin()[1];
                owner->GetOrigin()[1] = waterHeight + 4.096f;
                fx->Event(&fxinfo->onWaterImpact, NULL);

                if(fxinfo->bDestroyOnWaterSurface) {
                    fx->SetOwner(NULL);
                    fx->SetParent(NULL);
                    fx->Remove();
                    velocity.Clear();
                }

                owner->GetOrigin()[1] = tmpY;
        }
    }
}
