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
// DESCRIPTION: File System
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kernel.h"
#include "zone.h"
#include "unzip.h"

#define FILE_MAX_HASH_SIZE  32768

CVAR(kf_basepath, );
CVAR_EXTERNAL(developer);

typedef struct
{
    char            name[MAX_FILEPATH];
    unsigned long   position;
    unz_file_info   info;
    void*           cache;
} file_t;

typedef struct kpf_s
{
    unzFile         *filehandle;
    unsigned int    numfiles;
    char            filename[MAX_FILEPATH];
    file_t          *files;
    file_t          ***hashes;
    unsigned int    *hashcount;
    unsigned int    hashentries;
    struct kpf_s    *next;
} kpf_t;

kpf_t *kpf_rootfile;

//
// KF_BasePath
//

const char *KF_BasePath(void)
{
    static const char current_dir_dummy[] = {"."};
    static char *base;
    
    if(!base) // cache multiple requests
    {
        size_t len = strlen(*myargv);
        char *p = (base = Z_Malloc(len + 1, PU_STATIC, 0)) + len - 1;
        
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
            Z_Free(base);
            base = Z_Malloc(1024, PU_STATIC, 0);
            if(!getcwd(base, 1024))
            {
                strcpy(base, current_dir_dummy);
            }
        }
    }
    
    return base;
}

//
// KF_Shutdown
//

void KF_Shutdown(void)
{
    kpf_t *pack;

    for(pack = kpf_rootfile; pack; pack = pack->next)
    {
        unzClose(pack->filehandle);
    }

    Z_FreeTags(PU_FILE, PU_FILE);
}

//
// KF_HashFileName
//

static long KF_HashFileName(const char *fname, int hashSize)
{
    unsigned int hash   = 1315423911;
    unsigned int i      = 0;
    char *str           = (char*)fname;

    for(i = 0; i < strlen(fname)-1 && *str != '\0'; str++, i++)
    {
        hash ^= ((hash << 5) + toupper((int)*str) + (hash >> 2));
    }

    return hash & (hashSize-1);
}

//
// KF_LoadZipFile
//

void KF_LoadZipFile(const char *file)
{
    unzFile uf;
    unz_global_info gi;
    unz_file_info fi;
    char filename[MAX_FILEPATH];
    kpf_t *pack;
    unsigned int i;
    unsigned int entries;
    long hash;
    char *filepath;

    filepath = kva("%s\\%s", kf_basepath.string, file);
    Com_Printf("KF_LoadZipFile: Loading %s\n", filepath);

    // open zip file
    uf = unzOpen(filepath);

    // get info on zip file
    if(unzGetGlobalInfo(uf, &gi) != UNZ_OK)
    {
        return;
    }

    // allocate new pack file
    pack = (kpf_t*)Z_Calloc(sizeof(kpf_t), PU_FILE, 0);
    pack->filehandle = uf;
    strcpy(pack->filename, filepath);
    pack->numfiles = gi.number_entry;
    pack->next = kpf_rootfile;
    kpf_rootfile = pack;

    // point to start of zip files
    unzGoToFirstFile(pack->filehandle);

    // setup hash entires
    for(entries = 1; entries < FILE_MAX_HASH_SIZE; entries <<= 1)
    {
        if(entries > pack->numfiles)
        {
            break;
        }
    }

    // allocate file/hash list
    pack->hashentries = entries;
    pack->files = (file_t*)Z_Calloc(sizeof(file_t) * pack->numfiles, PU_FILE, 0);
    pack->hashes = (file_t***)Z_Calloc(sizeof(file_t**) * pack->hashentries, PU_FILE, 0);
    pack->hashcount = (unsigned int*)Z_Calloc(sizeof(int) * pack->hashentries, PU_FILE, 0);

    // fill in file lookup lists
    for(i = 0; i < pack->numfiles; i++)
    {
        file_t *fp;

        if(unzGetCurrentFileInfo(uf, &fi, filename, sizeof(filename),
            NULL, 0, NULL, 0) != UNZ_OK)
        {
            break;
        }

        fp = &pack->files[i];

        unzGetCurrentFileInfoPosition(pack->filehandle, &fp->position);
        strcpy(fp->name, filename);
        fp->info = fi;

        // get hash number
        hash = KF_HashFileName(filename, pack->hashentries);

        // resize hash table if needed
        pack->hashes[hash] = (file_t**)Z_Realloc(
            pack->hashes[hash],
            sizeof(file_t*) * (pack->hashcount[hash]+1), PU_FILE, 0);

        pack->hashes[hash][pack->hashcount[hash]++] = fp;

        unzGoToNextFile(pack->filehandle);
    }
}

//
// KF_OpenFileCache
//

int KF_OpenFileCache(const char *filename, byte **data, int tag)
{
    kpf_t *pack;
    long hash;

    for(pack = kpf_rootfile; pack; pack = pack->next)
    {
        hash = KF_HashFileName(filename, pack->hashentries);

        if(pack->hashes[hash])
        {
            unsigned int i;

            for(i = 0; i < pack->hashcount[hash]; i++)
            {
                file_t *file = pack->hashes[hash][i];

                if(!strcmp(file->name, filename))
                {
                    int time;

                    if(developer.value)
                    {
                        time = Sys_GetMilliseconds();
                    }

                    if(!file->cache)
                    {
                        Z_Malloc(file->info.uncompressed_size, tag, &file->cache);
                        unzSetCurrentFileInfoPosition(pack->filehandle, file->position);
                        unzOpenCurrentFile(pack->filehandle);
                        unzReadCurrentFile(pack->filehandle, file->cache,
                            file->info.uncompressed_size);
                        unzCloseCurrentFile(pack->filehandle);
                    }

                    *data = file->cache;
                    return file->info.uncompressed_size;
                }
            }
        }
    }

    return 0;
}

//
// KF_ReadTextFile
//

int KF_ReadTextFile(const char *name, byte **buffer)
{
    FILE *fp;

    errno = 0;
    
    if((fp = fopen(kva("%s\\%s", kf_basepath.string, name), "rb")))
    {
        size_t length;

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = Z_Calloc(length+1, PU_FILE, 0);

        if(fread(*buffer, 1, length, fp) == length)
        {
            fclose(fp);
            return length;
        }
        
        Z_Free(*buffer);
        *buffer = NULL;
        fclose(fp);
   }
   
   return -1;
}

//
// FCmd_LoadFile
//

static void FCmd_LoadFile(void)
{
    byte *data;
    int size;

    if(Cmd_GetArgc() < 2)
    {
        return;
    }

    size = KF_OpenFileCache(Cmd_GetArgv(1), &data, PU_STATIC);

    if(size)
    {
        Com_Printf("loaded %s\nsize = %i\ndata = 0x%p\n",
            Cmd_GetArgv(1), size, data);

        Z_Free(data);
    }
    else
    {
        Com_Warning("couldn't open %s\n", Cmd_GetArgv(1));
    }
}

//
// KF_Init
//

void KF_Init(void)
{
    Cvar_Register(&kf_basepath);
    Cmd_AddCommand("loadfile", FCmd_LoadFile);

    if(!strlen(kf_basepath.string))
    {
        Cvar_Set(kf_basepath.name, KF_BasePath());
    }
}
