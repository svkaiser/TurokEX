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
// DESCRIPTION: Matrix (left handed) operations
//
// Reference:
// _________________ 
// | 0   4   8   12 |
// | 1   5   9   13 |
// | 2   6  10   14 |
// | 3   7  11   15 |
// _________________
//
// translation
// _________________ 
// | 0   4   8   x  |
// | 1   5   9   y  |
// | 2   6  10   z  |
// | 3   7  11   15 |
// _________________
//
// rotation x
// _________________ 
// |(1)  4   8   x  |
// | 1   xc -xs  y  |
// | 2   xs  xs  z  |
// | 3   7  11  (1) |
// _________________
//
// rotation y
// _________________ 
// | yc  4  ys   12 |
// | 1  (1)  9   13 |
// |-ys  6  yc   14 |
// | 3   7  11  (1) |
// _________________
//
// rotation z
// _________________ 
// | zc -zs  8   12 |
// | zs  zc  9   13 |
// | 2   6  (1)  14 |
// | 3   7  11  (1) |
// _________________
//
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h>

#include "mathlib.h"
#include "gl.h"

//
// Mtx_ViewFrustum
//

void Mtx_ViewFrustum(int width, int height, float fovy, float znear, float zfar)
{
    float left;
    float right;
    float bottom;
    float top;
    float aspect;
    mtx_t m;
    
    aspect = (float)width / (float)height;
    top = znear * kexMath::Tan((float)fovy * M_PI / 360.0f);
    bottom = -top;
    left = bottom * aspect;
    right = top * aspect;
    
    m[ 0] = (2 * znear) / (right - left);
    m[ 4] = 0;
    m[ 8] = (right + left) / (right - left);
    m[12] = 0;
    
    m[ 1] = 0;
    m[ 5] = (2 * znear) / (top - bottom);
    m[ 9] = (top + bottom) / (top - bottom);
    m[13] = 0;
    
    m[ 2] = 0;
    m[ 6] = 0;
    m[10] = -(zfar + znear) / (zfar - znear);
    m[14] = -(2 * zfar * znear) / (zfar - znear);
    
    m[ 3] = 0;
    m[ 7] = 0;
    m[11] = -1;
    m[15] = 0;
    
    dglMultMatrixf(m);
}

//
// Mtx_AddTranslation
//

void Mtx_AddTranslation(mtx_t m, float x, float y, float z)
{
    m[12] = x + m[12];
    m[13] = y + m[13];
    m[14] = z + m[14];
}

//
// Mtx_SetTranslation
//

void Mtx_SetTranslation(mtx_t m, float x, float y, float z)
{
    m[ 0] = 1;
    m[ 1] = 0;
    m[ 2] = 0;
    m[ 3] = 0;
    m[ 4] = 0;
    m[ 5] = 1;
    m[ 6] = 0;
    m[ 7] = 0;
    m[ 8] = 0;
    m[ 9] = 0;
    m[10] = 1;
    m[11] = 0;
    m[12] = x;
    m[13] = y;
    m[14] = z;
    m[15] = 1;
}

//
// Mtx_Scale
//

void Mtx_Scale(mtx_t m, float x, float y, float z)
{
    m[ 0] = x * m[ 0];
    m[ 1] = x * m[ 1];
    m[ 2] = x * m[ 2];
    m[ 4] = y * m[ 4];
    m[ 5] = y * m[ 5];
    m[ 6] = y * m[ 6];
    m[ 8] = z * m[ 8];
    m[ 9] = z * m[ 9];
    m[10] = z * m[10];
}

//
// Mtx_MultiplyByAxis
//

