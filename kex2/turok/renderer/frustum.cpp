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
// DESCRIPTION: Frustum class
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "frustum.h"

//
// kexFrustum::kexFrustum
//

kexFrustum::kexFrustum(void) {
}

//
// kexFrustum::TransformToView
//

void kexFrustum::TransformToView(kexMatrix &proj, kexMatrix &model) {
    kexMatrix clip  = model * proj;
    
    float *_right   = p[0].ToVec4().ToFloatPtr();
    float *_left    = p[1].ToVec4().ToFloatPtr();
    float *_bottom  = p[2].ToVec4().ToFloatPtr();
    float *_top     = p[3].ToVec4().ToFloatPtr();
    float *_far     = p[4].ToVec4().ToFloatPtr();
    float *_near    = p[5].ToVec4().ToFloatPtr();
    
    for(int i = 0; i < 4; i++) {
        _right[i]    = clip.vectors[i].w - clip.vectors[i].x;
        _left[i]     = clip.vectors[i].w + clip.vectors[i].x;
        _top[i]      = clip.vectors[i].w - clip.vectors[i].y;
        _bottom[i]   = clip.vectors[i].w + clip.vectors[i].y;
        _far[i]      = clip.vectors[i].w - clip.vectors[i].z;
        _near[i]     = clip.vectors[i].w + clip.vectors[i].z;
    }
}

//
// kexFrustum::TestBoundingBox
//

bool kexFrustum::TestBoundingBox(const kexBBox &bbox) {
    for(int i = 0; i < 6; i++) {
        kexVec4 vec(p[i].ToVec4());
        
        float d1 = vec.x * bbox.min.x + vec.y * bbox.min.y + vec.z * bbox.min.z + vec.w;
        float d2 = vec.x * bbox.max.x + vec.y * bbox.min.y + vec.z * bbox.min.z + vec.w;
        float d3 = vec.x * bbox.min.x + vec.y * bbox.max.y + vec.z * bbox.min.z + vec.w;
        float d4 = vec.x * bbox.max.x + vec.y * bbox.max.y + vec.z * bbox.min.z + vec.w;
        float d5 = vec.x * bbox.min.x + vec.y * bbox.min.y + vec.z * bbox.max.z + vec.w;
        float d6 = vec.x * bbox.max.x + vec.y * bbox.min.y + vec.z * bbox.max.z + vec.w;
        float d7 = vec.x * bbox.min.x + vec.y * bbox.max.y + vec.z * bbox.max.z + vec.w;
        float d8 = vec.x * bbox.max.x + vec.y * bbox.max.y + vec.z * bbox.max.z + vec.w;
        
        if(((FLOATSIGNBIT(d1) << 0) |
            (FLOATSIGNBIT(d2) << 1) |
            (FLOATSIGNBIT(d3) << 2) |
            (FLOATSIGNBIT(d4) << 3) |
            (FLOATSIGNBIT(d5) << 4) |
            (FLOATSIGNBIT(d6) << 5) |
            (FLOATSIGNBIT(d7) << 6) |
            (FLOATSIGNBIT(d8) << 7)) == 0xff) {
                return false;
        }
    }
    
    return true;
}

//
// kexFrustum::TestTriangle
//

bool kexFrustum::TestTriangle(const kexTri &triangle) {
    for(int i = 0; i < 6; i++) {
        kexVec3 n(p[i].Normal());
        
        float d = 0;
        int bits = 0;
        
        for(int j = 0; j < 3; j++) {
            d = n.Dot(*triangle.point[j]);
            bits |= (FLOATSIGNBIT(d) << j);
        }
       
        if(bits != 0x7) {
            continue;
        }
       
        return false;
    }
    
    return true;
}

//
// kexFrustum::TestSphere
//

bool kexFrustum::TestSphere(const kexVec3 &org, const float radius) {
    for(int i = 0; i < 6; i++) {
        kexVec3 n(p[i].Normal());
        
        if(n.Dot(org) <= -radius)
            return false;
        
    }
    
    return true;
}
