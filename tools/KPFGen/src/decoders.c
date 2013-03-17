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

#include "types.h"
#include "common.h"

static int      dword_49CF20 = 0x1000;
static int      dword_514DE0 = 0;
static int      dword_514DDC = 0;
static int      dword_514DD4 = 0;
static int      dword_514DD8 = 0;
static byte*    dword_514DE4 = NULL;
static byte*    dword_514DE8 = NULL;
static int      dword_510B88[4] = { 0x0, 0x0F000, 0x1E800, 0x1CC00 };
static int      dword_510B98[4] = { 0x0, 0x0, 0x0FFFF1000, 0x0FFFF3000 };

const float flt_48EC8C = 3.0518509e-5f;
int off_49CF1C[0x1000];

void DC_SetupLUT1(byte *a1, int a2)
{
    int v3 = 0; // ecx@1
    int v4; // edx@1
    int v5; // edx@1
    int v6; // edx@1

    v3 = 0;
    v4 = *a1;
    dword_514DE0 = *a1;

    *(_WORD *)((char *)&v3 + 1) = *(a1 + 1);
    v5 = v3 | v4;
    dword_514DE0 = v5;
    v6 = (*(a1 + 2) << 16) | v5;

    dword_514DE0 = v6;
    dword_514DE0 = ((*(a1 + 3) << 24) | (unsigned int)v6) >> a2;
    dword_514DDC = 32 - a2;
    dword_514DE4 = a1 + 4;
}

void DC_SetupLUT2(byte *a1)
{
    int v2 = 0; // edx@1
    int v3; // ecx@1
    int v4; // ecx@1
    int v5; // ecx@1
    int v6; // edx@1

    v2 = 0;
    v3 = *a1;
    dword_514DD4 = *a1;

    *(_WORD *)((char *)&v2 + 1) = *(a1 + 1);

    v4 = v2 | v3;
    dword_514DD4 = v4;

    v5 = (*(a1 + 2) << 16) | v4;
    dword_514DD4 = v5;

    v6 = *(a1 + 3);

    dword_514DD8 = 32;
    dword_514DD4 = (v6 << 24) | v5;
    dword_514DE8 = a1 + 4;
}

int sub_47F550(int a1)
{
    int result; // eax@1
    int v2; // edx@1
    unsigned __int8 v3; // of@1
    char v4; // zf@1
    char v5; // sf@1
    int v6; // ecx@2

    result = dword_514DE0 & ~(-1 << a1);

    dword_514DE0 = (unsigned int)dword_514DE0 >> a1;
    v2 = dword_514DDC - a1;
    v3 = dword_514DDC - a1 > 24;
    v4 = dword_514DDC - a1 == 24;
    v5 = dword_514DDC - a1 < 24;
    dword_514DDC -= a1;

    if(v2 <= 24)
    {
        v6 = (*dword_514DE4++ << v2) | dword_514DE0;
        dword_514DE0 = v6;
        dword_514DDC = v2 + 8;
    }

    return result;
}

int sub_47F4E0(int a1)
{
    int result; // eax@1
    int v2; // edx@1
    unsigned __int8 v3; // of@1
    char v4; // zf@1
    char v5; // sf@1
    int v6; // ecx@2

    result = dword_514DD4 & ~(-1 << a1);

    dword_514DD4 = (unsigned int)dword_514DD4 >> a1;
    v2 = dword_514DD8 - a1;
    v3 = dword_514DD8 - a1 > 24;
    v4 = dword_514DD8 - a1 == 24;
    v5 = dword_514DD8 - a1 < 24;
    dword_514DD8 -= a1;

    if(v2 <= 24)
    {
        v6 = (*dword_514DE8++ << v2) | dword_514DD4;
        dword_514DD4 = v6;
        dword_514DD8 = v2 + 8;
    }

    return result;
}