void Mtx_MultiplyByAxis(mtx_t m1, mtx_t m2, float x, float y, float z)
{
    m1[ 0] = m2[ 0] * x;
    m1[ 1] = m2[ 1] * x;
    m1[ 2] = m2[ 2] * x;
    m1[ 3] = 0;
    m1[ 4] = m2[ 4] * y;
    m1[ 5] = m2[ 5] * y;
    m1[ 6] = m2[ 6] * y;
    m1[ 7] = 0;
    m1[ 8] = m2[ 8] * z;
    m1[ 9] = m2[ 9] * z;
    m1[10] = m2[10] * z;
    m1[11] = 0;
    m1[12] = m2[12];
    m1[13] = m2[13];
    m1[14] = m2[14];
    m1[15] = 1;
}

//
// Mtx_Transpose
//

void Mtx_Transpose(mtx_t m)
{
    float m4;
    float m5;
    float m6;
    float m8;
    float m9;
    float m10;

    m4  = m[ 4];
    m5  = m[ 5];
    m6  = m[ 6];
    m8  = m[ 8];
    m9  = m[ 9];
    m10 = m[10];

    m[ 8] = m4;
    m[ 4] = m8;
    m[ 9] = m5;
    m[ 5] = m9;
    m[ 6] = m10;
    m[10] = m6;
}

//
// Mtx_TransposeDup
//

void Mtx_TransposeDup(mtx_t m1, mtx_t m2)
{
    m1[ 0] = m2[ 0];
    m1[ 1] = m2[ 1];
    m1[ 2] = m2[ 2];
    m1[ 3] = 0;
    m1[ 4] = m2[ 8];
    m1[ 5] = m2[ 9];
    m1[ 6] = m2[10];
    m1[ 7] = 0;
    m1[ 8] = m2[ 4];
    m1[ 9] = m2[ 5];
    m1[10] = m2[ 6];
    m1[11] = 0;
    m1[12] = m2[12];
    m1[13] = m2[13];
    m1[14] = m2[14];
    m1[15] = 1;
}

//
// Mtx_Identity
//

void Mtx_Identity(mtx_t m)
{
    m[ 0] = 1;
    m[ 1] = 0;
    m[ 2] = 0;
    m[ 3] = 0;
    m[ 4] = 0;
    m[ 5] = 1;
    m[ 6] = 0;
    m[ 7] = 0;
    m[ 8] = 0;
    m[ 9] = 0;
    m[10] = 1;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

//
// Mtx_IdentityAxis
//

void Mtx_IdentityAxis(mtx_t m, float x, float y, float z)
{
    m[ 0] = x;
    m[ 1] = 0;
    m[ 2] = 0;
    m[ 3] = 0;
    m[ 4] = 0;
    m[ 5] = y;
    m[ 6] = 0;
    m[ 7] = 0;
    m[ 8] = 0;
    m[ 9] = 0;
    m[10] = z;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1; 
}

//
// Mtx_IdentityX
//

void Mtx_IdentityX(mtx_t m, float angle)
{
    float s;
    float c;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);

    m[ 0] = c;
    m[ 1] = 0;
    m[ 2] = -s;
    m[ 3] = 0;
    m[ 4] = 0;
    m[ 5] = 1;
    m[ 6] = 0;
    m[ 7] = 0;
    m[ 8] = s;
    m[ 9] = 0;
    m[10] = c;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

//
// Mtx_IdentityY
//

void Mtx_IdentityY(mtx_t m, float angle)
{
    float s;
    float c;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);

    m[ 0] = 1;
    m[ 1] = 0;
    m[ 2] = 0;
    m[ 3] = 0;
    m[ 4] = 0;
    m[ 5] = c;
    m[ 6] = s;
    m[ 7] = 0;
    m[ 8] = 0;
    m[ 9] = -s;
    m[10] = c;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

//
// Mtx_IdentityZ
//

void Mtx_IdentityZ(mtx_t m, float angle)
{
    float s;
    float c;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);

    m[ 0] = c;
    m[ 1] = s;
    m[ 2] = 0;
    m[ 3] = 0;
    m[ 4] = -s;
    m[ 5] = c;
    m[ 6] = 0;
    m[ 7] = 0;
    m[ 8] = 0;
    m[ 9] = 0;
    m[10] = 1;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
}

