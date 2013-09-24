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
// DESCRIPTION: Camera mechanics/behaviors
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "actor.h"
#include "camera.h"

DECLARE_CLASS(kexCamera, kexActor)

//
// kexCamera::kexCamera
//

kexCamera::kexCamera(void) {
    this->zFar          = -1;
    this->zNear         = 0.1f;
    this->fov           = 45.0f;
    this->bLetterBox    = false;
    this->bFixedFOV     = false;
    this->aspect        = 4.0f/3.0f;
}

//
// kexCamera::~kexCamera
//

kexCamera::~kexCamera(void) {
}

//
// kexCamera::UpdateAspect
//

void kexCamera::UpdateAspect(void) {
    aspect = (float)sysMain.VideoWidth() / (float)sysMain.VideoHeight();
}

//
// kexCamera::SetupMatrices
//

void kexCamera::SetupMatrices(void) {
    UpdateAspect();

    // projection
    projMatrix.Identity();

    float camFov    = (float)bFixedFOV ? fov : cvarClientFOV.GetFloat();
    float top       = zNear * (float)tan(camFov * M_PI / 360.0f);
    float bottom    = -top;
    float left      = bottom * aspect;
    float right     = top * aspect;
    
    projMatrix.vectors[0].x =  (2 * zNear) / (right - left);
    projMatrix.vectors[1].y =  (2 * zNear) / (top - bottom);
    projMatrix.vectors[3].z = -(2 * zFar * zNear) / (zFar - zNear);

    projMatrix.vectors[2].x =  (right + left) / (right - left);
    projMatrix.vectors[2].y =  (top + bottom) / (top - bottom);
    projMatrix.vectors[2].z = -(zFar + zNear) / (zFar - zNear);

    projMatrix.vectors[0].y = 0;
    projMatrix.vectors[0].z = 0;
    projMatrix.vectors[0].w = 0;
    projMatrix.vectors[1].x = 0;
    projMatrix.vectors[1].w = 0;
    projMatrix.vectors[1].z = 0;
    projMatrix.vectors[2].w = -1;
    projMatrix.vectors[3].x = 0;
    projMatrix.vectors[3].y = 0;
    projMatrix.vectors[3].w = 0;
    
    // model
    modelMatrix.Identity();

    kexQuat yaw(-angles.yaw + M_PI, kexVec3::vecUp);
    kexQuat pitch(angles.pitch, kexVec3::vecRight);
    kexQuat roll(angles.roll,
        kexVec3(0, (float)sin(angles.pitch), (float)cos(angles.pitch)));
    
    modelMatrix = kexMatrix((yaw * roll) * pitch);
    modelMatrix.AddTranslation(-(origin | modelMatrix));

    // frustum
    viewFrustum.TransformToView(projMatrix, modelMatrix);
}

//
// kexCamera::LocalTick
//

void kexCamera::LocalTick(void) {
    AttachmentTransform();
}

//
// kexCamera::InitObject
//

void kexCamera::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kCamera",
        sizeof(kexCamera),
        asOBJ_REF);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kCamera",
        asBEHAVE_ADDREF,
        "void f()",
        asMETHOD(kexCamera, AddRef),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectBehaviour(
        "kCamera",
        asBEHAVE_RELEASE,
        "void f()",
        asMETHOD(kexCamera, RemoveRef),
        asCALL_THISCALL);

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kCamera",                                  \
        str,                                        \
        asMETHODPR(kexCamera, a, b, c),             \
        asCALL_THISCALL)

    OBJMETHOD("kVec3 &GetOrigin(void)", GetOrigin, (void), kexVec3&);
    OBJMETHOD("void SetOrigin(const kVec3 &in)", SetOrigin, (const kexVec3 &org), void);
    OBJMETHOD("void SetTarget(kActor@)", SetTarget, (kexActor *targ), void);
    OBJMETHOD("kActor @GetTarget(void)", GetTarget, (void), kexActor*);
    OBJMETHOD("void SetAttachment(kActor@)", SetAttachment, (kexActor *targ), void);
    OBJMETHOD("kActor @GetAttachment(void)", GetAttachment, (void), kexActor*);
    OBJMETHOD("void SetOwner(kActor@)", SetOwner, (kexActor *targ), void);
    OBJMETHOD("kActor @GetOwner(void)", GetOwner, (void), kexActor*);
    OBJMETHOD("kQuat &GetRotation(void)", GetRotation, (void), kexQuat&);
    OBJMETHOD("void SetRotation(const kQuat &in)", SetRotation, (const kexQuat &rot), void);
    OBJMETHOD("kAngle &GetAngles(void)", GetAngles, (void), kexAngle&);
    OBJMETHOD("void SetAngles(const kAngle &in)", SetAngles, (const kexAngle &an), void);
    OBJMETHOD("kVec3 &GetAttachOffset(void)", GetAttachOffset, (void), kexVec3&);
    OBJMETHOD("void SetAttachOffset(const kVec3 &in)", SetAttachOffset, (const kexVec3 &vec), void);

#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kCamera",                                  \
        str,                                        \
        asOFFSET(kexCamera, p))

    OBJPROPERTY("float zFar", zFar);
    OBJPROPERTY("float zNear", zNear);
    OBJPROPERTY("float fov", fov);
    OBJPROPERTY("float aspect", aspect);
    OBJPROPERTY("bool bLetterBox", bLetterBox);
    OBJPROPERTY("bool bFixedFOV", bFixedFOV);
    OBJPROPERTY("bool bCollision", bCollision);
    OBJPROPERTY("bool bAttachRelativeAngles", bAttachRelativeAngles);
}
