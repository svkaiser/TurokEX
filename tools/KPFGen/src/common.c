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

#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#endif

#include "types.h"
#include "common.h"
#include "pak.h"

#define ASCII_SLASH		47
#define ASCII_BACKSLASH 92

static unsigned long com_fileoffset = 0;
static FILE *com_file = NULL;

byte *cartfile = NULL;

#define TEXTBUFFER_SIZE 65536*1024
static char com_textbuffer[TEXTBUFFER_SIZE];

#ifdef _WIN32
extern HWND hwndWait;
extern HWND hwndLoadBar;
#endif

//
// Com_Printf
//

void Com_Printf(char* s, ...)
{
    char msg[1024];
    va_list v;
    
    va_start(v,s);
    vsprintf(msg,s,v);
    va_end(v);
#ifdef _WIN32
    ShowWindow(hwndWait, SW_HIDE);
    UpdateWindow(hwndWait);
    MessageBoxA(NULL, (LPCSTR)(msg), (LPCSTR)"Info", MB_OK | MB_ICONINFORMATION);
    ShowWindow(hwndWait, SW_SHOW);
    UpdateWindow(hwndWait);
#else
    printf("%s\n", msg);
#endif
}

//
// Com_Error
//

void Com_Error(char *fmt, ...)
{
    va_list	va;
    char buff[1024];
    
    va_start(va, fmt);
    vsprintf(buff, fmt, va);
    va_end(va);
#ifdef _WIN32
    ShowWindow(hwndWait, SW_HIDE);
    UpdateWindow(hwndWait);
    MessageBoxA(NULL, (LPCSTR)(buff), (LPCSTR)"Error", MB_OK | MB_ICONINFORMATION);
    ShowWindow(hwndWait, SW_SHOW);
    UpdateWindow(hwndWait);
#else
    Com_Printf("\n**************************\n");
    Com_Printf("ERROR: %s", buff);
    Com_Printf("**************************\n");
#endif

    Com_SetWriteFile("kpf_err.txt");
    Com_FPrintf(buff);
    Com_CloseFile();

    exit(1);
}

//
// Com_UpdateProgress
//

void Com_UpdateProgress(char *fmt, ...)
{
    va_list	va;
    char	buff[1024];

    va_start(va, fmt);
    vsprintf(buff, fmt, va);
    va_end(va);

#ifdef _WIN32
    SendMessage(hwndLoadBar, PBM_STEPIT, 0, 0);
    SetDlgItemTextA(hwndWait, IDC_PROGRESSTEXT, buff);
    UpdateWindow(hwndWait);
#else

#endif
}

//
// Com_Alloc
//

void* Com_Alloc(int size)
{
    void *ret = calloc(1, size);
    if(!ret)
        Com_Error("Com_Alloc: Out of memory");

    return ret; 
}

//
// Com_Free
//

void Com_Free(void **ptr)
{
    if(!*ptr)
        Com_Error("Com_Free: Tried to free NULL");

    free(*ptr);
    *ptr = NULL;
}

//
// Com_ReadFile
//

int Com_ReadFile(const char* name, byte** buffer)
{
    FILE *fp;

    errno = 0;
    
    if((fp = fopen(name, "r")))
    {
        size_t length;

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = Com_Alloc(length);
      
        if(fread(*buffer, 1, length, fp) == length)
        {
            fclose(fp);
            return length;
        }
        
        fclose(fp);
   }
    else
    {
        Com_Error("Couldn't open %s", name);
    }
   
   return -1;
}

//
// Com_ReadFile
//

int Com_ReadBinaryFile(const char* name, byte** buffer)
{
    FILE *fp;

    errno = 0;
    
    if((fp = fopen(name, "rb")))
    {
        size_t length;

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = Com_Alloc(length);
      
        if(fread(*buffer, 1, length, fp) == length)
        {
            fclose(fp);
            return length;
        }
        
        fclose(fp);
   }
   
   return -1;
}

//
// Com_SetWriteFile
//

