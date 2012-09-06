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

#pragma warning(push)
#pragma warning(disable:4146)
#pragma warning(push)
#pragma warning(disable:4739)

int sub_40BFA0(byte *a1)
{
    return *(int*)a1;
}


int sub_40BFB0(byte *a1, int a2)
{
    int result; // eax@1

    result = a2;
    *(int*)a1 = a2;

    return result;
}

int sub_40BFF0(byte *a1, int a2, int a3)
{
    int result; // eax@1

    result = a3;
    *(int*)(a1 + 4 * a2 + 4) = a3;

    return result;
}

unsigned int RNC_GetDecompSize(byte *a1)
{
    int tmp = *(int*)(a1 + 4);

    return Com_Swap32(tmp);
}

byte *rnc_data = NULL;
byte *rnc_outstart = NULL;
byte *rnc_outend = NULL;
unsigned int rnc_outsize = 0;

byte byte_5A99C0 = 0;
byte byte_5AADA4 = 0;
int dword_5AADA8 = 0;

void RNC_GetDataSize(byte *in, byte *out)
{
    unsigned int v3;
    byte *result;

    rnc_outstart = out;
    rnc_data = in + 18;

    v3 = *((_DWORD*)in + 1);
    in = in + 4;

    byte_5A99C0 = 0;

    result = out + rnc_outsize;
    rnc_outend = result;
}

__int16 RNC_GetBit2(unsigned __int8 mask)
{
    int result; // eax@1
    char v2; // dl@2
    char v3; // cl@2
    int v4; // edi@2
    byte *v5; // esi@2
    char v6; // zf@1

    result = 0;
    v6 = mask-- == 0;

    if(!v6)
    {
        v2 = byte_5A99C0;
        v3 = byte_5AADA4;
        v5 = rnc_data;
        v4 = mask + 1;

        do
        {
            if(!v2)
            {
                v3 = *v5++;
                rnc_data = v5;
                v2 = 8;
            }

            result *= 2;

            if(v3 & 0x80)
                ++result;

            v3 *= 2;
            --v2;
            --v4;
            byte_5AADA4 = v3;
            byte_5A99C0 = v2;

        } while(v4);
    }

    return result;
}

__int16 sub_454400(void)
{
    __int16 a1;
    int v1; // esi@1
    __int16 v2; // eax@2
    __int16 v3;

    a1 = RNC_GetBit2(1);
    v1 = a1 + 4;
    if(RNC_GetBit2(1))
    {
        v3 = RNC_GetBit2(1);
        v2 = v3 + 2 * v1 - 2;
    }
    else
        v2 = v1;

    return v2;
}

int sub_454440(void)
{
    __int16 v0; // esi@1
    __int16 v2; // ax@2
    __int16 v3; // ax@3
    unsigned __int8 v4; // al@3
    __int16 v5; // ax@4
    int v6; // eax@7

    v0 = 0;
    if(RNC_GetBit2(1))
    {
        v2 = RNC_GetBit2(1);
        v0 = v2;
        if(RNC_GetBit2(1))
        {
            v3 = RNC_GetBit2(1);
            v4 = 2 * v0 + v3;
            v4 |= 4;
            v0 = v4;
            if(!RNC_GetBit2(1))
            {
                v5 = RNC_GetBit2(1);
                v0 = 2 * v0 + v5;
            }
        }
        else
        {
            if(!v0)
            {
                v0 = RNC_GetBit2(1);
                v0 += 2;
            }
        }
    }
    
    v6 = *rnc_data++;
    return (v0 << 8) + v6 + 1;
}

