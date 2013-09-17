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
    this->aspect        = 320.0f/240.0f;
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
    //
    // projection
    //
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
    
    //
    // model
    //
    modelMatrix.Identity();

    kexQuat yaw(-angles.yaw + M_PI, kexVec3::vecUp);
    kexQuat pitch(angles.pitch, kexVec3::vecRight);
    kexQuat roll(angles.roll,
        kexVec3(0, (float)sin(angles.pitch), (float)cos(angles.pitch)));
    
    modelMatrix = kexMatrix((yaw * roll) * pitch);
    modelMatrix.AddTranslation(-(origin | modelMatrix));
}

//
// kexCamera::LocalTick
//

void kexCamera::LocalTick(void) {
}
