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

template<class type>
class kexArray {
public:
                        kexArray();
                        kexArray(bool bIsPointer);
                        ~kexArray();

    void                Push(type o);
    void                Pop(void);
    void                Empty(void);
    void                EmptyContents(void);
    type                IndexOf(unsigned int index) const;
    void                Splice(const unsigned int start, unsigned int len);
    const unsigned int  Length(void) const { return length; }
    void                IsPointer(bool IsPointer) { bPointer = IsPointer; }
    type                GetData(const int index) { return data[index]; }

private:
    type                *data;
    unsigned int        length;
    bool                bPointer;
};

//
// kexArray::kexArray
//
template<class type>
kexArray<type>::kexArray() {
    data = NULL;
    length = 0;
    bPointer = false;
}

//
// kexArray::kexArray
//
template<class type>
kexArray<type>::kexArray(bool bIsPointer) {
    data = NULL;
    length = 0;
    bPointer = bIsPointer;
}

//
// kexArray::~kexArray
//
template<class type>
kexArray<type>::~kexArray() {
    if(bPointer)
        EmptyContents();

    if(data)
        delete[] data;
}

//
// kexArray::EmptyContents
//
template<class type>
void kexArray<type>::EmptyContents(void) {
    for(unsigned int i = 0; i < length; i++) {
        delete data[i];
        data[i] = NULL;
    }
}

//
// kexArray::Push
//
template<class type>
void kexArray<type>::Push(type o) {
    if(length == 0) {
        data = new type[length+1];
        data[length] = o;
    }
    else {
        type *tmp = data;
        data = new type[length+1];
        for(unsigned int i = 0; i < length; i++)
            data[i] = tmp[i];

        delete[] tmp;
        data[length] = o;
    }

    length++;
}

//
// kexArray::Pop
//
template<class type>
void kexArray<type>::Pop(void) {
    if(length == 0)
        return;

    if(length != 1) {
        type *tmp = data;
        
        data = new type[length-1];

        for(unsigned int i = 0; i < length-1; i++)
            data[i] = tmp[i];
        if(bPointer) {
            delete tmp[length-1];
            tmp[length-1] = NULL;
        }

        delete[] tmp;
    }
    else {
        if(bPointer) {
            delete data[0];
        }
        delete[] data;
        data = NULL;
    }

    length--;
}

//
// kexArray::Empty
//
template<class type>
void kexArray<type>::Empty(void) {
    while(length) {
        Pop();
    }
}

//
// kexArray::IndexOf
//
template<class type>
type kexArray<type>::IndexOf(unsigned int index) const {
    if(index >= length)
        index = length-1;

    return data[index];
}

//
// kexArray::Splice
//
template<class type>
void kexArray<type>::Splice(const unsigned int start, unsigned int len) {
    if(length == 0 || len == 0)
        return;

    if(len >= length)
        len = length;

    type *tmp = new type[len];

    if(bPointer && start > 0) {
        for(unsigned int i = 0; i < start; i++) {
            delete data[i];
            data[i] = NULL;
        }
    }
    for(unsigned int i = 0; i < len; i++) {
        tmp[i] = data[start+i];
    }
    if(bPointer && len < length) {
        for(unsigned int i = len; i < length; i++) {
            delete data[i];
            data[i] = NULL;
        }
    }

    delete[] data;
    data = tmp;
    length = length - len;
}

#endif
