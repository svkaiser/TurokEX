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

#ifndef __WORLD_OBJECT_H__
#define __WORLD_OBJECT_H__

#include "common.h"
#include "displayObject.h"
#include "script.h"
#include "linkedlist.h"
#include "physics.h"

typedef struct areaNode_s areaNode_t;

//-----------------------------------------------------------------------------
//
// kexWorldObject
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexWorldObject, kexDisplayObject);
public:
                                kexWorldObject(void);
                                ~kexWorldObject(void);

    virtual void                LocalTick(void) = 0;
    virtual void                Tick(void) = 0;
    virtual void                OnTouch(kexWorldObject *instigator);

    void                        SetBoundingBox(const kexVec3 &min, const kexVec3 &max);
    bool                        Trace(traceInfo_t *trace);
    bool                        TryMove(const kexVec3 &position, kexVec3 &dest, kexSector **sector = NULL);
    bool                        AlignToSurface(void);
    void                        LinkArea(void);
    void                        UnlinkArea(void);

    float                       Radius(void) { return radius; }
    float                       Height(void) { return height; }
    float                       BaseHeight(void) { return baseHeight; }
    float                       GetCenterHeight(void) { return centerHeight; }
    void                        SetCenterHeight(float f) { centerHeight = f; }
    float                       GetViewHeight(void) { return viewHeight; }
    void                        SetViewHeight(float f) { viewHeight = f; }
    kexPhysics                  *Physics(void) { return &physics; }
    kexBBox                     &Bounds(void) { return bbox; }

    kexLinklist<kexWorldObject> areaLink;
    areaNode_t                  *areaNode;

    bool                        bStatic;        // no tick/think behavior
    bool                        bCollision;     // handle collision with this actor
    bool                        bTouch;         // can be touched/picked up by other actors
    bool                        bOrientOnSlope;
    bool                        bCanPickup;

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

        kexDisplayObject::RegisterBaseProperties<type>(scriptClass);

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
        OBJPROPERTY("bool bOrientOnSlope", bOrientOnSlope);
        OBJPROPERTY("int health", health);
        OBJPROPERTY("float radius", radius);
        OBJPROPERTY("float height", height);
        OBJPROPERTY("float baseHeight", baseHeight);
        OBJPROPERTY("float centerHeight", centerHeight);
        OBJPROPERTY("float viewHeight", viewHeight);

    #undef OBJMETHOD
    #undef OBJPROPERTY
    }

protected:
    kexBBox                     bbox;           // bounding box
    kexBBox                     baseBBox;       // unmodified bounding box
    int                         health;
    float                       radius;
    float                       height;
    float                       baseHeight;
    float                       centerHeight;
    float                       viewHeight;
    kexPhysics                  physics;        // physics object
END_CLASS();

#endif
