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

DECLARE_ABSTRACT_CLASS(kexActor, kexObject)

//
// kexActor::kexActor
//

kexActor::kexActor(void) {
    this->mass          = 1200;
    this->friction      = 1.0f;
    this->airFriction   = 1.0f;
    this->refCount      = 0;
    this->bStatic       = true;
    this->bCollision    = false;
    this->bTouch        = false;
    this->bClientOnly   = false;
    this->bHidden       = false;
    
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

DECLARE_CLASS(kexWorldActor, kexActor)

unsigned int kexWorldActor::id = 0;

//
// kexWorldActor::kexWorldActor
//

kexWorldActor::kexWorldActor(void) {
    this->radius        = 30.72f;
    this->baseHeight    = 30.72f;
    this->viewHeight    = 16.384f;
    this->centerHeight  = 10.24f;
    this->bRotor        = false;
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
}

//
// kexWorldActor::Tick
//

void kexWorldActor::Tick(void) {
}

//
// kexWorldActor::Remove
//

void kexWorldActor::Remove(void) {
}

//
// kexWorldActor::Parse
//

void kexWorldActor::Parse(kexLexer *lexer) {
}

//
// kexWorldActor::UpdateTransform
//

void kexWorldActor::UpdateTransform(void) {
    if(bRotor) {
        angles.yaw      += (rotorVector.y * rotorSpeed * timeStamp);
        angles.pitch    += (rotorVector.x * rotorSpeed * timeStamp);
        angles.roll     += (rotorVector.z * rotorSpeed * timeStamp);
    }

    if(!bStatic || bRotor) {
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
// kexWorldActor::GroundDistance
//

float kexWorldActor::GroundDistance(void) {
    return 0;
}

//
// kexWorldActor::OnGround
//

bool kexWorldActor::OnGround(void) {
    return GroundDistance() <= ONPLANE_EPSILON;
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

void kexWorldActor::CreateComponent(void) {
    if(!(component = J_NewObjectEx(js_context, NULL, NULL, NULL)))
        return;

    JS_AddRoot(js_context, &component);
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