void RNC_DecodeVer2(byte *in, byte *out)
{
    __int16 v3; // ax@5
    __int16 v4; // ax@6
    byte *v5;
    unsigned __int16 v6;
    int v7;
    byte *v8;
    __int16 v10;
    __int16 v11;
    __int16 v12;
    int v13;
    __int16 v14;
    __int16 v15;
    __int16 v16;
    int a1;

    RNC_GetDataSize(in, out);
    RNC_GetBit2(2);

    if(rnc_outstart < rnc_outend)
    {
        while(1)
        {
            while(1)
            {
                while(1)
                {
                    for(; !RNC_GetBit2(1); ++rnc_data)
                        *rnc_outstart++ = *rnc_data;

                    if(RNC_GetBit2(1))
                        break;

                    //if(rnc_outstart >= rnc_outend)
                        //return;

                    v11 = sub_454400();
                    a1 = v11;

                    if(v11 == 9)
                    {
                        v12 = RNC_GetBit2(4);
                        v13 = 4 * v12 + 12;
                        v14 = v13;
                        v6 = v13 + 65535;

                        if(v14)
                        {
                            v7 = v6 + 1;
                            do
                            {
                                *rnc_outstart = *rnc_data;
                                a1 = (int)(rnc_outstart + 1);
                                --v7;
                                ++rnc_outstart;
                                ++rnc_data;

                            } while(v7);
                        }
                    }
                    else
                    {
                        v15 = sub_454440();
                        v16 = a1;
                        v8 = &rnc_outstart[-v15];
                        a1 += 65535;
                            
                        if(v16)
                        {
                            a1 = (_WORD)a1 + 1;
                            do
                            {
                                *rnc_outstart = *v8++;
                                --a1;
                                ++rnc_outstart;
                            }
                            while(a1);
                        }
                    }
                } // while

                if(RNC_GetBit2(1))
                    break;

                //if(rnc_outstart >= rnc_outend)
                    //return;

                a1 = 2;
                v3 = *rnc_data;
                v4 = v3 + 1;
                ++rnc_data;
LABEL_11:
                v5 = &rnc_outstart[-v4];
                v10 = a1;
                a1 += 65535;
                if(v10)
                {
                    a1 = (_WORD)a1 + 1;
                    do
                    {
                        *rnc_outstart = *v5++;
                        --a1;
                        ++rnc_outstart;
                    }
                    while(a1);
                }
            } // while

            if(!RNC_GetBit2(1))
                break;

            a1 = *rnc_data;
            a1 += 8;
            ++rnc_data;

            if((_WORD)a1 != 8)
                goto LABEL_10;

            RNC_GetBit2(1);

             if(rnc_outstart >= rnc_outend)
                 return;
        } // while

        a1 = 3;
LABEL_10:
        v4 = sub_454440();
        goto LABEL_11;
    }
}

byte rnc_idbyte;
short rnc_table[0x15f];
int dword_5DC958;

int RNC_GetBit1(byte a1)
{
    int result; // eax@1
    signed int v2; // edi@1
    byte v3; // dl@1
    char v4; // dl@2
    unsigned int v5; // ecx@2
    int v6; // ebx@2
    byte *v7; // esi@2
    byte v8; // [sp+8h] [bp+4h]@1

    result = 0;
    v3 = a1;
    v2 = 1;
    v8 = a1 - 1;
    if ( v3 )
    {
        v4 = rnc_idbyte;
        v5 = dword_5DC958;
        v7 = rnc_data;
        v6 = v8 + 1;
        do
        {
            if ( !v4 )
            {
                v5 = *(_DWORD *)v7;
                v7 += 2;
                rnc_data = v7;
                v4 = 16;
            }

            if ( v5 & 1 )
                result |= v2;

            v2 *= 2;
            v5 >>= 1;
            --v4;
            --v6;
            dword_5DC958 = v5;
            rnc_idbyte = v4;
        }
        while ( v6 );
    }

    return result;
}

void sub_453800(__int16 *a1, byte a2)
{
    byte v2; // cl@1
    int v3; // ecx@2
    __int16 *v4; // eax@4
    unsigned __int8 v6; // [sp+8h] [bp+8h]@1

    v2 = a2;
    v6 = a2 - 1;

    if ( v2 )
    {
        v4 = a1;
        v3 = v6 + 1;
        do
        {
            *(_DWORD *)v4 = 0;
            v4[2] = -1;
            *((_DWORD *)v4 + 2) = 0;
            v4[6] = 0;
            v4 += 8;
            --v3;
        }
        while ( v3 );
    }
}

void sub_453840(__int16 *a1, unsigned __int8 a2)
{
    int v2; // ebx@1
    unsigned int v3; // ebp@1
    unsigned int v4; // edi@1
    int v5; // eax@2
    __int16 *v6; // esi@3
    int v7; // eax@5
    int v8; // ecx@5
    int v9; // edx@6
    int v12; // [sp+10h] [bp-8h]@3

    v2 = 1;
    v3 = 0;
    v4 = -2147483648;
    do
    {
        LOWORD2(v5) = a2;
        if ( a2 )
        {
            v12 = a2;
            v6 = a1 + 4;
            do
            {
                if ( v6[2] == (_WORD)v2 )
                {
                    v7 = v3 / v4;
                    v8 = 0;
                    if ( (_BYTE)v2 )
                    {
                        v9 = (unsigned __int8)(v2 - 1) + 1;
                        do
                        {
                            v8 *= 2;
                            if ( v7 & 1 )
                                v8 |= 1u;

                            v7 = (unsigned int)v7 >> 1;
                            --v9;
                        }
                        while ( v9 );
                    }
                    
                    *(_DWORD *)v6 = v8;
                    v3 += v4;
                }
                v6 += 8;
                v5 = v12 - 1;
            }
            while ( v12-- != 1 );
        }

        ++v2;
        v4 >>= 1;
    }
    while ( (_WORD)v2 <= 0x10u );
}

