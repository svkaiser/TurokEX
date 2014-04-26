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

#ifndef __VIEWPORTALS_H__
#define __VIEWPORTALS_H__

#include "viewBounds.h"

typedef enum {
    PAS_TOP     = 0,
    PAS_BOTTOM,
    PAS_LEFT,
    PAS_RIGHT,
    PAS_FRONT,
    PAS_BACK,
    NUMPORTALSIDES,
    PAS_INVALID = -1
} portalAxisSide_t;

class kexPortal {
public:
                                kexPortal(void);
                                ~kexPortal(void);

    void                        SetPlanes(void);
    void                        LinkPortal(kexPortal *portal);

    kexBBox                     &Bounds(void) { return bounds; }

    typedef struct {
        kexBBox                 bounds;
        kexPortal               *portal;
        kexPlane                plane;
        int                     sideRef;
        bool                    bInView;
    } portalLink_t;

    kexArray<portalLink_t>      links;
    bool                        bInView;

    void                        ClipLinkToViewBounds(kexCamera *camera, portalLink_t *link,
                                                     kexViewBounds &viewBound);

private:
    kexBBox                     bounds;
    kexArray<kexWorldModel>     worldModels;

    kexPlane                    sides[NUMPORTALSIDES];
};

#endif
