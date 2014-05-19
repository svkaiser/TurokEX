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
//
// DESCRIPTION: Math functions
//
//-----------------------------------------------------------------------------

#ifndef EDITOR
#include "common.h"
#endif
#include "mathlib.h"
#ifndef EDITOR
#include "scriptAPI/scriptSystem.h"
#endif

//
// kexMath::Abs
//

int kexMath::Abs(int x) {
    int y = x >> 31;
    return ((x ^ y) - y);
}

//
// kexMath::Fabs
//

float kexMath::Fabs(float x) {
    int tmp = *reinterpret_cast<int*>(&x);
    tmp &= 0x7FFFFFFF;
    return *reinterpret_cast<float*>(&tmp);
}

//
// kexMath::RoundPowerOfTwo
//

int kexMath::RoundPowerOfTwo(int x) {
    int mask = 1;
    
    while(mask < 0x40000000) {
        if(x == mask || (x & (mask-1)) == x) {
            return mask;
        }
        
        mask <<= 1;
    }
    
    return x;
}

//
// kexMath::InvSqrt
//

float kexMath::InvSqrt(float x) {
    long i;
    float r;
    float y;
    
    y = x * 0.5f;
    i = *reinterpret_cast<long*>(&x);
    i = 0x5f3759df - (i >> 1);
    r = *reinterpret_cast<float*>(&i);
    r = r * (1.5f - r * r * y);
    
    return r;
}

//
// kexMath::Clamp
//

void kexMath::Clamp(float &f, const float min, const float max) {
    if(f < min) f = min;
    if(f > max) f = max;
}

//
// kexMath::CubicCurve
//

void kexMath::CubicCurve(const kexVec3 &start, const kexVec3 &end, const float time,
                         const kexVec3 &point, kexVec3 *vec) {
    int i;
    float xyz[3];

    for(i = 0; i < 3; i++) {
        xyz[i] = kexMath::Pow(1-time, 2) * start[i] +
            (2 * (1-time)) * time * point[i] + kexMath::Pow(time, 2) * end[i];
    }

    vec->x = xyz[0];
    vec->y = xyz[1];
    vec->z = xyz[2];
}

//
// kexMath::QuadraticCurve
//

void kexMath::QuadraticCurve(const kexVec3 &start, const kexVec3 &end, const float time,
                             const kexVec3 &pt1, const kexVec3 &pt2, kexVec3 *vec) {
    int i;
    float xyz[3];

    for(i = 0; i < 3; i++) {
        xyz[i] = kexMath::Pow(1-time, 3) * start[i] + (3 * kexMath::Pow(1-time, 2)) *
            time * pt1[i] + (3 * (1-time)) * kexMath::Pow(time, 2) * pt2[i] +
            kexMath::Pow(time, 3) * end[i];
    }

    vec->x = xyz[0];
    vec->y = xyz[1];
    vec->z = xyz[2];
}

#ifndef EDITOR
//
// kexMath::InitObject
//

