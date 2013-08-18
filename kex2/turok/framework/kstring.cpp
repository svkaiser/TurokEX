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
//
// DESCRIPTION: String Class
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kstring.h"

//
// kexStr::Resize
//

void kexStr::Resize(int size, bool bKeepString) {

    if(size <= 0)
        return;

    int newsize = size + ((32 - (size & 31)) & 31);
    char *newbuffer = new char[newsize];

    if(bKeepString)
        strncpy(newbuffer, charPtr, length);

    if(charPtr != defaultBuffer)
        delete[] charPtr;

    charPtr = newbuffer;
    bufferLength = newsize;
}

//
// kexStr::IndexOf
//

int kexStr::IndexOf(const char *pattern) const {
    int patlen = strlen(pattern);
    int i = 0;
    int j = 0;
    int index = -1;

    while(i + j < Length()) {
        if(charPtr[i + j] == pattern[j]) {
            if(++j == patlen) {
                return i;
            }
        }
        else {
            i++;
            j = 0;
        }
    }

    return -1;
}

//
// kexStr::IndexOf
//

int kexStr::IndexOf(const char *string, const char *pattern) {
    int patlen = strlen(pattern);
    int i = 0;
    int j = 0;
    int index = -1;

    while(i + j < (int)strlen(string)) {
        if(string[i + j] == pattern[j]) {
            if(++j == patlen) {
                return i;
            }
        }
        else {
            i++;
            j = 0;
        }
    }

    return -1;
}

//
// kexStr::NormalizeSlashes
//

kexStr &kexStr::NormalizeSlashes(void) {
    for(int i = 0; i < length; i++) {
        if((charPtr[i] == '/' || charPtr[i] == '\\') && charPtr[i] != DIR_SEPARATOR) {
            charPtr[i] = DIR_SEPARATOR;
        }
    }
    
    return *this;
}

//
// kexStr::StripPath
//

kexStr &kexStr::StripPath(void) {
    int pos = 0;
    int i = 0;

    pos = length;

    for(i = length - 1; charPtr[i] != '\\' && charPtr[i] != '/'; i--, pos--) {
        if(pos <= 0) {
            return *this;
        }
    }
    length = length - pos;
    for(i = 0; i < length; i++) {
        charPtr[i] = charPtr[pos+i];
    }
    
    CheckSize(length, true);
    charPtr[length] = '\0';
    return *this;
}

//
// kexStr::StripExtension
//

kexStr &kexStr::StripExtension(void) {
    int pos = IndexOf(".");
    
    if(pos == -1)
        return *this;
    
    length = pos;
    CheckSize(length, true);
    charPtr[length] = '\0';
    
    return *this;
}

//
// kexStr::Hash
//

int kexStr::Hash(void) {
    unsigned int hash   = 1315423911;
    unsigned int i      = 0;
    char *str           = (char*)charPtr;

    for(i = 0; i < (unsigned int)Length()-1 && *str != '\0'; str++, i++) {
        hash ^= ((hash << 5) + toupper((int)*str) + (hash >> 2));
    }

    return hash & (MAX_HASH-1);
}

//
// kexStr::Substr
//

kexStr kexStr::Substr(int start, int len) const {
    kexStr str;
    int l = Length();
    
    if(l <= 0 || start >= l)
        return str;
        
    if(start + len >= l)
        len = l - start;
        
    return str.Concat((const char*)&charPtr[start], len);
}

//
// kexStr::ToUpper
//

kexStr &kexStr::ToUpper(void) {
    char c;
    for(int i = 0; i < length; i++) {
        c = charPtr[i];
        if(c >= 'a' && c <= 'z')
            c -= 'a'-'A';
        charPtr[i] = c;
    }
    
    return *this;
}

//
// kexStr::ToLower
//

kexStr &kexStr::ToLower(void) {
    char c;
    for(int i = 0; i < length; i++) {
        c = charPtr[i];
        if(c >= 'A' && c <= 'Z')
            c += 32;
        charPtr[i] = c;
    }
    
    return *this;
}

//
// kexStr::Compare
//

bool kexStr::Compare(const kexStr &a, const kexStr &b) {
    const char *s1 = a.charPtr;
    const char *s2 = b.charPtr;
    
    while(*s1 && *s2) {
        if(*s1 != *s2)
            return (*s2 - *s1) != 0;
        s1++;
        s2++;
    }
    if(*s1 != *s2)
        return (*s2 - *s1) != 0;
        
    return false;
}