//
// Mtx_Multiply
//

void Mtx_Multiply(mtx_t out, mtx_t m1, mtx_t m2)
{
    out[ 0] = m1[ 1] * m2[ 4] + m2[ 8] * m1[ 2] + m1[ 3] * m2[12] + m1[ 0] * m2[ 0];
    out[ 1] = m1[ 0] * m2[ 1] + m2[ 5] * m1[ 1] + m2[ 9] * m1[ 2] + m2[13] * m1[ 3];
    out[ 2] = m1[ 0] * m2[ 2] + m2[10] * m1[ 2] + m2[14] * m1[ 3] + m2[ 6] * m1[ 1];
    out[ 3] = m1[ 0] * m2[ 3] + m2[15] * m1[ 3] + m2[ 7] * m1[ 1] + m2[11] * m1[ 2];
    out[ 4] = m2[ 0] * m1[ 4] + m1[ 7] * m2[12] + m1[ 5] * m2[ 4] + m1[ 6] * m2[ 8];
    out[ 5] = m1[ 4] * m2[ 1] + m1[ 5] * m2[ 5] + m1[ 7] * m2[13] + m1[ 6] * m2[ 9];
    out[ 6] = m1[ 5] * m2[ 6] + m1[ 7] * m2[14] + m1[ 4] * m2[ 2] + m1[ 6] * m2[10];
    out[ 7] = m1[ 6] * m2[11] + m1[ 7] * m2[15] + m1[ 5] * m2[ 7] + m1[ 4] * m2[ 3];
    out[ 8] = m2[ 0] * m1[ 8] + m1[10] * m2[ 8] + m1[11] * m2[12] + m1[ 9] * m2[ 4];
    out[ 9] = m1[ 8] * m2[ 1] + m1[10] * m2[ 9] + m1[11] * m2[13] + m1[ 9] * m2[ 5];
    out[10] = m1[ 9] * m2[ 6] + m1[10] * m2[10] + m1[11] * m2[14] + m1[ 8] * m2[ 2];
    out[11] = m1[ 9] * m2[ 7] + m1[11] * m2[15] + m1[10] * m2[11] + m1[ 8] * m2[ 3];
    out[12] = m2[ 0] * m1[12] + m2[12] * m1[15] + m2[ 4] * m1[13] + m2[ 8] * m1[14];
    out[13] = m2[13] * m1[15] + m2[ 1] * m1[12] + m2[ 9] * m1[14] + m2[ 5] * m1[13];
    out[14] = m2[ 6] * m1[13] + m2[14] * m1[15] + m2[10] * m1[14] + m2[ 2] * m1[12];
    out[15] = m2[ 3] * m1[12] + m2[ 7] * m1[13] + m2[11] * m1[14] + m2[15] * m1[15];
}

//
// Mtx_MultiplyRotation
//

void Mtx_MultiplyRotation(mtx_t out, mtx_t m1, mtx_t m2)
{
    out[ 0] = m2[ 4] * m1[ 1] + m1[ 2] * m2[ 8] + m2[ 0] * m1[ 0];
    out[ 1] = m1[ 0] * m2[ 1] + m2[ 9] * m1[ 2] + m2[ 5] * m1[ 1];
    out[ 2] = m1[ 0] * m2[ 2] + m1[ 1] * m2[ 6] + m1[ 2] * m2[10];
    out[ 3] = 0;
    out[ 4] = m2[ 0] * m1[ 4] + m2[ 4] * m1[ 5] + m1[ 6] * m2[ 8];
    out[ 5] = m2[ 5] * m1[ 5] + m1[ 6] * m2[ 9] + m1[ 4] * m2[ 1];
    out[ 6] = m1[ 5] * m2[ 6] + m1[ 6] * m2[10] + m1[ 4] * m2[ 2];
    out[ 7] = 0;
    out[ 8] = m2[ 0] * m1[ 8] + m1[10] * m2[ 8] + m1[ 9] * m2[ 4];
    out[ 9] = m1[ 8] * m2[ 1] + m1[ 9] * m2[ 5] + m1[10] * m2[ 9];
    out[10] = m1[ 8] * m2[ 2] + m1[ 9] * m2[ 6] + m1[10] * m2[10];
    out[11] = 0;
    out[12] = m2[ 0] * m1[12] + m1[14] * m2[ 8] + m1[13] * m2[ 4] + m2[12];
    out[13] = m1[13] * m2[ 5] + m1[14] * m2[ 9] + m1[12] * m2[ 1] + m2[13];
    out[14] = m1[12] * m2[ 2] + m1[14] * m2[10] + m1[13] * m2[ 6] + m2[14];
    out[15] = 1;
}

