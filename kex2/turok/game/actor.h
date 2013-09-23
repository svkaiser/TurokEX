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

#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "common.h"
#include "script.h"
#include "linkedlist.h"
#include "scriptAPI/scriptSystem.h"

BEGIN_EXTENDED_CLASS(kexActor, kexObject);
public:
                                kexActor(void);
                                ~kexActor(void);

    virtual void                LocalTick(void);
    virtual void                Tick(void);
    virtual void                Remove(void);

    void                        SetTarget(kexActor *targ);
    void                        SetOwner(kexActor *targ);
    void                        SetAttachment(kexActor *targ);

    kexVec3                     &GetOrigin(void) { return origin; }
    void                        SetOrigin(const kexVec3 &org) { origin = org; }
    kexVec3                     &GetVelocity(void) { return velocity; }
    void                        SetVelocity(const kexVec3 &vel) { velocity = vel; }
    kexQuat                     &GetRotation(void) { return rotation; }
    void                        SetRotation(const kexQuat &rot) { rotation = rot; }
    kexActor                    *GetOwner(void) { return owner; }
    kexActor                    *GetTarget(void) { return target; }
    kexActor                    *GetAttachment(void) { return attachment; }
    kexAngle                    &GetAngles(void) { return angles; }
    void                        SetAngles(const kexAngle &an) { angles = an; }
    const int                   RefCount(void) const { return refCount; }
    kexMatrix                   &Matrix(void) { return matrix; }
    kexBBox                     &BoundingBox(void) { return bbox; }
    const kmodel_t              *Model(void) const { return model; }

    bool                        bStatic;
    bool                        bCollision;
    bool                        bTouch;
    bool                        bClientOnly;
    bool                        bHidden;
    bool                        bCulled;

protected:
    int                         AddRef(void);
    int                         RemoveRef(void);

    kexVec3                     origin;
    kexQuat                     rotation;
    kexVec3                     velocity;
    kexAngle                    angles;
    kexBBox                     bbox;
    kexBBox                     baseBBox;
    float                       friction;
    float                       airFriction;
    float                       mass;
    float                       bounceDamp;
    float                       cullDistance;
    unsigned int                targetID;
    kexActor                    *owner;
    kexActor                    *target;
    kexActor                    *attachment;
    kexMatrix                   matrix;
    kexMatrix                   rotMatrix;
    kmodel_t                    *model;
    kexVec3                     scale;
    float                       timeStamp;
    float                       tickDistance;
    float                       tickIntervals;
    float                       nextTickInterval;
    unsigned int                physics;
    int                         waterlevel;

private:
    int                         refCount;
    bool                        bStale;
END_CLASS();

BEGIN_EXTENDED_CLASS(kexWorldActor, kexActor);
public:
                                kexWorldActor(void);
                                ~kexWorldActor(void);

    virtual void                LocalTick(void);
    virtual void                Tick(void);
    virtual void                Remove(void);
    virtual void                Parse(kexLexer *lexer);
    virtual void                UpdateTransform(void);
    virtual void                OnTouch(kexWorldActor *instigator);
    virtual void                Think(void);

    void                        Spawn(void);
    bool                        Event(const char *function, long *args, unsigned int nargs);
    bool                        ToJSVal(long *val);
    bool                        AlignToSurface(void);
    float                       GroundDistance(void);
    bool                        OnGround(void);
    kexVec3                     ToLocalOrigin(const float x, const float y, const float z);
    kexVec3                     ToLocalOrigin(const kexVec3 &org);
    void                        SpawnFX(const char *fxName, const float x, const float y, const float z);
    void                        SetModel(const char *modelFile);
    void                        CreateComponent(const char *name);

    gObject_t                   *Component(void) { return component; }
    const int                   Variant(void) const { return variant; }
    kexStr                      &GetName(void) { return name; }
    void                        SetName(kexStr &str) { name = str; }

    static unsigned int         id;

    static void                 InitObject(void);

    kexLinklist<kexWorldActor>  worldLink;
    kexLinklist<kexWorldActor>  gridLink;

    kexComponent                scriptComponent;

    // TODO - need some sort of skin system
    char                        ****textureSwaps;

protected:
    void                        ParseDefault(kexLexer *lexer);
    gObject_t                   *component;
    gObject_t                   *iterator;
    bool                        bRotor;
    bool                        bOrientOnSlope;
    kexStr                      name;
    int                         health;
    float                       radius;
    float                       height;
    float                       baseHeight;
    float                       centerHeight;
    float                       viewHeight;
    kexQuat                     lerpRotation;
    float                       rotorSpeed;
    float                       rotorFriction;
    kexVec3                     rotorVector;
    kexVec3                     *nodeOffsets_t;
    kexQuat                     *nodeOffsets_r;
    animstate_t                 animState;
    int                         variant;
    rcolor                      *vertexColors;
END_CLASS();

#endif