__int16 sub_4731F0(__int16 *a1)
{
    __int16 *v1; // eax@1
    int v2; // edx@1
    unsigned __int8 i; // bl@1
    __int16 v4; // cx@2
    int v5; // ebp@5
    signed int v6; // edi@5
    byte v7; // cl@5
    char v8; // al@6
    int v9; // ecx@6
    byte *v10; // esi@6
    int v11; // eax@15
    int v12; // ebp@16
    signed int v13; // edi@16
    int v14; // ecx@17
    unsigned __int8 v16; // [sp+18h] [bp+4h]@5
    unsigned __int8 v17; // [sp+18h] [bp+4h]@16

    v1 = a1;
    v2 = dword_5DC958;

    for ( i = 0; ; ++i )
    {
        v4 = v1[6];
        if ( v4 )
        {
            if ( (dword_5DC958 & ((1 << v4) - 1)) == *((_DWORD *)v1 + 2) )
                break;
        }

        v1 += 8;
    }

    v5 = 0;
    v7 = *((_BYTE *)v1 + 12);
    v6 = 1;
    v16 = v7 - 1;

    if ( v7 )
    {
        v8 = rnc_idbyte;
        v10 = rnc_data;
        v9 = v16 + 1;
        do
        {
            if ( !v8 )
            {
                v2 = *(_DWORD *)v10;
                v10 += 2;
                rnc_data = v10;
                v8 = 16;
            }

            if ( v2 & 1 )
                v5 |= v6;

            v6 *= 2;
            v2 = (unsigned int)v2 >> 1;

            --v8;
            --v9;

            dword_5DC958 = v2;
            rnc_idbyte = v8;
        }
        while ( v9 );
    }
    else
    {
        v8 = rnc_idbyte;
        v10 = rnc_data;
    }

    if ( i >= 2u )
    {
        v12 = 0;
        v13 = 1;
        v17 = i - 2;
        if ( i != 1 )
        {
            v14 = v17 + 1;
            do
            {
                if ( !v8 )
                {
                    v2 = *(_DWORD *)v10;
                    v10 += 2;
                    rnc_data = v10;
                    v8 = 16;
                }
                
                if ( v2 & 1 )
                    v12 |= v13;
                
                v13 *= 2;
                v2 = (unsigned int)v2 >> 1;

                --v8;
                --v14;

                dword_5DC958 = v2;
                rnc_idbyte = v8;
            }
            while ( v14 );
        }
        v11 = v12 | (1 << (i - 1));
    }
    else
    {
        LOWORD2(v11) = i;
    }

    return v11;
}

