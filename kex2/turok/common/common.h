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

#ifndef __COMMON_H__
#define __COMMON_H__

#include "SDL_endian.h"
#include "shared.h"

#ifdef _WIN32
#include "opndir.h"
#endif

#include "mathlib.h"
#include "kernel.h"

#define RGBA(r,g,b,a) ((rcolor)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define COLOR_WHITE         RGBA(0xFF, 0xFF, 0xFF, 0xFF)
#define COLOR_WHITE_A(a)    RGBA(0xFF, 0xFF, 0xFF, a)
#define COLOR_RED           RGBA(0xFF, 0, 0, 0xFF)
#define COLOR_GREEN         RGBA(0, 0xFF, 0, 0xFF)
#define COLOR_YELLOW        RGBA(0xFF, 0xFF, 0, 0xFF)
#define COLOR_CYAN          RGBA(0, 0xFF, 0xFF, 0xFF)

//
// ENDIAN SWAPS
//
#define Com_SwapLE16(x)   SDL_SwapLE16(x)
#define Com_SwapLE32(x)   SDL_SwapLE32(x)
#define Com_SwapBE16(x)   SDL_SwapBE16(x)
#define Com_SwapBE32(x)   SDL_SwapBE32(x)

// Defines for checking the endianness of the system.

#if SDL_BYTEORDER == SYS_LIL_ENDIAN
#define SYS_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SYS_BIG_ENDIAN
#define SYS_BIG_ENDIAN
#endif

//
// SYSTEM
//
#ifdef _WIN32
void Sys_ShowConsole(kbool show);
void Sys_SpawnConsole(void);
void Sys_DestroyConsole(void);
void Sys_UpdateConsole(void);
void Sys_Printf(const char* string, ...);
void Sys_Error(const char *string);
#endif

//
// COMMON
//

#define MAX_HASH    2048

extern  int	myargc;
extern  char** myargv;

char *kva(char *str, ...);
kbool fcmp(float f1, float f2);
void Com_Printf(char *string, ...);
void Com_CPrintf(rcolor color, char *string, ...);
void Com_Warning(char *string, ...);
void Com_DPrintf(char *string, ...);
void Com_Error(char* string, ...);
int Com_CheckParam(char *check);
void Com_Shutdown(void);
void Com_WriteConfigFile(void);
void Com_ReadConfigFile(const char *file);
unsigned int Com_HashFileName(const char *name);
void Com_NormalizeSlashes(char *str);
void Com_StripPath(char *name);
void Com_StripExt(char *name);

//
// CVARS
//
typedef struct cvar_s
{
    char*           name;
    char*           string;
    kbool           nonclient;
    void            (*callback)(void*);
    float           value;
    char*           defvalue;
    struct cvar_s*  next;
} cvar_t;

#define CVAR(name, str)                                     \
    cvar_t name = { # name, # str, 0, NULL }

#define CVAR_CMD(name, str)                                 \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, # str, 0, CvarCmd_ ## name };   \
    void CvarCmd_ ## name(cvar_t* cvar)

#define CVAR_PARAM(name, str, var, flags)                   \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, #str, 0, CvarCmd_ ## name };    \
    void CvarCmd_ ## name(cvar_t* cvar)                     \
    {                                                       \
        if(cvar->value > 0)                                 \
            var |= flags;                                   \
        else                                                \
            var &= ~flags;                                  \
    }

#define NETCVAR(name, str)                                  \
    cvar_t name = { # name, # str, 1, NULL }

#define NETCVAR_CMD(name, str)                              \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, # str, 1, CvarCmd_ ## name };   \
    void CvarCmd_ ## name(cvar_t* cvar)

#define NETCVAR_PARAM(name, str, var, flags)                \
    void CvarCmd_ ## name(cvar_t* cvar);                    \
    cvar_t name = { # name, #str, 0, CvarCmd_ ## name };    \
    void CvarCmd_ ## name(cvar_t* cvar)                     \
    {                                                       \
        if(cvar->value > 0)                                 \
            var |= flags;                                   \
        else                                                \
            var &= ~flags;                                  \
    }

#define CVAR_EXTERNAL(name)                                 \
    extern cvar_t name

extern cvar_t*  cvarcap;

void Cvar_Init(void);
void Cvar_Register(cvar_t *variable);
void Cvar_Set(char *var_name, const char *value);
void Cvar_SetValue(char *var_name, float value);
char *Cvar_String(char *name);
void Cvar_AutoComplete(char *partial);
void Cvar_WriteToFile(FILE *file);
cvar_t *Cvar_Get(char *name);

//
// COMMANDS
//
typedef void (*cmd_t)(void);
int Cmd_GetArgc(void);
char *Cmd_GetArgv(int argv);
void Cmd_ExecuteCommand(char *buffer);
kbool Cmd_CompleteCommand(char *partial);
void Cmd_AddCommand(char *name, cmd_t function);
void Cmd_Init(void);

typedef struct
{
    fint_t  angle[2];
    fint_t  mouse[2];
    short   buttons;
    fint_t  msec;
    byte    heldtime[NUM_CTRLKEYS];
} ticcmd_t;

//
// CLIENT PACKET TYPES
//
typedef enum
{
    cp_ping         = 1,
    cp_say,
    cp_cmd,
    cp_msgserver,
    cp_changeweapon,
    NUMCLIENTPACKETS
} cl_packets_t;

//
// SERVER PACKET TYPES
//
typedef enum
{
    sp_ping         = 1,
    sp_clientinfo,
    sp_msg,
    sp_pmove,
    sp_weaponinfo,
    NUMSERVERPACKETS
} sv_packets_t;

//
// SERVER MESSAGES
//
typedef enum
{
    sm_full         = 1,
    NUMSERVERMESSAGES
} sv_messages_t;

//
// RAY TRACING
//
typedef enum
{
    TRT_NOHIT       = 0,
    TRT_SLOPE       = 1,
    TRT_WALL        = 2,
    TRT_EDGE        = 3,
    TRT_OBJECT      = 4,
    TRT_INTERACT    = 5
} tracetype_e;

typedef struct
{
    vec3_t      start;
    vec3_t      end;
    float       offset;
    float       width;
    float       yaw;
    plane_t     *pl;
    plane_t     *hitpl;
    vec3_t      normal;
    float       frac;
    tracetype_e type;
} trace_t;

trace_t Trace(vec3_t start, vec3_t end, plane_t *plane,
              float width, float offset, float yaw);

#endif