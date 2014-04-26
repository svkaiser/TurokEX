// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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
// DESCRIPTION: View bounds
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "renderBackend.h"
#include "camera.h"
#include "viewBounds.h"

//
// kexViewBounds::kexViewBounds
//

kexViewBounds::kexViewBounds(void) {
    Clear();
}


//
// kexViewBounds::Clear
//

void kexViewBounds::Clear(void) {
    max[0]  = -M_INFINITY;
    max[1]  = -M_INFINITY;
    min[0]  =  M_INFINITY;
    min[1]  =  M_INFINITY;
    zmin    = 0.0f;
    zfar    = 1.0f;
}

//
// kexViewBounds::IsClosed
//

const bool kexViewBounds::IsClosed(void) const {
    return (min[0] > max[0] || min[1] > max[1]);
}

//
// kexViewBounds::Fill
//

void kexViewBounds::Fill(void) {
    min[0] = 0;
    min[1] = 0;
    max[0] = (float)sysMain.VideoWidth();
    max[1] = (float)sysMain.VideoHeight();
    zfar = 0;
}

//
// kexViewBounds::AddPoint
//

void kexViewBounds::AddPoint(const float x, const float y, const float z) {
    if(x < min[0]) min[0] = x;
    if(x > max[0]) max[0] = x;
    if(y < min[1]) min[1] = y;
    if(y > max[1]) max[1] = y;
    
    // get closest z-clip
    if(z < zfar) zfar = z;
}

//
// kexViewBounds::AddVector
//

void kexViewBounds::AddVector(kexCamera *camera, kexVec3 &vector) {
    int bits;
    float d;
    kexFrustum frustum;
    kexVec3 pmin;
    
    frustum = camera->Frustum();
    bits = 0;
    
    d = frustum.Left().Distance(vector) + frustum.Left().d;
    bits |= (FLOATSIGNBIT(d)) << 0;
    d = frustum.Top().Distance(vector) + frustum.Top().d;
    bits |= (FLOATSIGNBIT(d)) << 1;
    d = frustum.Right().Distance(vector) + frustum.Right().d;
    bits |= (FLOATSIGNBIT(d)) << 2;
    d = frustum.Bottom().Distance(vector) + frustum.Bottom().d;
    bits |= (FLOATSIGNBIT(d)) << 3;
    
    pmin = camera->ProjectPoint(vector, 0, 0);
    
    if(bits & 1) {
        pmin[0] = 0;
    }
    
    if(bits & 2) {
        pmin[1] = 0;
    }
    
    if(bits & 4) {
        pmin[0] = (float)sysMain.VideoWidth();
    }
    
    if(bits & 8) {
        pmin[1] = (float)sysMain.VideoHeight();
    }
    
    AddPoint(pmin[0], pmin[1], pmin[2]);
    vector = pmin;
}

//
// kexViewBounds::AddBox
//

void kexViewBounds::AddBox(kexCamera *camera, kexBBox &box) {
    kexVec3 points[8];
    kexVec3 pmin;
    kexFrustum frustum;
    int i;
    int bits;
    float d;

    frustum = camera->Frustum();
    
    box.ToVectors(points);

    bits = 0;
    
    kexPlane nearPlane = frustum.Near();
    kexVec3 n = nearPlane.Normal().Normalize();
    
    for(i = 0; i < 8; i++) {
        bits = 0;

        d = frustum.Near().Distance(points[i]) + nearPlane.d;
        if(d < 0) {
            points[i] += (n * -d);
        }

        AddVector(camera, points[i]);
    }
}

//
// kexViewBounds::ViewBoundInside
//

bool kexViewBounds::ViewBoundInside(const kexViewBounds &viewBounds) {
    if(((viewBounds.min[0] < min[0] && viewBounds.max[0] < min[0]) ||
        (viewBounds.min[0] > max[0] && viewBounds.max[0] > max[0]))) {
        return false;
    }
    if(((viewBounds.min[1] < min[1] && viewBounds.max[1] < min[1]) ||
        (viewBounds.min[1] > max[1] && viewBounds.max[1] > max[1]))) {
        return false;
    }
    
    if(viewBounds.zfar < zfar) {
        return false;
    }
    
    return true;
}

//
// kexViewBounds::operator=
//

kexViewBounds &kexViewBounds::operator=(const kexViewBounds &viewBounds) {
    min[0] = viewBounds.min[0];
    min[1] = viewBounds.min[1];
    max[0] = viewBounds.max[0];
    max[1] = viewBounds.max[1];

    zmin = viewBounds.zmin;
    zfar = viewBounds.zfar;

    return *this;
}

//
// kexViewBounds::DebugDraw
//

void kexViewBounds::DebugDraw(void) {
    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    renderBackend.SetState(GLSTATE_CULL, false);
    renderBackend.SetState(GLSTATE_BLEND, true);
    
    renderBackend.DisableShaders();
    renderer.BindDrawPointers();
    renderer.AddLine(min[0], min[1], 0, min[0], max[1], 0, 255, 0, 255, 255);
    renderer.AddLine(min[0], max[1], 0, max[0], max[1], 0, 255, 0, 255, 255);
    renderer.AddLine(max[0], max[1], 0, max[0], min[1], 0, 255, 0, 255, 255);
    renderer.AddLine(max[0], min[1], 0, min[0], min[1], 0, 255, 0, 255, 255);
    renderer.DrawLineElements();
    
    renderBackend.SetState(GLSTATE_TEXTURE0, true);
}
