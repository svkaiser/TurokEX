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

#ifndef __BLAM_TYPE__
#define __BLAM_TYPE__

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

extern int myargc;
extern char **myargv;

#define false 0
#define true (!false)

typedef int             dboolean;
typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   dword;
typedef float           bbox[6];

typedef union
{
    int     i;
    float   f;
} fint_t;

#ifdef _WIN32

#define DIR_SEPARATOR '\\'
#define PATH_SEPARATOR ';'

#else

#define DIR_SEPARATOR '/'
#define PATH_SEPARATOR ':'

#endif

#ifdef _MSC_VER
    #include <direct.h>
    #include <io.h>
    #define strcasecmp  _stricmp
    #define strncasecmp _strnicmp
#endif

#if defined(__GNUC__)
  #define d_inline __inline__
#elif defined(_MSC_VER)
  #define d_inline __inline
#else
  #define d_inline
#endif

#if defined(__GNUC__)
#define FMT_64 "ll"
typedef          long long ll;
typedef unsigned long long ull;
#elif defined(_MSC_VER)
typedef          __int64 ll;
typedef unsigned __int64 ull;
#define FMT_64 "I64"
#elif defined (__BORLANDC__)
typedef          __int64 ll;
typedef unsigned __int64 ull;
#define FMT_64 "L"
#else
#error "unknown compiler"
#endif
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

// Partially defined types:
#define _BYTE  char
#define _WORD  short
#define _DWORD long
#define _QWORD ll
#define _LONGLONG __int128

#ifndef _WINDOWS_
typedef char BYTE;
typedef short WORD;
typedef long DWORD;
typedef long LONG;
#endif
typedef ll QWORD;
#ifndef __cplusplus
typedef int bool;       // we want to use bool in our C programs
#endif

// Some convenience macros to make partial accesses nicer
// first unsigned macros:
#define LOBYTE2(x)   (*((_BYTE*)&(x)))   // low byte
#define LOWORD2(x)   (*((_WORD*)&(x)))   // low word
#define LODWORD2(x)  (*((_DWORD*)&(x)))  // low dword
#define HIBYTE2(x)   (*((_BYTE*)&(x)+1))
#define HIWORD2(x)   (*((_WORD*)&(x)+1))
#define HIDWORD2(x)  (*((_DWORD*)&(x)+1))
#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define WORDn(x, n)   (*((_WORD*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)
#define BYTE2(x)   BYTEn(x,  2)
#define BYTE3(x)   BYTEn(x,  3)
#define BYTE4(x)   BYTEn(x,  4)
#define BYTE5(x)   BYTEn(x,  5)
#define BYTE6(x)   BYTEn(x,  6)
#define BYTE7(x)   BYTEn(x,  7)
#define BYTE8(x)   BYTEn(x,  8)
#define BYTE9(x)   BYTEn(x,  9)
#define BYTE10(x)  BYTEn(x, 10)
#define BYTE11(x)  BYTEn(x, 11)
#define BYTE12(x)  BYTEn(x, 12)
#define BYTE13(x)  BYTEn(x, 13)
#define BYTE14(x)  BYTEn(x, 14)
#define BYTE15(x)  BYTEn(x, 15)
#define WORD1(x)   WORDn(x,  1)
#define WORD2(x)   WORDn(x,  2)         // third word of the object, unsigned
#define WORD3(x)   WORDn(x,  3)
#define WORD4(x)   WORDn(x,  4)
#define WORD5(x)   WORDn(x,  5)
#define WORD6(x)   WORDn(x,  6)
#define WORD7(x)   WORDn(x,  7)

// now signed macros (the same but with sign extension)
#define SLOBYTE(x)   (*((char*)&(x)))
#define SLOWORD(x)   (*((short*)&(x)))
#define SLODWORD(x)  (*((long*)&(x)))
#define SHIBYTE(x)   (*((char*)&(x)+1))
#define SHIWORD(x)   (*((short*)&(x)+1))
#define SHIDWORD(x)  (*((long*)&(x)+1))
#define SBYTEn(x, n)   (*((char*)&(x)+n))
#define SWORDn(x, n)   (*((short*)&(x)+n))
#define SBYTE1(x)   SBYTEn(x,  1)
#define SBYTE2(x)   SBYTEn(x,  2)
#define SBYTE3(x)   SBYTEn(x,  3)
#define SBYTE4(x)   SBYTEn(x,  4)
#define SBYTE5(x)   SBYTEn(x,  5)
#define SBYTE6(x)   SBYTEn(x,  6)
#define SBYTE7(x)   SBYTEn(x,  7)
#define SBYTE8(x)   SBYTEn(x,  8)
#define SBYTE9(x)   SBYTEn(x,  9)
#define SBYTE10(x)  SBYTEn(x, 10)
#define SBYTE11(x)  SBYTEn(x, 11)
#define SBYTE12(x)  SBYTEn(x, 12)
#define SBYTE13(x)  SBYTEn(x, 13)
#define SBYTE14(x)  SBYTEn(x, 14)
#define SBYTE15(x)  SBYTEn(x, 15)
#define SWORD1(x)   SWORDn(x,  1)
#define SWORD2(x)   SWORDn(x,  2)
#define SWORD3(x)   SWORDn(x,  3)
#define SWORD4(x)   SWORDn(x,  4)
#define SWORD5(x)   SWORDn(x,  5)
#define SWORD6(x)   SWORDn(x,  6)
#define SWORD7(x)   SWORDn(x,  7)

// Macros to represent some assembly instructions
// Feel free to modify them

#define __ROL__(x, y) __rotl__(x, y)       // Rotate left
#define __ROR__(x, y) __rotr__(x, y)       // Rotate right
#define __RCL__(x, y) invalid_operation    // Rotate left thru carry
#define __RCR__(x, y) invalid_operation    // Rotate right thru carry
#define __MKCADD__(x, y) invalid_operation // Generate carry flag for an addition
#define __MKOADD__(x, y) invalid_operation // Generate overflow flag for an addition
#define __MKCSHL__(x, y) invalid_operation // Generate carry flag for a shift left
#define __MKCSHR__(x, y) invalid_operation // Generate carry flag for a shift right
#define __MKCRCL__(x, y) invalid_operation // Generate carry flag for a RCL
#define __MKCRCR__(x, y) invalid_operation // Generate carry flag for a RCR
#define __SETO__(x, y)   invalid_operation // Generate overflow flags for (x-y)


// In the decompilation listing there are some objects declarared as _UNKNOWN
// because we could not determine their types. Since the C compiler does not
// accept void item declarations, we replace them by anything of our choice,
// for example a char:

#define _UNKNOWN char

#endif