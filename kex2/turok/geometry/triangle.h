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

#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "mathlib.h"

class kexTri {
public:
                    kexTri(void);
                    
    kexVec3         *point[3];
    kexTri          *edgeLink[3];
    bool            bEdgeBlock[3];
    kexPlane        plane;
    kexBBox         bounds;
    
    void            SetPoint(const int idx, float *p);
    void            SetBounds(void);
    bool            PointInRange(const kexVec3 &pt, const float expand);
    const float     GetDistance(const kexVec3 &pos) const;
    const float     GetEdgeYaw(const int idx) const;
    const kexVec3   GetCenterPoint(void) const;

    // debugging
    int             id;
    bool            bTraced;

    static int      globalID;
};

#endif
