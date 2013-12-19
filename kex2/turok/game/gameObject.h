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

#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "common.h"
#include "scriptAPI/component.h"

//-----------------------------------------------------------------------------
//
// kexGameObject
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexGameObject, kexObject);
public:
                                kexGameObject(void);
                                ~kexGameObject(void);

    virtual void                LocalTick(void) = 0;
    virtual void                Tick(void) = 0;
    virtual void                Remove(void);

    void                        Spawn(void);
    int                         AddRef(void);
    int                         RemoveRef(void);
    void                        SetTarget(kexGameObject *targ);
    void                        SetOwner(kexGameObject *targ);
    const bool                  Removing(void) const;
    void                        StartSound(const char *name);
    void                        StartSound(const kexStr &name);

    kexVec3                     &GetOrigin(void) { return origin; }
    void                        SetOrigin(const kexVec3 &org) { origin = org; }
    kexAngle                    &GetAngles(void) { return angles; }
    void                        SetAngles(const kexAngle &an) { angles = an; }
    kexGameObject               *GetOwner(void) { return owner; }
    kexGameObject               *GetTarget(void) { return target; }
    unsigned int                &TargetID(void) { return targetID; }
    
    static unsigned int         id;
    
    const int                   RefCount(void) const { return refCount; }
    const bool                  IsStale(void) const { return bStale; }

    bool                        bClientOnly;    // ignored by server / only updated by LocalTick

    //
    // template for registering default script actor methods and properties
    //
    template<class type>
    static void                 RegisterBaseProperties(const char *scriptClass) {
    #define OBJMETHOD(str, a, b, c)                     \
        scriptManager.Engine()->RegisterObjectMethod(   \
            scriptClass,                                \
            str,                                        \
            asMETHODPR(type, a, b, c),                  \
            asCALL_THISCALL)

        OBJMETHOD("kVec3 &GetOrigin(void)", GetOrigin, (void), kexVec3&);
        OBJMETHOD("void SetOrigin(const kVec3 &in)", SetOrigin, (const kexVec3 &org), void);
        OBJMETHOD("void SetTarget(kActor@)", SetTarget, (kexGameObject *targ), void);
        OBJMETHOD("kActor @GetTarget(void)", GetTarget, (void), kexGameObject*);
        OBJMETHOD("void SetOwner(kActor@)", SetOwner, (kexGameObject *targ), void);
        OBJMETHOD("kActor @GetOwner(void)", GetOwner, (void), kexGameObject*);
        OBJMETHOD("kAngle &GetAngles(void)", GetAngles, (void), kexAngle&);
        OBJMETHOD("void SetAngles(const kAngle &in)", SetAngles, (const kexAngle &an), void);
        OBJMETHOD("const kStr ClassName(void) const", ClassString, (void) const, const kexStr);
        OBJMETHOD("void StartSound(const kStr &in)", StartSound, (const kexStr &name), void);
        OBJMETHOD("void Remove(void)", Remove, (void), void);
        OBJMETHOD("const int RefCount(void)const", RefCount, (void)const, const int);

    #define OBJPROPERTY(str, p)                         \
        scriptManager.Engine()->RegisterObjectProperty( \
            scriptClass,                                \
            str,                                        \
            asOFFSET(type, p))

        OBJPROPERTY("bool bClientOnly", bClientOnly);
        OBJPROPERTY("uint targetID", targetID);

    #undef OBJMETHOD
    #undef OBJPROPERTY
    }

protected:
    kexVec3                     origin;         // (xyz) position
    kexAngle                    angles;         // yaw, pitch, roll
    unsigned int                targetID;
    kexGameObject               *owner;
    kexGameObject               *target;
    float                       timeStamp;
    float                       tickDistance;
    float                       tickIntervals;
    float                       nextTickInterval;

private:
    int                         refCount;
    bool                        bStale;         // freed on next game tick
END_CLASS();

#endif
