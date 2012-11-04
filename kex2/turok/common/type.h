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


#ifndef __TYPE__
#define __TYPE__

#ifdef _WIN32
#include "SDL_config.h"
#else
#include <stdint.h>
#endif

#define false 0
#define true (!false)

typedef int             kbool;
typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   ulong;
typedef int8_t          int8;
typedef uint8_t         uint8;
typedef int16_t         int16;
typedef uint16_t        uint16;
typedef int32_t         int32;
typedef uint32_t        uint32;
typedef unsigned int    dtexture;
typedef unsigned int    rcolor;
typedef float           vec3_t[3];
typedef float           vec4_t[4];
typedef float           mtx_t[16];

typedef struct
{
    vec3_t min;
    vec3_t max;
} bbox_t;

typedef union
{
    int     i;
    float   f;
} fint_t;

typedef struct actor_s actor_t;
typedef struct gclient_s gclient_t;
typedef struct svclient_s svclient_t;

#ifndef _MSC_VER
	typedef signed long long int64;
	typedef unsigned long long uint64;
#else
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#endif

#include <limits.h>
#define D_MININT INT_MIN
#define D_MAXINT INT_MAX
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef BETWEEN
#define BETWEEN(l,u,x) ((l)>(x)?(l):(x)>(u)?(u):(x))
#endif

#define MAX_FILEPATH    256

#ifdef _WIN32

#define DIR_SEPARATOR '\\'
#define PATH_SEPARATOR ';'

#else

#define DIR_SEPARATOR '/'
#define PATH_SEPARATOR ':'

#endif