//
// Mtx_Invert
//

void Mtx_Invert(mtx_t out, mtx_t in)
{
    float d;

    d = in[ 0] * in[10] * in[ 5] -
        in[ 0] * in[ 9] * in[ 6] -
        in[ 1] * in[ 4] * in[10] +
        in[ 2] * in[ 4] * in[ 9] +
        in[ 1] * in[ 6] * in[ 8] -
        in[ 2] * in[ 5] * in[ 8];

    if(d != 0.0f)
    {
        float d2 = (1.0f / d);

        out[ 0] = (  in[10] * in[ 5] - in[ 9] * in[ 6]) * d2;
        out[ 1] = -((in[ 1] * in[10] - in[ 2] * in[ 9]) * d2);
        out[ 2] = (  in[ 1] * in[ 6] - in[ 2] * in[ 5]) * d2;
        out[ 3] = 0;
        out[ 4] = (  in[ 6] * in[ 8] - in[ 4] * in[10]) * d2;
        out[ 5] = (  in[ 0] * in[10] - in[ 2] * in[ 8]) * d2;
        out[ 6] = -((in[ 0] * in[ 6] - in[ 2] * in[ 4]) * d2);
        out[ 7] = 0;
        out[ 8] = -((in[ 5] * in[ 8] - in[ 4] * in[ 9]) * d2);
        out[ 9] = (  in[ 1] * in[ 8] - in[ 0] * in[ 9]) * d2;
        out[10] = -((in[ 1] * in[ 4] - in[ 0] * in[ 5]) * d2);
        out[11] = 0;
        out[12] = (
            ( in[13] * in[10] - in[14] * in[ 9]) * in[ 4]
            + in[14] * in[ 5] * in[ 8]
            - in[13] * in[ 6] * in[ 8]
            - in[12] * in[10] * in[ 5]
            + in[12] * in[ 9] * in[ 6]) * d2;
        out[13] = (
              in[ 0] * in[14] * in[ 9]
            - in[ 0] * in[13] * in[10]
            - in[14] * in[ 1] * in[ 8]
            + in[13] * in[ 2] * in[ 8]
            + in[12] * in[ 1] * in[10]
            - in[12] * in[ 2] * in[ 9]) * d2;
        out[14] = -(
            ( in[ 0] * in[14] * in[ 5]
            - in[ 0] * in[13] * in[ 6]
            - in[14] * in[ 1] * in[ 4]
            + in[13] * in[ 2] * in[ 4]
            + in[12] * in[ 1] * in[ 6]
            - in[12] * in[ 2] * in[ 5]) * d2);
        out[15] = 1.0f;
    }
    else
    {
        Mtx_Copy(out, in);
    }
}

//
// Mtx_ApplyVector
//

void Mtx_ApplyVector(mtx_t m, vec3_t vec)
{
    m[12] = m[ 0] * vec[0] + m[ 4] * vec[1] + m[ 8] * vec[2] + m[12];
    m[13] = m[ 1] * vec[0] + m[ 5] * vec[1] + m[ 9] * vec[2] + m[13];
    m[14] = m[ 2] * vec[0] + m[ 6] * vec[1] + m[10] * vec[2] + m[14];
}

//
// Mtx_ApplyCoordinates
//