dboolean Com_SetWriteFile(char const* name, ...)
{
    char filename[1024];
    va_list v;
    
    va_start(v, name);
    vsprintf(filename, name, v);
    va_end(v);

    if(!(com_file = fopen(filename, "w")))
        return 0;
   
    return 1;
}

//
// Com_SetWriteFile
//

dboolean Com_SetWriteBinaryFile(char const* name, ...)
{
    char filename[1024];
    va_list v;
    
    va_start(v, name);
    vsprintf(filename, name, v);
    va_end(v);

    if(!(com_file = fopen(filename, "wb")))
        return 0;
   
    return 1;
}

//
// Com_CloseFile
//

void Com_CloseFile(void)
{
    fclose(com_file);
}

//
// Com_StrcatClear
//

void Com_StrcatClear(void)
{
    memset(com_textbuffer, 0, TEXTBUFFER_SIZE);
}

//
// Com_Strcat
//

void Com_Strcat(const char *string, ...)
{
    char s[1024];
    va_list v;
    
    va_start(v, string);
    vsprintf(s, string, v);
    va_end(v);

    strcat(com_textbuffer, s);
}

//
// Com_StrcatAddToFile
//

void Com_StrcatAddToFile(const char *name)
{
    PK_AddFile(name, com_textbuffer, strlen(com_textbuffer), true);
}

//
// Com_GetFileBuffer
//

byte *Com_GetFileBuffer(byte *buffer)
{
    return buffer + com_fileoffset;
}

//
// Com_Read8
//

byte Com_Read8(byte* buffer)
{
    byte result;

    result = buffer[com_fileoffset++];

    return result;
}

//
// Com_Read16
//

short Com_Read16(byte* buffer)
{
    int result;

    result = Com_Read8(buffer);
    result |= Com_Read8(buffer) << 8;

    return result;
}

//
// Com_Read32
//

int Com_Read32(byte* buffer)
{
    int result;

    result = Com_Read8(buffer);
    result |= Com_Read8(buffer) << 8;
    result |= Com_Read8(buffer) << 16;
    result |= Com_Read8(buffer) << 24;

    return result;
}

//
// Com_Write8
//

void Com_Write8(byte value)
{
    fwrite(&value, 1, 1, com_file);
    com_fileoffset++;
}

//
// Com_Write16
//

void Com_Write16(short value)
{
    Com_Write8(value & 0xff);
    Com_Write8((value >> 8) & 0xff);
}

//
// Com_Write32
//

void Com_Write32(int value)
{
    Com_Write8(value & 0xff);
    Com_Write8((value >> 8) & 0xff);
    Com_Write8((value >> 16) & 0xff);
    Com_Write8((value >> 24) & 0xff);
}

//
// Com_FPrintf
//

void Com_FPrintf(char* s, ...)
{
    char msg[1024];
    va_list v;
    
    va_start(v,s);
    vsprintf(msg,s,v);
    va_end(v);

    fprintf(com_file, msg);
}

//
// Com_FileLength
//

long Com_FileLength(FILE *handle)
{ 
    long savedpos;
    long length;

    // save the current position in the file
    savedpos = ftell(handle);
    
    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}

//
// Com_FileExists
// Check if a wad file exists
//

int Com_FileExists(const char *filename)
{
    FILE *fstream;

    fstream = fopen(filename, "r");

    if(fstream != NULL)
    {
        fclose(fstream);
        return 1;
    }
    else
    {
        // If we can't open because the file is a directory, the 
        // "file" exists at least!

        if(errno == 21)
            return 2;
    }

    return 0;
}

//
// Com_GetCartFile
//

#ifdef _WIN32
byte *Com_GetCartFile(const char *filter, const char *title)
{
    char filepath[MAX_PATH];
    OPENFILENAMEA ofn;

    memset(&ofn, 0, sizeof(ofn));
    memset(filepath, 0, MAX_PATH);
    cartfile = NULL;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = (LPCSTR)filter;
    ofn.lpstrFile = (LPSTR)filepath;
    ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = (LPCSTR)title;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

    ShowWindow(hwndWait, SW_HIDE);
    UpdateWindow(hwndWait);

    if(GetOpenFileNameA(&ofn))
    {
		Com_ReadFile(filepath, &cartfile);
        ShowWindow(hwndWait, SW_SHOW);
        UpdateWindow(hwndWait);

		return cartfile;
    }

    ShowWindow(hwndWait, SW_SHOW);
    UpdateWindow(hwndWait);

    Com_Error("Canceled by user");

	return NULL;
}
#else
#error TODO: support Com_GetCartFile in *unix
#endif

