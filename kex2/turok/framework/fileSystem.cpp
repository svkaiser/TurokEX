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
#include "filesystem.h"
#include "zone.h"
#include "unzip.h"

#define FILE_MAX_HASH_SIZE  32768

kexCvar cvarBasePath("kf_basepath", CVF_STRING|CVF_CONFIG, "", "Base file path to look for files");

kexFileSystem fileSystem;

//
// kexFileSystem::kexFileSystem
//

kexFileSystem::kexFileSystem() {
}

//
// kexFileSystem::~kexFileSystem
//

kexFileSystem::~kexFileSystem() {
}

//
// kexFileSystem::BasePath
//

const char *kexFileSystem::BasePath(void) {
    static const char dummyDirectory[] = {"."};
    // cache multiple requests
    if(!base) {
        size_t len = strlen(*myargv);
        char *p = (base = (char*)Z_Malloc(len + 1, PU_STATIC, 0)) + len - 1;
        
        strcpy(base, *myargv);
        while (p > base && *p!='/' && *p!='\\') {
            *p--=0;
        }
        
        if(*p=='/' || *p=='\\') {
            *p--=0;
        }

        if(strlen(base) < 2) {
            Z_Free(base);
            base = (char*)Z_Malloc(1024, PU_STATIC, 0);
            if(!getcwd(base, 1024)) {
                strcpy(base, dummyDirectory);
            }
        }
    }
    
    return base;
}

//
// kexFileSystem::Shutdown
//

void kexFileSystem::Shutdown(void) {
    kpf_t *pack;

    for(pack = root; pack; pack = pack->next) {
        unzClose(pack->filehandle);
    }

    Z_FreeTags(PU_FILE, PU_FILE);
}

//
// kexFileSystem::HashFileName
//

long kexFileSystem::HashFileName(const char *fname, int hashSize) const {
    unsigned int hash   = 1315423911;
    char *str           = (char*)fname;

    for(unsigned int i = 0; i < strlen(fname)-1 && *str != '\0'; str++, i++) {
        hash ^= ((hash << 5) + toupper((int)*str) + (hash >> 2));
    }

    return hash & (hashSize-1);
}

//
// kexFileSystem::LoadZipFile
//

void kexFileSystem::LoadZipFile(const char *file) {
    unzFile uf;
    unz_global_info gi;
    unz_file_info fi;
    char filename[MAX_FILEPATH];
    kpf_t *pack;
    unsigned int i;
    unsigned int entries;
    long hash;
    char *filepath;

    filepath = kva("%s\\%s", cvarBasePath.GetValue(), file);
    common.Printf("KF_LoadZipFile: Loading %s\n", filepath);

    // open zip file
    if(!(uf = unzOpen(filepath)))
        common.Error("KF_LoadZipFile: Unable to find %s", filepath);

    // get info on zip file
    if(unzGetGlobalInfo(uf, &gi) != UNZ_OK)
        return;

    // allocate new pack file
    pack = (kpf_t*)Z_Calloc(sizeof(kpf_t), PU_FILE, 0);
    pack->filehandle = (unzFile*)uf;
    strcpy(pack->filename, filepath);
    pack->numfiles = gi.number_entry;
    pack->next = root;
    root = pack;

    // point to start of zip files
    unzGoToFirstFile(pack->filehandle);

    // setup hash entires
    for(entries = 1; entries < FILE_MAX_HASH_SIZE; entries <<= 1) {
        if(entries > pack->numfiles)
            break;
    }

    // allocate file/hash list
    pack->hashentries = entries;
    pack->files = (file_t*)Z_Calloc(sizeof(file_t) * pack->numfiles, PU_FILE, 0);
    pack->hashes = (file_t***)Z_Calloc(sizeof(file_t**) * pack->hashentries, PU_FILE, 0);
    pack->hashcount = (unsigned int*)Z_Calloc(sizeof(int) * pack->hashentries, PU_FILE, 0);

    // fill in file lookup lists
    for(i = 0; i < pack->numfiles; i++) {
        file_t *fp;

        if(unzGetCurrentFileInfo(uf, &fi, filename, sizeof(filename),
            NULL, 0, NULL, 0) != UNZ_OK) {
            break;
        }

        fp = &pack->files[i];

        unzGetCurrentFileInfoPosition(pack->filehandle, &fp->position);
        strcpy(fp->name, filename);
        fp->info = fi;

        // get hash number
        hash = HashFileName(filename, pack->hashentries);

        // resize hash table if needed
        pack->hashes[hash] = (file_t**)Z_Realloc(
            pack->hashes[hash],
            sizeof(file_t*) * (pack->hashcount[hash]+1), PU_FILE, 0);

        pack->hashes[hash][pack->hashcount[hash]++] = fp;

        unzGoToNextFile(pack->filehandle);
    }
}

