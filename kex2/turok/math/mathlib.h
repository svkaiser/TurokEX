// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#include "type.h"

#define M_PI    3.1415926535897932384626433832795f
#define M_RAD   (M_PI / 180.0f)
#define M_DEG   (180.0f / M_PI)

//
// MATRIX OPERATIONS
//
void Mtx_ViewFrustum(int width, int height, float fovy, float znear);
void Mtx_AddTranslation(mtx_t m, float x, float y, float z);
void Mtx_SetTranslation(mtx_t m, float x, float y, float z);
void Mtx_Scale(mtx_t m, float x, float y, float z);
void Mtx_MultiplyByAxis(mtx_t m1, mtx_t m2, float x, float y, float z);
void Mtx_Transpose(mtx_t m);
void Mtx_TransposeDup(mtx_t m1, mtx_t m2);
void Mtx_Identity(mtx_t m);
void Mtx_IdentityAxis(mtx_t m, float x, float y, float z);
void Mtx_IdentityX(mtx_t m, float angle);
void Mtx_IdentityY(mtx_t m, float angle);
void Mtx_IdentityZ(mtx_t m, float angle);
void Mtx_Multiply(mtx_t out, mtx_t m1, mtx_t m2);
void Mtx_MultiplyRotation(mtx_t out, mtx_t m1, mtx_t m2);
void Mtx_ApplyToVector(mtx_t m, vec3_t vec, vec3_t out);
void Mtx_ApplyVector(mtx_t m, vec3_t vec);
void Mtx_ApplyCoordinates(mtx_t m, vec3_t src, vec3_t out);
void Mtx_SetFromAxis(mtx_t m, float angle, float x, float y, float z);
void Mtx_Copy(mtx_t dest, mtx_t src);
void Mtx_RotateX(mtx_t m, float angle);
void Mtx_RotateY(mtx_t m, float angle);
void Mtx_RotateZ(mtx_t m, float angle);

#endif