//
// Com_CloseCartFile
//

void Com_CloseCartFile(void)
{
    Com_Free(&cartfile);
    cartfile = NULL;
}

//
// Com_StripExt
//

void Com_StripExt(char *name)
{
    char *search;

    search = name + strlen(name) - 1;

    while(*search != ASCII_BACKSLASH && *search != ASCII_SLASH
        && search != name)
    {
        if(*search == '.')
        {
            *search = '\0';
            return;
        }

        search--;
    }
}

void Com_StripPath(char *name)
{
    char *search;
    int len = 0;
    int pos = 0;
    int i = 0;

    len = strlen(name) - 1;
    pos = len + 1;

    for(search = name + len;
        *search != ASCII_BACKSLASH && *search != ASCII_SLASH; search--, pos--)
    {
        if(search == name)
        {
            return;
        }
    }

    if(pos <= 0)
    {
        return;
    }

    for(i = 0; pos < len+1; pos++, i++)
    {
        name[i] = name[pos];
    }

    name[i] = '\0';
}

//
// Com_GetExePath
//

const char *Com_GetExePath(void)
{
    static const char current_dir_dummy[] = {"."};
    static char *base;
    
    if(!base) // cache multiple requests
    {
        size_t len = strlen(*myargv);
        char *p = (base = Com_Alloc(len + 1)) + len - 1;
        
        strcpy(base, *myargv);
        while (p > base && *p!='/' && *p!='\\')
        {
            *p--=0;
        }
        
        if(*p=='/' || *p=='\\')
        {
            *p--=0;
        }

        if(strlen(base) < 2)
        {
            Com_Free(&base);
            base = Com_Alloc(1024);
            if(!_getcwd(base, 1024))
            {
                strcpy(base, current_dir_dummy);
            }
        }
    }
    
    return base;
}

//
// va
//

char *va(char *str, ...)
{
    va_list v;
    static char vastr[1024];
	
    va_start(v, str);
    vsprintf(vastr, str,v);
    va_end(v);
    
    return vastr;	
}

//
// Com_GetCartOffset
//

int Com_GetCartOffset(byte *buf, int count, int *size)
{
    if(size)
        *size = *(int*)(buf + count + 4) - *(int*)(buf + count);

    return *(int*)(buf + count);
}

//
// Com_GetCartData
//

byte *Com_GetCartData(byte *buf, int count, int *size)
{
    return &buf[Com_GetCartOffset(buf, count, size)];
}

//
// Com_UpdateCartData
//

void Com_UpdateCartData(byte* data, int entries, int next)
{
    *(int*)data = entries;
    *(int*)(data + 4 * *(int*)data + 4) = next;
}

//
// Com_SetCartHeader
//

void Com_SetCartHeader(byte* data, int size, int count)
{
    *(int*)data = size;
    *(int*)(data + 4) = count;
}

//
// Com_WriteCartData
//

void Com_WriteCartData(byte *buf, int count, char* s, ...)
{
    char msg[1024];
    va_list v;
    int size;
    byte *data;
    FILE *f;
    
    va_start(v, s);
    vsprintf(msg, s, v);
    va_end(v);

    f = fopen(msg, "wb");
    data = Com_GetCartData(buf, count, &size);
    fwrite(data, 1, size, f);
    fclose(f);
}

//
// Com_Swap16
//

int Com_Swap16(int x)
{
	return(((word)(x & 0xff) << 8) | ((word)x >> 8));
}

//
// Com_Swap32
//

dword Com_Swap32(unsigned int x)
{
    return(
        ((x & 0xff) << 24)          |
        (((x >> 8) & 0xff) << 16)   |
        (((x >> 16) & 0xff) << 8)   |
        ((x >> 24) & 0xff)
        );
}
