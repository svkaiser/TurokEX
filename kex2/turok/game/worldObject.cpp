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
// DESCRIPTION: World Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "worldObject.h"
#include "world.h"

DECLARE_ABSTRACT_CLASS(kexWorldObject, kexDisplayObject)

//
// kexWorldObject::kexWorldObject
//

kexWorldObject::kexWorldObject(void) {
    this->bStatic       = true;
    this->bCollision    = false;
    this->bTouch        = false;
    this->radius        = 10.24f;
    this->baseHeight    = 10.24f;
    this->viewHeight    = 8.192f;
    this->centerHeight  = 5.12f; 
    
    this->physics.SetOwner(this);
}

//
// kexWorldObject::~kexWorldObject
//

kexWorldObject::~kexWorldObject(void) {
}

//
// kexWorldObject::SetBoundingBox
//

void kexWorldObject::SetBoundingBox(const kexVec3 &min, const kexVec3 &max) {
    baseBBox.min = min;
    baseBBox.max = max;
}

//
// kexWorldObject::Trace
//

bool kexWorldObject::Trace(traceInfo_t *trace) {
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

    float frac = (cp - kexMath::Sqrt(rd)) * (1.0f / len);

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
// kexWorldObject::AlignToSurface
//

bool kexWorldObject::AlignToSurface(void) {
    return false;
}
