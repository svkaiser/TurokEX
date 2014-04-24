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

#ifndef __DISPLAYOBJECT_H__
#define __DISPLAYOBJECT_H__

#include "common.h"
#include "gameObject.h"
#include "attachment.h"

typedef enum {
    ODT_NORMAL      = 0,
    ODT_CLIENTVIEW,
    ODT_FOREGROUND
} objDisplayType_t;

//-----------------------------------------------------------------------------
//
// kexDisplayObject
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexDisplayObject, kexGameObject);
public:
                                kexDisplayObject(void);
                                ~kexDisplayObject(void);

    virtual void                LocalTick(void) = 0;
    virtual void                Tick(void) = 0;
    virtual void                UpdateTransform(void);
    
    void                        Spawn(void);
    void                        Save(kexBinFile *saveFile);
    void                        Load(kexBinFile *loadFile);

    kexQuat                     &GetRotation(void) { return rotation; }
    void                        SetRotation(const kexQuat &rot) { rotation = rot; }
    kexVec3                     &GetScale(void) { return scale; }
    void                        SetScale(const kexVec3 &s) { scale = s; }
    kexMatrix                   &Matrix(void) { return matrix; }
    kexMatrix                   &RotationMatrix(void) { return rotMatrix; }
    kexAttachment               &Attachment(void) { return attachment; }
    float                       &CullDistance(void) { return cullDistance; }
    int                         &DisplayType(void) { return displayType; }

    bool                        bHidden;        // don't draw by renderer
    bool                        bCulled;        // currently culled by frustum or distance

    unsigned int                queryIndex;

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

        kexGameObject::RegisterBaseProperties<type>(scriptClass);

        OBJMETHOD("kQuat &GetRotation(void)", GetRotation, (void), kexQuat&);
        OBJMETHOD("void SetRotation(const kQuat &in)", SetRotation, (const kexQuat &rot), void);
        OBJMETHOD("kVec3 &GetScale(void)", GetScale, (void), kexVec3&);
        OBJMETHOD("void SetScale(const kVec3 &in)", SetScale, (const kexVec3 &s), void);
        OBJMETHOD("kAttachment &Attachment(void)", Attachment, (void), kexAttachment&);
        OBJMETHOD("void UpdateTransform(void)", UpdateTransform, (void), void);

    #define OBJPROPERTY(str, p)                         \
        scriptManager.Engine()->RegisterObjectProperty( \
            scriptClass,                                \
            str,                                        \
            asOFFSET(type, p))

        OBJPROPERTY("bool bHidden", bHidden);
        OBJPROPERTY("bool bCulled", bCulled);
        OBJPROPERTY("int displayType", displayType);

    #undef OBJMETHOD
    #undef OBJPROPERTY
    }

protected:
    virtual void                EmitSound(const char *name);

    kexQuat                     rotation;       // rotations in quaternions
    kexAttachment               attachment;     // attachment object
    float                       cullDistance;
    kexMatrix                   matrix;         // modelview matrix
    kexMatrix                   rotMatrix;
    kexVec3                     scale;
    int                         displayType;
END_CLASS();

#endif
