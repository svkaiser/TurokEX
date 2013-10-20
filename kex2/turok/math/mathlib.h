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
#include "kstring.h"

#ifdef M_PI
#undef M_PI
#endif

#define M_PI        3.1415926535897932384626433832795f
#define M_RAD       (M_PI / 180.0f)
#define M_DEG       (180.0f / M_PI)
#define M_INFINITY  1e30f

#define DEG2RAD(x) ((x) * M_RAD)
#define RAD2DEG(x) ((x) * M_DEG)

#define FLOATSIGNBIT(f)  ((*(const unsigned long*)&(f)) >> 31)

//
// VECTOR OPERATIONS
//
void  Vec_Copy3(vec3_t out, vec3_t vec);
void  Vec_Copy4(vec3_t out, vec3_t vec);
void  Vec_Set3(vec3_t vec, float x, float y, float z);
void  Vec_Set4(vec4_t vec, float x, float y, float z, float w);
kbool Vec_Compare3(vec3_t v1, vec3_t v2);
char *Vec_ToString(vec3_t vec);
void  Vec_Cross(vec3_t out, vec3_t vec1, vec3_t vec2);
float Vec_Dot(vec3_t vec1, vec3_t vec2);
void  Vec_Add(vec3_t out, vec3_t vec1, vec3_t vec2);
void  Vec_Sub(vec3_t out, vec3_t vec1, vec3_t vec2);
void  Vec_Mult(vec3_t out, vec3_t vec1, vec3_t vec2);
void  Vec_Scale(vec3_t out, vec3_t vec1, float val);
float Vec_Magnitude2(vec3_t vec);
float Vec_Magnitude3(vec3_t vec);
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
void  Vec_PointAt(vec3_t org1, vec3_t org2, vec4_t rotation, float maxAngle, vec4_t out);
void  Vec_PointToAxis(vec3_t out, vec3_t p1, vec3_t p2);

//
// MATRIX OPERATIONS
//
void  Mtx_ViewFrustum(int width, int height, float fovy, float znear, float zfar);
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
void  Mtx_Invert(mtx_t out, mtx_t in);
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
float Ang_Round(float angle);
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
void  Plane_GetNormalizedRotation(vec4_t out, plane_t *p);
void  Plane_GetInclinationVector(plane_t *plane, vec3_t out);
kbool Plane_PointInRange(plane_t *p, float x, float z);

//
// RANDOM OPERATIONS
//
void Random_SetSeed(int seed);
int Random_Int(void);
int Random_Max(int max);
float Random_Float(void);
float Random_CFloat(void);

//
// BOUNDING BOX OPERATIONS
//
void BBox_Transform(bbox_t srcBox, mtx_t matrix, bbox_t *out);

class kexRand {
public:
    static void             SetSeed(const int randSeed);
    static int              SysRand(void);
    static int              Int(void);
    static int              Max(const int max);
    static float            Float(void);
    static float            CFloat(void);
    
private:
    static int              seed;
};

class kexVec3;
class kexVec4;
class kexMatrix;

class kexQuat {
public:
                            kexQuat(void);

                            explicit kexQuat(const float angle, const float x, const float y, const float z);
                            explicit kexQuat(const float angle, kexVec3 &vector);
                            explicit kexQuat(const float angle, const kexVec3 &vector);

    void                    Set(const float x, const float y, const float z, const float w);
    void                    Clear(void);
    float                   Dot(const kexQuat &quat) const;
    float                   UnitSq(void) const;
    float                   Unit(void) const;
    kexQuat                 &Normalize(void);
    kexQuat                 Slerp(const kexQuat &quat, float movement) const;

    kexQuat                 operator-(void) const;
    kexQuat                 operator+(const kexQuat &quat);
    kexQuat                 &operator+=(const kexQuat &quat);
    kexQuat                 operator-(const kexQuat &quat);
    kexQuat                 &operator-=(const kexQuat &quat);
    kexQuat                 operator*(const kexQuat &quat);
    kexQuat                 operator*(const float val) const;
    kexQuat                 &operator*=(const kexQuat &quat);
    kexQuat                 &operator*=(const float val);
    kexQuat                 &operator=(const kexQuat &quat);
    kexQuat                 &operator=(const kexVec4 &vec);
    kexQuat                 &operator=(const float *vecs);

    float                   x;
    float                   y;
    float                   z;
    float                   w;

    static void             ObjectConstruct1(kexQuat *thisq);
    static void             ObjectConstruct2(float a, float x, float y, float z, kexVec3 *thisq);
    static void             ObjectConstruct3(float a, kexVec3 &in, kexQuat *thisq);
    static void             ObjectConstructCopy(const kexQuat &in, kexQuat *thisq);
};

class kexVec3 {
public:
                            kexVec3(void);
                            explicit kexVec3(const float x, const float y, const float z);

