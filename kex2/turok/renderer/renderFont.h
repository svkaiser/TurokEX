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

#ifndef __RENDERFONT_H__
#define __RENDERFONT_H__

class kexFont {
public:
                    kexFont(void);
                    ~kexFont(void);

    void            LoadKFont(const char *file);
    void            DrawString(const char *string, float x, float y, float scale,
                               bool center, byte *rgba1, byte *rgba2);
    float           StringWidth(const char* string, float scale, int fixedLen);

    const bool      IsLoaded(void) const { return bLoaded; }
    kexTexture      *Texture(void) { return texture; }

    filepath_t      filePath;
    kexFont         *next;

private:
    kexTexture      *texture;
    atlas_t         atlas[256];
    bool            bLoaded;
};

#endif
