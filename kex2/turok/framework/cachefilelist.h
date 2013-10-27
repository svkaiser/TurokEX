// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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

#ifndef __CACHEFILELIST_H__
#define __CACHEFILELIST_H__

template<class type>
class kexFileCacheList {
public:
    void                Add(type *o);
    type                *Create(const char *filename, int zoneTag = PU_STATIC);
    type                *Find(const char *name) const;
    type                *hashlist[MAX_HASH];
};

//
// kexFileCacheList::Add
//
template<class type>
void kexFileCacheList<type>::Add(type *o) {
    unsigned int hash;

    // add to hash for future reference
    hash = common.HashFileName(o->filePath);
    o->next = hashlist[hash];
    hashlist[hash] = o;
}

//
// kexFileCacheList::Create
//
template<class type>
type *kexFileCacheList<type>::Create(const char *filename, int zoneTag) {
    type *o = (type*)Z_Calloc(sizeof(type), zoneTag, 0);
    strncpy(o->filePath, filename, MAX_FILEPATH);

    return o;
}

//
// kexFileCacheList::Find
//
template<class type>
type *kexFileCacheList<type>::Find(const char *name) const {
    type *t;
    unsigned int hash;

    hash = common.HashFileName(name);

    for(t = hashlist[hash]; t; t = t->next) {
        if(!strcmp(name, t->filePath)) {
            return t;
        }
    }

    return NULL;
}

#endif