    void                    Set(const float x, const float y, const float z);
    void                    Clear(void);
    float                   Dot(const kexVec3 &vec) const;
    static float            Dot(const kexVec3 &vec1, const kexVec3 &vec2);
    kexVec3                 Cross(const kexVec3 &vec) const;
    kexVec3                 &Cross(const kexVec3 &vec1, const kexVec3 &vec2);
    float                   UnitSq(void) const;
    float                   Unit(void) const;
    float                   DistanceSq(const kexVec3 &vec) const;
    float                   Distance(const kexVec3 &vec) const;
    kexVec3                 &Normalize(void);
    kexVec3                 PointAt(kexVec3 &location) const;
    kexVec3                 Lerp(const kexVec3 &next, float movement) const;
    kexVec3                 &Lerp(const kexVec3 &start, const kexVec3 &next, float movement);
    kexQuat                 ToQuat(void);
    float                   ToYaw(void) const;
    float                   ToPitch(void) const;
    kexStr                  ToString(void) const;

    kexVec3                 operator+(const kexVec3 &vec);
    kexVec3                 operator+(const kexVec3 &vec) const;
    kexVec3                 operator+(kexVec3 &vec);
    kexVec3                 operator-(void) const;
    kexVec3                 operator-(const kexVec3 &vec) const;
    kexVec3                 operator*(const kexVec3 &vec);
    kexVec3                 operator*(const float val);
    kexVec3                 operator*(const float val) const;
    kexVec3                 operator/(const kexVec3 &vec);
    kexVec3                 operator/(const float val);
    kexVec3                 operator|(const kexQuat &quat);
    kexVec3                 operator|(const kexMatrix &mtx);
    kexVec3                 &operator=(const kexVec3 &vec);
    kexVec3                 &operator=(const float *vecs);
    kexVec3                 &operator+=(const kexVec3 &vec);
    kexVec3                 &operator-=(const kexVec3 &vec);
    kexVec3                 &operator*=(const kexVec3 &vec);
    kexVec3                 &operator*=(const float val);
    kexVec3                 &operator/=(const kexVec3 &vec);
    kexVec3                 &operator/=(const float val);
    kexVec3                 &operator|=(const kexQuat &quat);
    kexVec3                 &operator|=(const kexMatrix &mtx);
    float                   operator[](int index) const;
    float                   operator[](int index);

    operator                float *(void) { return reinterpret_cast<float*>(&x); }

    static const kexVec3    vecForward;
    static const kexVec3    vecUp;
    static const kexVec3    vecRight;

    float                   x;
    float                   y;
    float                   z;

    static void             ObjectConstruct1(kexVec3 *thisvec);
    static void             ObjectConstruct2(float x, float y, float z, kexVec3 *thisvec);
    static void             ObjectConstructCopy(const kexVec3 &in, kexVec3 *thisvec);
};

class kexVec4 {
public:
                            kexVec4(void);
                            explicit kexVec4(const float x, const float y, const float z, const float w);

    void                    Set(const float x, const float y, const float z, const float w);
    void                    Clear(void);
    float                   *ToFloatPtr(void);
    
    const kexVec3           &ToVec3(void) const;
    kexVec3                 &ToVec3(void);
    float                   operator[](int index) const;
    float                   operator[](int index);

    float                   x;
    float                   y;
    float                   z;
    float                   w;
};

class kexMatrix {
public:
                            kexMatrix(void);
                            kexMatrix(const kexMatrix &mtx);
                            kexMatrix(const float x, const float y, const float z);
                            kexMatrix(const kexQuat &quat);
                            kexMatrix(const float angle, const int axis);

    kexMatrix               &Identity(void);
    kexMatrix               &Identity(const float x, const float y, const float z);
    kexMatrix               &SetTranslation(const float x, const float y, const float z);
    kexMatrix               &SetTranslation(const kexVec3 &vector);
    kexMatrix               &AddTranslation(const float x, const float y, const float z);
    kexMatrix               &AddTranslation(const kexVec3 &vector);
    kexMatrix               &Scale(const float x, const float y, const float z);
    kexMatrix               &Scale(const kexVec3 &vector);
    static kexMatrix        Scale(const kexMatrix &mtx, const float x, const float y, const float z);
    kexMatrix               &Transpose(void);
    static kexMatrix        Transpose(const kexMatrix &mtx);
    float                   *ToFloatPtr(void);
    void                    SetViewProjection(float aspect, float fov, float zNear, float zFar);
    
    kexMatrix               operator*(const kexVec3 &vector);
    kexMatrix               &operator*=(const kexVec3 &vector);
    kexMatrix               operator*(kexMatrix &matrix);
    kexMatrix               &operator=(const kexMatrix &matrix);
    
    kexVec4                 vectors[4];
};