int RNC_DecodeVer1(byte *input, byte *output)
{
    byte v2; // al@1
    unsigned int v3; // edx@1
    byte *v4; // ecx@1
    byte *v5; // ebx@1
    signed int v6; // edi@1
    byte *v7; // esi@1
    char v8; // zf@6
    unsigned int v9; // eax@8
    byte v10; // dl@8
    char v11; // cl@8
    signed int v12; // ebx@8
    signed int v13; // edi@8
    byte *v14; // esi@8
    __int16 *v15; // edi@18
    int v16; // esi@18
    unsigned int v17; // eax@21
    byte v18; // dl@21
    char v19; // cl@21
    signed int v20; // ebx@21
    signed int v21; // edi@21
    byte *v22; // esi@21
    __int16 *v23; // edi@31
    int v24; // esi@31
    unsigned __int8 v25; // al@34
    __int16 *v26; // edi@38
    int v27; // esi@38
    unsigned int v28; // eax@41
    byte *v29; // edx@41
    char v30; // cl@41
    int v31; // ebx@41
    signed int v32; // edi@41
    signed int v33; // esi@41
    int v34; // edi@47
    int v35; // eax@48
    int v36; // eax@48
    __int16 v37; // cx@48
    int v38; // eax@49
    byte *v39; // edx@49
    int v40; // eax@53
    __int16 *v41; // edx@54
    unsigned __int8 v42; // bl@54
    __int16 v43; // cx@55
    unsigned __int16 v44; // ax@59
    __int16 *v45; // eax@61
    unsigned __int8 i; // bl@61
    byte *v47; // esi@61
    byte *v48; // esi@61
    __int16 v49; // cx@62
    int v50; // eax@65
    int v51; // eax@68
    int v52; // eax@68
    __int16 v53; // cx@68
    int v54; // eax@69
    unsigned __int8 v56; // [sp+10h] [bp-4h]@34
    int v57; // [sp+18h] [bp+4h]@1
    unsigned __int8 v58; // [sp+18h] [bp+4h]@14
    signed int v59; // [sp+1Ch] [bp+8h]@1
    unsigned __int8 v60; // [sp+1Ch] [bp+8h]@27
    unsigned int count;

    count = 0;
    dword_5DC958 = 0;

    v5 = output;
    v3 = dword_5DC958;
    v4 = input + 18;
    rnc_data = input + 18;
    rnc_outstart = output;
    v7 = output + rnc_outsize;

    v2 = 0;
    rnc_outend = output + rnc_outsize;
    rnc_idbyte = 0;
    v57 = 0;
    v6 = 1;
    v59 = 2;

    do
    {
        if ( !v2 )
        {
            v3 = *(_DWORD *)v4;
            v4 += 2;
            rnc_data = v4;
            v2 = 16;
        }

        if ( v3 & 1 )
            v57 |= v6;

        v6 *= 2;
        v3 >>= 1;

        --v2;

        v8 = v59 == 1;
        dword_5DC958 = v3;
        rnc_idbyte = v2;

        --v59;
    }
    while ( !v8 );

    if ( v5 < v7 )
    {
        while ( 1 )
        {
            sub_453800(rnc_table, 0x10u);
            v14 = rnc_data;
            v9 = dword_5DC958;
            v11 = rnc_idbyte;
            *(_DWORD *)&v10 = 0;
            v13 = 1;
            v12 = 5;
            do
            {
                if ( !v11 )
                {
                    v9 = *(_DWORD *)v14;
                    v14 += 2;
                    rnc_data = v14;
                    v11 = 16;
                }

                if ( v9 & 1 )
                    *(_DWORD *)&v10 |= v13;
                v13 *= 2;
                v9 >>= 1;
                --v11;
                --v12;

                dword_5DC958 = v9;
                rnc_idbyte = v11;
            }
            while ( v12 );

            v58 = v10;

            if ( v10 )
                break;

LABEL_21:

            sub_453800(rnc_table + 128, 0x10u);
            v22 = rnc_data;
            v17 = dword_5DC958;
            v19 = rnc_idbyte;
            *(_DWORD *)&v18 = 0;

            v21 = 1;
            v20 = 5;

            do
            {
                if ( !v19 )
                {
                    v17 = *(_DWORD *)v22;
                    v22 += 2;
                    rnc_data = v22;
                    v19 = 16;
                }

                if ( v17 & 1 )
                    *(_DWORD *)&v18 |= v21;

                v21 *= 2;
                v17 >>= 1;

                --v19;
                --v20;

                dword_5DC958 = v17;
                rnc_idbyte = v19;
            }
            while ( v20 );

            v60 = v18;

            if ( v18 )
            {
                if ( v18 <= 0x10u )
                {
                    if ( v18 )
                        goto LABEL_31;
                }
                else
                {
                    v60 = 16;
LABEL_31:
                    v23 = rnc_table + 134;
                    v24 = v60;
                    do
                    {
                        *v23 = RNC_GetBit1(4u);
                        v23 += 8;
                        --v24;
                    }
                    while ( v24 );
                }
                sub_453840(rnc_table + 128, v60);
            }

            sub_453800(rnc_table + 260, 0x10u);

            v25 = RNC_GetBit1(5u);
            v56 = v25;
            if ( !v25 )
                goto LABEL_41;

            if ( v25 > 0x10u )
            {
                v56 = 16;
LABEL_38:
                v26 = rnc_table + 266;
                v27 = v56;
                do
                {
                    *v26 = RNC_GetBit1(4u);
                    v26 += 8;
                    --v27;
                }
                while ( v27 );

                goto LABEL_40;
            }

            if ( v25 )
                goto LABEL_38;

LABEL_40:

            sub_453840(rnc_table + 260, v56);

LABEL_41:

            v29 = rnc_data;
            v28 = dword_5DC958;
            v30 = rnc_idbyte;
            v31 = 0;
            v33 = 1;
            v32 = 16;

            do
            {
                if ( !v30 )
                {
                    v28 = *(_DWORD *)v29;
                    v29 += 2;
                    rnc_data = v29;
                    v30 = 16;
                }

                if ( v28 & 1 )
                    v31 |= v33;

                v33 *= 2;
                v28 >>= 1;

                --v30;
                --v32;

                dword_5DC958 = v28;
                rnc_idbyte = v30;
            }
            while ( v32 );

            v34 = v31;

            while ( 1 )
            {
                LOWORD2(v36) = sub_4731F0(rnc_table);
                v37 = v36;
                v35 = v36 + 65535;
                if ( v37 )
                {
                    v39 = rnc_data;
                    v38 = (unsigned __int16)v35 + 1;
                    do
                    {
                        *rnc_outstart = *v39;
                        v39 = rnc_data + 1;
                        --v38;
                        ++rnc_outstart;
                        ++count;
                        ++rnc_data;

                        if(count >= rnc_outsize)
                            return 0;
                    }
                    while ( v38 );
                }
                else
                {
                    v39 = rnc_data;
                }

                v34 += 65535;
                v40 = (dword_5DC958 & ((1 << rnc_idbyte) - 1)) + ((*v39 + ((v39[1] + (v39[2] << 8)) << 8)) << rnc_idbyte);
                dword_5DC958 = (dword_5DC958 & ((1 << rnc_idbyte) - 1))
                    + ((*v39 + ((v39[1] + (v39[2] << 8)) << 8)) << rnc_idbyte);

                if ( !(_WORD)v34 )
                    break;

                v42 = 0;
                v41 = rnc_table + 128;

                while ( 1 )
                {
                    v43 = v41[6];
                    if ( v43 )
                    {
                        if ( (v40 & ((1 << v43) - 1)) == *((_DWORD *)v41 + 2) )
                            break;
                    }
                    v41 += 8;
                    ++v42;
                }

                RNC_GetBit1(*((_BYTE *)v41 + 12));

                if ( v42 >= 2u )
                    *(_DWORD *)&v44 = (1 << (v42 - 1)) | RNC_GetBit1((byte)(v42 - 1));
                else
                    v44 = v42;
                    v48 = &rnc_outstart[-v44];
                    v45 = rnc_table + 260;
                    v47 = v48 - 1;
                for ( i = 0; ; ++i )
                {
                    v49 = v45[6];
                    if ( v49 )
                    {
                        if ( (dword_5DC958 & ((1 << v49) - 1)) == *((_DWORD *)v45 + 2) )
                            break;
                    }
        
                    v45 += 8;
                }

                v50 = RNC_GetBit1(*((_BYTE *)v45 + 12));

                if ( i >= 2u )
                    v50 = (1 << (i - 1)) | RNC_GetBit1((byte)(i - 1));
                else
                    LOWORD2(v50) = i;
                v52 = v50 + 2;
                v53 = v52;
                v51 = v52 + 65535;

                if ( v53 )
                {
                    v54 = (unsigned __int16)v51 + 1;
                    do
                    {
                        *rnc_outstart = *v47++;
                        --v54;
                        ++rnc_outstart;
                        ++count;

                        if(count >= rnc_outsize)
                            return 0;
                    }
                    while ( v54 );
                }
            }

            if ( rnc_outstart >= rnc_outend )
                return 0;
        }
    
        if ( v10 <= 0x10u )
        {
            if ( !v10 )
            {

LABEL_20:
                sub_453840(rnc_table, v58);
                goto LABEL_21;
            }
        }
        else
        {
            v58 = 16;
        }

        v15 = rnc_table + 6;
        v16 = v58;

        do
        {
            *v15 = RNC_GetBit1(4u);
            v15 += 8;
            --v16;
        }
        while ( v16 );

        goto LABEL_20;
    }

    return 0;
}

int RNC_Decode(byte *input, byte *output)
{
    if(*(input + 3) == 1)
    {
        RNC_DecodeVer1(input, output);
    }
    else if(*(input + 3) == 2)
    {
        RNC_DecodeVer2(input, output);
    }
    else
        return 0;

    return 1;
}

byte* RNC_ParseFile(byte* data, dword size, dword* outsize)
{
    byte* output;

    output = (byte*)Com_Alloc(850000);
    memset(output, 0, 850000);
    rnc_outsize = Com_Swap32(*(int*)(data + 4));

    RNC_Decode(data, output);

    if(outsize)
        *outsize = rnc_outsize;

    return output;
}

#pragma warning(pop)
#pragma warning(pop)
