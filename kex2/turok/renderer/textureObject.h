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

#ifndef __TEXTURE_OBJ_H__
#define __TEXTURE_OBJ_H__

typedef struct {
    int x;
    int y;
    int w;
    int h;
} atlas_t;

typedef enum {
    TC_CLAMP    = 0,
    TC_REPEAT
} texClampMode_t;

typedef enum {
    TF_LINEAR   = 0,
    TF_NEAREST
} texFilterMode_t;

typedef enum {
    TCR_RGB     = 0,
    TCR_RGBA    = 1
} texColorMode_t;

class kexTexture {
public:
                            kexTexture(void);
                            ~kexTexture(void);

    void                    Upload(byte **data, texClampMode_t clamp, texFilterMode_t filter);
    void                    SetParameters(void);
    byte                    *LoadFromFile(const char *file);
    byte                    *LoadFromScreenBuffer(void);
    byte                    *PadImage(byte **data);
    void                    VerticalFlipImage(byte **data);
    void                    Bind(void);
    void                    Delete(void);

    const int               Width(void) const { return width; }
    const int               Height(void) const { return height; }
    const int               OriginalWidth(void) const { return origwidth; }
    const int               OriginalHeight(void) const { return origheight; }
    dtexture                *TextureID(void) { return &texid; }
    bool                    IsLoaded(void) const { return bLoaded; }
    texClampMode_t          GetClampMode(void) { return clampMode; }
    void                    SetClampMode(texClampMode_t cm) { clampMode = cm; }
    texFilterMode_t         GetFilterMode(void) { return filterMode; }
    void                    SetFilterMode(texFilterMode_t fm) { filterMode = fm; }
    bool                    GetMasked(void) { return bMasked; }
    void                    SetMasked(bool b) { bMasked = b; }

    filepath_t              filePath;
    kexTexture              *next;

    static kexHeapBlock     hb_texture;

private:
    byte                    *LoadFromTGA(byte *input);
    byte                    *LoadFromBMP(byte *input);
    byte                    GetRGBGamma(int c);
    int                     width;
    int                     height;
    int                     origwidth;
    int                     origheight;
    texClampMode_t          clampMode;
    texFilterMode_t         filterMode;
    texColorMode_t          colorMode;
    dtexture                texid;
    bool                    bLoaded;
    bool                    bMasked;
};

#endif