void Mtx_ApplyCoordinates(mtx_t m, vec3_t src, vec3_t out)
{
    float w;

    Vec_TransformToWorld(m, src, out);

    w = m[11] * src[2] + m[ 8] * src[1] + m[ 3] * src[0] + m[15];

    if(w != 0.0f)
    {
        w = 1.0f / w;

        out[0] = out[0] * w;
        out[1] = out[1] * w;
        out[2] = out[2] * w;
    }
}

//
// Mtx_SetFromAxis
//

void Mtx_SetFromAxis(mtx_t m, float angle, float x, float y, float z)
{
    float s    = kexMath::Sin(angle);
    float c    = kexMath::Cos(angle);
    float xx   = x * x;
    float yy   = y * y;
    float zz   = z * z;
    float sx   = s * x;
    float sy   = s * y;
    float sz   = s * z;
    float cyz  = ((1.0f - c) * y * z);
    float cyx  = ((1.0f - c) * y * x);

    m[ 0] = (1.0f - xx) * c + xx;
    m[ 1] = cyx + sz;
    m[ 2] = cyz - sy;
    m[ 3] = 0;
    m[ 4] = cyx - sz;
    m[ 5] = (1.0f - yy) * c + yy;
    m[ 6] = cyz + sx;
    m[ 7] = 0;
    m[ 8] = cyz + sy;
    m[ 9] = cyz - sx;
    m[10] = (1.0f - zz) * c + zz;
    m[11] = 0;
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1.0f;
}

//
// Mtx_Copy
//

void Mtx_Copy(mtx_t dest, mtx_t src)
{
    memcpy(dest, src, sizeof(mtx_t));
}

//
// Mtx_RotateX
//

void Mtx_RotateX(mtx_t m, float angle)
{
    float s;
    float c;
    float tm0;
    float tm8;
    float tm4;
    float tm12;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);

    tm0     = m[ 0];
    tm4     = m[ 4];
    tm8     = m[ 8];
    tm12    = m[12];

    m[ 0] = s * m[ 2] + tm0  * c;
    m[ 2] = c * m[ 2] - tm0  * s;
    m[ 4] = s * m[ 6] + tm4  * c;
    m[ 6] = c * m[ 6] - tm4  * s;
    m[ 8] = s * m[10] + tm8  * c;
    m[10] = c * m[10] - tm8  * s;
    m[12] = s * m[14] + tm12 * c;
    m[14] = c * m[14] - tm12 * s;
}

//
// Mtx_RotateY
//

void Mtx_RotateY(mtx_t m, float angle)
{
    float s;
    float c;
    float tm0;
    float tm1;
    float tm2;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);

    tm0     = m[ 0];
    tm1     = m[ 1];
    tm2     = m[ 2];

    m[ 0] = tm0 * c - s * m[ 8];
    m[ 8] = c * m[ 8] + tm0 * s;
    m[ 1] = tm1 * c - s * m[ 9];
    m[ 9] = c * m[ 9] + tm1 * s;
    m[ 2] = tm2 * c - s * m[10];
    m[10] = c * m[10] + tm2 * s;
}

//
// Mtx_RotateZ
//

void Mtx_RotateZ(mtx_t m, float angle)
{
    float s;
    float c;
    float tm1;
    float tm5;
    float tm9;
    float tm13;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);

    tm1     = m[ 1];
    tm5     = m[ 5];
    tm9     = m[ 9];
    tm13    = m[13];

    m[ 1] = tm1  * c - s * m[ 2];
    m[ 2] = c * m[ 2] + tm1  * s;
    m[ 5] = tm5  * c - s * m[ 6];
    m[ 6] = c * m[ 6] + tm5  * s;
    m[ 9] = tm9  * c - s * m[10];
    m[10] = c * m[10] + tm9  * s;
    m[13] = tm13 * c - s * m[14];
    m[14] = c * m[14] + tm13 * s;
}

//
// Mtx_ApplyRotation
//

