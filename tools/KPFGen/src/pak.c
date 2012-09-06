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
#include "zlib.h"

typedef struct pkfile_s
{
    struct pkfile_s* prev;
    struct pkfile_s* next;
    int magic;
    int version;
    short type;
    short date1;
    short date2;
    int crc;
    unsigned long cmpsize;
    int size;
    short namelen;
    short extralen;
    char *name;
    byte *data;
    int offset;
} pkfile_t;

typedef struct pklist_s
{
    struct pklist_s* prev;
    struct pklist_s* next;
    int magic;
    short version_1;
    short version_2;
    short bitflag;
    short type;
    short date1;
    short date2;
    int crc;
    unsigned long cmpsize;
    int size;
    short namelen;
    short extralen;
    short commentlen;
    short diskstart;
    short attribute_internal;
    int attribute_external;
    int offset;
    char *name;
} pklist_t;

static int pkfile_offset = 0;
static int pkfile_count = 0;
static int pklist_size = 0;

static pkfile_t pkfile_cap;
static pklist_t pklist_cap;

//
// PK_WriteFile
//

void PK_WriteFile(const char *name)
{
    pkfile_t *pkfile;
    pklist_t *pklist;
    int i;

    Com_SetWriteBinaryFile("%s", name);
    Com_UpdateProgress("Writing pak file...");

    for(pkfile = pkfile_cap.next; pkfile != &pkfile_cap; pkfile = pkfile->next)
    {
        Com_Write32(pkfile->magic);
        Com_Write32(pkfile->version);
        Com_Write16(pkfile->type);
        Com_Write16(pkfile->date1);
        Com_Write16(pkfile->date2);
        Com_Write32(pkfile->crc);
        Com_Write32(pkfile->cmpsize);
        Com_Write32(pkfile->size);
        Com_Write16(pkfile->namelen);
        Com_Write16(pkfile->extralen);

        for(i = 0; i < pkfile->namelen; i++)
        {
            Com_Write8(pkfile->name[i]);
        }

        for(i = 0; i < (int)pkfile->cmpsize; i++)
        {
            Com_Write8(pkfile->data[i]);
        }
    }

    for(pklist = pklist_cap.next; pklist != &pklist_cap; pklist = pklist->next)
    {
        Com_Write32(pklist->magic);
        Com_Write16(pklist->version_1);
        Com_Write16(pklist->version_2);
        Com_Write16(pklist->bitflag);
        Com_Write16(pklist->type);
        Com_Write16(pklist->date1);
        Com_Write16(pklist->date2);
        Com_Write32(pklist->crc);
        Com_Write32(pklist->cmpsize);
        Com_Write32(pklist->size);
        Com_Write16(pklist->namelen);
        Com_Write16(pklist->extralen);
        Com_Write16(pklist->commentlen);
        Com_Write16(pklist->diskstart);
        Com_Write16(pklist->attribute_internal);
        Com_Write32(pklist->attribute_external);
        Com_Write32(pklist->offset);

        for(i = 0; i < pklist->namelen; i++)
        {
            Com_Write8(pklist->name[i]);
        }
    }

    Com_Write32(0x6054b50);
    Com_Write16(0);
    Com_Write16(0);
    Com_Write16(pkfile_count);
    Com_Write16(pkfile_count);
    Com_Write32(pklist_size);
    Com_Write32(pkfile_offset);
    Com_Write16(0);

    Com_CloseFile();
    Com_Printf("Wrote %s", name);
}

//
// PK_AddFileToCentralDirectory
//

static void PK_AddFileToCentralDirectory(pkfile_t *file)
{
    pklist_t *pklist;

    pklist = (pklist_t*)Com_Alloc(sizeof(pklist_t));

    pklist->magic = 0x2014b50;
    pklist->version_1 = 0x3f;
    pklist->version_2 = file->version;
    pklist->bitflag = 0;
    pklist->type = file->type;
    pklist->date1 = file->date1;
    pklist->date2 = file->date2;
    pklist->crc = file->crc;
    pklist->cmpsize = file->cmpsize;
    pklist->size = file->size;
    pklist->namelen = file->namelen;
    pklist->name = file->name;
    pklist->extralen = 0;
    pklist->commentlen = 0;
    pklist->diskstart = 0;
    pklist->attribute_internal = 0;
    pklist->attribute_external = 0x20;
    pklist->offset = file->offset;

    pklist_cap.prev->next = pklist;
    pklist->next = &pklist_cap;
    pklist->prev = pklist_cap.prev;
    pklist_cap.prev = pklist;

    pklist_size += (46 + pklist->namelen);
}

//
// PK_AddFile
//

void PK_AddFile(const char *name, byte *data, int size, dboolean compress)
{
    pkfile_t *pkfile;
    z_stream zstream;

    Com_UpdateProgress("Adding %s...", name);

    pkfile = (pkfile_t*)Com_Alloc(sizeof(pkfile_t));

    pkfile->magic = 0x4034B50;
    pkfile->version = 0x14;
    pkfile->type = compress ? Z_DEFLATED : 0;
    pkfile->date1 = 0;
    pkfile->date2 = 0;
    pkfile->crc = crc32(0, data, size);
    pkfile->namelen = strlen(name);
    pkfile->name = _strdup(name);
    pkfile->extralen = 0;
    pkfile->size = size;
    pkfile->offset = pkfile_offset;
    pkfile->data = (byte*)Com_Alloc(size);

    if(compress)
    {
        zstream.zalloc = Z_NULL;
        zstream.zfree = Z_NULL;
        zstream.opaque = Z_NULL;

        deflateInit2(&zstream, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);

        zstream.avail_in = size;
        zstream.next_in = data;
        zstream.avail_out = size;
        zstream.next_out = pkfile->data;

        deflate(&zstream, Z_FINISH);

        pkfile->cmpsize = size - zstream.avail_out;

        deflateEnd(&zstream);
    }
    else
    {
        pkfile->cmpsize = size;
        memcpy(pkfile->data, data, size);
    }

    pkfile_cap.prev->next = pkfile;
    pkfile->next = &pkfile_cap;
    pkfile->prev = pkfile_cap.prev;
    pkfile_cap.prev = pkfile;

    PK_AddFileToCentralDirectory(pkfile);

    pkfile_offset += (30 + pkfile->namelen + pkfile->cmpsize);
    pkfile_count++;
}

//
// PK_AddFolder
//

void PK_AddFolder(const char *name)
{
    pkfile_t *pkfile;

    pkfile = (pkfile_t*)Com_Alloc(sizeof(pkfile_t));

    pkfile->magic = 0x4034B50;
    pkfile->version = 0x14;
    pkfile->type = 0;
    pkfile->date1 = 0;
    pkfile->date2 = 0;
    pkfile->crc = 0;
    pkfile->namelen = strlen(name);
    pkfile->name = _strdup(name);
    pkfile->extralen = 0;
    pkfile->size = 0;
    pkfile->offset = pkfile_offset;
    pkfile->data = NULL;
    pkfile->cmpsize = 0;

    pkfile_cap.prev->next = pkfile;
    pkfile->next = &pkfile_cap;
    pkfile->prev = pkfile_cap.prev;
    pkfile_cap.prev = pkfile;

    PK_AddFileToCentralDirectory(pkfile);

    pkfile_offset += (30 + pkfile->namelen + pkfile->cmpsize);
    pkfile_count++;
}

//
// PK_Init
//

void PK_Init(void)
{
    pkfile_cap.prev = pkfile_cap.next  = &pkfile_cap;
    pklist_cap.prev = pklist_cap.next  = &pklist_cap;
}
