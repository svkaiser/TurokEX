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
#include "worldObject.h"
#include "renderModel.h"
#include "animation.h"
#include "clipmesh.h"

class kexClipMesh;
class kexAI;

//-----------------------------------------------------------------------------
//
// kexActor - common actor type used by game world. majority of the level
// content is made up of these types of actors
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexActor, kexWorldObject);
public:
                                kexActor(void);
                                ~kexActor(void);

    virtual void                LocalTick(void);
    virtual void                Tick(void);
    virtual void                Parse(kexLexer *lexer);
    virtual void                UpdateTransform(void);
    virtual void                OnTouch(kexActor *instigator);
    virtual void                OnTrigger(void);
    virtual void                OnDamage(kexWorldObject *instigator, int damage, kexKeyMap *damageDef);
    virtual void                OnDeath(kexWorldObject *instigator, kexKeyMap *damageDef);

    void                        Spawn(void);
    kexVec3                     ToLocalOrigin(const float x, const float y, const float z);
    kexVec3                     ToLocalOrigin(const kexVec3 &org);
    void                        SpawnFX(const char *fxName, const float x, const float y, const float z);
    void                        SpawnFX(const kexStr &str, const float x, const float y, const float z);
    void                        SetModel(const char *modelFile);
    void                        SetModel(const kexStr &modelFile);
    void                        CreateComponent(const char *name);
    void                        SetRotationOffset(const int node, const float angle,
                                                  const float x, const float y, const float z);

    const int                   Variant(void) const { return variant; }
    kexStr                      &GetName(void) { return name; }
    void                        SetName(kexStr &str) { name = str; }
    kexClipMesh                 &ClipMesh(void) { return clipMesh; }
    kexVec3                     *GetNodeTranslations(void) { return nodeOffsets_t; }
    kexQuat                     *GetNodeRotations(void) { return nodeOffsets_r; }
    kexAnimState                *AnimState(void) { return &animState; }
    const kexModel_t            *Model(void) const { return model; }
    kexAI                       *ToAI(void) { return reinterpret_cast<kexAI*>(this); }

    static unsigned int         id;

    static void                 InitObject(void);

    kexLinklist<kexActor>       worldLink;
    kexActorComponent           scriptComponent;
    kexKeyMap                   args;

    // TODO - need some sort of skin system
    char                        ****textureSwaps;
    bool                        bTraced;
    int                         validcount;
    bool                        bNoFixedTransform;

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

        kexWorldObject::RegisterBaseProperties<type>(scriptClass);

        OBJMETHOD("void SetModel(const kStr &in)", SetModel, (const kexStr &modelFile), void);
        OBJMETHOD("void SpawnFX(const kStr &in, float, float, float)", SpawnFX,
            (const kexStr &str, float x, float y, float z), void);
        OBJMETHOD("void SetRotationOffset(const int, const float, const float, const float, const float)",
            SetRotationOffset, (const int, const float, const float, const float, const float), void);

    #define OBJPROPERTY(str, p)                         \
        scriptManager.Engine()->RegisterObjectProperty( \
            scriptClass,                                \
            str,                                        \
            asOFFSET(type, p))

    OBJPROPERTY("ref @obj", scriptComponent.Handle());
    OBJPROPERTY("kQuat lerpRotation", lerpRotation);
    OBJPROPERTY("kKeyMap args", args);
    OBJPROPERTY("bool bCanPickup", bCanPickup);

    #undef OBJMETHOD
    #undef OBJPROPERTY
    }

protected:
    void                        ParseDefault(kexLexer *lexer);

    kexClipMesh                 clipMesh;
    kexStr                      name;
    kexModel_t                  *model;
    kexQuat                     lerpRotation;
    kexVec3                     *nodeOffsets_t;
    kexQuat                     *nodeOffsets_r;
    kexAnimState                animState;
    int                         variant;
    rcolor                      *vertexColors;
END_CLASS();

#endif
