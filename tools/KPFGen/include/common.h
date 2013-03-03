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

#ifndef __KPF_COMMON__
#define __KPF_COMMON__

void StoreExternalFile(const char *name, const char *store);

extern byte *cartfile;

void Com_Printf(char* s, ...);
void Com_Error(char *fmt, ...);
void* Com_Alloc(int size);
void Com_Free(void **ptr);
int Com_ReadFile(const char* name, byte** buffer);
int Com_ReadBinaryFile(const char* name, byte** buffer);
dboolean Com_SetWriteFile(char const* name, ...);
dboolean Com_SetWriteBinaryFile(char const* name, ...);
void Com_CloseFile(void);
long Com_FileLength(FILE *handle);
int Com_FileExists(const char *filename);
byte *Com_GetFileBuffer(byte *buffer);
byte Com_Read8(byte* buffer);
short Com_Read16(byte* buffer);
int Com_Read32(byte* buffer);
void Com_Write8(byte value);
void Com_Write16(short value);
void Com_Write32(int value);
void Com_FPrintf(char* s, ...);
void Com_StripExt(char *name);
void Com_StripPath(char *name);
const char *Com_GetExePath(void);
char *va(char *str, ...);
int Com_GetCartOffset(byte *buf, int count, int *size);
byte *Com_GetCartData(byte *buf, int count, int *size);
void Com_UpdateCartData(byte* data, int entries, int next);
void Com_SetCartHeader(byte* data, int size, int count);
void Com_WriteCartData(byte *buf, int count, char* s, ...);
int Com_Swap16(int x);
dword Com_Swap32(unsigned int x);
void Com_CloseCartFile(void);
void Com_UpdateProgress(char *fmt, ...);
void Com_UpdateDataProgress(void);
void Com_SetDataProgress(int range);
void Com_StrcatClear(void);
void Com_StrcatAddToFile(const char *name);
void Com_Strcat(const char *string, ...);

#ifdef _WIN32
byte *Com_GetCartFile(const char *filter, const char *title);
#else
#error TODO: support Com_GetCartFile in *unix
#endif

#define Com_WriteMem8(p, x)             \
    *p = x; p++

#define Com_WriteMem16(p, x)            \
    Com_WriteMem8(p, x & 0xff);         \
    Com_WriteMem8(p, (x >> 8) & 0xff)

#define Com_WriteMem32(p, x)            \
    Com_WriteMem8(p, x & 0xff);         \
    Com_WriteMem8(p, (x >> 8) & 0xff);  \
    Com_WriteMem8(p, (x >> 16) & 0xff); \
    Com_WriteMem8(p, (x >> 24) & 0xff)

#define _PAD4(x)	x += (4 - ((uint) x & 3)) & 3
#define _PAD8(x)	x += (8 - ((uint) x & 7)) & 7
#define _PAD16(x)	x += (16 - ((uint) x & 15)) & 15

#endif