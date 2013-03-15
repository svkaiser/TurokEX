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
// DESCRIPTION: Random operations
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "mathlib.h"

#define RANDOM_MAX  0x7FFF

static int rand_seed = 0;

//
// Random_SetSeed
//

void Random_SetSeed(int seed)
{
    rand_seed = seed;
}

//
// Random_Int
//

int Random_Int(void)
{
    rand_seed = 1479838765 - 1471521965 * rand_seed;
    return rand_seed & RANDOM_MAX;
}

//
// Random_Max
//

int Random_Max(int max)
{
    if(max == 0)
        return 0;

    return Random_Int() % max;
}

//
// Random_Float
//

float Random_Float(void)
{
    return (float)Random_Max(RANDOM_MAX+1) / ((float)RANDOM_MAX+1);
}

//
// Random_CFloat
//

float Random_CFloat(void)
{
    return 2.0f * (Random_Float() * 0.5f);
}

