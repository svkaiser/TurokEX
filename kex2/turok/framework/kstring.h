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

#ifndef __KSTRING_H__
#define __KSTRING_H__

#include <string.h>
#include "common.h"
#include "keywds.h"

class kexStr {
public:
                        kexStr(void);
                        kexStr(const char *string);
                        kexStr(const kexStr &string);
                        ~kexStr(void);

    void                CheckSize(int size, bool bKeepString);
    int                 IndexOf(const char *pattern) const;
    kexStr              &Concat(const char *string);
    kexStr              &Concat(const char *string, int len);
    kexStr              &NormalizeSlashes(void);
    kexStr              &StripPath(void);
    kexStr              &StripExtension(void);
    kexStr              &Copy(const kexStr &src, int len);
    kexStr              &Copy(const kexStr &src);
    kexStr              &ToUpper(void);
    kexStr              &ToLower(void);
    int                 Hash(void);
    kexStr              Substr(int start, int len) const;
    int                 Length(void) const { return length; }
    const char          *c_str(void) const { return charPtr; }

    void                operator=(const kexStr &str);
    void                operator=(const char *str);
    void                operator=(const bool b);
    kexStr              operator+(const kexStr &str);
    kexStr              operator+(const char *str);
    kexStr              operator+(const bool b);
    kexStr              &operator+=(const kexStr &str);
    kexStr              &operator+=(const char *str);
    kexStr              &operator+=(const bool b);

    operator            const char *(void) const { return c_str(); }
    operator            const char *(void) { return c_str(); }
    
    static bool         Compare(const kexStr &a, const kexStr &b);
    static int          IndexOf(const char *string, const char *pattern);

private:
    void                Resize(int size, bool bKeepString);
    void                CopyNew(const char *string, int len);

protected:
    void                Init(void);

    static const int    STRING_DEFAULT_SIZE = 32;

    char                *charPtr;
    char                defaultBuffer[STRING_DEFAULT_SIZE];
    int                 length;
    int                 bufferLength;
};

//
// kexStr::Init
//

d_inline void kexStr::Init(void) {
    length = 0;
    bufferLength = STRING_DEFAULT_SIZE;
    charPtr = defaultBuffer;
    charPtr[0] = '\0';
}

//
// kexStr::CheckSize
//

d_inline void kexStr::CheckSize(int size, bool bKeepString) {
    if(size <= bufferLength)
        return;

    Resize(size, bKeepString);
}

//
// kexStr::CopyNew
//

d_inline void kexStr::CopyNew(const char *string, int len) {
    CheckSize(len+1, false);
    strcpy(charPtr, string);
    length = len;
}

//
// kexStr::kexStr
//

d_inline kexStr::kexStr(void) {
    Init();
}

//
// kexStr::kexStr
//

d_inline kexStr::kexStr(const char *string) {
    Init();

    if(string == NULL)
        return;

    CopyNew(string, strlen(string));
}

//
// kexStr::kexStr
//

d_inline kexStr::kexStr(const kexStr &string) {
    Init();

    if(string.charPtr == NULL)
        return;

    CopyNew(string.charPtr, string.Length());
}

//
// kexStr::~kexStr
//

d_inline kexStr::~kexStr(void) {
    if(charPtr != defaultBuffer) {
        delete[] charPtr;
        charPtr = defaultBuffer;
    }

    charPtr[0] = '\0';
    length = 0;
}

//
// kexStr::Concat
//

d_inline kexStr &kexStr::Concat(const char *string) {
    return Concat(string, strlen(string));
}

//
// kexStr::Concat
//

d_inline kexStr &kexStr::Concat(const char *string, int len) {
    CheckSize((length + len)+1, true);

    for(int i = 0; i < len; i++) {
        charPtr[length+i] = string[i];
    }

    length += len;
    charPtr[length] = '\0';
    
    return *this;
}

//
// kexStr::Copy
//

d_inline kexStr &kexStr::Copy(const kexStr &src, int len) {
    int i = 0;
    const char *p = src;
    CheckSize((length + len)+1, true);
    
    while((len--) >= 0) {
        charPtr[i] = p[i];
        i++;
    }

    return *this;
}

//
// kexStr::Copy
//

d_inline kexStr &kexStr::Copy(const kexStr &src) {
    return Copy(src, src.Length());
}

//
// kexStr::operator=
//

d_inline void kexStr::operator=(const kexStr &str) {
    int len = str.Length();
    
    CheckSize(len+1, false);
    strncpy(charPtr, str.charPtr, len);
    length = len;
    charPtr[length] = '\0';
}

//
// kexStr::operator=
//

d_inline void kexStr::operator=(const char *str) {
    int len = strlen(str);
    
    CheckSize(len+1, false);
    strncpy(charPtr, str, len);
    length = len;
    charPtr[length] = '\0';
}

//
// kexStr::operator=
//

d_inline void kexStr::operator=(const bool b) {
    const char *str = b ? "true" : "false";
    int len = strlen(str);
    
    CheckSize(len+1, false);
    strncpy(charPtr, str, len);
    length = len;
    charPtr[length] = '\0';
}

//
// kexStr::operator+
//

d_inline kexStr kexStr::operator+(const kexStr &str) {
    kexStr out(*this);
    
    return out.Concat(str.c_str());
}

//
// kexStr::operator+
//

d_inline kexStr kexStr::operator+(const char *str) {
    kexStr out(*this);
    
    return out.Concat(str);
}

//
// kexStr::operator+
//

d_inline kexStr kexStr::operator+(const bool b) {
    kexStr out(*this);
    
    return out.Concat(b ? "true" : "false");
}

//
// kexStr::operator+=
//

d_inline kexStr &kexStr::operator+=(const kexStr &str) {
    return Concat(str.c_str());
}

//
// kexStr::operator+=
//

d_inline kexStr &kexStr::operator+=(const char *str) {
    return Concat(str);
}

//
// kexStr::operator+=
//

d_inline kexStr &kexStr::operator+=(const bool b) {
    return Concat(b ? "true" : "false");
}


#endif