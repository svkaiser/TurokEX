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

#include <math.h>
#include "shared.h"

#define M_PI    3.1415926535897932384626433832795f
#define M_RAD   (M_PI / 180.0f)
#define M_DEG   (180.0f / M_PI)

#define DEG2RAD(x) ((x) * M_RAD)
#define RAD2DEG(x) ((x) * M_DEG)

//
// VECTOR OPERATIONS
//
void  Vec_Copy3(vec3_t out, vec3_t vec);
void  Vec_Copy4(vec3_t out, vec3_t vec);
void  Vec_Set3(vec3_t vec, float x, float y, float z);
void  Vec_Set4(vec4_t vec, float x, float y, float z, float w);
kbool Vec_Compare3(vec3_t v1, vec3_t v2);
void  Vec_Cross(vec3_t out, vec3_t vec1, vec3_t vec2);
float Vec_Dot(vec3_t vec1, vec3_t vec2);
void  Vec_Add(vec3_t out, vec3_t vec1, vec3_t vec2);
void  Vec_Sub(vec3_t out, vec3_t vec1, vec3_t vec2);
void  Vec_Mult(vec3_t out, vec3_t vec1, vec3_t vec2);
void  Vec_Scale(vec3_t out, vec3_t vec1, float val);
float Vec_Length2(vec3_t v1, vec3_t v2);
float Vec_Length3(vec3_t v1, vec3_t v2);
float Vec_Unit2(vec3_t vec);
float Vec_Unit3(vec3_t vec);
void  Vec_Normalize3(vec3_t out);
void  Vec_Normalize4(vec4_t out);
void  Vec_TransformToWorld(mtx_t m, vec3_t vec, vec3_t out);
void  Vec_ApplyQuaternion(vec3_t out, vec3_t vec, vec4_t rot);
void  Vec_Lerp3(vec3_t out, float movement, vec3_t curr, vec3_t next);
void  Vec_Slerp(vec4_t out, float movement, vec4_t vec1, vec4_t vec2);
void  Vec_SetQuaternion(vec4_t vec, float angle, float x, float y, float z);
void  Vec_QuaternionToAxis(float *angle, vec3_t vec3, vec4_t vec4);
void  Vec_ToQuaternion(vec4_t out, vec3_t vec);
void  Vec_MultQuaternion(vec4_t out, vec4_t q1, vec4_t q2);
void  Vec_AdjustQuaternion(vec4_t out, vec4_t rot, float angle);
void  Vec_PointToAxis(vec3_t out, vec3_t p1, vec3_t p2);
void  Vec_PointToAngle(vec4_t out, vec3_t p1, vec3_t p2);

//
// MATRIX OPERATIONS
//
void  Mtx_ViewFrustum(int width, int height, float fovy, float znear);
void  Mtx_AddTranslation(mtx_t m, float x, float y, float z);
void  Mtx_SetTranslation(mtx_t m, float x, float y, float z);
void  Mtx_Scale(mtx_t m, float x, float y, float z);
void  Mtx_MultiplyByAxis(mtx_t m1, mtx_t m2, float x, float y, float z);
void  Mtx_Transpose(mtx_t m);
void  Mtx_TransposeDup(mtx_t m1, mtx_t m2);
void  Mtx_Identity(mtx_t m);
void  Mtx_IdentityAxis(mtx_t m, float x, float y, float z);
void  Mtx_IdentityX(mtx_t m, float angle);
void  Mtx_IdentityY(mtx_t m, float angle);
void  Mtx_IdentityZ(mtx_t m, float angle);
void  Mtx_Multiply(mtx_t out, mtx_t m1, mtx_t m2);
void  Mtx_MultiplyRotation(mtx_t out, mtx_t m1, mtx_t m2);
void  Mtx_ApplyVector(mtx_t m, vec3_t vec);
void  Mtx_ApplyCoordinates(mtx_t m, vec3_t src, vec3_t out);
void  Mtx_SetFromAxis(mtx_t m, float angle, float x, float y, float z);
void  Mtx_Copy(mtx_t dest, mtx_t src);
void  Mtx_RotateX(mtx_t m, float angle);
void  Mtx_RotateY(mtx_t m, float angle);
void  Mtx_RotateZ(mtx_t m, float angle);
void  Mtx_ApplyRotation(vec4_t rot, mtx_t out);

//
// ANGLE OPERATIONS
//
float Ang_AlignPitchToVector(vec3_t vec);
float Ang_AlignYawToVector(float angle, vec3_t v1, vec3_t v2);
float Ang_VectorToAngle(vec3_t vec);
float Ang_ClampInvert(float angle);
float Ang_ClampInvertSums(float angle1, float angle2);
void  Ang_Clamp(float *angle);
float Ang_Diff(float angle1, float angle2);

//
// PLANE OPERATIONS
//

#include "level.h"

void  Plane_SetTemp(plane_t *plane, vec3_t p1, vec3_t p2, vec3_t p3);
void  Plane_GetNormal(vec3_t normal, plane_t *plane);
void  Plane_GetCeilingNormal(vec3_t normal, plane_t *plane);
kbool Plane_IsFacing(plane_t *plane, float angle);
float Plane_GetDistance(plane_t *plane, vec3_t pos);
float Plane_GetHeight(plane_t *plane, vec3_t pos);
kbool Plane_IsAWall(plane_t *plane);
float Plane_GetYaw(plane_t *p);
float Plane_GetEdgeYaw(plane_t *p, int point);
float Plane_GetPitch(plane_t *p);
float Plane_GetSlope(plane_t *plane, float x1, float z1, float x2, float z2);
void  Plane_GetRotation(vec4_t vec, plane_t *p);
void  Plane_AdjustRotation(vec4_t out, plane_t *p);
kbool Plane_PointInRange(plane_t *p, float x, float z);

//
// RANDOM OPERATIONS

void Random_SetSeed(int seed);
int Random_Int(void);
int Random_Max(int max);
float Random_Float(void);
float Random_CFloat(void);

#endif

