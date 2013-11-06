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

//-----------------------------------------------------------------------------
//
// kexAttachment - each actor thats attached is counted towards its
// reference counter
//
//-----------------------------------------------------------------------------

class kexAttachment {
public:
    void                        Transform(void);
    void                        AttachToActor(kexActor *targ);
    void                        DettachActor(void);

    kexVec3                     &GetAttachOffset(void) { return attachOffset; }
    kexVec3                     &GetSourceOffset(void) { return sourceOffset; }
    void                        SetAttachOffset(const kexVec3 &vec) { attachOffset = vec; }
    void                        SetSourceOffset(const kexVec3 &vec) { sourceOffset = vec; }
    kexActor                    *GetOwner(void) { return owner; }
    void                        SetOwner(kexActor *o) { owner = o; }
    kexActor                    *GetAttachedActor(void) { return actor; }
    
    bool                        bAttachRelativeAngles;

private:
    kexVec3                     attachOffset;
    kexVec3                     sourceOffset;
    kexActor                    *actor;
    kexActor                    *owner;
};

//-----------------------------------------------------------------------------
//
// kexActor - base class for all actor types
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexActor, kexObject);
public:
                                kexActor(void);
                                ~kexActor(void);

    virtual void                LocalTick(void) = 0;
    virtual void                Tick(void) = 0;
    virtual void                Remove(void);

    int                         AddRef(void);
    int                         RemoveRef(void);
    void                        SetTarget(kexActor *targ);
    void                        SetOwner(kexActor *targ);
    const bool                  Removing(void) const;

    void                        SetBoundingBox(const kexVec3 &min, const kexVec3 &max);
    bool                        Trace(traceInfo_t *trace);
    void                        PlaySound(const char *name);
    void                        PlaySound(const kexStr &name);

    kexVec3                     &GetOrigin(void) { return origin; }
    void                        SetOrigin(const kexVec3 &org) { origin = org; }
    kexQuat                     &GetRotation(void) { return rotation; }
    void                        SetRotation(const kexQuat &rot) { rotation = rot; }
    kexVec3                     &GetScale(void) { return scale; }
    void                        SetScale(const kexVec3 &s) { scale = s; }
    kexActor                    *GetOwner(void) { return owner; }
    kexActor                    *GetTarget(void) { return target; }
    kexAttachment               &Attachment(void) { return attachment; }
    float                       Radius(void) { return radius; }
    float                       Height(void) { return height; }
    float                       BaseHeight(void) { return baseHeight; }
    float                       GetCenterHeight(void) { return centerHeight; }
    void                        SetCenterHeight(float f) { centerHeight = f; }
    float                       GetViewHeight(void) { return viewHeight; }
    void                        SetViewHeight(float f) { viewHeight = f; }
    kexPhysics                  *Physics(void) { return &physics; }
    kexAngle                    &GetAngles(void) { return angles; }
    void                        SetAngles(const kexAngle &an) { angles = an; }
    const int                   RefCount(void) const { return refCount; }
    kexMatrix                   &Matrix(void) { return matrix; }
    kexBBox                     &BoundingBox(void) { return bbox; }
    const kexModel_t            *Model(void) const { return model; }
    const bool                  IsStale(void) const { return bStale; }

    struct gridBound_s          *gridBound;

    bool                        bStatic;        // no tick/think behavior
    bool                        bCollision;     // handle collision with this actor
    bool                        bTouch;         // can be touched/picked up by other actors
    bool                        bClientOnly;    // ignored by server / only updated by LocalTick
    bool                        bHidden;        // don't draw by renderer
    bool                        bCulled;        // currently culled by frustum or distance
    bool                        bClientView;    // can only be rendered through user-commands

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
        OBJMETHOD("kVec3 &GetScale(void)", GetScale, (void), kexVec3&);
        OBJMETHOD("void SetScale(const kVec3 &in)", SetScale, (const kexVec3 &s), void);
        OBJMETHOD("kAttachment &Attachment(void)", Attachment, (void), kexAttachment&);
        OBJMETHOD("const kStr ClassName(void) const", GetClassString, (void) const, const kexStr);
        OBJMETHOD("void PlaySound(const kStr &in)", PlaySound, (const kexStr &name), void);
        OBJMETHOD("void SetBoundingBox(const kVec3 &in, const kVec3 &in)",
            SetBoundingBox, (const kexVec3 &min, const kexVec3 &max), void);

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
        OBJPROPERTY("bool bClientView", bClientView);
        OBJPROPERTY("float radius", radius);
        OBJPROPERTY("float height", height);
        OBJPROPERTY("float baseHeight", baseHeight);
        OBJPROPERTY("float centerHeight", centerHeight);
        OBJPROPERTY("float viewHeight", viewHeight);

    #undef OBJMETHOD
    #undef OBJPROPERTY
    }

protected:
    kexVec3                     origin;         // (xyz) position
    kexQuat                     rotation;       // rotations in quaternions
    kexAngle                    angles;         // yaw, pitch, roll
    kexBBox                     bbox;           // bounding box
    kexBBox                     baseBBox;       // unmodified bounding box
    float                       cullDistance;
    unsigned int                targetID;
    kexActor                    *owner;
    kexActor                    *target;
    float                       radius;
    float                       height;
    float                       baseHeight;
    float                       centerHeight;
    float                       viewHeight;
    kexAttachment               attachment;     // attachment object
    kexPhysics                  physics;        // physics object
    kexMatrix                   matrix;         // modelview matrix
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
    bool                        bStale;         // freed on next game tick
END_CLASS();

class kexClipMesh;

//-----------------------------------------------------------------------------
//
// kexWorldActor - common actor type used by game world. majority of the level
// content is made up of these types of actors
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexWorldActor, kexActor);
public:
                                kexWorldActor(void);
                                ~kexWorldActor(void);

    virtual void                LocalTick(void);
    virtual void                Tick(void);
    virtual void                Parse(kexLexer *lexer);
    virtual void                UpdateTransform(void);
    virtual void                OnTouch(kexWorldActor *instigator);
    virtual void                Think(void);

    void                        Spawn(void);
    bool                        AlignToSurface(void);
    kexVec3                     ToLocalOrigin(const float x, const float y, const float z);
    kexVec3                     ToLocalOrigin(const kexVec3 &org);
    void                        SpawnFX(const char *fxName, const float x, const float y, const float z);
    void                        SpawnFX(const kexStr &str, const float x, const float y, const float z);
    void                        SetModel(const char *modelFile);
    void                        SetModel(const kexStr &modelFile);
    void                        CreateComponent(const char *name);

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
    kexQuat                     lerpRotation;
    kexVec3                     *nodeOffsets_t;
    kexQuat                     *nodeOffsets_r;
    kexAnimState                animState;
    int                         variant;
    rcolor                      *vertexColors;
END_CLASS();

#endif