void kexMath::InitObject(void) {
    scriptManager.Engine()->SetDefaultNamespace("Math");
    scriptManager.Engine()->RegisterGlobalFunction("float Sin(float)",
        asFUNCTION(kexMath::Sin), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Cos(float)",
        asFUNCTION(kexMath::Cos), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Tan(float)",
        asFUNCTION(kexMath::Tan), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float ATan2(float)",
        asFUNCTION(kexMath::ATan2), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Fabs(float)",
        asFUNCTION(kexMath::Fabs), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Acos(float)",
        asFUNCTION(kexMath::ACos), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Sqrt(float)",
        asFUNCTION(kexMath::Sqrt), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("int Abs(int)",
        asFUNCTION(kexMath::Abs), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Ceil(float)",
        asFUNCTION(kexMath::Ceil), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Floor(float)",
        asFUNCTION(kexMath::Floor), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Log(float)",
        asFUNCTION(kexMath::Log), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Pow(float, float)",
        asFUNCTION(kexMath::Pow), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Deg2Rad(float)",
        asFUNCTION(kexMath::Deg2Rad), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Rad2Deg(float)",
        asFUNCTION(kexMath::Rad2Deg), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float InvSqrt(float)",
        asFUNCTION(kexMath::InvSqrt), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("int SysRand(void)",
        asFUNCTION(kexRand::SysRand), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("int Rand(void)",
        asFUNCTION(kexRand::Int), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("int RandMax(const int)",
        asFUNCTION(kexRand::Max), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float RandFloat(void)",
        asFUNCTION(kexRand::Float), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float RandCFloat(void)",
        asFUNCTION(kexRand::CFloat), asCALL_CDECL);
    scriptManager.Engine()->RegisterGlobalFunction("float Range(const float, const float)",
        asFUNCTION(kexRand::Range), asCALL_CDECL);
    scriptManager.Engine()->SetDefaultNamespace("");

    //
    // vectors
    //
    scriptManager.Engine()->RegisterObjectType("kVec3", sizeof(kexVec3),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
    scriptManager.Engine()->RegisterObjectBehaviour("kVec3", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(kexVec3::ObjectConstruct1), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kVec3", asBEHAVE_CONSTRUCT,
        "void f(float, float, float)", asFUNCTION(kexVec3::ObjectConstruct2), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kVec3", asBEHAVE_CONSTRUCT,
        "void f(const kVec3 &in)", asFUNCTION(kexVec3::ObjectConstructCopy), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 &Normalize(void)",
        asMETHODPR(kexVec3, Normalize, (void), kexVec3&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "float Dot(const kVec3 &in) const",
        asMETHODPR(kexVec3, Dot, (const kexVec3 &vec)const, float), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "float Unit(void) const",
        asMETHODPR(kexVec3, Unit, (void) const, float), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "float Distance(const kVec3 &in) const",
        asMETHODPR(kexVec3, Distance, (const kexVec3 &vec) const, float), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kStr ToString(void)",
        asMETHODPR(kexVec3, ToString, (void)const, kexStr), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "float ToYaw(void)",
        asMETHODPR(kexVec3, ToYaw, (void)const, float), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "float ToPitch(void)",
        asMETHODPR(kexVec3, ToPitch, (void)const, float), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "void Clear(void)",
        asMETHODPR(kexVec3, Clear, (void), void), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "void Set(const float, const float, const float)",
        asMETHODPR(kexVec3, Set, (const float, const float, const float), void), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 Lerp(const kVec3 &in, const float) const",
        asMETHODPR(kexVec3, Lerp, (const kexVec3 &next, const float movement) const, kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 &Lerp(const kVec3 &in, const float)",
        asMETHODPR(kexVec3, Lerp, (const kexVec3 &next, const float movement), kexVec3&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opAdd(const kVec3 &in)",
        asMETHODPR(kexVec3, operator+, (const kexVec3&), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opNeg(void)",
        asMETHODPR(kexVec3, operator-, (void)const, kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opSub(const kVec3 &in)",
        asMETHODPR(kexVec3, operator-, (const kexVec3&)const, kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opMul(const kVec3 &in)",
        asMETHODPR(kexVec3, operator*, (const kexVec3&), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opMul(const float val)",
        asMETHODPR(kexVec3, operator*, (const float), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opDiv(const kVec3 &in)",
        asMETHODPR(kexVec3, operator/, (const kexVec3&), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opDiv(const float val)",
        asMETHODPR(kexVec3, operator/, (const float), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 &opAssign(const kVec3 &in)",
        asMETHODPR(kexVec3, operator=, (const kexVec3&), kexVec3&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "float opIndex(uint)const",
        asMETHODPR(kexVec3, operator[], (int index)const, float), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "float opIndex(uint)",
        asMETHODPR(kexVec3, operator[], (int index), float&), asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty("kVec3", "float x", asOFFSET(kexVec3, x));
    scriptManager.Engine()->RegisterObjectProperty("kVec3", "float y", asOFFSET(kexVec3, y));
    scriptManager.Engine()->RegisterObjectProperty("kVec3", "float z", asOFFSET(kexVec3, z));

    //
    // quaternions
    //
    scriptManager.Engine()->RegisterObjectType("kQuat", sizeof(kexQuat),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
    scriptManager.Engine()->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(kexQuat::ObjectConstruct1), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT,
        "void f(float, float, float, float)", asFUNCTION(kexQuat::ObjectConstruct2), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT,
        "void f(float, kVec3 &in)", asFUNCTION(kexQuat::ObjectConstruct3), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT,
        "void f(const kQuat &in)", asFUNCTION(kexQuat::ObjectConstructCopy), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectMethod("kQuat", "kQuat &Normalize(void)",
        asMETHODPR(kexQuat, Normalize, (void), kexQuat&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kQuat",
        "kQuat RotateFrom(const kVec3 &in, const kVec3 &in, float)",
        asMETHODPR(kexQuat, RotateFrom, (const kexVec3 &location, const kexVec3 &target, float maxAngle), kexQuat),
        asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kQuat", "kQuat opAdd(const kQuat &in)",
        asMETHODPR(kexQuat, operator+, (const kexQuat &in), kexQuat), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kQuat", "kQuat opSub(const kQuat &in)",
        asMETHODPR(kexQuat, operator-, (const kexQuat &in), kexQuat), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kQuat", "kQuat opMul(const kQuat &in)",
        asMETHODPR(kexQuat, operator*, (const kexQuat &in), kexQuat), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kQuat", "kQuat &opAssign(const kQuat &in)",
        asMETHODPR(kexQuat, operator=, (const kexQuat&), kexQuat&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 opMul(const kQuat &in)",
        asMETHODPR(kexVec3, operator*, (const kexQuat&), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kVec3 &opMulAssign(const kQuat &in)",
        asMETHODPR(kexVec3, operator*=, (const kexQuat&), kexVec3&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kVec3", "kQuat ToQuaternion(void)",
        asMETHODPR(kexVec3, ToQuat, (void), kexQuat), asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float x", asOFFSET(kexQuat, x));
    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float y", asOFFSET(kexQuat, y));
    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float z", asOFFSET(kexQuat, z));
    scriptManager.Engine()->RegisterObjectProperty("kQuat", "float w", asOFFSET(kexQuat, w));

    //
    // angles
    //
    scriptManager.Engine()->RegisterObjectType("kAngle", sizeof(kexAngle),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
    scriptManager.Engine()->RegisterObjectBehaviour("kAngle", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(kexAngle::ObjectConstruct1), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kAngle", asBEHAVE_CONSTRUCT,
        "void f(float, float, float)", asFUNCTION(kexAngle::ObjectConstruct2), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kAngle", asBEHAVE_CONSTRUCT,
        "void f(const kVec3 &in)", asFUNCTION(kexAngle::ObjectConstruct3), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectBehaviour("kAngle", asBEHAVE_CONSTRUCT,
        "void f(const kAngle &in)", asFUNCTION(kexAngle::ObjectConstructCopy), asCALL_CDECL_OBJLAST);
    scriptManager.Engine()->RegisterObjectMethod("kAngle", "kAngle &Clamp180(void)",
        asMETHODPR(kexAngle, Clamp180, (void), kexAngle&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kAngle", "kAngle &Clamp180Invert(void)",
        asMETHODPR(kexAngle, Clamp180Invert, (void), kexAngle&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kAngle", "kAngle &Clamp180InvertSum(const kAngle &in)",
        asMETHODPR(kexAngle, Clamp180InvertSum, (const kexAngle&), kexAngle&), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kAngle", "kVec3 ToForwardAxis(void)",
        asMETHODPR(kexAngle, ToForwardAxis, (void), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kAngle", "kVec3 ToUpAxis(void)",
        asMETHODPR(kexAngle, ToUpAxis, (void), kexVec3), asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod("kAngle", "kVec3 ToRightAxis(void)",
        asMETHODPR(kexAngle, ToRightAxis, (void), kexVec3), asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty("kAngle", "float yaw", asOFFSET(kexAngle, yaw));
    scriptManager.Engine()->RegisterObjectProperty("kAngle", "float pitch", asOFFSET(kexAngle, pitch));
    scriptManager.Engine()->RegisterObjectProperty("kAngle", "float roll", asOFFSET(kexAngle, roll));
}
#endif
