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

#ifndef __KEYMAP_H__
#define __KEYMAP_H__

class kexHashKey {
public:
                            kexHashKey(const char *key, const char *value);
                            ~kexHashKey(void);

    const char              *GetName(void) { return key.c_str(); }
    const char              *GetString(void) { return value.c_str(); }

private:
    kexStr                  key;
    kexStr                  value;
};

class kexKeyMap {
public:
                            kexKeyMap(void);
                            ~kexKeyMap(void);

    void                    Add(const char *key, const char *value);
    kexHashKey              *Find(const char *name);
    void                    Empty(void);

private:
    kexArray<kexHashKey*>   hashlist[MAX_HASH];
};

#endif