void Mtx_ApplyRotation(vec4_t rot, mtx_t out)
{
    float xx = rot[0] * rot[0];
    float yx = rot[1] * rot[0];
    float zx = rot[2] * rot[0];
    float wx = rot[3] * rot[0];
    float yy = rot[1] * rot[1];
    float zy = rot[2] * rot[1];
    float wy = rot[3] * rot[1];
    float zz = rot[2] * rot[2];
    float wz = rot[3] * rot[2];
    float ww = rot[3] * rot[3];

    out[ 0] = ((ww + xx) - yy) - zz;
    out[ 1] = (wz + wz) + (yx + yx);
    out[ 2] = (zx + zx) - (wy + wy);
    out[ 3] = 0;
    out[ 4] = (yx + yx) - (wz + wz);
    out[ 5] = (yy + (ww - xx)) - zz;
    out[ 6] = (wx + wx) + (zy + zy);
    out[ 7] = 0;
    out[ 8] = (wy + wy + zx + zx);
    out[ 9] = (zy + zy) - (wx + wx);
    out[10] = ((ww - xx) - yy) + zz;
    out[11] = 0;
    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    out[15] = 1;
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(void) {
    Identity();
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const kexMatrix &mtx) {
    for(int i = 0; i < 4; i++) {
        vectors[i].x = mtx.vectors[i].x;
        vectors[i].y = mtx.vectors[i].y;
        vectors[i].z = mtx.vectors[i].z;
        vectors[i].w = mtx.vectors[i].w;
    }
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const float x, const float y, const float z) {
    Identity(x, y, z);
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const kexQuat &quat) {
    float xx = quat.x * quat.x;
    float yx = quat.y * quat.x;
    float zx = quat.z * quat.x;
    float wx = quat.w * quat.x;
    float yy = quat.y * quat.y;
    float zy = quat.z * quat.y;
    float wy = quat.w * quat.y;
    float zz = quat.z * quat.z;
    float wz = quat.w * quat.z;
    float ww = quat.w * quat.w;

    vectors[0].Set(
        ((ww + xx) - yy) - zz,
        (wz + wz) + (yx + yx),
        (zx + zx) - (wy + wy),
        0);
    vectors[1].Set(
        (yx + yx) - (wz + wz),
        (yy + (ww - xx)) - zz,
        (wx + wx) + (zy + zy),
        0);
    vectors[2].Set(
        (wy + wy + zx + zx),
        (zy + zy) - (wx + wx),
        ((ww - xx) - yy) + zz,
        0);
    vectors[3].Set(0, 0, 0, 1);
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const float angle, const int axis) {
    float s;
    float c;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);
    
    Identity();

    switch(axis) {
    case 0:
        this->vectors[0].x = c;
        this->vectors[0].z = -s;
        this->vectors[3].x = s;
        this->vectors[3].z = c;
        break;
    case 1:
        this->vectors[1].y = c;
        this->vectors[1].z = s;
        this->vectors[2].y = -s;
        this->vectors[2].z = c;
        break;
    case 2:
        this->vectors[0].x = c;
        this->vectors[0].y = s;
        this->vectors[1].x = -s;
        this->vectors[1].y = c;
        break;
    }
}

//
// kexMatrix::Identity
//

kexMatrix &kexMatrix::Identity(void) {
    vectors[0].Set(1, 0, 0, 0);
    vectors[1].Set(0, 1, 0, 0);
    vectors[2].Set(0, 0, 1, 0);
    vectors[3].Set(0, 0, 0, 1);

    return *this;
}

//
// kexMatrix::Identity
//

kexMatrix &kexMatrix::Identity(const float x, const float y, const float z) {
    vectors[0].Set(x, 0, 0, 0);
    vectors[1].Set(0, y, 0, 0);
    vectors[2].Set(0, 0, z, 0);
    vectors[3].Set(0, 0, 0, 1);

    return *this;
}

//
// kexMatrix::SetTranslation
//