class kexPluecker {
public:
                            kexPluecker(void);
                            kexPluecker(const kexVec3 &start, const kexVec3 &end, bool bRay = false);

    void                    Clear(void);
    void                    SetLine(const kexVec3 &start, const kexVec3 &end);
    void                    SetRay(const kexVec3 &start, const kexVec3 &dir);
    float                   InnerProduct(const kexPluecker &pluecker) const;

    float                   p[6];
};

class kexPlane {
public:
                            kexPlane(void);
                            kexPlane(const float a, const float b, const float c, const float d);
                            kexPlane(const kexVec3 &pt1, const kexVec3 &pt2, const kexVec3 &pt3);
                            kexPlane(const kexVec3 &normal, const kexVec3 &point);
                            kexPlane(const kexPlane &plane);

    const kexVec3           &Normal(void) const;
    kexVec3                 &Normal(void);
    kexPlane                &SetNormal(const kexVec3 &normal);
    kexPlane                &SetNormal(const kexVec3 &pt1, const kexVec3 &pt2, const kexVec3 &pt3);
    float                   Distance(const kexVec3 &point);
    kexPlane                &SetDistance(const kexVec3 &point);
    bool                    IsFacing(const float yaw);
    float                   ToYaw(void);
    float                   ToPitch(void);
    kexQuat                 ToQuat(void);
    const kexVec4           &ToVec4(void) const;
    kexVec4                 &ToVec4(void);
    kexVec3                 GetInclination(void);

    kexPlane                &operator|(const kexQuat &quat);
    kexPlane                &operator|=(const kexQuat &quat);
    kexPlane                &operator|(const kexMatrix &mtx);
    kexPlane                &operator|=(const kexMatrix &mtx);

    static void             ObjectConstruct(kexPlane *p);
    static void             ObjectConstruct(const float a, const float b, const float c, const float d,
                                            kexPlane *p);
    static void             ObjectConstruct(const kexVec3 &pt1, const kexVec3 &pt2, const kexVec3 &pt3,
                                            kexPlane *p);
    static void             ObjectConstruct(const kexVec3 &normal, const kexVec3 &point, kexPlane *p);
    static void             ObjectConstructCopy(const kexPlane &in, kexPlane *p);

    float                   a;
    float                   b;
    float                   c;
    float                   d;
};

class kexAngle {
public:
                            kexAngle(void);
                            kexAngle(const float yaw, const float pitch, const float roll);
                            kexAngle(const kexVec3 &vector);
                            kexAngle(const kexAngle &an);

    kexAngle                &Round(void);
    kexAngle                &Clamp180(void);
    kexAngle                &Clamp180Invert(void);
    kexAngle                &Clamp180InvertSum(const kexAngle &angle);
    kexAngle                Diff(kexAngle &angle);
    void                    ToAxis(kexVec3 *forward, kexVec3 *up, kexVec3 *right);
    kexVec3                 ToForwardAxis(void);
    kexVec3                 ToUpAxis(void);
    kexVec3                 ToRightAxis(void);
    const kexVec3           &ToVec3(void) const;
    kexVec3                 &ToVec3(void);

    kexAngle                operator+(const kexAngle &angle);
    kexAngle                operator-(const kexAngle &angle);
    kexAngle                &operator+=(const kexAngle &angle);
    kexAngle                &operator-=(const kexAngle &angle);
    kexAngle                &operator=(const kexAngle &angle);
    kexAngle                &operator=(const kexVec3 &vector);
    kexAngle                &operator=(const float *vecs);
    kexAngle                operator-(void);
    float                   operator[](int index) const;
    float                   operator[](int index);

    float                   yaw;
    float                   pitch;
    float                   roll;

    static void             ObjectConstruct1(kexAngle *an);
    static void             ObjectConstruct2(const float a, const float b, const float c, kexAngle *an);
    static void             ObjectConstruct3(const kexVec3 &vec, kexAngle *an);
    static void             ObjectConstructCopy(const kexAngle &in, kexAngle *an);
};

class kexBBox {
public:
                            kexBBox(void);
                            explicit kexBBox(const kexVec3 &vMin, const kexVec3 &vMax);
                        
    void                    Clear(void);
    kexVec3                 Center(void) const;
    float                   Radius(void) const;
    bool                    PointInside(const kexVec3 &vec) const;
    bool                    IntersectingBox(const kexBBox &box) const;
    float                   DistanceToPlane(kexPlane &plane);
    bool                    LineIntersect(const kexVec3 &start, const kexVec3 &end);
    
    kexBBox                 operator+(const float radius) const;
    kexBBox                 operator|(const kexMatrix &matrix) const;
    kexBBox                 &operator|=(const kexMatrix &matrix);
    kexBBox                 &operator=(const kexBBox &bbox);
                        
    kexVec3                 min;
    kexVec3                 max;
};

#endif

