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
// DESCRIPTION: Occlusion portals (PVS)
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "worldModel.h"
#include "renderBackend.h"
#include "camera.h"
#include "viewPortals.h"
#include "renderUtils.h"

//
// kexPortal::kexPortal
//

kexPortal::kexPortal(void) {
    this->bInView = false;
}

//
// kexPortal::~kexPortal
//

kexPortal::~kexPortal(void) {
    links.Empty();
    worldModels.Empty();
}

//
// kexPortal::SetPlanes
//

void kexPortal::SetPlanes(void) {
    sides[PAS_TOP]      = kexPlane(0, 1, 0, bounds.max.y);
    sides[PAS_BOTTOM]   = kexPlane(0, -1, 0, bounds.min.y);
    sides[PAS_LEFT]     = kexPlane(1, 0, 0, bounds.max.x);
    sides[PAS_RIGHT]    = kexPlane(-1, 0, 0, bounds.min.x);
    sides[PAS_FRONT]    = kexPlane(0, 0, 1, bounds.max.z);
    sides[PAS_BACK]     = kexPlane(0, 0, -1, bounds.min.z);
}

//
// kexPortal::LinkPortal
//

void kexPortal::LinkPortal(kexPortal *portal) {
    float dist;
    float d;
    float min_x;
    float min_y;
    float min_z;
    float max_x;
    float max_y;
    float max_z;

    if(portal == this) {
        // don't link self
        return;
    }

    for(int i = 0; i < NUMPORTALSIDES; i++) {
        for(int j = 0; j < NUMPORTALSIDES; j++) {
            d = sides[i].Normal().Dot(portal->sides[j].Normal());
            dist = sides[i].d - portal->sides[j].d;

            if(d <= -1 && dist == 0) {
                kexBBox box = portal->bounds;
                portalLink_t portalLink;

                switch(i) {
                    case PAS_FRONT:
                    case PAS_BACK:
                        min_x = (box.min.x < bounds.min.x) ? bounds.min.x : box.min.x;
                        max_x = (box.max.x > bounds.max.x) ? bounds.max.x : box.max.x;
                        min_y = (box.min.y < bounds.min.y) ? bounds.min.y : box.min.y;
                        max_y = (box.max.y > bounds.max.y) ? bounds.max.y : box.max.y;

                        min_z = max_z = (i == PAS_FRONT) ? bounds.max.z : bounds.min.z;
                        break;

                    case PAS_TOP:
                    case PAS_BOTTOM:
                        min_x = (box.min.x < bounds.min.x) ? bounds.min.x : box.min.x;
                        max_x = (box.max.x > bounds.max.x) ? bounds.max.x : box.max.x;
                        min_z = (box.min.z < bounds.min.z) ? bounds.min.z : box.min.z;
                        max_z = (box.max.z > bounds.max.z) ? bounds.max.z : box.max.z;

                        min_y = max_y = (i == PAS_TOP) ? bounds.max.y : bounds.min.y;
                        break;

                    case PAS_LEFT:
                    case PAS_RIGHT:
                        min_z = (box.min.z < bounds.min.z) ? bounds.min.z : box.min.z;
                        max_z = (box.max.z > bounds.max.z) ? bounds.max.z : box.max.z;
                        min_y = (box.min.y < bounds.min.y) ? bounds.min.y : box.min.y;
                        max_y = (box.max.y > bounds.max.y) ? bounds.max.y : box.max.y;

                        min_x = max_x = (i == PAS_LEFT) ? bounds.max.x : bounds.min.x;
                        break;
                }

                portalLink.bounds.min.Set(min_x, min_y, min_z);
                portalLink.bounds.max.Set(max_x, max_y, max_z);
                portalLink.portal = portal;
                portalLink.bInView = false;

                links.Push(portalLink);
            }
        }
    }
}

//
// kexPortal::ClipLinkToViewBounds
//

void kexPortal::ClipLinkToViewBounds(kexCamera *camera, portalLink_t *link, kexViewBounds &viewBound) {
    kexVec3 pt1(link->bounds.min.x, link->bounds.min.y, link->bounds.min.z);
    kexVec3 pt2(link->bounds.max.x, link->bounds.min.y, link->bounds.max.z);
    kexVec3 pt3(link->bounds.min.x, link->bounds.max.y, link->bounds.max.z);
    kexVec3 pt4(link->bounds.max.x, link->bounds.max.y, link->bounds.max.z);
    kexVec3 pt5(link->bounds.min.x, link->bounds.max.y, link->bounds.min.z);
    kexVec3 pt6(link->bounds.min.x, link->bounds.min.y, link->bounds.min.z);
    kexVec3 pt7(link->bounds.max.x, link->bounds.max.y, link->bounds.max.z);
    kexVec3 pt8(link->bounds.max.x, link->bounds.min.y, link->bounds.max.z);

    kexVec3 hit[8];
    int clipbits[8];

    if(camera->Frustum().ClipSegment(hit[0], hit[1], clipbits[0], clipbits[1], pt1, pt2)) {
        viewBound.AddVector(camera, hit[0]);
        viewBound.AddVector(camera, hit[1]);
    }

    if(camera->Frustum().ClipSegment(hit[2], hit[3], clipbits[2], clipbits[3], pt3, pt4)) {
        viewBound.AddVector(camera, hit[2]);
        viewBound.AddVector(camera, hit[3]);
    }
    
    if(camera->Frustum().ClipSegment(hit[4], hit[5], clipbits[4], clipbits[5], pt5, pt6)) {
        viewBound.AddVector(camera, hit[4]);
        viewBound.AddVector(camera, hit[5]);
    }
    
    if(camera->Frustum().ClipSegment(hit[6], hit[7], clipbits[6], clipbits[7], pt7, pt8)) {
        viewBound.AddVector(camera, hit[6]);
        viewBound.AddVector(camera, hit[7]);
    }
}
