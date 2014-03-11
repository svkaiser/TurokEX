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

#ifndef __PHYSICS_FX_H__
#define __PHYSICS_FX_H__

#include "common.h"
#include "physics/physics.h"

class kexFx;

BEGIN_EXTENDED_CLASS(kexFxPhysics, kexPhysics);
public:
                            kexFxPhysics(void);
                            ~kexFxPhysics(void);

    virtual void            Think(const float timeDelta);
    void                    ImpactObject(kexFx *fx, kexWorldObject *obj, kexVec3 &normal);
    void                    ImpactSurface(kexFx *fx, kexTri *geom, kexVec3 &normal);

END_CLASS();

#endif
