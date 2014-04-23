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
#include "renderUtils.h"

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
    float d;
    for(int i = 0; i < NUMFRUSTUMPLANES; i++) {
        d = p[i].a * bbox.min.x + p[i].b * bbox.min.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.min.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * bbox.min.x + p[i].b * bbox.max.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.max.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * bbox.min.x + p[i].b * bbox.min.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.min.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * bbox.min.x + p[i].b * bbox.max.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.max.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }

        return false;
    }
    
    return true;
}

//
// kexFrustum::TestTriangle
//

bool kexFrustum::TestTriangle(const kexTri &triangle) {
    float d;
    kexVec3 *pt1 = triangle.point[0];
    kexVec3 *pt2 = triangle.point[1];
    kexVec3 *pt3 = triangle.point[2];
    for(int i = 0; i < NUMFRUSTUMPLANES; i++) {
        d = p[i].a * pt1->x + p[i].b * pt1->y + p[i].c * pt1->z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * pt2->x + p[i].b * pt2->y + p[i].c * pt2->z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * pt3->x + p[i].b * pt3->y + p[i].c * pt3->z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
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
    for(int i = 0; i < NUMFRUSTUMPLANES; i++) {
        if(p[i].Distance(org) + p[i].d <= -radius) {
            return false;
        }
        
    }
    return true;
}

//
// kexFrustum::TestSegment
//

bool kexFrustum::TestSegment(const kexVec3 pt1, const kexVec3 &pt2) {
    float d;
    for(int i = 0; i < NUMFRUSTUMPLANES; i++) {
        d = p[i].a * pt1.x + p[i].b * pt1.y + p[i].c * pt1.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        d = p[i].a * pt2.x + p[i].b * pt2.y + p[i].c * pt2.z + p[i].d;
        if(!FLOATSIGNBIT(d)) {
            continue;
        }
        
        return false;
    }
    
    return true;
}

//
// kexFrustum::BoxDistance
//

bool kexFrustum::BoxDistance(const kexBBox &box, const float distance) {
    kexPlane nearPlane = Near();
    
    return (nearPlane.Distance(box.Center()) + nearPlane.d) > distance;
}

//
// kexFrustum::ClipSegment
//
//

bool kexFrustum::ClipSegment(kexVec3 &out1, kexVec3 &out2,
                             int &clipbits1, int &clipbits2,
                             const kexVec3 &start, const kexVec3 &end) {
    float d1;
    float d2;
    float frac;
    float f1;
    float f2;
    int bit1;
    int bit2;
    kexVec3 hit;

    clipbits1 = 0;
    clipbits2 = 0;
    f1 = 1.0f;
    f2 = 1.0f;
    
    out1 = start;
    out2 = end;
    
    if(!TestSegment(start, end)) {
        clipbits1 = FRUSTUM_CLIPPED;
        clipbits2 = FRUSTUM_CLIPPED;
        return false;
    }
    
    for(int i = 0; i < NUMFRUSTUMPLANES; i++) {
        d1 = p[i].Distance(start) + p[i].d;
        d2 = p[i].Distance(end) + p[i].d;
        
        bit1 = FLOATSIGNBIT(d1);
        bit2 = FLOATSIGNBIT(d2);
        
        if(d1 <= 0 && d2 <= 0) {
            clipbits1 |= FRUSTUM_CLIPPED | (bit1 << i);
            clipbits2 |= FRUSTUM_CLIPPED | (bit2 << i);
            return false;
        }

        if(bit1 ^ bit2) {
            if(d2 < d1) {
                frac = (d1 / (d1 - d2));

                if(frac > 1 || frac > f1) {
                    continue;
                }
                clipbits2 |= (bit2 << i);
                out2 = start.Lerp(end, frac);
                f1 = frac;
            }
            else {
                frac = (d2 / (d2 - d1));

                if(frac > 1 || frac > f2) {
                    continue;
                }
                clipbits1 |= (bit1 << i);
                out1 = end.Lerp(start, frac);
                f2 = frac;
            }
        }
    }
    
    return true;
}
