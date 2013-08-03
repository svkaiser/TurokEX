// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
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
// DESCRIPTION: Bounding box operations
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "mathlib.h"

//
// BBox_Transform
//

void BBox_Transform(bbox_t srcBox, mtx_t matrix, bbox_t *out)
{
    bbox_t box;
    vec3_t c;
    vec3_t h;
    vec3_t ct;
    vec3_t ht;
    mtx_t m;

    Vec_Copy3(box.min, srcBox.min);
    Vec_Copy3(box.max, srcBox.max);

    Mtx_Copy(m, matrix);

    Vec_Add(c, box.min, box.max);
    Vec_Scale(c, c, 0.5f);
    Vec_Sub(h, box.max, c);
    Vec_TransformToWorld(m, c, ct);

    m[ 0] = (float)fabs(m[ 0]);
    m[ 1] = (float)fabs(m[ 1]);
    m[ 2] = (float)fabs(m[ 2]);
    m[ 4] = (float)fabs(m[ 4]);
    m[ 5] = (float)fabs(m[ 5]);
    m[ 6] = (float)fabs(m[ 6]);
    m[ 8] = (float)fabs(m[ 8]);
    m[ 9] = (float)fabs(m[ 9]);
    m[10] = (float)fabs(m[10]);
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;

    Vec_TransformToWorld(m, h, ht);

    Vec_Sub(box.min, ct, ht);
    Vec_Add(box.max, ct, ht);

    Vec_Copy3(out->min, box.min);
    Vec_Copy3(out->max, box.max);
}