//
// kexFileSystem::OpenFile
//

int kexFileSystem::OpenFile(const char *filename, byte **data, int tag) const {
    long hash;
    for(kpf_t *pack = root; pack; pack = pack->next) {
        hash = HashFileName(filename, pack->hashentries);

        if(pack->hashes[hash]) {
            unsigned int i;

            for(i = 0; i < pack->hashcount[hash]; i++) {
                file_t *file = pack->hashes[hash][i];

                if(!strcmp(file->name, filename)) {
                    if(!file->cache) {
                        Z_Calloc(file->info.uncompressed_size+1, tag, &file->cache);
                        unzSetCurrentFileInfoPosition(pack->filehandle, file->position);
                        unzOpenCurrentFile(pack->filehandle);
                        unzReadCurrentFile(pack->filehandle, file->cache,
                            file->info.uncompressed_size);
                        unzCloseCurrentFile(pack->filehandle);
                    }

                    *data = (byte*)file->cache;
                    return file->info.uncompressed_size;
                }
            }
        }
    }

    return 0;
}

//
// kexFileSystem::GetMatchingFiles
//

kexArray<kexStr*> *kexFileSystem::GetMatchingFiles(const char *search) {
    kexArray<kexStr*> *strlist = new kexArray<kexStr*>(true);

    for(kpf_t *pack = root; pack; pack = pack->next) {
        for(unsigned int i = 0; i < pack->numfiles; i++) {
            file_t *file = &pack->files[i];

            if(strstr(file->name, search)) {
                if(kexStr::IndexOf(file->name, ".") == -1)
                    continue;

                strlist->Push(new kexStr(file->name));
            }
        }
    }

    return strlist;
}

//
// kexFileSystem::ReadExternalTextFile
//

int kexFileSystem::ReadExternalTextFile(const char *name, byte **buffer) const {
    static char filepath[1024];
    FILE *fp;

    errno = 0;

    sprintf(filepath, "%s\\%s", cvarBasePath.GetValue(), name);
    if((fp = fopen(filepath, "rb"))) {
        size_t length;

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = (byte*)Z_Calloc(length+1, PU_FILE, 0);

        if(fread(*buffer, 1, length, fp) == length) {
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

static void FCmd_LoadFile(void) {
    byte *data;
    int size;

    if(command.GetArgc() < 2)
        return;

    size = fileSystem.OpenFile(command.GetArgv(1), (byte**)&data, PU_STATIC);

    if(size) {
        common.Printf("loaded %s\nsize = %i\ndata = 0x%p\n",
            command.GetArgv(1), size, data);

        Z_Free(data);
    }
    else {
        common.Warning("couldn't open %s\n", command.GetArgv(1));
    }
}

//
// kexFileSystem::Init
//

void kexFileSystem::Init(void) {
    command.Add("loadfile", FCmd_LoadFile);

    if(!strlen(cvarBasePath.GetValue()))
        cvarBasePath.Set(BasePath());

    LoadZipFile("game.kpf");
    common.Printf("File System Initialized\n");
}
