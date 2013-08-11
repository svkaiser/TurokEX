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
// DESCRIPTION: Common Stuff
//
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include <stdio.h>

#include "common.h"
#include "zone.h"
#include "kernel.h"
#include "filesystem.h"

int     myargc;
char**  myargv;

kexCommon common;

//
// kexCommon::Printf
//

void kexCommon::Printf(const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);

#ifdef _WIN32
    Sys_Printf(buffer);
#endif

    Con_Printf(COLOR_WHITE, buffer);
    printf(buffer);
}

//
// kexCommon::CPrintf
//

void kexCommon::CPrintf(rcolor color, const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);

#ifdef _WIN32
    Sys_Printf(buffer);
#endif

    Con_Printf(color, buffer);
    printf(buffer);
}

//
// kexCommon::Warning
//

void kexCommon::Warning(const char *string, ...) {
    va_list	va;
    
    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);

#ifdef _WIN32
    Sys_Printf(buffer);
#endif

    Con_Printf(COLOR_YELLOW, buffer);
    printf(buffer);
}

//
// kexCommon::DPrintf
//

void kexCommon::DPrintf(const char *string, ...) {
    if(cvarDeveloper.GetBool()) {
        static char buffer[1024];
        va_list	va;
        
        va_start(va, string);
        vsprintf(buffer, string, va);
        va_end(va);

        CPrintf(RGBA(0xE0, 0xE0, 0xE0, 0xff), buffer);
    }
}

//
// kexCommon::Error
//

void kexCommon::Error(const char* string, ...) {
    va_list	va;

    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);
    
    fprintf(stderr, "Error - %s\n", buffer);
    fflush(stderr);

#ifdef _WIN32
    Sys_Error(buffer);
#endif

    printf(buffer);

    exit(0);    // just in case...
}

//
// kexCommon::CheckParam
//

int kexCommon::CheckParam(const char *check) {
    for(int i = 1; i < myargc; i++) {
        if(!stricmp(check, myargv[i]))
            return i;
    }
    return 0;
}

//
// kexCommon::Shutdown
//

void kexCommon::Shutdown(void) {
    Z_FreeTags(PU_STATIC, PU_STATIC);
}

//
// kexCommon::ReadConfigFile
//

void kexCommon::ReadConfigFile(const char *file) {
    char *buffer;
    int len;

    len = fileSystem.ReadExternalTextFile(file, (byte**)(&buffer));

    if(len == -1) {
        Warning("Warning: %s not found\n", file);
        return;
    }

    command.Execute(buffer);
    Z_Free(buffer);
}

//
// kexCommon::WriteConfigFile
//
extern kexCvar cvarBasePath;

void kexCommon::WriteConfigFile(void) {
    FILE * f = fopen(kva("%s\\config.cfg", cvarBasePath.GetValue()), "w");
    if(f) {
        inputKey.WriteBindings(f);
        cvarManager.WriteToFile(f);
        fclose(f);
    }
}

//
// kexCommon::HashFileName
//

unsigned int kexCommon::HashFileName(const char *name) {
    unsigned int hash   = 1315423911;
    unsigned int i      = 0;
    char *str           = (char*)name;

    for(i = 0; i < strlen(name)-1 && *str != '\0'; str++, i++) {
        hash ^= ((hash << 5) + toupper((int)*str) + (hash >> 2));
    }

    return hash & (MAX_HASH-1);
}

//
// kexCommon::NormalizeSlashes
//
// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// killough 11/98: rewritten
//

void kexCommon::NormalizeSlashes(char *str) {
    char *p;
   
    // Convert all slashes/backslashes to DIR_SEPARATOR
    for(p = str; *p; p++) {
        if((*p == '/' || *p == '\\') && *p != DIR_SEPARATOR) {
            *p = DIR_SEPARATOR;
        }
    }

    // Collapse multiple slashes
    for(p = str; (*str++ = *p); ) {
        if(*p++ == DIR_SEPARATOR) {
            while(*p == DIR_SEPARATOR) p++;
        }
    }
}

//
// kexCommon::StripPath
//

void kexCommon::StripPath(char *name) {
    char *search;
    int len = 0;
    int pos = 0;
    int i = 0;

    len = strlen(name) - 1;
    pos = len + 1;

    for(search = name + len;
        *search != ASCII_BACKSLASH && *search != ASCII_SLASH; search--, pos--) {
        if(search == name) {
            return;
        }
    }

    if(pos <= 0) {
        return;
    }

    for(i = 0; pos < len+1; pos++, i++) {
        name[i] = name[pos];
    }

    name[i] = '\0';
}

//
// kexCommon::StripExt
//

void kexCommon::StripExt(char *name) {
    char *search;

    search = name + strlen(name) - 1;

    while(*search != ASCII_BACKSLASH && *search != ASCII_SLASH
        && search != name) {
        if(*search == '.') {
            *search = '\0';
            return;
        }

        search--;
    }
}

//
// fcmp
//

kbool fcmp(float f1, float f2) {
    float precision = 0.00001f;
    if(((f1 - precision) < f2) && 
        ((f1 + precision) > f2)) {
        return true;
    }
    else {
        return false;
    }
}

//
// kva
//

// TODO - this seems really dodgy as fuck...
char *kva(char *str, ...) {
    va_list v;
    static char vastr[1024];
	
    va_start(v, str);
    vsprintf(vastr, str,v);
    va_end(v);
    
    return vastr;	
}