int DC_WriteAnimTable(byte *a1, int a2)
{
    int result; // eax@1
    int v3; // esi@1
    char v4; // bl@2
    int v5; // ebp@2
    int v6; // edi@2
    int v7; // eax@5
    int v8; // eax@7
    char v9; // al@1
    __int16 v10; // cx@1
    char v11; // zf@3
    unsigned __int8 v12; // al@4
    char v13; // cl@4
    int v14; // eax@4
    signed int v15; // edx@7
    int v16; // eax@7
    char v17; // zf@11
    int v18; // [sp+8h] [bp-8h]@1
    signed int v19; // [sp+4h] [bp-Ch]@1
    int v20; // [sp+Ch] [bp-4h]@1

    v3 = 0;
    v18 = 0;
    v19 = 0;

    result = 0;

    v9 = sub_47F550(8);

    LOBYTE2(v10) = 0;
    HIBYTE2(v10) = v9;

    v20 = v10;

    if(a2 > 0)
    {
        v6 = *a1;
        v5 = *a1;
        v4 = *a1;
        a2 = a2;

        do
        {
            v11 = v19-- == 0;

            if(v11)
            {
                v19 = 3;
                v12 = sub_47F4E0(6);
                v13 = v12 & 0xF;
                v14 = (v12 >> 4) & 3;
                v4 = v13;

                v6 = dword_510B88[v14];
                v5 = dword_510B98[v14];
            }

            v7 = (signed __int16)sub_47F550(3);
            if(v7 & 4)
                v7 |= 0xFFFFFFF8u;

            v15 = v18 * v5 + v3 * v6;
            v18 = v3;
            v16 = (v15 >> 16) + (v7 << v4);
            v3 = v16;
            v8 = v20 + v16;
            v20 = v8;

            //
            // clamp values
            //
            if(v8 > 32767)
                v8 = 32767;
            if(v8 < -32768)
                v8 = -32768;

            *(_WORD *)a1 = v8;

            v17 = a2 == 1;

            a1 += 2;
            result += 2;
            --a2;
        } while(!v17);
    }
    
    return result;
}

void DC_BuildAnimTable(int **a1, byte *a2, int a3)
{
    int v3 = 0; // ebx@1
    int v4; // ebp@1
    byte *v5;
    int **v6; // edi@2
    int id;
    int idbyte;

    BYTE3(v3) = 0;
    *(_WORD *)((char *)&v3 + 1) = *a2;
    LOBYTE2(v3) = *(a2 + 1);

    v5 = a2 + 2;

    idbyte = ((v3 + 3) >> 2);
    id = ((idbyte + idbyte * 2) << 1);

    DC_SetupLUT1(v5 + (id >> 3), id & 7);

    v4 = a3;

    if(a3 > 0)
    {
        v6 = a1;
        do
        {
            DC_SetupLUT2(v5);
            DC_WriteAnimTable((byte *)*v6, v3);
            ++v6;
            --v4;
        }

        while(v4);
    }
}

void DC_DecodeData(byte *a1, byte *a2, int a3)
{
    byte *v3; // edx@1
    int v4; // ebx@1
    int v5; // ebp@1
    byte *v6; // edi@1
    int v7; // esi@1
    byte *v8; // eax@1
    byte *v9; // eax@3
    int v10; // edi@3
    byte v11; // cl@4
    byte *v13; // [sp+10h] [bp-4h]@1

    v8 = a1;
    v6 = a2;
    v7 = 0;
    v13 = (a1 + 8);
    *(_DWORD *)a2 = *(_DWORD *)a1;
    *((_DWORD *)a2 + 1) = *((_DWORD *)v8 + 1);
    v3 = a2 + 8;
    v5 = *(_DWORD *)(a1 + 4);
    v4 = *(_DWORD *)a1;
    if(v5 > 0)
    {
        do
        {
            if(v4 > 0)
            {
                v10 = v4;
                v9 = (v13 + v7);
                do
                {
                    v11 = *v9;
                    v9 += v5;
                    *v3++ = v11;
                    --v10;

                } while(v10);
            }
            ++v7;
        } while(v7 < v5);

        v6 = a2;
    }

    if(a3)
    {
        memcpy(a1, v6, v4 * v5 + 8);
    }
}

int DC_LookupSndFXIndex(int *a1, int a2, int a3)
{
    int result; // eax@2
    int v4; // edx@2
    int v5; // ecx@3

    if ( !a2 )
        return -1;

    v4 = a2 - 1;
    result = 0;
    if ( a2 != 1 )
    {
        do
        {
            v5 = (v4 + result) >> 1;
            if ( a1[v5] >= a3 )
                v4 = (v4 + result) >> 1;
            else
                result = v5 + 1;
        }
        while ( result < v4 );
    }

    if ( a1[result] != a3 )
        result = -1;

    return result;
}
