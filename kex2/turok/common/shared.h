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

#ifndef __SHARED_H__
#define __SHARED_H__

#define false 0
#define true (!false)

#define MAX_FILEPATH    256

typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   ulong;
typedef int             kbool;
typedef unsigned int    dtexture;
typedef unsigned int    rcolor;
typedef float           vec3_t[3];
typedef float           vec4_t[4];
typedef float           mtx_t[16];
typedef char            filepath_t[MAX_FILEPATH];

typedef struct
{
    vec3_t  min;
    vec3_t  max;
} bbox_t;

typedef union
{
    int     i;
    float   f;
} fint_t;

typedef struct gclient_s gclient_t;
typedef struct gActor_s gActor_t;
typedef struct svclient_s svclient_t;
typedef struct JSObject gObject_t;

#define ASCII_SLASH		47
#define ASCII_BACKSLASH 92

#ifdef _WIN32

#define DIR_SEPARATOR '\\'
#define PATH_SEPARATOR ';'

#else

#define DIR_SEPARATOR '/'
#define PATH_SEPARATOR ':'

#endif

#include <limits.h>
#define D_MININT INT_MIN
#define D_MAXINT INT_MAX

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef BETWEEN
#define BETWEEN(l,u,x) ((l)>(x)?(l):(x)>(u)?(u):(x))
#endif

#endif