kexMatrix &kexMatrix::SetTranslation(const float x, const float y, const float z) {
    vectors[3].ToVec3().Set(x, y, z);
    return *this;
}

//
// kexMatrix::SetTranslation
//

kexMatrix &kexMatrix::SetTranslation(const kexVec3 &vector) {
    vectors[3].ToVec3() = vector;
    return *this;
}

//
// kexMatrix::AddTranslation
//

kexMatrix &kexMatrix::AddTranslation(const float x, const float y, const float z) {
    vectors[3].x += x;
    vectors[3].y += y;
    vectors[3].z += z;
    return *this;
}

//
// kexMatrix::AddTranslation
//

kexMatrix &kexMatrix::AddTranslation(const kexVec3 &vector) {
    vectors[3].ToVec3() += vector;
    return *this;
}

//
// kexMatrix::Scale
//

kexMatrix &kexMatrix::Scale(const float x, const float y, const float z) {
    for(int i = 0; i < 3; i++) {
        vectors[i].x *= x;
        vectors[i].y *= y;
        vectors[i].z *= z;
    }

    return *this;
}

//
// kexMatrix::Scale
//

kexMatrix &kexMatrix::Scale(const kexVec3 &vector) {
    for(int i = 0; i < 3; i++)
        vectors[i].ToVec3() *= vector;

    return *this;
}

//
// kexMatrix::Scale
//

kexMatrix kexMatrix::Scale(const kexMatrix &mtx, const float x, const float y, const float z) {
    kexMatrix out;
    for(int i = 0; i < 3; i++) {
        out.vectors[i].x = mtx.vectors[i].x * x;
        out.vectors[i].y = mtx.vectors[i].y * y;
        out.vectors[i].z = mtx.vectors[i].z * z;
    }

    return out;
}

//
// kexMatrix::Transpose
//

kexMatrix &kexMatrix::Transpose(void) {
    kexVec3 v1 = vectors[1].ToVec3();
    kexVec3 v2 = vectors[2].ToVec3();
    
    vectors[1].ToVec3() = v2;
    vectors[2].ToVec3() = v1;
    return *this;
}

//
// kexMatrix::Transpose
//

kexMatrix kexMatrix::Transpose(const kexMatrix &mtx) {
    kexMatrix out;
    
    out.vectors[0].ToVec3() = mtx.vectors[0].ToVec3();
    out.vectors[1].ToVec3() = mtx.vectors[2].ToVec3();
    out.vectors[2].ToVec3() = mtx.vectors[1].ToVec3();
    out.vectors[3].ToVec3() = mtx.vectors[3].ToVec3();

    return out;
}

//
// kexMatrix::SetViewProjection
//

void kexMatrix::SetViewProjection(float aspect, float fov, float zNear, float zFar) {
    float top       = zNear * kexMath::Tan(fov * M_PI / 360.0f);
    float bottom    = -top;
    float left      = bottom * aspect;
    float right     = top * aspect;
    
    vectors[0].x =  (2 * zNear) / (right - left);
    vectors[1].y =  (2 * zNear) / (top - bottom);
    vectors[3].z = -(2 * zFar * zNear) / (zFar - zNear);

    vectors[2].x =  (right + left) / (right - left);
    vectors[2].y =  (top + bottom) / (top - bottom);
    vectors[2].z = -(zFar + zNear) / (zFar - zNear);

    vectors[0].y = 0;
    vectors[0].z = 0;
    vectors[0].w = 0;
    vectors[1].x = 0;
    vectors[1].w = 0;
    vectors[1].z = 0;
    vectors[2].w = -1;
    vectors[3].x = 0;
    vectors[3].y = 0;
    vectors[3].w = 0;
}

//
// kexMatrix::operator*
//

kexMatrix kexMatrix::operator*(const kexVec3 &vector) {
    kexMatrix out(*this);
    
    out.vectors[3].ToVec3() +=
        vectors[0].ToVec3() * vector.x +
        vectors[1].ToVec3() * vector.y +
        vectors[2].ToVec3() * vector.z;
    return out;
}

