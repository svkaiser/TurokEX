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
#include "scriptAPI/scriptSystem.h"

BEGIN_EXTENDED_CLASS(kexActor, kexObject);
public:
                        kexActor(void);
                        ~kexActor(void);

    virtual void        LocalTick(void);
    virtual void        Tick(void);
    virtual void        Remove(void);

    void                SetTarget(kexActor *targ);

    kexVec3             &GetOrigin(void) { return origin; }
    void                SetOrigin(const kexVec3 &org) { origin = org; }
    kexVec3             &GetVelocity(void) { return velocity; }
    void                SetVelocity(const kexVec3 &vel) { velocity = vel; }

protected:
    int                 AddRef(void);
    int                 RemoveRef(void);

    kexVec3             origin;
    kexQuat             rotation;
    kexVec3             velocity;
    bool                bStatic;
    bool                bCollision;
    bool                bTouch;
    bool                bClientOnly;
    bool                bHidden;
    bbox_t              bbox;
    bbox_t              baseBBox;
    float               friction;
    float               airFriction;
    float               mass;
    float               bounceDamp;
    float               cullDistance;
    unsigned int        targetID;
    kexActor            *owner;
    kexActor            *target;
    kexMatrix           matrix;
    kexMatrix           rotMatrix;
    kmodel_t            *model;
    kexVec3             scale;
    float               timestamp;
    float               tickDistance;
    float               tickIntervals;
    float               nextTickInterval;
    unsigned int        physics;
    int                 waterlevel;

private:
    int                 refCount;
    bool                bStale;
    bool                bCulled;
END_CLASS();

BEGIN_EXTENDED_CLASS(kexWorldActor, kexActor);
public:
                        kexWorldActor(void);
                        ~kexWorldActor(void);

    virtual void        LocalTick(void);
    virtual void        Tick(void);
    virtual void        Remove(void);
    virtual void        Parse(kexLexer *lexer);
    virtual void        UpdateTransform(void);

    bool                Event(const char *function, long *args, unsigned int nargs);

protected:
    gObject_t           *components;
    gObject_t           *iterator;
    bool                bRotor;
    kexAngle            angles;
    kexStr              name;
    float               radius;
    float               height;
    float               baseHeight;
    float               centerHeight;
    float               viewHeight;
    float               rotorSpeed;
    float               rotorFriction;
    kexVec3             rotorVector;
    kexVec3             *nodeOffsets_t;
    kexVec3             *nodeOffsets_r;

    static unsigned int id;
END_CLASS();

#endif
