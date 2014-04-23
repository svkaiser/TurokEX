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

#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__

#include "triangle.h"

typedef enum {
    FP_RIGHT    = 0,
    FP_LEFT,
    FP_BOTTOM,
    FP_TOP,
    FP_FAR,
    FP_NEAR,
    NUMFRUSTUMPLANES
} frustumPlane_t;

#define FRUSTUM_CLIPPED BIT(NUMFRUSTUMPLANES)

class kexFrustum {
public:
                        kexFrustum(void);
                        
    void                TransformToView(kexMatrix &proj, kexMatrix &model);
    bool                TestBoundingBox(const kexBBox &bbox);
    bool                TestTriangle(const kexTri &triangle);
    bool                TestSphere(const kexVec3 &org, const float radius);
    bool                TestSegment(const kexVec3 pt1, const kexVec3 &pt2);
    bool                BoxDistance(const kexBBox &box, const float distance);
    bool                ClipSegment(kexVec3 &out1, kexVec3 &out2,
                                    int &clipbits1, int &clipbits2,
                                    const kexVec3 &start, const kexVec3 &end);
    
    kexPlane            &Right(void) { return p[FP_RIGHT]; }
    kexPlane            &Left(void) { return p[FP_LEFT]; }
    kexPlane            &Bottom(void) { return p[FP_BOTTOM]; }
    kexPlane            &Top(void) { return p[FP_TOP]; }
    kexPlane            &Far(void) { return p[FP_FAR]; }
    kexPlane            &Near(void) { return p[FP_NEAR]; }
    
private:
    kexPlane            p[NUMFRUSTUMPLANES];
};

#endif