//
// kexMatrix::operator*=
//

kexMatrix &kexMatrix::operator*=(const kexVec3 &vector) {
    vectors[3].ToVec3() +=
        vectors[0].ToVec3() * vector.x +
        vectors[1].ToVec3() * vector.y +
        vectors[2].ToVec3() * vector.z;
    return *this;
}

//
// kexMatrix::ToFloatPtr
//

float *kexMatrix::ToFloatPtr(void) {
    return reinterpret_cast<float*>(vectors);
}

//
// kexMatrix::operator*
//

kexMatrix kexMatrix::operator*(kexMatrix &matrix) {
    kexMatrix out;
    float *m1 = ToFloatPtr();
    float *m2 = matrix.ToFloatPtr();
    float *mOut = out.ToFloatPtr();
    
    mOut[ 0] = m1[ 1] * m2[ 4] + m2[ 8] * m1[ 2] + m1[ 3] * m2[12] + m1[ 0] * m2[ 0];
    mOut[ 1] = m1[ 0] * m2[ 1] + m2[ 5] * m1[ 1] + m2[ 9] * m1[ 2] + m2[13] * m1[ 3];
    mOut[ 2] = m1[ 0] * m2[ 2] + m2[10] * m1[ 2] + m2[14] * m1[ 3] + m2[ 6] * m1[ 1];
    mOut[ 3] = m1[ 0] * m2[ 3] + m2[15] * m1[ 3] + m2[ 7] * m1[ 1] + m2[11] * m1[ 2];
    mOut[ 4] = m2[ 0] * m1[ 4] + m1[ 7] * m2[12] + m1[ 5] * m2[ 4] + m1[ 6] * m2[ 8];
    mOut[ 5] = m1[ 4] * m2[ 1] + m1[ 5] * m2[ 5] + m1[ 7] * m2[13] + m1[ 6] * m2[ 9];
    mOut[ 6] = m1[ 5] * m2[ 6] + m1[ 7] * m2[14] + m1[ 4] * m2[ 2] + m1[ 6] * m2[10];
    mOut[ 7] = m1[ 6] * m2[11] + m1[ 7] * m2[15] + m1[ 5] * m2[ 7] + m1[ 4] * m2[ 3];
    mOut[ 8] = m2[ 0] * m1[ 8] + m1[10] * m2[ 8] + m1[11] * m2[12] + m1[ 9] * m2[ 4];
    mOut[ 9] = m1[ 8] * m2[ 1] + m1[10] * m2[ 9] + m1[11] * m2[13] + m1[ 9] * m2[ 5];
    mOut[10] = m1[ 9] * m2[ 6] + m1[10] * m2[10] + m1[11] * m2[14] + m1[ 8] * m2[ 2];
    mOut[11] = m1[ 9] * m2[ 7] + m1[11] * m2[15] + m1[10] * m2[11] + m1[ 8] * m2[ 3];
    mOut[12] = m2[ 0] * m1[12] + m2[12] * m1[15] + m2[ 4] * m1[13] + m2[ 8] * m1[14];
    mOut[13] = m2[13] * m1[15] + m2[ 1] * m1[12] + m2[ 9] * m1[14] + m2[ 5] * m1[13];
    mOut[14] = m2[ 6] * m1[13] + m2[14] * m1[15] + m2[10] * m1[14] + m2[ 2] * m1[12];
    mOut[15] = m2[ 3] * m1[12] + m2[ 7] * m1[13] + m2[11] * m1[14] + m2[15] * m1[15];
    
    return out;
}

//
// kexMatrix::operator=
//

kexMatrix &kexMatrix::operator=(const kexMatrix &matrix) {
    vectors[0] = matrix.vectors[0];
    vectors[1] = matrix.vectors[1];
    vectors[2] = matrix.vectors[2];
    vectors[3] = matrix.vectors[3];
    
    return *this;
}

