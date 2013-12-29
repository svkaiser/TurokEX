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

#ifndef EDITOR
#include "common.h"
#else
#include "editorCommon.h"
#endif
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
    for(int i = 0; i < MAX_HASH; i++) {
        hashlist[i].IsPointer(true);
    }
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
    hashlist[kexStr::Hash(key)].Push(new kexHashKey(key, value));
}

//
// kexKeyMap::Empty
//

void kexKeyMap::Empty(void) {
    for(int i = 0; i < MAX_HASH; i++) {
        hashlist[i].~kexPtrArray();
    }
}

//
// kexKeyMap::Find
//

kexHashKey *kexKeyMap::Find(const char *name) {
    kexHashKey *k;
    kexPtrArray<kexHashKey*> *keyList;

    keyList = &hashlist[kexStr::Hash(name)];

    for(unsigned int i = 0; i < keyList->Length(); i++) {
        k = keyList->GetData(i);
        if(!strcmp(name, k->GetName())) {
            return k;
        }
    }
    return NULL;
}

//
// kexKeyMap::GetFloat
//

bool kexKeyMap::GetFloat(const char *key, float &out, const float defaultValue) {
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key))) {
        return false;
    }

    out = (float)atof(k->GetString());
    return true;
}

//
// kexKeyMap::GetFloat
//

bool kexKeyMap::GetFloat(const kexStr &key, float &out, const float defaultValue) {
    return GetFloat(key.c_str(), out, defaultValue);
}

//
// kexKeyMap::GetInt
//

bool kexKeyMap::GetInt(const char *key, int &out, const int defaultValue) {
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key))) {
        return false;
    }

    out = atoi(k->GetString());
    return true;
}

//
// kexKeyMap::GetInt
//

bool kexKeyMap::GetInt(const kexStr &key, int &out, const int defaultValue) {
    return GetInt(key.c_str(), out, defaultValue);
}

//
// kexKeyMap::GetBool
//

bool kexKeyMap::GetBool(const char *key, bool &out, const bool defaultValue) {
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key))) {
        return false;
    }

    out = (atoi(k->GetString()) != 0);
    return true;
}

//
// kexKeyMap::GetBool
//

bool kexKeyMap::GetBool(const kexStr &key, bool &out, const bool defaultValue) {
    return GetBool(key.c_str(), out, defaultValue);
}

//
// kexKeyMap::GetString
//

bool kexKeyMap::GetString(const char *key, kexStr &out) {
    kexHashKey *k;

    if(!(k = Find(key))) {
        return false;
    }

    out = k->GetString();
    return true;
}

//
// kexKeyMap::GetString
//

bool kexKeyMap::GetString(const kexStr &key, kexStr &out) {
    return GetString(key.c_str(), out);
}

//
// kexKeyMap::GetVector
//

bool kexKeyMap::GetVector(const char *key, kexVec3 &out) {
    kexHashKey *k;

    out.Clear();

    if(!(k = Find(key))) {
        return false;
    }

    sscanf(k->GetString(), "%f %f %f", &out.x, &out.y, &out.z);
    return true;
}

//
// kexKeyMap::GetVector
//

bool kexKeyMap::GetVector(const kexStr &key, kexVec3 &out) {
    return GetVector(key.c_str(), out);
}

#ifndef EDITOR
#include "scriptAPI/scriptSystem.h"
//
// kexKeyMap::InitObject
//

void kexKeyMap::InitObject(void) {
    kexScriptManager::RegisterDataObject<kexKeyMap>("kKeyMap");
    scriptManager.RegisterMethod("kKeyMap", "bool GetFloat(const kStr &in, float &out, const float defaultValue = 0)",
        asMETHODPR(kexKeyMap, GetFloat,
        (const kexStr &key, float &out, const float defaultValue), bool));
    scriptManager.RegisterMethod("kKeyMap", "bool GetInt(const kStr &in, int &out, const int defaultValue = 0)",
        asMETHODPR(kexKeyMap, GetInt,
        (const kexStr &key, int &out, const int defaultValue), bool));
    scriptManager.RegisterMethod("kKeyMap", "bool GetBool(const kStr &in, bool &out, const bool defaultValue = 0)",
        asMETHODPR(kexKeyMap, GetBool,
        (const kexStr &key, bool &out, const bool defaultValue), bool));
    scriptManager.RegisterMethod("kKeyMap", "bool GetString(const kStr &in, kStr &out)",
        asMETHODPR(kexKeyMap, GetString,
        (const kexStr &key, kexStr &out), bool));
}
#endif
