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

#ifndef __KEXARRAY_H__
#define __KEXARRAY_H__

#include <assert.h>

template<class type>
class kexArray {
public:
                        kexArray(void);
                        ~kexArray(void);

    void                Push(type o);
    void                Pop(void);
    void                Empty(void);
    void                Init(void);
    void                Resize(unsigned int size);
    type                IndexOf(unsigned int index) const;
    void                Splice(const unsigned int start, unsigned int len);
    const unsigned int  Length(void) const { return length; }
    type                GetData(const int index) { return data[index]; }

    type                &operator[](unsigned int index);
    kexArray<type>      &operator=(const kexArray<type> &arr);

protected:
    type                *data;
    unsigned int        length;
};

//
// kexArray::kexArray
//
template<class type>
kexArray<type>::kexArray(void) {
    Init();
}

//
// kexArray::~kexArray
//
template<class type>
kexArray<type>::~kexArray(void) {
    Empty();
}

//
// kexArray::Init
//
template<class type>
void kexArray<type>::Init(void) {
    data = NULL;
    length = 0;
}

//
// kexArray::Resize
//
template<class type>
void kexArray<type>::Resize(unsigned int size) {
    type *tmp;

    if(size == length) {
        return;
    }

    if(length == 0) {
        data = new type[size];
        return;
    }

    tmp = data;
    data = new type[size];

    for(unsigned int i = 0; i < length; i++) {
        data[i] = tmp[i];
    }

    delete[] tmp;
}

//
// kexArray::Push
//
template<class type>
void kexArray<type>::Push(type o) {
    Resize(length+1);
    data[length++] = o;
}

//
// kexArray::Pop
//
template<class type>
void kexArray<type>::Pop(void) {
    if(length == 0) {
        return;
    }
    
    Resize(length-1);
    length--;
}

//
// kexArray::Empty
//
template<class type>
void kexArray<type>::Empty(void) {
    if(data) {
        delete[] data;
        data = NULL;
        length = 0;
    }
}

//
// kexArray::IndexOf
//
template<class type>
type kexArray<type>::IndexOf(unsigned int index) const {
    if(index >= length) {
        index = length-1;
    }

    return data[index];
}

//
// kexArray::Splice
//
template<class type>
void kexArray<type>::Splice(const unsigned int start, unsigned int len) {
    if(length == 0 || len == 0) {
        return;
    }

    if(len >= length) {
        len = length;
    }

    type *tmp = new type[len];

    for(unsigned int i = 0; i < len; i++) {
        tmp[i] = data[start+i];
    }

    delete[] data;
    data = tmp;
    length = length - len;
}

//
// kexArray::operator[]
//
template <class type>
type &kexArray<type>::operator[](unsigned int index) {
    assert(index < length);
    return data[index];
}

//
// kexArray::operator=
//
template <class type>
kexArray<type> &kexArray<type>::operator=(const kexArray<type> &arr) {
    if(data) {
        delete[] data;
    }
    
    data = NULL;
    length = arr.length;
    
    if(arr.length > 0) {
        data = new type[arr.length];
        
        for(unsigned int i = 0; i < arr.length; i++) {
            data[i] = arr.data[i];
        }
    }
    
    return *this;
}

#endif
