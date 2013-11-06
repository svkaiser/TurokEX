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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "common.h"
#include "frustum.h"

BEGIN_EXTENDED_CLASS(kexCamera, kexActor);
public:
                        kexCamera(void);
                        ~kexCamera(void);

    virtual void        LocalTick(void);
    virtual void        Tick(void);
    virtual void        Remove(void);

    void                SetupMatrices(void);
    void                UpdateAspect(void);

    const float         Aspect(void) const { return aspect; }
    kexMatrix           &Projection(void) { return projMatrix; }
    kexMatrix           &ModelView(void) { return modelMatrix; }
    const kexFrustum    &Frustum(void) const { return viewFrustum; }
    kexAngle            &GetOffsetAngle(void) { return offsetAngle; }
    void                SetOffsetAngle(const kexAngle &an) { offsetAngle = an; }
    kexActor            *ToActor(void) { return static_cast<kexActor*>(this); }

    static void         InitObject(void);

private:
    kexMatrix           projMatrix;
    kexMatrix           modelMatrix;
    kexFrustum          viewFrustum;
    kexAngle            offsetAngle;
    float               zFar;
    float               zNear;
    float               fov;
    float               aspect;
    bool                bFixedFOV;
    bool                bLetterBox;
END_CLASS();

#endif
