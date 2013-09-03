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
// DESCRIPTION: Hashkey lookups
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kstring.h"
#include "keymap.h"

//
// kexHashKey::kexHashKey
//

kexHashKey::kexHashKey(const char *key, const char *value) {
    this->key = key;
    this->value = value;
}

//
// kexHashKey::~kexHashKey
//

kexHashKey::~kexHashKey(void) {
}

//
// kexKeyMap::kexKeyMap
//

kexKeyMap::kexKeyMap(void) {
    for(int i = 0; i < MAX_HASH; i++)
        hashlist[i].IsPointer(true);
}

//
// kexKeyMap::~kexKeyMap
//

kexKeyMap::~kexKeyMap(void) {
    Empty();
}

//
// kexKeyMap::Add
//

void kexKeyMap::Add(const char *key, const char *value) {
    hashlist[common.HashFileName(key)].Push(new kexHashKey(key, value));
}

//
// kexKeyMap::Empty
//

void kexKeyMap::Empty(void) {
    for(int i = 0; i < MAX_HASH; i++)
        hashlist[i].~kexPtrArray();
}

//
// kexKeyMap::Find
//

kexHashKey *kexKeyMap::Find(const char *name) {
    kexHashKey *k;
    kexPtrArray<kexHashKey*> *keyList;

    keyList = &hashlist[common.HashFileName(name)];

    for(unsigned int i = 0; i < keyList->Length(); i++) {
        k = keyList->GetData(i);
        if(!strcmp(name, k->GetName())) {
            return k;
        }
    }
    return NULL;
}
