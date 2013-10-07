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
#include "renderModel.h"
#include "animation.h"
#include "linkedlist.h"
#include "clipmesh.h"
#include "physics.h"
#include "scriptAPI/scriptSystem.h"

class kexAttachment {
public:
    void                        Transform(void);
    void                        AttachToActor(kexActor *targ);
    void                        DettachActor(void);

    kexVec3                     &GetAttachOffset(void) { return attachOffset; }
    void                        SetAttachOffset(const kexVec3 &vec) { attachOffset = vec; }
    kexActor                    *GetOwner(void) { return owner; }
    void                        SetOwner(kexActor *o) { owner = o; }
    kexActor                    *GetAttachedActor(void) { return actor; }
    
    bool                        bAttachRelativeAngles;

private:
    kexVec3                     attachOffset;
    kexActor                    *actor;
    kexActor                    *owner;
};

BEGIN_EXTENDED_CLASS(kexActor, kexObject);
public:
                                kexActor(void);
                                ~kexActor(void);

    virtual void                LocalTick(void);
    virtual void                Tick(void);
    virtual void                Remove(void);

    int                         AddRef(void);
    int                         RemoveRef(void);
    void                        SetTarget(kexActor *targ);
    void                        SetOwner(kexActor *targ);

    kexVec3                     &GetOrigin(void) { return origin; }
    void                        SetOrigin(const kexVec3 &org) { origin = org; }
    kexQuat                     &GetRotation(void) { return rotation; }
    void                        SetRotation(const kexQuat &rot) { rotation = rot; }
    kexVec3                     &GetScale(void) { return scale; }
    void                        SetScale(const kexVec3 &vel) { scale = vel; }
    kexActor                    *GetOwner(void) { return owner; }
    kexActor                    *GetTarget(void) { return target; }
    kexAttachment               &Attachment(void) { return attachment; }
    kexPhysics                  *Physics(void) { return &physics; }
    kexAngle                    &GetAngles(void) { return angles; }
    void                        SetAngles(const kexAngle &an) { angles = an; }
    const int                   RefCount(void) const { return refCount; }
    kexMatrix                   &Matrix(void) { return matrix; }
    kexBBox                     &BoundingBox(void) { return bbox; }
    const kexModel_t            *Model(void) const { return model; }

    struct gridBound_s          *gridBound;

    bool                        bStatic;
    bool                        bCollision;
    bool                        bTouch;
    bool                        bClientOnly;
    bool                        bHidden;
    bool                        bCulled;

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
        OBJMETHOD("void SetTarget(kActor@)", SetTarget, (kexActor *targ), void);
        OBJMETHOD("kActor @GetTarget(void)", GetTarget, (void), kexActor*);
        OBJMETHOD("void SetOwner(kActor@)", SetOwner, (kexActor *targ), void);
        OBJMETHOD("kActor @GetOwner(void)", GetOwner, (void), kexActor*);
        OBJMETHOD("kQuat &GetRotation(void)", GetRotation, (void), kexQuat&);
        OBJMETHOD("void SetRotation(const kQuat &in)", SetRotation, (const kexQuat &rot), void);
        OBJMETHOD("kAngle &GetAngles(void)", GetAngles, (void), kexAngle&);
        OBJMETHOD("void SetAngles(const kAngle &in)", SetAngles, (const kexAngle &an), void);
        OBJMETHOD("kAttachment &Attachment(void)", Attachment, (void), kexAttachment&);
        OBJMETHOD("const kStr ClassName(void) const", GetClassString, (void) const, const kexStr);

    #define OBJPROPERTY(str, p)                         \
        scriptManager.Engine()->RegisterObjectProperty( \
            scriptClass,                                \
            str,                                        \
            asOFFSET(type, p))

        OBJPROPERTY("bool bStatic", bStatic);
        OBJPROPERTY("bool bCollision", bCollision);
        OBJPROPERTY("bool bTouch", bTouch);
        OBJPROPERTY("bool bHidden", bHidden);
        OBJPROPERTY("bool bClientOnly", bClientOnly);
        OBJPROPERTY("bool bCulled", bCulled);

    #undef OBJMETHOD
    #undef OBJPROPERTY
    }

protected:
    kexVec3                     origin;
    kexQuat                     rotation;
    kexAngle                    angles;
    kexBBox                     bbox;
    kexBBox                     baseBBox;
    float                       cullDistance;
    unsigned int                targetID;
    kexActor                    *owner;
    kexActor                    *target;
    kexAttachment               attachment;
    kexPhysics                  physics;
    kexMatrix                   matrix;
    kexMatrix                   rotMatrix;
    kexModel_t                  *model;
    kexVec3                     scale;
    float                       timeStamp;
    float                       tickDistance;
    float                       tickIntervals;
    float                       nextTickInterval;
    int                         waterlevel;

private:
    int                         refCount;
    bool                        bStale;
END_CLASS();

class kexClipMesh;

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
    float                       Radius(void) { return radius; }
    float                       Height(void) { return height; }
    kexVec3                     ToLocalOrigin(const float x, const float y, const float z);
    kexVec3                     ToLocalOrigin(const kexVec3 &org);
    void                        SpawnFX(const char *fxName, const float x, const float y, const float z);
    void                        SetModel(const char *modelFile);
    void                        CreateComponent(const char *name);
    bool                        Trace(kexPhysics *physics,
                                      const kexVec3 &start,
                                      const kexVec3 &end,
                                      const kexVec3 &dir);

    gObject_t                   *Component(void) { return component; }
    const int                   Variant(void) const { return variant; }
    kexStr                      &GetName(void) { return name; }
    void                        SetName(kexStr &str) { name = str; }
    kexClipMesh                 &ClipMesh(void) { return clipMesh; }
    kexVec3                     *GetNodeTranslations(void) { return nodeOffsets_t; }
    kexQuat                     *GetNodeRotations(void) { return nodeOffsets_r; }
    kexAnimState                *AnimState(void) { return &animState; }

    static unsigned int         id;

    static void                 InitObject(void);

    kexLinklist<kexWorldActor>  worldLink;
    kexComponent                scriptComponent;

    // TODO - need some sort of skin system
    char                        ****textureSwaps;
    bool                        bTraced;

protected:
    void                        ParseDefault(kexLexer *lexer);

    gObject_t                   *component;
    gObject_t                   *iterator;
    kexClipMesh                 clipMesh;
    kexStr                      name;
    int                         health;
    float                       radius;
    float                       height;
    float                       baseHeight;
    float                       centerHeight;
    float                       viewHeight;
    kexQuat                     lerpRotation;
    kexVec3                     *nodeOffsets_t;
    kexQuat                     *nodeOffsets_r;
    kexAnimState                animState;
    int                         variant;
    rcolor                      *vertexColors;
END_CLASS();

#endif
